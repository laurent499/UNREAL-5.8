// WorldUtils.h
#pragma once

#include "CoreMinimal.h"

class UObject;
class UWorld;
class UGameInstance;

template<typename T> class TSubclassOf;

class TRAILUTILS_API FWorldUtils
{
public:
	static UWorld* GetWorld(const UObject* WorldContextObject);
	static UWorld* GetWorldChecked(const UObject* WorldContextObject);

	static UWorld* GetRuntimeWorldFallback();

	static bool IsPIE(const UObject* WorldContextObject);
	static bool IsGameWorld(const UObject* WorldContextObject);

	// ---- GameInstance ----
	static UGameInstance* GetGameInstance(const UObject* WorldContextObject);
	static UGameInstance* GetGameInstanceChecked(const UObject* WorldContextObject);

	// ---- Subsystems ----
	// GameInstanceSubsystem (le plus courant chez toi)
	template<typename TSubsystem>
	static TSubsystem* GetGISubsystem(const UObject* WorldContextObject);

	template<typename TSubsystem>
	static TSubsystem* GetGISubsystemChecked(const UObject* WorldContextObject);

	// With Log
	template<typename TSubsystem>
	static TSubsystem* GetGISubsystemOrLog(const UObject* WorldContextObject, const TCHAR* DebugContext, bool bLogOnce = true);
	
	
	// WorldSubsystem
	template<typename TSubsystem>
	static TSubsystem* GetWorldSubsystem(const UObject* WorldContextObject);

	template<typename TSubsystem>
	static TSubsystem* GetWorldSubsystemChecked(const UObject* WorldContextObject);
	
	// With Log
	template<typename TSubsystem>
	static TSubsystem* GetWorldSubsystemOrLog(const UObject* WorldContextObject, const TCHAR* DebugContext, bool bLogOnce = true);

};
#include "WorldUtils.inl"
