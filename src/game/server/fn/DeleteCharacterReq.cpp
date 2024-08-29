//
// Delete FN character
//

#include "rapidjson/document_safe.h"
#include "DeleteCharacterRequest.h"
#include "FNShareddefs.h"
#include "msdllheaders.h"
#include "player.h"
#include "util.h"

DeleteCharacterRequest::DeleteCharacterRequest(ID64 steamID, ID64 slot, const char* url) :
	SteamHttpRequest(EHTTPMethod::k_EHTTPMethodDELETE, url, NULL, NULL, steamID, slot)
{
}

void DeleteCharacterRequest::OnResponse(bool bSuccessful)
{
	if ((pJSONData == NULL) || (bSuccessful == false))
		FNShared::Print("Unable to delete character for SteamID %llu!\n", param1);

	CBasePlayer* pPlayer = UTIL_PlayerBySteamID(param1);
	if (pPlayer == NULL)
		return;

	charinfo_t& CharInfo = pPlayer->m_CharInfo[param2];
	CharInfo.Status = CDS_NOTFOUND;
	CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
}