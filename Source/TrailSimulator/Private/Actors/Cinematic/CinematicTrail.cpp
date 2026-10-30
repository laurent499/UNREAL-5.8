// All Rights Reserved

#include "Actors/Cinematic/CinematicTrail.h"
#include "Actors/Path.h"
#include "Actors/Generic/Checkpoint_Generic.h"
#include "Actors/Generic/Poi_Generic.h"
#include "Actors/GTWS/Checkpoint_GTWS.h"
#include "Actors/GTWS/Poi_GTWS.h"
#include "Actors/Nike/Checkpoint_Nike.h"
#include "Actors/Nike/Poi_Nike.h"
#include "Actors/UTMB/Checkpoint_UTMB.h"
#include "Actors/UTMB/Poi_UTMB.h"
#include "CheckpointInterface.h"
#include "CheckpointSubsystem.h"
#include "JsonObjectConverter.h"
#include "PoiInterface.h"
#include "PoiSubsystem.h"
#include "RaceSubsystem.h"
#include "ScaleSubsystem.h"
#include "TemplateSubsystem.h"
#include "Components/TextRenderComponent.h"
#include "Engine/GameInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CesiumGeoreference.h"
#include "Cesium3DTileset.h"
#include "CesiumSampleHeightResult.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "PathSubsystem.h"
#include "SlateNotificationsBFL.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Interfaces/IHttpResponse.h"
#include "UObject/ConstructorHelpers.h"

ACinematicTrail::ACinematicTrail()
{
	PrimaryActorTick.bCanEverTick = true;
	// Apres l'evaluation de Sequencer, comme la camera
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	SetActorEnableCollision(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Trail = CreateDefaultSubobject<USplineComponent>(TEXT("Trail"));
	Trail->SetupAttachment(Root);
	Trail->ClearSplinePoints();

	Rail = CreateDefaultSubobject<USplineComponent>(TEXT("Rail"));
	Rail->SetupAttachment(Root);
	Rail->ClearSplinePoints();

	// Boule lumineuse en tete du trace : evite une fin de tube coupee net pendant qu'il se deroule
	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeteTrace"));
	Head->SetupAttachment(Root);
	Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Head->SetCastShadow(false);
	ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere.Succeeded())
	{
		Head->SetStaticMesh(Sphere.Object);
	}
	HeadMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Cinematics/MontBlanc/M_TeteTrace.M_TeteTrace")));

	// Meme cylindre (et donc meme materiau) que le trace live
	ConstructorHelpers::FObjectFinder<UStaticMesh> SM(
		TEXT("/Game/LTVContent/Meshes/SM/Stack/newUTMB/SM_Cylindre_1x1.SM_Cylindre_1x1"));
	if (SM.Succeeded())
	{
		TrailMesh = SM.Object;
	}
	DecalottageCollection = TSoftObjectPtr<UMaterialParameterCollection>(
		FSoftObjectPath(TEXT("/Game/Cinematics/MontBlanc/MPC_Decalottage.MPC_Decalottage")));
}

void ACinematicTrail::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Rebuild();
	AppliedDecalottage = -1.f;
	ApplyDecalottage();
}

namespace CinematicTrail
{
	/** GET sur la regie avec les en-tetes d'acces ; OnOk recoit le corps si la reponse est 2xx */
	static void Get(const FString& Endpoint, TFunction<void(const FString&)> OnOk, TFunction<void(int32)> OnFail)
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = UPathSubsystem::CreatePathRequest(Endpoint);
		Req->OnProcessRequestComplete().BindLambda(
			[OnOk = MoveTemp(OnOk), OnFail = MoveTemp(OnFail)](FHttpRequestPtr, const FHttpResponsePtr Response, bool bWasSuccessful)
			{
				if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
				{
					OnOk(Response->GetContentAsString());
				}
				else
				{
					OnFail(Response.IsValid() ? Response->GetResponseCode() : -1);
				}
			});
		Req->ProcessRequest();
	}

	static void Notify(const FString& Msg, bool bError)
	{
		if (bError) UE_LOG(LogTemp, Error, TEXT("%s"), *Msg);
		USlateNotificationsBFL::SlateNotify(FText::FromString(Msg), bError ? EMessageType::Error : EMessageType::Success);
	}

	/** race/<id> renvoie { "setup": {...} } */
	static bool ParseSetup(const FString& Json, FRaceSetup& Out)
	{
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) return false;
		const TSharedPtr<FJsonObject>* Setup = nullptr;
		return Root->TryGetObjectField(TEXT("setup"), Setup)
			&& FJsonObjectConverter::JsonObjectToUStruct(Setup->ToSharedRef(), &Out, 0, 0);
	}
}

void ACinematicTrail::LoadRace()
{
	// Trace (qui porte les checkpoints), puis setup de la course, puis POI
	const FString Base = MainURL / TEXT("race") / LexToString(RaceID);
	TWeakObjectPtr<ACinematicTrail> WeakThis(this);
	CinematicTrail::Get(Base / TEXT("path"), [WeakThis, Base](const FString& PathJson)
	{
		ACinematicTrail* Self = WeakThis.Get();
		if (!Self) return;
		FRacePath Path;
		if (!UPathSubsystem::ParsePathJson(PathJson, Path) || Path.Points.Num() < 2)
		{
			CinematicTrail::Notify(FString::Printf(TEXT("Trace cinema : trace de la course %lld vide ou illisible"), Self->RaceID), true);
			return;
		}

		Self->Modify();
		Self->PointsLLH.Reset(Path.Points.Num());
		Self->Checkpoints.Reset();
		Self->CheckpointPointIndices.Reset();
		for (int32 i = 0; i < Path.Points.Num(); ++i)
		{
			const FRacePathPoint& P = Path.Points[i];
			Self->PointsLLH.Add(FVector(P.lon, P.lat, P.ele));
			// Comme APath::DrawPath : un point qui porte un nom est un checkpoint
			if (!P.datas.name.IsEmpty())
			{
				Self->Checkpoints.Add(P.datas);
				Self->CheckpointPointIndices.Add(i);
			}
		}

		if (Self->bCenterGeoreferenceOnStart)
		{
			if (ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(Self))
			{
				Georef->Modify();
				Georef->SetOriginLongitudeLatitudeHeight(Self->PointsLLH[0]);
			}
		}

		CinematicTrail::Get(Base, [WeakThis, Base](const FString& SetupJson)
		{
			ACinematicTrail* Self = WeakThis.Get();
			if (!Self) return;
			if (!CinematicTrail::ParseSetup(SetupJson, Self->RaceSetup))
			{
				CinematicTrail::Notify(TEXT("Trace cinema : setup de la course illisible, template par defaut au rendu"), true);
			}

			CinematicTrail::Get(Base / TEXT("pois"), [WeakThis](const FString& PoisJson)
			{
				ACinematicTrail* Self = WeakThis.Get();
				if (!Self) return;
				Self->Pois.Reset();
				FJsonObjectConverter::JsonArrayStringToUStruct(PoisJson, &Self->Pois, 0, 0);
				Self->Rebuild();
				CinematicTrail::Notify(FString::Printf(TEXT("Trace cinema : %s chargee (%.1f km, %d checkpoints, %d POI)"),
					*Self->RaceSetup.raceName, Self->Trail->GetSplineLength() / 100000.f, Self->Checkpoints.Num(), Self->Pois.Num()), false);
			},
			[WeakThis](int32 Code)
			{
				if (ACinematicTrail* Self = WeakThis.Get()) Self->Rebuild();
				CinematicTrail::Notify(FString::Printf(TEXT("Trace cinema : POI non charges (code %d)"), Code), true);
			});
		},
		[WeakThis](int32 Code)
		{
			if (ACinematicTrail* Self = WeakThis.Get()) Self->Rebuild();
			CinematicTrail::Notify(FString::Printf(TEXT("Trace cinema : setup et POI non charges (code %d)"), Code), true);
		});
	},
	[Base](int32 Code)
	{
		CinematicTrail::Notify(FString::Printf(TEXT("Trace cinema : echec du chargement de %s/path (code %d)"), *Base, Code), true);
	});
}

void ACinematicTrail::Rebuild()
{
	Trail->ClearSplinePoints(false);
	Rail->ClearSplinePoints(false);

	ACesiumGeoreference* Georef = HasTrail() ? ACesiumGeoreference::GetDefaultGeoreference(this) : nullptr;
	if (Georef)
	{
		if (DrapedLLH.Num() >= 2)
		{
			// Trace drape : le bas du tube passe DrapeOffsetM au-dessus du relief
			const double RadiusM = TrailMesh ? TrailMesh->GetBounds().BoxExtent.X * TrailWidth / 100.0 : 0.0;
			for (int32 i = 0; i < DrapedLLH.Num(); ++i)
			{
			if (RemovedPoints.Contains(bReverseDirection ? DrapedLLH.Num() - 1 - i : i)) continue;
			const FVector& LLH = DrapedLLH[bReverseDirection ? DrapedLLH.Num() - 1 - i : i];
				Trail->AddSplinePoint(Georef->TransformLongitudeLatitudeHeightPositionToUnreal(
					FVector(LLH.X, LLH.Y, LLH.Z + DrapeOffsetM + RadiusM)), ESplineCoordinateSpace::World, false);
			}
		}
		else
		{
			// Altitude GPS (niveau de la mer) -> hauteur ellipsoidale attendue par Cesium, comme APath
			for (int32 i = 0; i < PointsLLH.Num(); ++i)
			{
				const FVector& LLH = PointsLLH[bReverseDirection ? PointsLLH.Num() - 1 - i : i];
				const double HeightM = LLH.Z + APath::GetGeoidUndulationM(LLH.X, LLH.Y) + HeightOffsetM;
				Trail->AddSplinePoint(Georef->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(LLH.X, LLH.Y, HeightM)),
					ESplineCoordinateSpace::World, false);
			}
		}
		Trail->UpdateSpline();
		APath::BuildSmoothCameraRail(Trail, Rail, RailSmoothM);
	}
	else
	{
		Trail->UpdateSpline();
		Rail->UpdateSpline();
	}
	// Table Reveal -> distance sur le rail : projections du trace sur le rail, rendues monotones puis lissees
	RailMap.Reset();
	if (Georef && Trail->GetNumberOfSplinePoints() >= 2 && Rail->GetNumberOfSplinePoints() >= 2)
	{
		constexpr int32 Samples = 800;
		const float TrailLength = Trail->GetSplineLength();
		const float RailLength = Rail->GetSplineLength();
		RailMap.SetNum(Samples);
		float Prev = 0.f;
		for (int32 i = 0; i < Samples; ++i)
		{
			const FVector P = Trail->GetLocationAtDistanceAlongSpline(TrailLength * i / (Samples - 1), ESplineCoordinateSpace::World);
			const float D = Rail->GetDistanceAlongSplineAtSplineInputKey(Rail->FindInputKeyClosestToWorldLocation(P));
			Prev = FMath::Max(Prev, D);
			RailMap[i] = Prev;
		}
		RailMap[0] = 0.f;
		RailMap[Samples - 1] = RailLength;
		for (int32 Pass = 0; Pass < 3 && RailMapSmooth > 0; ++Pass)
		{
			TArray<float> Smoothed = RailMap;
			for (int32 i = 1; i < Samples - 1; ++i)
			{
				const int32 R = FMath::Min(RailMapSmooth, FMath::Min(i, Samples - 1 - i));
				float Sum = 0.f;
				for (int32 k = -R; k <= R; ++k) Sum += RailMap[i + k];
				Smoothed[i] = Sum / (2 * R + 1);
			}
			RailMap = MoveTemp(Smoothed);
		}
	}
	BuildTrailMeshes();
	AppliedDecalottage = -1.f;
	BuildMarkers();
	ApplyReveal();

	ComputeSplit();
	CheckpointReveal.Reset();
	CheckpointSections.Reset();
	const float TrailLen = Trail->GetSplineLength();
	for (int32 i = 0; i < Checkpoints.Num(); ++i)
	{
	const float Dist = TrailLen > 0.f ? GetDistanceOnTrail(GetCheckpointLocation(i)) : 0.f;
	CheckpointReveal.Add(TrailLen > 0.f ? Dist / TrailLen : 0.f);
	CheckpointSections.Add(FString::Printf(TEXT("%s : %s (a %.0f m)"), *Checkpoints[i].name,
	Dist > SplitDistCm + 300.f ? TEXT("couleur 2 (bleu)") : TEXT("couleur 1 (rouge)"), Dist / 100.f));
	}
}

void ACinematicTrail::ComputeSplit()
{
	SplitDistCm = TNumericLimits<float>::Max();
	if (Trail->GetNumberOfSplinePoints() < 2) return;
	for (int32 i = 0; i < Checkpoints.Num(); ++i)
	{
		if (!ColorSplitCheckpoint.IsEmpty() && Checkpoints[i].name.Contains(ColorSplitCheckpoint))
		{
			SplitDistCm = GetDistanceOnTrail(GetCheckpointLocation(i));
		}
	}
}

void ACinematicTrail::BuildTrailMeshes()
{
	for (USplineMeshComponent* SMC : TrailMeshes)
	{
		if (IsValid(SMC))
		{
			RemoveInstanceComponent(SMC);
			SMC->DestroyComponent();
		}
	}
	TrailMeshes.Reset();
	AppliedVisible = -1;
	PartialIndex = INDEX_NONE;
	if (!bShowTrail || !TrailMesh || Trail->GetNumberOfSplinePoints() < 2) return;

	// Morceaux de longueur fixe le long du trace : beaucoup moins de composants qu'un par point GPS
	const float Len = Trail->GetSplineLength();
	const int32 Num = FMath::Max(1, FMath::CeilToInt(Len / (TrailStepM * 100.f)));
	ComputeSplit();
	const float SplitDist = SplitDistCm;
	const float Step = Len / Num;
	for (int32 i = 0; i < Num; ++i)
	{
		const float StartDist = Step * i;
		const float EndDist = Step * (i + 1);
		const FVector P1 = Trail->GetLocationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
		const FVector P2 = Trail->GetLocationAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local);
		const FVector T1 = Trail->GetDirectionAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local) * Step;
		const FVector T2 = Trail->GetDirectionAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local) * Step;

		// Composants d'instance, sauvegardes avec le level : le PIE du rendu ne relance pas la construction
		USplineMeshComponent* SMC = NewObject<USplineMeshComponent>(this);
		SMC->CreationMethod = EComponentCreationMethod::Instance;
		AddInstanceComponent(SMC);
		TrailMeshes.Add(SMC);
		SMC->SetMobility(EComponentMobility::Movable);
		SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SMC->SetGenerateOverlapEvents(false);
		SMC->SetStaticMesh(TrailMesh);
		SMC->SetupAttachment(Trail);
		SMC->SetForwardAxis(ESplineMeshAxis::Z, false);
		SMC->SetStartScale(FVector2D(TrailWidth), false);
		SMC->SetEndScale(FVector2D(TrailWidth), false);
		SMC->SetStartAndEnd(P1, T1, P2, T2, false);
		SMC->SetTranslucentSortPriority(-20);

		// Memes donnees que le trace live (APath::RebuildPathSplineMeshes), en valeurs par defaut : elles sont
		// sauvegardees avec le level et recopiees dans le PIE du rendu (les valeurs dynamiques ne le sont pas)
		SMC->SetDefaultCustomPrimitiveDataFloat(0, StartDist / Len);
		SMC->SetDefaultCustomPrimitiveDataFloat(1, Step / Len);
		SMC->SetDefaultCustomPrimitiveDataVector3(2, P1);
		SMC->SetDefaultCustomPrimitiveDataVector3(6, P2);
		SMC->SetDefaultCustomPrimitiveDataFloat(10, 0.f);
		SMC->SetDefaultCustomPrimitiveDataFloat(11, 0.f);
		SMC->SetDefaultCustomPrimitiveDataVector4(12, StartDist + 0.5f * Step >= SplitDist ? TrailColor2 : TrailColor);
		SMC->SetDefaultCustomPrimitiveDataFloat(16, Glow);
		SMC->RegisterComponent();
	}
}

FVector ACinematicTrail::GetCheckpointLocation(int32 Index) const
{
	const int32 Point = CheckpointPointIndices.IsValidIndex(Index) ? CheckpointPointIndices[Index] : INDEX_NONE;
	const ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(this);
	if (!PointsLLH.IsValidIndex(Point) || !Georef || Trail->GetNumberOfSplinePoints() < 2)
	{
		return GetActorLocation();
	}
	const FVector& LLH = PointsLLH[Point];
	const FVector Gps = Georef->TransformLongitudeLatitudeHeightPositionToUnreal(
		FVector(LLH.X, LLH.Y, LLH.Z + APath::GetGeoidUndulationM(LLH.X, LLH.Y)));
	return Trail->FindLocationClosestToWorldLocation(Gps, ESplineCoordinateSpace::World);
}

FVector ACinematicTrail::GetPoiLocation(const FRacePOI& Poi) const
{
	// Comme ARaceManager::HandleRacePoisDatasGathered : altitude brute, sans geoide ni decalage
	const ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(this);
	return Georef ? Georef->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Poi.lon, Poi.lat, Poi.elevation)) : GetActorLocation();
}

void ACinematicTrail::BuildMarkers()
{
	for (UTextRenderComponent* Marker : Markers)
	{
		if (IsValid(Marker)) Marker->DestroyComponent();
	}
	Markers.Reset();
	MarkerDistances.Reset();
	const UWorld* World = GetWorld();
	if (!World || World->IsGameWorld() || !HasTrail()) return;

	// Reperes de cadrage pour l'editeur seulement : au rendu, les vrais panneaux les remplacent
	auto Add = [this](const FString& Text, const FVector& Location, const FColor& Color)
	{
		UTextRenderComponent* Marker = NewObject<UTextRenderComponent>(this, NAME_None, RF_Transient);
		Marker->bIsEditorOnly = true;
		Marker->SetHiddenInGame(true);
		Marker->SetupAttachment(Root);
		Marker->SetText(FText::FromString(Text));
		Marker->SetTextRenderColor(Color);
		Marker->SetWorldSize(MarkerTextSize);
		Marker->SetHorizontalAlignment(EHTA_Center);
		Marker->SetVerticalAlignment(EVRTA_TextBottom);
		Marker->RegisterComponent();
		Marker->SetWorldLocation(Location + FVector(0.f, 0.f, MarkerTextSize));
		Markers.Add(Marker);
		MarkerDistances.Add(GetDistanceOnTrail(Location));
	};
	if (bSpawnCheckpoints)
	{
		for (int32 i = 0; i < Checkpoints.Num(); ++i)
		{
			Add(Checkpoints[i].name, GetCheckpointLocation(i), FColor(255, 90, 40));
		}
	}
	if (bSpawnPois)
	{
		for (const FRacePOI& Poi : Pois)
		{
			Add(Poi.name, GetPoiLocation(Poi), FColor(60, 140, 255));
		}
	}
}

void ACinematicTrail::BeginPlay()
{
	Super::BeginPlay();
	ComputeSplit();
	if (UScaleSubsystem* Scale = GetWorld()->GetSubsystem<UScaleSubsystem>())
	{
	Scale->SetNearFar(PanelScaleNearM * 100.f, PanelScaleFarM * 100.f);
	Scale->SetGlobalMinMax(PanelScaleMin, PanelScaleMax);
	}
	if (bWaitForTiles)
	{
		// Cesium ne branche ce mode que sur les sequences presentes au BeginPlay du tileset ; celle du rendu est creee apres
		for (TActorIterator<ACesium3DTileset> It(GetWorld()); It; ++It)
		{
			It->PlayMovieSequencer();
		}
	}
	UGameInstance* GI = GetGameInstance();
	if (!GI || (!bSpawnCheckpoints && !bSpawnPois) || (Checkpoints.IsEmpty() && Pois.IsEmpty())) return;

	// Les acteurs du live retrouvent le setup par son id ; le template est relu sur la regie, sinon valeurs d'origine
	if (RaceSetup.raceId == 0)
	{
		RaceSetup.raceId = RaceID;
	}
	if (URaceSubsystem* Races = GI->GetSubsystem<URaceSubsystem>())
	{
		Races->AddRaceSetup(RaceSetup);
	}
	UTemplateSubsystem* Templates = GI->GetSubsystem<UTemplateSubsystem>();
	if (Templates && !RaceSetup.templateName.IsEmpty())
	{
		Templates->RequestTemplate(RaceSetup.templateName, [WeakThis = TWeakObjectPtr<ACinematicTrail>(this)]()
		{
			if (ACinematicTrail* Self = WeakThis.Get()) Self->SpawnRaceActors();
		});
	}
	else
	{
		SpawnRaceActors();
	}
}

void ACinematicTrail::SpawnRaceActors()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = GetGameInstance();
	if (!World || !GI) return;

	// Memes classes que APath::GetCheckpointClassForRace et ARaceManager::GetPoiClassForRace
	if (PanelCornerRadius >= 0.f)
	{
		if (UTemplateSubsystem* Templates = GI->GetSubsystem<UTemplateSubsystem>())
		{
			FTrailTemplate Template = Templates->GetTemplate(RaceSetup.templateName);
			Template.Globals.CornerRadius = PanelCornerRadius;
			if (bTintPanelsBySection)
			{
				// Le template GTWS d'origine a un bandeau orange fixe : il suit aussi la couleur de la portion
				Template.Checkpoint.NameBkg = Template.Checkpoint.NameBorder = TEXT("race");
				Template.Poi.NameBkg = Template.Poi.NameBorder = TEXT("race");
			}
			Templates->SetTemplate(RaceSetup.templateName, Template);
		}
	}
	// Couleur de la course des panneaux : celle de leur portion du trace
	// Picto principal : le MID du template ne recoit pas la couleur a la creation
	auto WhitePicto = [this](AActor* Actor)
	{
		if (!Actor || PictoColor.A <= 0.f) return;
		TArray<UMeshComponent*> Meshes;
		Actor->GetComponents(Meshes);
		for (UMeshComponent* Mesh : Meshes)
		{
			if (!Mesh->GetName().Contains(TEXT("MainPicto"))) continue;
			UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
			if (!MID) MID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
			if (MID) MID->SetVectorParameterValue(TEXT("Color"), PictoColor);
		}
	};
	auto SetupAt = [this](float DistCm)
	{
		FRaceSetup Setup = RaceSetup;
		if (bTintPanelsBySection)
		{
			Setup.color = TEXT("#") + (DistCm > SplitDistCm + 300.f ? TrailColor2 : TrailColor).ToFColor(true).ToHex();
		}
		return Setup;
	};
	const FString Base = UTemplateSubsystem::GetForRace(this, RaceSetup).Base;
	const FVector Scale(40.f);
	// Sans reglages de la course dans la regie, les panneaux prennent une echelle min/max nulle : on garde celle par defaut du live
	UScaleSubsystem* ScaleSubsystem = World->GetSubsystem<UScaleSubsystem>();
	auto KeepDefaultScale = [this, ScaleSubsystem](AActor* Actor)
	{
		if (ScaleSubsystem) ScaleSubsystem->SetActorScaleMinMax(Actor, FMinMax(FVector(PanelScaleMin), FVector(PanelScaleMax)));
	};

	UCheckpointSubsystem* CheckpointSubsystem = GI->GetSubsystem<UCheckpointSubsystem>();
	if (bSpawnCheckpoints && CheckpointSubsystem)
	{
		const TSubclassOf<AActor> Class = Base == TEXT("UTMB") ? ACheckpoint_UTMB::StaticClass()
			: Base == TEXT("GTWS") ? ACheckpoint_GTWS::StaticClass()
			: Base == TEXT("Nike") ? ACheckpoint_Nike::StaticClass()
			: ACheckpoint_Generic::StaticClass();
		for (int32 i = 0; i < Checkpoints.Num(); ++i)
		{
			AActor* Actor = CheckpointSubsystem->SpawnCheckpointActor(RaceSetup.raceId, Checkpoints[i].checkpointId, World, Class,
				FTransform(FRotator::ZeroRotator, GetCheckpointLocation(i), Scale));
			if (ICheckpointInterface* Checkpoint = Cast<ICheckpointInterface>(Actor))
			{
				const float Dist = GetDistanceOnTrail(GetCheckpointLocation(i));
				Checkpoint->UpdateCheckpoint(Checkpoints[i], SetupAt(Dist));
				KeepDefaultScale(Actor);
				WhitePicto(Actor);
				RevealActors.Emplace(Actor, Dist);
			}
		}
	}

	UPoiSubsystem* PoiSubsystem = GI->GetSubsystem<UPoiSubsystem>();
	if (bSpawnPois && PoiSubsystem)
	{
		const TSubclassOf<AActor> Class = Base == TEXT("UTMB") ? APoi_UTMB::StaticClass()
			: Base == TEXT("GTWS") ? APoi_GTWS::StaticClass()
			: Base == TEXT("Nike") ? APoi_Nike::StaticClass()
			: APoi_Generic::StaticClass();
		for (const FRacePOI& Poi : Pois)
		{
			AActor* Actor = PoiSubsystem->SpawnPoi(RaceSetup.raceId, Poi.poiId, World, Class,
				FTransform(FRotator::ZeroRotator, GetPoiLocation(Poi), Scale));
			if (IPoiInterface* PoiActor = Cast<IPoiInterface>(Actor))
			{
				const float Dist = GetDistanceOnTrail(GetPoiLocation(Poi));
				PoiActor->UpdatePoi(Poi, SetupAt(Dist));
				KeepDefaultScale(Actor);
				WhitePicto(Actor);
				RevealActors.Emplace(Actor, Dist);
			}
		}
	}
	AppliedReveal = -1.f;
}

float ACinematicTrail::GetRailDistanceForReveal(float InReveal) const
{
	if (RailMap.Num() < 2) return -1.f;
	const float F = FMath::Clamp(InReveal, 0.f, 1.f) * (RailMap.Num() - 1);
	const int32 I = FMath::Min(FMath::FloorToInt(F), RailMap.Num() - 2);
	return FMath::Lerp(RailMap[I], RailMap[I + 1], F - I);
}

float ACinematicTrail::GetDistanceOnTrail(const FVector& WorldLocation) const
{
	return Trail->GetNumberOfSplinePoints() < 2 ? 0.f
		: Trail->GetDistanceAlongSplineAtSplineInputKey(Trail->FindInputKeyClosestToWorldLocation(WorldLocation));
}

FVector ACinematicTrail::GetRevealLocation() const
{
	return Trail->GetLocationAtDistanceAlongSpline(FMath::Clamp(Reveal, 0.f, 1.f) * Trail->GetSplineLength(), ESplineCoordinateSpace::World);
}

void ACinematicTrail::Tick(float DeltaTime)
{
Super::Tick(DeltaTime);
#if WITH_EDITOR
if (bShowPointNumbers && !GetWorld()->IsGameWorld() && DrapedLLH.Num() > 0)
{
if (ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(this))
{
const int32 To = FMath::Min(PointNumbersTo, DrapedLLH.Num() - 1);
for (int32 i = FMath::Max(0, PointNumbersFrom); i <= To; i += 3)
{
const FVector P = Georef->TransformLongitudeLatitudeHeightPositionToUnreal(DrapedLLH[i] + FVector(0, 0, DrapeOffsetM + 2.0));
DrawDebugPoint(GetWorld(), P, 14.f, RemovedPoints.Contains(i) ? FColor::Red : FColor::Yellow, false, 0.f);
DrawDebugString(GetWorld(), P + FVector(0, 0, 250), FString::FromInt(i), nullptr, FColor::White, 0.f, false, 1.4f);
}
}
}
#endif
	if (Reveal != AppliedReveal)
	{
		ApplyReveal();
	}
	if (Decalottage != AppliedDecalottage)
	{
		ApplyDecalottage();
	}
}

void ACinematicTrail::ApplyDecalottage()
{
	UWorld* World = GetWorld();
	UMaterialParameterCollection* Collection = DecalottageCollection.LoadSynchronous();
	UMaterialParameterCollectionInstance* Instance = World && Collection ? World->GetParameterCollectionInstance(Collection) : nullptr;
	if (!Instance) return;
	Instance->SetScalarParameterValue(TEXT("Decalottage"), Decalottage);
	AppliedDecalottage = Decalottage;
	ApplyZoneFade();
	// Les panneaux se masquent avec le decalottage : relance ApplyReveal
	AppliedReveal = -1.f;
}

void ACinematicTrail::ApplyZoneFade()
{
	const int32 Num = TrailMeshes.Num();
	if (Num == 0 || Trail->GetNumberOfSplinePoints() < 2) return;
	const float Fade = 1.f - FMath::SmoothStep(TrailFadeStart, FMath::Max(TrailFadeEnd, TrailFadeStart + 0.01f), Decalottage);
	const float Step = Trail->GetSplineLength() / Num;
	for (int32 i = 0; i < Num; ++i)
	{
		USplineMeshComponent* SMC = TrailMeshes[i].Get();
		if (!SMC) continue;
		const FVector P = Trail->GetLocationAtDistanceAlongSpline(Step * (i + 0.5f), ESplineCoordinateSpace::World);
		const FVector2D Q((P.X - ZoneCenter.X) / ZoneRadius.X, (P.Y - ZoneCenter.Y) / ZoneRadius.Y);
		const bool bFading = bFadeWholeTrail || Q.SizeSquared() < 1.0;
		// Jamais a l'echelle 0 (hull de collision invalide) : on amincit jusqu'a un minimum, puis on masque
		const float Scale = TrailWidth * (bFading ? FMath::Max(Fade, 0.05f) : 1.f);
		SMC->SetHiddenInGame(bFading && Fade < 0.02f);
		if (!FMath::IsNearlyEqual(SMC->GetStartScale().X, Scale, 0.01f))
		{
			SMC->SetStartScale(FVector2D(Scale), false);
			SMC->SetEndScale(FVector2D(Scale), true);
		}
	}
}

void ACinematicTrail::SetSegmentEnd(int32 Index, float EndDist)
{
	USplineMeshComponent* SMC = TrailMeshes.IsValidIndex(Index) ? TrailMeshes[Index].Get() : nullptr;
	if (!SMC) return;
	const float Step = Trail->GetSplineLength() / TrailMeshes.Num();
	const float StartDist = Step * Index;
	const float Length = FMath::Max(EndDist - StartDist, 1.f);
	const FVector P1 = Trail->GetLocationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
	const FVector P2 = Trail->GetLocationAtDistanceAlongSpline(StartDist + Length, ESplineCoordinateSpace::Local);
	SMC->SetStartAndEnd(P1, Trail->GetDirectionAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local) * Length,
		P2, Trail->GetDirectionAtDistanceAlongSpline(StartDist + Length, ESplineCoordinateSpace::Local) * Length, true);
	SMC->SetCustomPrimitiveDataVector3(6, P2);
}

void ACinematicTrail::ApplyReveal()
{
	AppliedReveal = Reveal;
	const float Len = Trail->GetSplineLength();
	const float Dist = FMath::Clamp(Reveal, 0.f, 1.f) * Len;
	const int32 Num = TrailMeshes.Num();

	if (Num > 0)
	{
		// Morceaux derriere la tete visibles ; celui qui la contient est raccourci jusqu'a elle
		const float Step = Len / Num;
		const int32 Visible = Reveal >= 1.f ? Num : FMath::Clamp(FMath::CeilToInt(Dist / Step), 0, Num);
		const bool bFull = AppliedVisible < 0;
		const int32 From = bFull ? 0 : FMath::Min(Visible, AppliedVisible);
		const int32 To = bFull ? Num : FMath::Max(Visible, AppliedVisible);
		for (int32 i = From; i < To; ++i)
		{
			if (TrailMeshes[i])
			{
				TrailMeshes[i]->SetVisibility(i < Visible);
				if (bFull) SetSegmentEnd(i, Step * (i + 1));
			}
		}
		if (PartialIndex != INDEX_NONE && PartialIndex != Visible - 1)
		{
			SetSegmentEnd(PartialIndex, Step * (PartialIndex + 1));
			PartialIndex = INDEX_NONE;
		}
		if (Visible > 0 && Dist < Step * Visible)
		{
			SetSegmentEnd(Visible - 1, Dist);
			PartialIndex = Visible - 1;
		}
		AppliedVisible = Visible;
	}

	// Boule en tete, de la couleur du troncon, tant que le trace se deroule
	const bool bShowHead = HeadSizeM > 0.f && Reveal > 0.001f && Reveal < 0.999f && Trail->GetNumberOfSplinePoints() >= 2;
	Head->SetVisibility(bShowHead);
	if (bShowHead)
	{
		if (!HeadMID)
		{
			if (UMaterialInterface* Base = HeadMaterial.LoadSynchronous())
			{
				HeadMID = UMaterialInstanceDynamic::Create(Base, this);
				Head->SetMaterial(0, HeadMID);
			}
		}
		Head->SetWorldLocation(GetRevealLocation());
		Head->SetWorldScale3D(FVector(HeadSizeM));
		if (HeadMID)
		{
			HeadMID->SetVectorParameterValue(TEXT("Couleur"), Dist >= SplitDistCm ? TrailColor2 : TrailColor);
			HeadMID->SetScalarParameterValue(TEXT("Intensite"), HeadGlow);
		}
	}

	for (int32 i = 0; i < Markers.Num(); ++i)
	{
		if (Markers[i] && MarkerDistances.IsValidIndex(i))
		{
			Markers[i]->SetVisibility(!(bHidePanelsOnDecalottage && Decalottage > 0.001f) && (bAlwaysShowPanels || MarkerDistances[i] <= Dist));
		}
	}
	for (const TPair<TWeakObjectPtr<AActor>, float>& Pair : RevealActors)
	{
		if (AActor* Actor = Pair.Key.Get())
		{
			Actor->SetActorHiddenInGame((bHidePanelsOnDecalottage && Decalottage > 0.001f) || (!bAlwaysShowPanels && Pair.Value > Dist));
		}
	}
}

void ACinematicTrail::ClearDrape()
{
	Modify();
	DrapedLLH.Reset();
	Rebuild();
}

void ACinematicTrail::DrapeOnTerrain()
{
	ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(this);
	TActorIterator<ACesium3DTileset> It(GetWorld());
	ACesium3DTileset* Tileset = It ? *It : nullptr;
	if (!Georef || !Tileset || !HasTrail())
	{
		CinematicTrail::Notify(TEXT("Trace cinema : pas de trace ou pas de tileset Cesium pour le drape"), true);
		return;
	}

	TArray<FVector> Query;
	for (int32 i = 0; i < PointsLLH.Num(); ++i)
	{
		const FVector A(PointsLLH[i].X, PointsLLH[i].Y, PointsLLH[i].Z + APath::GetGeoidUndulationM(PointsLLH[i].X, PointsLLH[i].Y));
		Query.Add(A);
		if (i + 1 == PointsLLH.Num()) break;
		const FVector B(PointsLLH[i + 1].X, PointsLLH[i + 1].Y, PointsLLH[i + 1].Z + APath::GetGeoidUndulationM(PointsLLH[i + 1].X, PointsLLH[i + 1].Y));
		const double Dx = (B.X - A.X) * 111320.0 * FMath::Cos(FMath::DegreesToRadians(A.Y));
		const double Dy = (B.Y - A.Y) * 110540.0;
		const int32 Steps = FMath::CeilToInt(FMath::Sqrt(Dx * Dx + Dy * Dy) / FMath::Max(DrapeStepM, 1.f));
		for (int32 k = 1; k < Steps; ++k)
		{
			Query.Add(FMath::Lerp(A, B, double(k) / Steps));
		}
	}
	TWeakObjectPtr<ACinematicTrail> WeakThis(this);
	Tileset->SampleHeightMostDetailed(Query, FCesiumSampleHeightMostDetailedCallback::CreateLambda(
		[WeakThis, Query](ACesium3DTileset*, const TArray<FCesiumSampleHeightResult>& Results, const TArray<FString>&)
		{
			ACinematicTrail* Self = WeakThis.Get();
			ACesiumGeoreference* Georef = Self ? ACesiumGeoreference::GetDefaultGeoreference(Self) : nullptr;
			if (!Georef) return;

			TArray<UPrimitiveComponent*> Meshes;
			for (AActor* Actor : Self->DrapeMeshes)
			{
				if (IsValid(Actor))
				{
					TArray<UPrimitiveComponent*> Components;
					Actor->GetComponents(Components);
					Meshes.Append(Components);
				}
			}

			int32 OnMeshes = 0, OnCesium = 0;
			TArray<FVector> Draped;
			TArray<bool> Measured;
			for (int32 i = 0; i < Query.Num(); ++i)
			{
				const bool bCesium = Results.IsValidIndex(i) && Results[i].SampleSuccess;
				double Height = bCesium ? Results[i].LongitudeLatitudeHeight.Z : Query[i].Z;
				OnCesium += bCesium ? 1 : 0;

				// Les meshes cales (terrain LiDAR, glace) remplacent Google la ou ils existent : on prend leur surface
				const FVector Pos = Georef->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Query[i].X, Query[i].Y, Height));
				const FVector Start = Pos + FVector(0, 0, 300000.0);
				const FVector End = Pos - FVector(0, 0, 300000.0);
				double BestZ = -UE_DOUBLE_BIG_NUMBER;
				for (UPrimitiveComponent* Mesh : Meshes)
				{
					FHitResult Hit;
					if (Mesh->LineTraceComponent(Hit, Start, End, FCollisionQueryParams(NAME_None, true)) && Hit.ImpactPoint.Z > BestZ)
					{
						BestZ = Hit.ImpactPoint.Z;
					}
				}
				if (BestZ > -UE_DOUBLE_BIG_NUMBER)
				{
					// Meshes et tuiles Cesium se recouvrent sur la bande de raccord : le trace passe au-dessus du plus haut des deux
					const double MeshHeight = Georef->TransformUnrealPositionToLongitudeLatitudeHeight(FVector(Pos.X, Pos.Y, BestZ)).Z;
					Height = bCesium ? FMath::Max(Height, MeshHeight) : MeshHeight;
					++OnMeshes;
				}
				Draped.Add(FVector(Query[i].X, Query[i].Y, Height));
				Measured.Add(bCesium || BestZ > -UE_DOUBLE_BIG_NUMBER);
				}

				// Points sans mesure (tuile pas chargee) : interpoles entre les mesures voisines, pas laisses a l'altitude GPS
				int32 Interpolated = 0, Spikes = 0;
				const int32 N = Draped.Num();
				for (int32 i = 0; i < N; ++i)
				{
				if (Measured[i]) continue;
				int32 A = i - 1, B = i + 1;
				while (A >= 0 && !Measured[A]) --A;
				while (B < N && !Measured[B]) ++B;
				if (A >= 0 && B < N) Draped[i].Z = FMath::Lerp(Draped[A].Z, Draped[B].Z, double(i - A) / (B - A));
				else if (A >= 0) Draped[i].Z = Draped[A].Z;
				else if (B < N) Draped[i].Z = Draped[B].Z;
				++Interpolated;
				}
				// Pics isoles : un point trop loin de la mediane de ses voisins y est ramene
				const TArray<FVector> Raw = Draped;
				for (int32 i = 0; i < N; ++i)
				{
				TArray<double, TInlineAllocator<9>> Near;
				for (int32 k = FMath::Max(0, i - 4); k <= FMath::Min(N - 1, i + 4); ++k) Near.Add(Raw[k].Z);
				Near.Sort();
				const double Median = Near[Near.Num() / 2];
				if (FMath::Abs(Raw[i].Z - Median) > Self->MaxGroundJumpM)
				{
				Draped[i].Z = Median;
				++Spikes;
				}
				}

			Self->Modify();
			Self->DrapedLLH = MoveTemp(Draped);
			Self->Rebuild();
			CinematicTrail::Notify(FString::Printf(TEXT("Trace cinema : %d points drapes (%d sur les meshes, %d sur Cesium, %d interpoles, %d pics corriges)"),
			Query.Num(), OnMeshes, OnCesium, Interpolated, Spikes), false);
		}));
}
