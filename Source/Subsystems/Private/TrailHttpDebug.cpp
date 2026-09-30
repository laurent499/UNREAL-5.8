#include "TrailHttpDebug.h"
#include "Misc/DefaultValueHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogTrailHttp, Log, All);

static bool IsLikelyBinaryContent(const FString& ContentType)
{
	// Très simple: si ce n’est pas texte/json/xml, on considère binaire
	if (ContentType.StartsWith(TEXT("text/"))) return false;
	if (ContentType.Contains(TEXT("json"))) return false;
	if (ContentType.Contains(TEXT("xml"))) return false;
	return true;
}

void FTrailHttpDebug::LogStart(const TCHAR* Context, const FHttpRequestPtr& Request)
{
	const FString Url  = Request.IsValid() ? Request->GetURL()  : TEXT("<null>");
	const FString Verb = Request.IsValid() ? Request->GetVerb() : TEXT("<null>");
	UE_LOG(LogTrailHttp, VeryVerbose, TEXT("[%s][HTTP] START %s %s"), Context, *Verb, *Url);
}

bool FTrailHttpDebug::LogAndIsSuccess(
	const TCHAR* Context,
	const FHttpRequestPtr& Request,
	const FHttpResponsePtr& Response,
	bool bWasSuccessful,
	const FTrailHttpDebugOptions& Options,
	int32* OutCode
)
{
	const FString Url  = Request.IsValid() ? Request->GetURL()  : TEXT("<null>");
	const FString Verb = Request.IsValid() ? Request->GetVerb() : TEXT("<null>");

	// Transport / client
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTrailHttp, Error,
			TEXT("[%s][HTTP] FAIL %s %s | bWasSuccessful=%d | ResponseValid=%d"),
			Context, *Verb, *Url, bWasSuccessful ? 1 : 0, Response.IsValid() ? 1 : 0);

		// Parfois Response est valide malgré tout
		if (Response.IsValid())
		{
			const int32 Code = Response->GetResponseCode();
			if (OutCode) *OutCode = Code;

			UE_LOG(LogTrailHttp, Error, TEXT("[%s][HTTP] Code=%d"), Context, Code);
		}
		else
		{
			if (OutCode) *OutCode = -1;
		}

		return false;
	}

	// HTTP
	const int32 Code = Response->GetResponseCode();
	if (OutCode) *OutCode = Code;

	const FString ContentType = Response->GetContentType();
	UE_LOG(LogTrailHttp, Log, TEXT("[%s][HTTP] %s %s -> %d | Content-Type=%s"),
		Context, *Verb, *Url, Code, *ContentType);

	if (Options.bLogHeaders)
	{
		for (const FString& H : Response->GetAllHeaders())
		{
			UE_LOG(LogTrailHttp, VeryVerbose, TEXT("[%s][HTTP][H] %s"), Context, *H);
		}
	}

	const bool bHttpOk = EHttpResponseCodes::IsOk(Code); // 200-299

	// Body (optionnel)
	if (Options.bLogBody)
	{
		const bool bBinary = Options.bSkipBodyIfBinary && IsLikelyBinaryContent(ContentType);
		if (!bBinary)
		{
			FString Body = Response->GetContentAsString();
			if (Options.MaxBodyChars > 0 && Body.Len() > Options.MaxBodyChars)
			{
				Body = Body.Left(Options.MaxBodyChars) + TEXT("... <truncated>");
			}

			UE_LOG(LogTrailHttp, VeryVerbose, TEXT("[%s][HTTP] Body: %s"), Context, *Body);
		}
		else
		{
			UE_LOG(LogTrailHttp, VeryVerbose, TEXT("[%s][HTTP] Body skipped (binary content)."), Context);
		}
	}

	if (!bHttpOk)
	{
		UE_LOG(LogTrailHttp, Error, TEXT("[%s][HTTP] Server error %d for %s %s"),
			Context, Code, *Verb, *Url);
		return false;
	}

	return true;
}
