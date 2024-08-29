//
// Update FN character
//

#include "rapidjson/document_safe.h"
#include "UpdateCharacterRequest.h"
#include "FNShareddefs.h"
#include "msdllheaders.h"
#include "player.h"
#include "util.h"

UpdateCharacterRequest::UpdateCharacterRequest(ID64 steamID, ID64 slot, const char* url, uint8* body, size_t bodySize) :
	SteamHttpRequest(EHTTPMethod::k_EHTTPMethodPUT, url, body, bodySize, steamID, slot)
{
}

void UpdateCharacterRequest::OnResponse(bool bSuccessful)
{
}