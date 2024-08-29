//
// Validate if FN connectivity could be established!
//

#ifndef HTTP_VALIDATE_CONNECTIVITY_REQUEST_H
#define HTTP_VALIDATE_CONNECTIVITY_REQUEST_H

#include "SteamHttpRequest.h"

class ValidateConnectivityRequest : public SteamHttpRequest
{
public:
	ValidateConnectivityRequest(const char* url);
	void OnResponse(bool bSuccessful);
	const char* GetName() { return "ValidateConnectivityRequest"; }

private:
	ValidateConnectivityRequest(const ValidateConnectivityRequest&);
};

#endif // HTTP_VALIDATE_CONNECTIVITY_REQUEST_H