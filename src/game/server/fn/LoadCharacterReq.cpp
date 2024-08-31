//
// Load FN character
//

#include "rapidjson/document_safe.h"
#include "base64/base64.h"
#include "LoadCharacterReq.h"
#include "FNShareddefs.h"
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
		return;

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
		pPlayer->KickPlayer("You have been banned from FN!");
		return;
	}

	requestBodySize = doc["data"]["size"].GetInt();
	requestBody = new uint8[requestBodySize];
	memcpy(requestBody, (char*)base64_decode(doc["data"]["data"].GetString()).c_str(), requestBodySize);

	CharInfo.AssignChar(slot, LOC_CENTRAL, (char*)requestBody, requestBodySize, pPlayer);
	strncpy(CharInfo.Guid, doc["data"]["id"].GetString(), MSSTRING_SIZE);
	CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
}