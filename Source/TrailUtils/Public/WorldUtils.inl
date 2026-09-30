#pragma once

#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TrailUtilsLog.h"

template<typename TSubsystem>
TSubsystem* FWorldUtils::GetGISubsystem(const UObject* WorldContextObject)
{
	UGameInstance* GI = GetGameInstance(WorldContextObject);
	return GI ? GI->GetSubsystem<TSubsystem>() : nullptr;
}

template<typename TSubsystem>
TSubsystem* FWorldUtils::GetGISubsystemChecked(const UObject* WorldContextObject)
{
	UGameInstance* GI = GetGameInstanceChecked(WorldContextObject);
	TSubsystem* SS = GI->GetSubsystem<TSubsystem>();
	check(SS);
	return SS;
}

template<typename TSubsystem>
TSubsystem* FWorldUtils::GetWorldSubsystem(const UObject* WorldContextObject)
{
	UWorld* W = GetWorld(WorldContextObject);
	return W ? W->GetSubsystem<TSubsystem>() : nullptr;
}

template<typename TSubsystem>
TSubsystem* FWorldUtils::GetWorldSubsystemChecked(const UObject* WorldContextObject)
{
	UWorld* W = GetWorldChecked(WorldContextObject);
	TSubsystem* SS = W->GetSubsystem<TSubsystem>();
	check(SS);
	return SS;
}

#pragma once


template<typename TSubsystem>
TSubsystem* FWorldUtils::GetGISubsystemOrLog(const UObject* WorldContextObject, const TCHAR* DebugContext, bool bLogOnce)
{
    if (TSubsystem* SS = GetGISubsystem<TSubsystem>(WorldContextObject))
    {
        return SS;
    }

    // static bool bDidLog = false;
    // if (bLogOnce && bDidLog)
    // {
    //     return nullptr;
    // }
    // bDidLog = true;

    const TCHAR* Ctx = (DebugContext && *DebugContext) ? DebugContext : TEXT("<NoContext>");
    const FString ObjName  = WorldContextObject ? WorldContextObject->GetName() : TEXT("<null>");
    const FString ObjClass = WorldContextObject ? WorldContextObject->GetClass()->GetName() : TEXT("<null>");
    const FString SSName   = TSubsystem::StaticClass() ? TSubsystem::StaticClass()->GetName() : TEXT("<UnknownSubsystem>");

    UWorld* W = GetWorld(WorldContextObject);
    if (!W)
    {
        UE_LOG(LogTrailUtils, Warning, TEXT("[%s] GetGISubsystemOrLog failed: No World. Obj=%s (%s) Subsystem=%s"),
            Ctx, *ObjName, *ObjClass, *SSName);
        return nullptr;
    }

    UGameInstance* GI = W->GetGameInstance();
    if (!GI)
    {
        UE_LOG(LogTrailUtils, Warning, TEXT("[%s] GetGISubsystemOrLog failed: No GameInstance. WorldType=%d Obj=%s (%s) Subsystem=%s"),
            Ctx, int32(W->WorldType), *ObjName, *ObjClass, *SSName);
        return nullptr;
    }

    UE_LOG(LogTrailUtils, Warning, TEXT("[%s] GetGISubsystemOrLog failed: GI exists (%s) but subsystem is null. WorldType=%d Subsystem=%s"),
        Ctx, *GI->GetName(), int32(W->WorldType), *SSName);

    return nullptr;
}

template<typename TSubsystem>
TSubsystem* FWorldUtils::GetWorldSubsystemOrLog(const UObject* WorldContextObject, const TCHAR* DebugContext, bool bLogOnce)
{
    if (TSubsystem* SS = GetWorldSubsystem<TSubsystem>(WorldContextObject))
    {
        return SS;
    }

    static bool bDidLog = false;
    if (bLogOnce && bDidLog)
    {
        return nullptr;
    }
    bDidLog = true;

    const TCHAR* Ctx = (DebugContext && *DebugContext) ? DebugContext : TEXT("<NoContext>");
    const FString ObjName  = WorldContextObject ? WorldContextObject->GetName() : TEXT("<null>");
    const FString ObjClass = WorldContextObject ? WorldContextObject->GetClass()->GetName() : TEXT("<null>");
    const FString SSName   = TSubsystem::StaticClass() ? TSubsystem::StaticClass()->GetName() : TEXT("<UnknownSubsystem>");

    UWorld* W = GetWorld(WorldContextObject);
    if (!W)
    {
        UE_LOG(LogTrailUtils, Warning, TEXT("[%s] GetWorldSubsystemOrLog failed: No World. Obj=%s (%s) Subsystem=%s"),
            Ctx, *ObjName, *ObjClass, *SSName);
        return nullptr;
    }

    UE_LOG(LogTrailUtils, Warning, TEXT("[%s] GetWorldSubsystemOrLog failed: World exists (WorldType=%d) but subsystem is null. Subsystem=%s"),
        Ctx, int32(W->WorldType), *SSName);

    return nullptr;
}
