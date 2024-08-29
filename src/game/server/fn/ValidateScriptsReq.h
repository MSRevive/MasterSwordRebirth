//
// Validate sc.dll
//

#ifndef HTTP_VALIDATE_SCRIPTS_REQUEST_H
#define HTTP_VALIDATE_SCRIPTS_REQUEST_H

#include "SteamHttpRequest.h"

class ValidateScriptsRequest : public SteamHttpRequest
{
public:
	ValidateScriptsRequest(const char* url);
	void OnResponse(bool bSuccessful);
	const char* GetName() { return "ValidateScriptsRequest"; }

private:
	ValidateScriptsRequest(const ValidateScriptsRequest&);
};

#endif // HTTP_VALIDATE_SCRIPTS_REQUEST_H