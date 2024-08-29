//
// Verify if a map is eligible for FN play
//

#include "rapidjson/document_safe.h"
#include "ValidateMapRequest.h"
#include "FNShareddefs.h"
#include "msdllheaders.h"
#include "global.h"

ValidateMapRequest::ValidateMapRequest(const char* url) :
	SteamHttpRequest(EHTTPMethod::k_EHTTPMethodGET, url)
{
}

void ValidateMapRequest::OnResponse(bool bSuccessful)
{
	if (bSuccessful == false || pJSONData == NULL)
	{
		FNShared::Print("Map '%s' is not verified for FN!\n", MSGlobals::MapName.c_str());
		SERVER_COMMAND("changelevel edana\n");
		return;
	}

	const JSONValue& value = (*pJSONData);
	if (!value["data"].GetBool())
	{
		FNShared::Print("Map '%s' is not verified for FN!\n", MSGlobals::MapName.c_str());
		SERVER_COMMAND("changelevel edana\n");
	}
}