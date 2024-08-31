//
// Load FN character
//

#ifndef HTTP_LOAD_CHARACTER_REQUEST_H
#define HTTP_LOAD_CHARACTER_REQUEST_H

#include "SteamHTTPReq.h"

class LoadCharacterRequest : public HTTPRequest
{
public:
	LoadCharacterRequest(ID64 steamID, ID64 slot, const char* url);
	void OnResponse(bool bSuccessful);

private:
	LoadCharacterRequest(const LoadCharacterRequest&);
};

#endif // HTTP_LOAD_CHARACTER_REQUEST_H