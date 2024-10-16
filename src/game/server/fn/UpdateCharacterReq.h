//
// Update FN character
//

#ifndef HTTP_UPDATE_CHARACTER_REQUEST_H
#define HTTP_UPDATE_CHARACTER_REQUEST_H

#include "HTTPRequest.h"

class UpdateCharacterRequest : public HTTPRequest
{
public:
	UpdateCharacterRequest(ID64 steamID, ID64 slot, const char* url, uint8* body, size_t bodySize);
	void OnResponse(bool bSuccessful, int iRespCode);
	const char* GetName() { return "UpdateCharacterRequest"; }

private:
	UpdateCharacterRequest(const UpdateCharacterRequest&);
};

#endif // HTTP_UPDATE_CHARACTER_REQUEST_H