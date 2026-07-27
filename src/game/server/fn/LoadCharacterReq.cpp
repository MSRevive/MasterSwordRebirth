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

// Sanity cap on a single character blob. Adjust to whatever your largest
// legitimate save actually is.
static constexpr int kMaxCharacterBytes = 50 * 1024; // set max size to 50kb, there's no way a character file should be any larger

LoadCharacterRequest::LoadCharacterRequest(ID64 steamID, ID64 slot, const char* url) :
	HTTPRequest(HTTPMethod::GET, url, NULL, NULL, steamID, slot)
{
}

void LoadCharacterRequest::OnResponse(int iRespCode)
{
	CBasePlayer* pPlayer = UTIL_PlayerBySteamID(m_iSteamID64);
	if (pPlayer == nullptr)
	{
		FNShared::Print("FATALITY: Unable to get player with SteamID64 %llu", m_iSteamID64);
		return;
	}

	charinfo_t& CharInfo = pPlayer->m_CharInfo[m_iSlot];

	auto MarkNotFound = [&CharInfo, this]()
	{
		CharInfo.Index = m_iSlot;
		CharInfo.Location = LOC_CENTRAL;
		CharInfo.Status = CDS_NOTFOUND;
		CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
	};

	if ((iRespCode != 200) && (iRespCode != 204))
	{
		FNShared::Print("Unable to load character %i for SteamID %llu!", (m_iSlot + 1), m_iSteamID64);
		MarkNotFound();
		return;
	}

	if (iRespCode == 204)
	{
		MarkNotFound();
		return;
	}

	JSONDocument doc = ParseJSON(m_sResponseBody.c_str());
	if (!doc.HasMember("data") || !doc["data"].IsObject())
	{
		FNShared::Print("Malformed character payload for SteamID %llu!", m_iSteamID64);
		MarkNotFound();
		return;
	}

	const rapidjson::Value& payload = doc["data"];

	if (!payload.HasMember("flags")  || !payload["flags"].IsInt()   ||
	    !payload.HasMember("size")   || !payload["size"].IsInt()    ||
	    !payload.HasMember("data")   || !payload["data"].IsString() ||
	    !payload.HasMember("id")     || !payload["id"].IsString())
	{
		FNShared::Print("Incomplete character payload for SteamID %llu!", m_iSteamID64);
		MarkNotFound();
		return;
	}

	const int flags = payload["flags"].GetInt();

	if (FNShared::IsBanned(flags) == true)
	{
		FNShared::Print("Account banned from FN! %llu!", m_iSteamID64);
		pPlayer->KickPlayer("You have been banned from FN!");
		return;
	}

	const int declaredSize = payload["size"].GetInt();
	if ((declaredSize <= 0) || (declaredSize > kMaxCharacterBytes))
	{
		FNShared::Print("Bad character size (%d) for SteamID %llu!", declaredSize, m_iSteamID64);
		MarkNotFound();
		return;
	}

	const std::string decoded = base64_decode(payload["data"].GetString());
	if (decoded.size() < static_cast<size_t>(declaredSize))
	{
		FNShared::Print("Truncated character data for SteamID %llu (%u < %d)!",
			m_iSteamID64, static_cast<unsigned int>(decoded.size()), declaredSize);
		MarkNotFound();
		return;
	}

	delete[] m_sRequestBody; // Defensive: GET requests start with nullptr.
	m_iRequestBodySize = static_cast<size_t>(declaredSize);
	m_sRequestBody = new char[m_iRequestBodySize];
	memcpy(m_sRequestBody, decoded.data(), m_iRequestBodySize);

	CharInfo.AssignChar(m_iSlot, LOC_CENTRAL, m_sRequestBody, m_iRequestBodySize, pPlayer);

	strncpy(CharInfo.Guid, payload["id"].GetString(), MSSTRING_SIZE - 1);
	CharInfo.Guid[MSSTRING_SIZE - 1] = '\0';

	CharInfo.Flags = flags;
	CharInfo.Status = CDS_LOADED;
	CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
}