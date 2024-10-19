//
// Validate sc.dll
//

#include "rapidjson/document.h"
#include "ValidateScriptsReq.h"
#include "FNSharedDefs.h"
#include "msdllheaders.h"
#include "global.h"

ValidateScriptsRequest::ValidateScriptsRequest(const char* url) :
	HTTPRequest(HTTPMethod::GET, url)
{
}

void ValidateScriptsRequest::OnResponse(bool bSuccessful, JSONDocument* jsonDoc, int iRespCode)
{
	if (bSuccessful == false)
	{
		// MSGlobals::CentralEnabled = false;
		// FNShared::Print("FuzzNet has been disabled!\n");
		return;
	}

	JSONDocument& doc = (*jsonDoc);
	if (!doc["data"].GetBool())
	{
		FNShared::Print("Script file not verified for FN!\n");
		MSGlobals::CentralEnabled = false;
	}

	FNShared::Print("Scripts verified for FN.\n");
}