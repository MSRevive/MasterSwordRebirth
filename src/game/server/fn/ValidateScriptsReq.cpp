//
// Validate sc.dll
//

#include "rapidjson/document_safe.h"
#include "ValidateScriptsReq.h"
#include "FNShareddefs.h"
#include "msdllheaders.h"
#include "global.h"

ValidateScriptsRequest::ValidateScriptsRequest(const char* url) :
	HTTPRequest(EHTTPMethod::k_EHTTPMethodGET, url)
{
}

void ValidateScriptsRequest::OnResponse(bool bSuccessful)
{
	if (bSuccessful == false || pJSONData == NULL)
	{
		MSGlobals::CentralEnabled = false;
		FNShared::Print("FuzzNet has been disabled!\n");
		return;
	}

	const JSONValue& value = (*pJSONData);
	if (!value["data"].GetBool())
	{
		FNShared::Print("Script file not verified for FN!\n");
		MSGlobals::CentralEnabled = false;
	}
}