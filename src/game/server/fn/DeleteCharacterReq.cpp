//
// Delete FN character
//

#include "rapidjson/document.h"
#include "DeleteCharacterReq.h"
#include "FNSharedDefs.h"
#include "msdllheaders.h"
#include "player.h"
#include "util.h"

DeleteCharacterRequest::DeleteCharacterRequest(ID64 steamID, ID64 slot, const char* url, int prevStatus) :
	HTTPRequest(HTTPMethod::DEL, url, NULL, NULL, steamID, slot),
	m_iPrevStatus(prevStatus)
{
}

void DeleteCharacterRequest::OnResponse(int iRespCode)
{
	CBasePlayer* pPlayer = UTIL_PlayerBySteamID(m_iSteamID64);
	if (pPlayer == NULL)
		return;

	charinfo_t& CharInfo = pPlayer->m_CharInfo[m_iSlot];

	if (iRespCode >= 400)
	{
		FNShared::Print("Unable to delete character for SteamID %llu!", m_iSteamID64);

		// Restoring the *previous* status rather than stamping CDS_NOTFOUND
		// matters: the delete failed, so the character still exists server-side.
		CharInfo.Status = static_cast<decltype(CharInfo.Status)>(m_iPrevStatus);
		CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
		return;
	}

	CharInfo.Status = CDS_NOTFOUND;
	CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
}