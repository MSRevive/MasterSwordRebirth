//
// Validate if FN connectivity could be established!
//

#include "rapidjson/document_safe.h"
#include "ValidateConReq.h"
#include "FNShareddefs.h"
#include "msdllheaders.h"
#include "global.h"

ValidateConnectivityRequest::ValidateConnectivityRequest(const char* url) :
	HTTPRequest(EHTTPMethod::k_EHTTPMethodGET, url, true)
{
}

void ValidateConnectivityRequest::OnResponse(bool bSuccessful)
{
	if (bSuccessful == false || pJSONData == NULL)
	{
		FNShared::Print("FuzzNet has been disabled!\n");
		MSGlobals::CentralEnabled = false;
		return;
	}

	const JSONValue& value = (*pJSONData);
	if (!value["data"].GetBool())
	{
		FNShared::Print("FuzzNet connection failed!\n");
		MSGlobals::CentralEnabled = false;
		return;
	}

	FNShared::Print("FuzzNet connected!\n");
}