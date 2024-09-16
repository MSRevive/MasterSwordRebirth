//
// Update FN character
//

#include "rapidjson/document_safe.h"
#include "UpdateCharacterReq.h"
#include "FNShareddefs.h"
#include "msdllheaders.h"
#include "player.h"
#include "util.h"

UpdateCharacterRequest::UpdateCharacterRequest(ID64 steamID, ID64 slot, const char* url, uint8* body, size_t bodySize) :
	HTTPRequest(EHTTPMethod::k_EHTTPMethodPUT, url, false, body, bodySize, steamID, slot)
{
}

void UpdateCharacterRequest::OnResponse(bool bSuccessful)
{
}