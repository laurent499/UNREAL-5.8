// WorldUtils.cpp
#include "WorldUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"          // GEngine, FWorldContext
#include "Engine/World.h"           // UWorld
#include "UObject/UObjectGlobals.h" // GetWorldFromContextObject

UWorld* FWorldUtils::GetWorld(const UObject* WorldContextObject)
{
    if (!WorldContextObject) return nullptr;

    // Souvent suffisant (Actors, Components, Subsystems, etc.)
    if (UWorld* W = WorldContextObject->GetWorld())
    {
        return W;
    }

    if (!GEngine) return nullptr;

    return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
}

UWorld* FWorldUtils::GetWorldChecked(const UObject* WorldContextObject)
{
    check(WorldContextObject);
    if (UWorld* W = WorldContextObject->GetWorld())
    {
        return W;
    }

    check(GEngine);
    return GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
}

UWorld* FWorldUtils::GetRuntimeWorldFallback()
{
    if (!GEngine) return nullptr;

    // En pratique, préfère l’appeler sur le GameThread
    ensureMsgf(IsInGameThread(), TEXT("GetRuntimeWorldFallback should be called on the GameThread."));

    UWorld* BestWorld = nullptr;
    int32 BestScore = -1;

    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        UWorld* W = Ctx.World();
        if (!W) continue;

        int32 Score = -1;

        // Priorités “runtime”
        if (Ctx.WorldType == EWorldType::PIE)              Score = 300;
        else if (Ctx.WorldType == EWorldType::Game)        Score = 200;
        else if (Ctx.WorldType == EWorldType::GamePreview) Score = 190;
        else if (W->IsGameWorld())                         Score = 100;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestWorld = W;
        }
    }

    return BestWorld;
}

bool FWorldUtils::IsPIE(const UObject* WorldContextObject)
{
    if (const UWorld* W = GetWorld(WorldContextObject))
    {
        return W->WorldType == EWorldType::PIE;
    }
    return false;
}

bool FWorldUtils::IsGameWorld(const UObject* WorldContextObject)
{
    if (const UWorld* W = GetWorld(WorldContextObject))
    {
        return W->IsGameWorld();
    }
    return false;
}


UGameInstance* FWorldUtils::GetGameInstance(const UObject* WorldContextObject)
{
    if (UWorld* W = GetWorld(WorldContextObject))
    {
        return W->GetGameInstance();
    }
    return nullptr;
}

UGameInstance* FWorldUtils::GetGameInstanceChecked(const UObject* WorldContextObject)
{
    UWorld* W = GetWorldChecked(WorldContextObject);
    UGameInstance* GI = W->GetGameInstance();
    check(GI);
    return GI;
}
