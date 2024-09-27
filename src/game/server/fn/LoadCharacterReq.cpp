//
// Load FN character
//

#include "rapidjson/document.h"
#include "base64/base64.h"
#include "LoadCharacterReq.h"
#include "FNSharedDefs.h"
#include "msdllheaders.h"
#include "player.h"
#include "util.h"

LoadCharacterRequest::LoadCharacterRequest(ID64 steamID, ID64 slot, const char* url) :
	HTTPRequest(EHTTPMethod::k_EHTTPMethodGET, url, NULL, NULL, steamID, slot)
{
}

void LoadCharacterRequest::OnResponse(bool bSuccessful)
{
	if ((pJSONData == NULL) || (bSuccessful == false))
		FNShared::Print("Unable to load character %i for SteamID %llu!\n", (slot + 1), steamID64);

	CBasePlayer* pPlayer = UTIL_PlayerBySteamID(steamID64);
	if (pPlayer == NULL)
	{
		FNShared::Print("Critical failure, unable to get player with steamID64 %llu\n", steamID64);
		return;
	}
	
	charinfo_t& CharInfo = pPlayer->m_CharInfo[slot];

	if ((pJSONData == NULL) || (bSuccessful == false))
	{
		CharInfo.Index = slot;
		CharInfo.Location = LOC_CENTRAL;
		CharInfo.Status = CDS_NOTFOUND;
		CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
		return;
	}

	const JSONDocument& doc = (*pJSONData);

	const int flags = FNShared::GetPlayerFlags(doc);
	if ((flags & FN_FLAG_BANNED) != 0)
	{
		FNShared::Print("Account banned from FN! %llu!\n", steamID64);
		pPlayer->KickPlayer("You have been banned from FN!");
		return;
	}

	requestBodySize = doc["data"]["size"].GetInt();
	requestBody = new uint8[requestBodySize];
	memcpy(requestBody, (char*)base64_decode(doc["data"]["data"].GetString()).c_str(), requestBodySize);

	CharInfo.AssignChar(slot, LOC_CENTRAL, (char*)requestBody, requestBodySize, pPlayer);
	strncpy(CharInfo.Guid, doc["data"]["id"].GetString(), MSSTRING_SIZE);
	CharInfo.Status = CDS_LOADED;
	CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
}