//
// Validate sc.dll
//

#include "rapidjson/document_safe.h"
#include "ValidateScriptsRequest.h"
#include "FNShareddefs.h"
#include "msdllheaders.h"
#include "global.h"

ValidateScriptsRequest::ValidateScriptsRequest(const char* url) :
	SteamHttpRequest(EHTTPMethod::k_EHTTPMethodGET, url)
{
}

void ValidateScriptsRequest::OnResponse(bool bSuccessful)
{
	if (bSuccessful == false || pJSONData == NULL)
	{
		FNShared::Print("Script file not verified for FN!\n");
		MSGlobals::CentralEnabled = false;
		return;
	}

	const JSONValue& value = (*pJSONData);
	if (!value["data"].GetBool())
	{
		FNShared::Print("Script file not verified for FN!\n");
		MSGlobals::CentralEnabled = false;
	}
}