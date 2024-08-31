//
// Update FN character
//

#ifndef HTTP_UPDATE_CHARACTER_REQUEST_H
#define HTTP_UPDATE_CHARACTER_REQUEST_H

#include "SteamHTTPReq.h"

class UpdateCharacterRequest : public HTTPRequest
{
public:
	UpdateCharacterRequest(ID64 steamID, ID64 slot, const char* url, uint8* body, size_t bodySize);
	void OnResponse(bool bSuccessful);

private:
	UpdateCharacterRequest(const UpdateCharacterRequest&);
};

#endif // HTTP_UPDATE_CHARACTER_REQUEST_H