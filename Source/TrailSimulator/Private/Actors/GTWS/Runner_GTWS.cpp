// All Rights Reserved


#include "Actors/GTWS/Runner_GTWS.h"
#include "TemplateSubsystem.h"
#include "RaceManager.h"
#include "GTWS/GTWSStyle.h"
#include "Components/TrailTextWidgetComponent.h"
#include "Components/PhotoComponent.h"
#include "Components/ClubComponent.h"
#include "Components/GTWS/GTWS_ClubComponent.h"
#include "Components/GTWS/GTWS_FlagComponent.h"
#include "Components/GTWS/GTWS_NameComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

/**
 * @brief Fond de panneau avec coins arrondis : M_PlainRounded (même rendu que M_Plain, qui
 *        ignore son masque d'opacité). Primitive data : 0-3 couleur, 5 alpha, 6 arrondi, 7 coins.
 */
static void UseRoundedBkg_RunnerGTWS(UPrimitiveComponent* Comp, bool bGradient = false)
{
	// M_PlainRoundedGradient : copie de M_PlainRounded (mêmes primitive data 0-9) avec un dégradé
	// gauche -> droite vers BaseColorB (primitive data 10-13). Tools/Templates/create_plain_rounded_gradient.py
	static TWeakObjectPtr<UMaterialInterface> Rounded;
	static TWeakObjectPtr<UMaterialInterface> Gradient;
	if (!Rounded.IsValid())
	{
		Rounded = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LTVContent/Materials/Masters/M_PlainRounded.M_PlainRounded"));
	}
	if (bGradient && !Gradient.IsValid())
	{
		Gradient = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LTVContent/Materials/Masters/M_PlainRoundedGradient.M_PlainRoundedGradient"));
	}
	UMaterialInterface* Wanted = (bGradient && Gradient.IsValid()) ? Gradient.Get() : Rounded.Get();
	if (Comp && Wanted && Comp->GetMaterial(0) != Wanted)
	{
		Comp->SetMaterial(0, Wanted);
	}
}

/**
 * @brief Boîte englobante de Root et de ses descendants visibles, dans le repère de Ref.
 * @note Sert à poser photo et drapeau contre la barre du nom quelle que soit la taille de leurs meshes.
 */
static FBox BoxIn_RunnerGTWS(const USceneComponent* Root, const FTransform& Ref, bool bWithChildren = true)
{
	FBox Box(ForceInit);
	if (!Root) return Box;
	TArray<USceneComponent*> Comps;
	if (bWithChildren) Root->GetChildrenComponents(true, Comps);
	Comps.Add(const_cast<USceneComponent*>(Root));
	for (const USceneComponent* C : Comps)
	{
		const UPrimitiveComponent* P = Cast<UPrimitiveComponent>(C);
		if (P && P->IsVisible())
		{
			Box += P->CalcBounds(P->GetComponentTransform().GetRelativeTransform(Ref)).GetBox();
		}
	}
	return Box;
}

/** Texte 3D du nom (membre protégé de UNameComponent). */
static UTextRenderComponent* NameText_RunnerGTWS(const USceneComponent* NameComp)
{
	TArray<USceneComponent*> Children;
	if (NameComp) NameComp->GetChildrenComponents(false, Children);
	for (USceneComponent* C : Children)
	{
		if (UTextRenderComponent* T = Cast<UTextRenderComponent>(C)) return T;
	}
	return nullptr;
}

/** Écart entre le drapeau et le nom quand le drapeau est dans la barre du nom (cm). */
static constexpr float FlagNameGap_RunnerGTWS = 20.f;
/** Position d'origine du drapeau sous ClubHook. */
static const FVector FlagClubLocation_RunnerGTWS(0.f, -6.f, 0.f);
/** Position d'origine de la photo sous NameHook. */
static const FVector PhotoLeftLocation_RunnerGTWS(31.f, 130.f, -60.f);

/**
 * @brief Barre du club affichée ? Avec le drapeau dans la barre du nom, un coureur sans club
 *        n'a plus rien à y mettre : la barre disparaît (fond, texte, place du drapeau).
 *        Drapeau dans la barre du club : comportement d'origine (seul le choix opérateur / template compte).
 */
static bool ClubBarShown_RunnerGTWS(bool bClubVisible, const FRunnerStruct& Datas, const FTrailTemplateRunner& Style)
{
	if (!bClubVisible) return false;
	return !(Style.FlagPosition.Equals(TEXT("name"), ESearchCase::IgnoreCase) && Datas.club.TrimStartAndEnd().IsEmpty());
}

/**
 * @brief Pied (tige sous la carte) : touche le bas de la dernière barre visible.
 * @note Barre du club masquée (club vide avec le drapeau dans le nom, ou choix opérateur) : le haut
 *       du pied est calé sur le bas du fond du nom, mesuré dans le repère du parent du pied ; son bas
 *       (ancrage au sol) ne bouge pas. Le fond du club n'entre pas dans le calcul : masqué, il n'est
 *       plus mis en forme et ses bornes ne disent rien. Barre du club affichée : échelle et position
 *       d'origine (celles du CDO). La photo n'entre pas en compte.
 */
static void FitFoot_RunnerGTWS(UStaticMeshComponent* Foot, const UStaticMeshComponent* DefaultFoot,
	const UPrimitiveComponent* NameBkg, bool bClubBar)
{
	if (!Foot || !DefaultFoot || !Foot->GetStaticMesh()) return;
	const FVector DefaultScale = DefaultFoot->GetRelativeScale3D();
	const FVector DefaultLocation = DefaultFoot->GetRelativeLocation();
	FVector Scale = DefaultScale;
	FVector Location = DefaultLocation;
	if (!bClubBar && NameBkg && DefaultScale.Z > KINDA_SMALL_NUMBER)
	{
		// Repère du parent du pied : unités de sa position et de son échelle relatives
		// (indépendant de l'échelle automatique de l'acteur, qui s'applique à tout)
		const USceneComponent* Parent = Foot->GetAttachParent();
		const FTransform Ref = Parent ? Parent->GetComponentTransform() : FTransform::Identity;
		const FBox NameBox = BoxIn_RunnerGTWS(NameBkg, Ref, false);
		// Bornes du mesh du pied, quel que soit son pivot (centre ou extrémité)
		const FBox MeshBox = Foot->GetStaticMesh()->GetBoundingBox();
		const float MeshHeight = MeshBox.Max.Z - MeshBox.Min.Z;
		const float Bottom = DefaultLocation.Z + MeshBox.Min.Z * DefaultScale.Z;
		const float DefaultTop = DefaultLocation.Z + MeshBox.Max.Z * DefaultScale.Z;
		if (NameBox.IsValid && MeshHeight > KINDA_SMALL_NUMBER && NameBox.Min.Z > DefaultTop)
		{
			Scale.Z = (NameBox.Min.Z - Bottom) / MeshHeight;
			Location.Z = Bottom - MeshBox.Min.Z * Scale.Z;
		}
		// TEMPORAIRE (diagnostic du pied) : à retirer une fois le rendu validé
		if (!Foot->GetRelativeScale3D().Equals(Scale, 1e-4f))
		{
			UE_LOG(LogTemp, Log, TEXT("[FootFit] %s Name Z %.1f..%.1f | foot mesh Z %.1f..%.1f, defaut loc %.1f scale %.3f (haut %.1f, bas %.1f) | courant loc %.1f scale %.3f -> loc %.1f scale %.3f"),
				*GetNameSafe(Foot->GetOwner()), NameBox.Min.Z, NameBox.Max.Z, MeshBox.Min.Z, MeshBox.Max.Z,
				DefaultLocation.Z, DefaultScale.Z, DefaultTop, Bottom,
				Foot->GetRelativeLocation().Z, Foot->GetRelativeScale3D().Z, Location.Z, Scale.Z);
		}
	}
	if (!Foot->GetRelativeScale3D().Equals(Scale, 1e-4f)) Foot->SetRelativeScale3D(Scale);
	if (!Foot->GetRelativeLocation().Equals(Location, 0.01f)) Foot->SetRelativeLocation(Location);
}

ARunner_GTWS::ARunner_GTWS()
{
	
	PresentationRoot->SetRelativeLocation(FVector(0.f, 0.f, 700.f));
	
	// Name
	NameComponent = CreateDefaultSubobject<UGTWS_NameComponent>(TEXT("NameComponent"));
	NameComponent->SetupAttachment(NameHook);
	NameComponent->SetComponentTickEnabled(false);
	NameComponent->SetRelativeTransform(FTransform::Identity);
	NameComponent->ShowWidgetName(false);
	NameComponent->Show3DName(true);
	NameHook->SetRelativeLocation(FVector(30.f, 0.f, 0.f));
	
	// Photo
	PhotoHook = CreateDefaultSubobject<USceneComponent>(TEXT("PhotoHook"));
	PhotoHook->SetupAttachment(NameHook);
	PhotoHook->SetComponentTickEnabled(false);
	PhotoHook->SetRelativeTransform(FTransform::Identity);
	PhotoHook->SetRelativeLocation(FVector(31.f, 130.f, -60.f));
	
	PhotoComponent = CreateDefaultSubobject<UPhotoComponent>(TEXT("PhotoComponent"));
	PhotoComponent->SetupAttachment(PhotoHook);
	PhotoComponent->SetComponentTickEnabled(false);
	PhotoComponent->SetGenerateOverlapEvents(false);
	PhotoComponent->SetRelativeTransform(FTransform::Identity);
	PhotoComponent->SetRelativeScale3D(FVector(0.8f, 1.f, 0.8f));
	
	// Club
	ClubHook = CreateDefaultSubobject<USceneComponent>(TEXT("ClubHook"));
	ClubHook->SetupAttachment(FootHook);
	ClubHook->SetComponentTickEnabled(false);
	ClubHook->SetRelativeTransform(FTransform::Identity);
	ClubHook->SetRelativeLocation(FVector(30.f, 0.f, -63.f));
	ClubHook->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	
	ClubComponent = CreateDefaultSubobject<UGTWS_ClubComponent>(TEXT("ClubComponent"));
	ClubComponent->SetupAttachment(ClubHook);
	ClubComponent->SetComponentTickEnabled(false);
	ClubComponent->SetRelativeTransform(FTransform::Identity);
	ClubComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.0f));
	ClubComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ClubComponent->ShowWidgetName(false);
	ClubComponent->Show3DName(true);
	
	// Flag
	FlagComponent = CreateDefaultSubobject<UGTWS_FlagComponent>(TEXT("FlagComponent"));
	FlagComponent->SetupAttachment(ClubHook);
	FlagComponent->SetComponentTickEnabled(false);
	FlagComponent->SetRelativeTransform(FTransform::Identity);
	FlagComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	FlagComponent->SetRelativeLocation(FVector(0.f, -6.f, 0.0f));
	FlagComponent->SetRelativeScale3D(FVector(0.515f, 1.f, 0.515f));
}

void ARunner_GTWS::UpdateRunner(FRunnerStruct Runner, FRaceSetup RaceSetup)
{
	Super::UpdateRunner(Runner, RaceSetup);

	// Couleurs et marges lues dans le template de la course (régie, page Templates)
	TOptional<FTrailTemplate> TemplateStorage;
	const FTrailTemplate& Tpl = UTemplateSubsystem::ResolveForRace(this, RaceSetup, TemplateStorage);
	const FTrailTemplateRunner& Style = Tpl.Runner;
	// Arrondi des coins des fonds (M_Plain, primitive data 6). UpdateRunner revient à chaque
	// snapshot : on ne touche au rendu que si la valeur change.
	// Mode (primitive data 7) : 0 tous les coins, 1 ceux du haut, 2 ceux du bas ; nom et club
	// forment ainsi un seul bloc arrondi à l'extérieur, comme le synthé OnGraphics.
	auto SetCornerRadius = [&Tpl](UPrimitiveComponent* Comp, float Mode, bool bGradient)
	{
		UseRoundedBkg_RunnerGTWS(Comp, bGradient);
		const TArray<float>& Data = Comp->GetCustomPrimitiveData().Data;
		if (!Data.IsValidIndex(6) || Data[6] != Tpl.Globals.CornerRadius)
		{
			Comp->SetCustomPrimitiveDataFloat(6, Tpl.Globals.CornerRadius);
		}
		if (!Data.IsValidIndex(7) || Data[7] != Mode)
		{
			Comp->SetCustomPrimitiveDataFloat(7, Mode);
		}
		// Biseau en degrés (primitive data 8)
		if (!Data.IsValidIndex(8) || Data[8] != Tpl.Globals.Slant)
		{
			Comp->SetCustomPrimitiveDataFloat(8, Tpl.Globals.Slant);
		}
	};
	// Hauteur de référence du biseau (primitive data 9) : nom et club pivotent sur leur jointure
	// et doivent perdre la même largeur pour que leurs côtés penchés se prolongent.
	auto SetSlantRefHeight = [](UPrimitiveComponent* Comp, float RefHeight)
	{
		const TArray<float>& Data = Comp->GetCustomPrimitiveData().Data;
		if (!Data.IsValidIndex(9) || !FMath::IsNearlyEqual(Data[9], RefHeight, 0.01f))
		{
			Comp->SetCustomPrimitiveDataFloat(9, RefHeight);
		}
	};
	// Hauteur d'un fond (cm) : la dimension médiane de ses bornes locales mises à l'échelle
	// (la plus mince est l'épaisseur, la plus grande la largeur)
	auto BkgHeight = [](const UPrimitiveComponent* Comp)
	{
		const FVector Size = Comp->CalcBounds(FTransform::Identity).GetBox().GetSize() * Comp->GetComponentScale().GetAbs();
		return FMath::Max(FMath::Min(Size.X, Size.Y), FMath::Min(FMath::Max(Size.X, Size.Y), Size.Z));
	};
	auto Lin = [&RaceSetup](const FString& Value) { return FTrailTemplate::ToLinear(Value, RaceSetup.color); };
	auto Srgb = [&RaceSetup](const FString& Value) { return FTrailTemplate::ToSRGB(Value, RaceSetup.color); };
	
	// Foot
	if (FootComponent)
	{
		FootComponent->SetColorParameterValueOnMaterials("BaseColor", Lin(Style.Foot));
	}
	
	// Options du template (défauts = rendu d'origine) : photo sur la barre du nom, drapeau avant
	// le nom, dégradé sur le fond du nom
	const bool bPhotoAbove = Style.PhotoPosition.Equals(TEXT("above"), ESearchCase::IgnoreCase);
	const bool bFlagInName = FlagComponent && IsFlagVisible() && !Runner.pays.IsEmpty()
		&& Style.FlagPosition.Equals(TEXT("name"), ESearchCase::IgnoreCase);
	const bool bGradient = Style.NameGradient;
	// Barre du club : masquée si le drapeau est dans le nom et que le coureur n'a pas de club.
	// Le reste de la carte suit comme pour un club masqué (largeur du nom seul, nom arrondi tout autour).
	const bool bClubBar = ClubBarShown_RunnerGTWS(IsClubVisible(), Runner, Style);
	if (ClubComponent)
	{
		ClubComponent->ToggleClub(bClubBar);
	}

	if (ClubComponent && NameComponent)
	{
		// Les deux textes sont posés d'abord : il faut qu'ils le soient pour être mesurables
		NameComponent->SetColors(Lin(bGradient ? Style.NameGradientFrom : Style.NameBkg), Srgb(Style.NameText));
		ClubComponent->SetColors(Lin(Style.ClubBkg), Srgb(Style.ClubText));
		// Club masqué : le nom reste seul et s'arrondit tout autour
		if (UPrimitiveComponent* NameBkg = NameComponent->NameBkgComponent)
		{
			SetCornerRadius(NameBkg, bClubBar ? 1.f : 0.f, bGradient);
			if (bGradient)
			{
				// Couleur d'arrivée du dégradé (M_PlainRoundedGradient, primitive data 10-13)
				const FLinearColor To = Lin(Style.NameGradientTo);
				const TArray<float>& Data = NameBkg->GetCustomPrimitiveData().Data;
				if (!Data.IsValidIndex(13) || !FLinearColor(Data[10], Data[11], Data[12], Data[13]).Equals(To))
				{
					NameBkg->SetCustomPrimitiveDataVector4(10, To);
				}
			}
		}
		if (ClubComponent->ClubBkgComponent) SetCornerRadius(ClubComponent->ClubBkgComponent, 2.f, false);
		NameComponent->UpdateName(Runner.nom);
		ClubComponent->UpdateClub(Runner);

		// Drapeau dans la barre du nom : le nom se décale vers la droite de la largeur du drapeau
		// (UpdateName vient de remettre le texte à sa place d'origine)
		float FlagShift = 0.f;
		if (bFlagInName)
		{
			const FBox FlagBox = BoxIn_RunnerGTWS(FlagComponent, NameHook->GetComponentTransform());
			UTextRenderComponent* NameText = NameText_RunnerGTWS(NameComponent);
			if (FlagBox.IsValid && NameText)
			{
				FlagShift = FlagBox.GetSize().Y + FlagNameGap_RunnerGTWS;
				NameText->AddRelativeLocation(FVector(0.f, -FlagShift, 0.f));
			}
		}

		// Le plus large des deux impose la largeur des deux fonds
		const float ClubTextWidth = bClubBar ? ClubComponent->GetLocalSize().Y : 0.f;
		const FStartAndEnd SharedBounds = GTWSStyle::MakeSharedBkgBounds(
			NameComponent->GetLocalSize().Y + FlagShift, ClubTextWidth, Style.BkgLeftMargin, Style.BkgRightMargin);

		NameComponent->UpdateMesh(SharedBounds.StartLocation, SharedBounds.EndLocation);
		if (bClubBar)
		{
			ClubComponent->UpdateMesh(SharedBounds.StartLocation, SharedBounds.EndLocation);
		}

		UPrimitiveComponent* NameBkg = NameComponent->NameBkgComponent;
		UPrimitiveComponent* ClubBkg = ClubComponent->ClubBkgComponent;
		if (NameBkg && ClubBkg)
		{
			// Nom seul : 0, le fond se biseaute sur sa propre hauteur
			const float RefHeight = (bClubBar && FMath::Abs(Tpl.Globals.Slant) >= 0.01f)
				? FMath::Max(BkgHeight(NameBkg), BkgHeight(ClubBkg)) : 0.f;
			SetSlantRefHeight(NameBkg, RefHeight);
			SetSlantRefHeight(ClubBkg, RefHeight);
		}
	}
	
	if (PhotoComponent)
	{		
		if (IsPhotoVisible()){
			
			FString NewPhotoUrl = Runner.photo;
			if (!Runner.photo.Contains("jpg") && !Runner.photo.Contains("jpeg") && !Runner.photo.Contains("png") && !Runner.photo.Contains("gif"))
			{
				NewPhotoUrl += ".png";
			}
			PhotoComponent->UpdatePhoto(NewPhotoUrl, FVector(0.75f, 1.0f, 0.75f));
			ClubHook->SetRelativeLocation(FVector(30.f, 0.f, -63.f));
			NameHook->SetRelativeLocation(FVector(30.f, 0.f, 0.f));
		} else
		{
			float MiddleClub = ClubComponent->GetMiddle() * 0.5f;
			ClubHook->SetRelativeLocation(FVector(30.f, -MiddleClub-0.515f, -63.f));
			NameHook->SetRelativeLocation(FVector(30.f, -MiddleClub-0.515f, 0.f));
		}
	}
	
	if (FlagComponent && !Runner.pays.IsEmpty() && IsFlagVisible()){
		FlagComponent->UpdateFlag(Runner.pays);
	}

	// Placements mesurés une fois les crochets posés : tout se calcule dans le repère de NameHook
	// (gauche de l'écran = +Y, haut = +Z), quelle que soit la taille des meshes de la photo et du drapeau
	const FTransform NameRef = NameHook->GetComponentTransform();
	UPrimitiveComponent* NameBarBkg = NameComponent ? NameComponent->NameBkgComponent.Get() : nullptr;
	const FBox NameBarBox = NameBarBkg ? BoxIn_RunnerGTWS(NameBarBkg, NameRef, false) : FBox(ForceInit);

	// Photo au-dessus de la barre du nom : posée sur son bord haut, alignée sur son bord gauche
	if (PhotoHook)
	{
		PhotoHook->SetRelativeLocation(PhotoLeftLocation_RunnerGTWS);
		if (bPhotoAbove && PhotoComponent && IsPhotoVisible() && NameBarBox.IsValid)
		{
			const FBox PhotoBox = BoxIn_RunnerGTWS(PhotoComponent, NameRef);
			if (PhotoBox.IsValid)
			{
				PhotoHook->AddRelativeLocation(FVector(0.f, NameBarBox.Max.Y - PhotoBox.Max.Y, NameBarBox.Max.Z - PhotoBox.Min.Z));
			}
		}
	}

	// Drapeau dans la barre du nom : juste avant le nom, centré sur la hauteur de la barre
	if (FlagComponent)
	{
		FlagComponent->SetRelativeLocation(FlagClubLocation_RunnerGTWS);
		UTextRenderComponent* NameText = NameComponent ? NameText_RunnerGTWS(NameComponent) : nullptr;
		if (bFlagInName && NameText && NameBarBox.IsValid)
		{
			const FBox FlagBox = BoxIn_RunnerGTWS(FlagComponent, NameRef);
			const FBox TextBox = BoxIn_RunnerGTWS(NameText, NameRef, false);
			if (FlagBox.IsValid && TextBox.IsValid)
			{
				const FVector Delta(0.f,
					TextBox.Max.Y + FlagNameGap_RunnerGTWS - FlagBox.Min.Y,
					NameBarBox.GetCenter().Z - FlagBox.GetCenter().Z);
				FlagComponent->AddWorldOffset(NameRef.TransformVector(Delta));
			}
		}
	}

	// Pied : jusqu'au bas de la dernière barre visible (une fois les crochets posés)
	if (NameComponent && ClubComponent)
	{
		const ARunner_GTWS* Defaults = GetClass()->GetDefaultObject<ARunner_GTWS>();
		FitFoot_RunnerGTWS(FootComponent, Defaults ? Defaults->GetFootMesh() : nullptr,
			NameComponent->NameBkgComponent, bClubBar);
	}
}

void ARunner_GTWS::ToggleClub(bool bDisplay)
{
	Super::ToggleClub(bDisplay);
	if (ClubComponent)
	{
		// Même règle qu'UpdateRunner : pas de barre du club vide quand le drapeau est dans le nom
		TOptional<FTrailTemplate> Storage;
		const FTrailTemplateRunner& Style = UTemplateSubsystem::ResolveForRace(this, TemplateRaceSetup, Storage).Runner;
		const bool bClubBar = ClubBarShown_RunnerGTWS(IsClubVisible(), RunnerDatas, Style);
		ClubComponent->ToggleClub(bClubBar);
		// Le pied suit tout de suite (sans attendre le prochain relevé)
		if (NameComponent)
		{
			const ARunner_GTWS* Defaults = GetClass()->GetDefaultObject<ARunner_GTWS>();
			FitFoot_RunnerGTWS(FootComponent, Defaults ? Defaults->GetFootMesh() : nullptr,
				NameComponent->NameBkgComponent, bClubBar);
		}
	}
}

void ARunner_GTWS::ToggleFlag(bool bDisplay)
{
	Super::ToggleFlag(bDisplay);
	if (FlagComponent)
	{
		FlagComponent->ToggleFlag(IsFlagVisible());
	}
}

void ARunner_GTWS::TogglePhoto(bool bDisplay)
{
	Super::TogglePhoto(bDisplay);
	if (PhotoComponent)
	{
		PhotoComponent->TogglePhoto(IsPhotoVisible());
	}
}

void ARunner_GTWS::UpdateDayNight(bool bIsDay)
{
	Super::UpdateDayNight(bIsDay);
	
	if (PhotoComponent)
	{
		PhotoComponent->UpdateDayNight(bIsDay);
	}
	if (NameComponent)
	{
		NameComponent->UpdateDayNight(bIsDay);
		// UNameComponent::UpdateDayNight écrit la primitive data 7 (illumination des autres
		// templates), qui est ici le choix des coins arrondis de M_PlainRounded : on le remet.
		// Appelé à chaque relevé de l'heure, sinon le nom alterne entre deux formes.
		if (UPrimitiveComponent* NameBkg = NameComponent->NameBkgComponent)
		{
			TOptional<FTrailTemplate> Storage;
			const FTrailTemplateRunner& Style = UTemplateSubsystem::ResolveForRace(this, TemplateRaceSetup, Storage).Runner;
			NameBkg->SetCustomPrimitiveDataFloat(7, ClubBarShown_RunnerGTWS(IsClubVisible(), RunnerDatas, Style) ? 1.f : 0.f);
		}
	}
	if (FlagComponent)
	{
		FlagComponent->UpdateDayNight(bIsDay);
	}
}

void ARunner_GTWS::BeginPlay()
{
	Super::BeginPlay();
	AActor* RMActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARaceManager::StaticClass());
	RaceManager = Cast<ARaceManager>(RMActor);
	if (RaceManager)
	{
		RaceManager->OnChangeDayNight.AddDynamic(this, &ARunner_GTWS::UpdateDayNight);
	}
}