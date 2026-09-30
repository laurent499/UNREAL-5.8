#pragma once

#include "CoreMinimal.h"
#include "Http.h" // FHttpRequestPtr / FHttpResponsePtr

struct FTrailHttpDebugOptions
{
	bool bLogHeaders = false;
	bool bLogBody = true;

	// Evite de spam si le body est énorme
	int32 MaxBodyChars = 2000;

	// Si tu télécharges des images/binaire, évite de log le body en string
	bool bSkipBodyIfBinary = true;
};

class FTrailHttpDebug
{
public:
	// Retourne "succès" au sens: transport OK + code HTTP 2xx
	static bool LogAndIsSuccess(
		const TCHAR* Context,
		const FHttpRequestPtr& Request,
		const FHttpResponsePtr& Response,
		bool bWasSuccessful,
		const FTrailHttpDebugOptions& Options = FTrailHttpDebugOptions(),
		int32* OutCode = nullptr
	);

	static void LogStart(const TCHAR* Context, const FHttpRequestPtr& Request);
};
