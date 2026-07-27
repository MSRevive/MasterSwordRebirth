//
// Create a new FN character
//

#include "rapidjson/document.h"
#include "CreateCharacterReq.h"
#include "FNSharedDefs.h"
#include "msdllheaders.h"
#include "player.h"
#include "util.h"

CreateCharacterRequest::CreateCharacterRequest(ID64 steamID, ID64 slot, const char* url, const char* body, size_t bodySize) :
	HTTPRequest(HTTPMethod::POST, url, body, bodySize, steamID, slot)
{
}

void CreateCharacterRequest::OnResponse(int iRespCode)
{
	CBasePlayer* pPlayer = UTIL_PlayerBySteamID(m_iSteamID64);
	if (pPlayer == NULL)
		return;

	charinfo_t& CharInfo = pPlayer->m_CharInfo[m_iSlot];

	auto MarkNotFound = [&CharInfo, this]()
	{
		CharInfo.Index = m_iSlot;
		CharInfo.Location = LOC_CENTRAL;
		CharInfo.Status = CDS_NOTFOUND;
		CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
	};

	if (iRespCode != 200)
	{
		FNShared::Print("Unable to create character for SteamID %llu!", m_iSteamID64);
		MarkNotFound();
		return;
	}

	JSONDocument doc = ParseJSON(m_sResponseBody.c_str());

	if (!doc.HasMember("data") || !doc["data"].IsObject())
	{
		FNShared::Print("Malformed create response for SteamID %llu!", m_iSteamID64);
		MarkNotFound();
		return;
	}

	const rapidjson::Value& payload = doc["data"];

	if (!payload.HasMember("flags") || !payload["flags"].IsInt() ||
	    !payload.HasMember("id")    || !payload["id"].IsString())
	{
		FNShared::Print("Incomplete create response for SteamID %llu!", m_iSteamID64);
		MarkNotFound();
		return;
	}

	const int flags = payload["flags"].GetInt();

	CharInfo.AssignChar(m_iSlot, LOC_CENTRAL, (char*)m_sRequestBody, m_iRequestBodySize, pPlayer);

	strncpy(CharInfo.Guid, payload["id"].GetString(), MSSTRING_SIZE - 1);
	CharInfo.Guid[MSSTRING_SIZE - 1] = '\0';

	CharInfo.Flags = flags;
	CharInfo.Status = CDS_LOADED;
	CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
}