//
// Delete FN character
//

#include "rapidjson/document.h"
#include "DeleteCharacterReq.h"
#include "FNSharedDefs.h"
#include "msdllheaders.h"
#include "player.h"
#include "util.h"

DeleteCharacterRequest::DeleteCharacterRequest(ID64 steamID, ID64 slot, const char* url) :
	HTTPRequest(EHTTPMethod::k_EHTTPMethodDELETE, url, NULL, NULL, steamID, slot)
{
}

void DeleteCharacterRequest::OnResponse(bool bSuccessful, int iRespCode)
{
	if ((pJSONData == NULL) || (bSuccessful == false))
		FNShared::Print("Unable to delete character for SteamID %llu!\n", steamID64);

	CBasePlayer* pPlayer = UTIL_PlayerBySteamID(steamID64);
	if (pPlayer == NULL)
		return;

	charinfo_t& CharInfo = pPlayer->m_CharInfo[slot];
	CharInfo.Status = CDS_NOTFOUND;
	CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
}