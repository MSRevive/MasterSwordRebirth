//
// Delete FN character
//

#ifndef HTTP_DELETE_CHARACTER_REQUEST_H
#define HTTP_DELETE_CHARACTER_REQUEST_H

#include "SteamHttpRequest.h"

class DeleteCharacterRequest : public SteamHttpRequest
{
public:
	DeleteCharacterRequest(ID64 steamID, ID64 slot, const char* url);
	void OnResponse(bool bSuccessful);

private:
	DeleteCharacterRequest(const DeleteCharacterRequest&);
};

#endif // HTTP_DELETE_CHARACTER_REQUEST_H