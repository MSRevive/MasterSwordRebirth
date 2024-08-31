//
// Delete FN character
//

#ifndef HTTP_DELETE_CHARACTER_REQUEST_H
#define HTTP_DELETE_CHARACTER_REQUEST_H

#include "SteamHTTPReq.h"

class DeleteCharacterRequest : public HTTPRequest
{
public:
	DeleteCharacterRequest(ID64 steamID, ID64 slot, const char* url);
	void OnResponse(bool bSuccessful);

private:
	DeleteCharacterRequest(const DeleteCharacterRequest&);
};

#endif // HTTP_DELETE_CHARACTER_REQUEST_H