//
// Delete FN character
//

#ifndef HTTP_DELETE_CHARACTER_REQUEST_H
#define HTTP_DELETE_CHARACTER_REQUEST_H

#include "HTTPRequest.h"

class DeleteCharacterRequest : public HTTPRequest
{
public:
	// `prevStatus` is the slot's status before FNShared::DeleteCharacter pinned
	// it to CDS_LOADING. Stored as int so this header doesn't have to pull in
	// player.h for the enum type; cast back at the point of use.
	DeleteCharacterRequest(ID64 steamID, ID64 slot, const char* url, int prevStatus);
	void OnResponse(int iRespCode);
	const char* GetName() { return "DeleteCharacterRequest"; }

private:
	int m_iPrevStatus;

	DeleteCharacterRequest(const DeleteCharacterRequest&);
	DeleteCharacterRequest& operator=(const DeleteCharacterRequest&);
};

#endif // HTTP_DELETE_CHARACTER_REQUEST_H