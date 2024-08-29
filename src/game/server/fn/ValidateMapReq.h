//
// Verify if a map is eligible for FN play
//

#ifndef HTTP_VALIDATE_MAP_REQUEST_H
#define HTTP_VALIDATE_MAP_REQUEST_H

#include "SteamHttpRequest.h"

class ValidateMapRequest : public SteamHttpRequest
{
public:
	ValidateMapRequest(const char* url);
	void OnResponse(bool bSuccessful);
	const char* GetName() { return "ValidateMapRequest"; }

private:
	ValidateMapRequest(const ValidateMapRequest&);
};

#endif // HTTP_VALIDATE_MAP_REQUEST_H