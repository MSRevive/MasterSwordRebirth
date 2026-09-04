//
// FN Shared Definitions
//

#include "rapidjson/document.h"
#include "FNSharedDefs.h"
#include "msdllheaders.h"
#include "player.h"
#include "global.h"
#include "crc/crchash.h"
#include "RequestManager.h"
#include "mslogger.h"

// Requests
#include "ValidateConReq.h"
#include "ValidateScriptsReq.h"
#include "ValidateMapReq.h"
#include "CreateCharacterReq.h"
#include "UpdateCharacterReq.h"
#include "LoadCharacterReq.h"
#include "DeleteCharacterReq.h"

constexpr unsigned int STRING_BUFFER = 1024;

// AsyncSendRequest does not make use the connection pooling.
// Frequent requests should make use the RequestManager to take advantage of
// libcurl's async features and connection pooling.

void FNShared::Print(const char* fmt, ...)
{
	static char string[STRING_BUFFER];

	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	MS_INFO("[FuzzNet] %s", string);
}

bool FNShared::IsSlotValid(int slot)
{
	return ((slot >= 0) && (slot < MAX_CHARSLOTS));
}

bool FNShared::IsEnabled(void)
{
	return (MSGlobals::CentralEnabled && !MSGlobals::IsLanGame && MSGlobals::ServerSideChar);
}

static bool SendBlockingRequest(HTTPRequest* req)
{
	CURLSH* share = g_FNRequestManager.GetShareHandle();
	if (share)
		req->SetShareHandle(share);

	return req->AsyncSendRequest();
}

static bool QueueSlotRequest(HTTPRequest* pReq, charinfo_t& CharInfo, decltype(charinfo_t::Status) prevStatus)
{
	if (g_FNRequestManager.QueueRequest(pReq))
		return true;

	CharInfo.Status = prevStatus;
	CharInfo.m_CachedStatus = CDS_UNLOADED; // force an update!
	return false;
}

// Send validation requests to the FN backend.
bool FNShared::Validate(void)
{
	if (IsEnabled() == false)
		return false;

	if (ValidateMap() && ValidateSC())
		return true;

	return false;
}

bool FNShared::ValidateFN(void)
{
	if (IsEnabled() == false)
		return true;

	std::unique_ptr<HTTPRequest> pReq(new ValidateConRequest("/api/v2/internal/ping"));
	const auto req = pReq.get();
	if (SendBlockingRequest(req))
	{
		JSONDocument doc = HTTPRequest::ParseJSON(req->m_sResponseBody.c_str());
		if (!doc.HasMember("data") || !doc["data"].IsBool())
			return false;

		return doc["data"].GetBool();
	}

	return false;
}

bool FNShared::ValidateMap(void)
{
	if (IsEnabled() == false)
		return true;

	char mapFile[MAX_PATH];
	_snprintf(mapFile, sizeof(mapFile), "%s/maps/%s.bsp", MSGlobals::AbsGamePath.c_str(), MSGlobals::MapName.c_str());
	unsigned int mapFileHash = GetFileCheckSum(mapFile);

	std::unique_ptr<HTTPRequest> pReq(new ValidateMapRequest(UTIL_VarArgs("/api/v2/internal/map/%s/%u", MSGlobals::MapName.c_str(), mapFileHash)));
	const auto req = pReq.get();
	if (SendBlockingRequest(req))
	{
		JSONDocument doc = HTTPRequest::ParseJSON(req->m_sResponseBody.c_str());
		if (!doc.HasMember("data") || !doc["data"].IsBool())
			return false;

		return doc["data"].GetBool();
	}

	return false;
}

bool FNShared::ValidateSC(void)
{
	if (IsEnabled() == false)
		return true;

	char scFile[MAX_PATH];
	_snprintf(scFile, sizeof(scFile), "%s/scripts.pak", MSGlobals::AbsGamePath.c_str());
	unsigned int scFileHash = GetFileCheckSum(scFile);

	std::unique_ptr<HTTPRequest> pReq(new ValidateScriptsRequest(UTIL_VarArgs("/api/v2/internal/sc/%u", scFileHash)));
	const auto req = pReq.get();
	if (SendBlockingRequest(req))
	{
		JSONDocument doc = HTTPRequest::ParseJSON(req->m_sResponseBody.c_str());
		if (!doc.HasMember("data") || !doc["data"].IsBool())
			return false;

		return doc["data"].GetBool();
	}

	return false;
}

// Check if player has BANNED flag.
bool FNShared::IsBanned(int flags)
{
	return (flags & FN_FLAG_BANNED) == FN_FLAG_BANNED;
}

// Check if player has DONOR flag.
bool FNShared::IsDonor(int flags)
{
	return (flags & FN_FLAG_DONOR) == FN_FLAG_DONOR;
}

// Check if player has ADMIN flag.
bool FNShared::IsAdmin(int flags)
{
	return (flags & FN_FLAG_ADMIN) == FN_FLAG_ADMIN;
}

// Load all characters!
void FNShared::LoadCharacter(CBasePlayer* pPlayer)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL))
		return;

	for (unsigned int i = 0; i < MAX_CHARSLOTS; i++)
	{
		charinfo_t& CharInfo = pPlayer->m_CharInfo[i];

		if (CharInfo.Status == CDS_LOADING)
			continue;

		const auto prevStatus = CharInfo.Status;

		CharInfo.m_CachedStatus = CDS_UNLOADED;
		CharInfo.Status = CDS_LOADING;

		if (!QueueSlotRequest(new LoadCharacterRequest(pPlayer->steamID64, i,
				UTIL_VarArgs("/api/v2/internal/character/%llu/%i", pPlayer->steamID64, i)),
				CharInfo, prevStatus))
		{
			FNShared::Print("Failed to queue character load for %llu slot %u!", pPlayer->steamID64, i);

			// The only failure mode today is a manager that isn't loaded, so the
			// remaining slots would fail identically. Remove this break if
			// QueueRequest ever grows per-request failure modes.
			break;
		}
	}
}

// Load a specific character!
void FNShared::LoadCharacter(CBasePlayer* pPlayer, int slot)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL) || !IsSlotValid(slot))
		return;

	charinfo_t& CharInfo = pPlayer->m_CharInfo[slot];

	if (CharInfo.Status == CDS_LOADING)
		return;

	const auto prevStatus = CharInfo.Status;

	CharInfo.m_CachedStatus = CDS_UNLOADED;
	CharInfo.Status = CDS_LOADING;

	if (!QueueSlotRequest(new LoadCharacterRequest(pPlayer->steamID64, slot,
			UTIL_VarArgs("/api/v2/internal/character/%llu/%i", pPlayer->steamID64, slot)),
			CharInfo, prevStatus))
	{
		FNShared::Print("Failed to queue character load for %llu slot %i!", pPlayer->steamID64, slot);
	}
}

// Create or Update FN character!
void FNShared::CreateOrUpdateCharacter(CBasePlayer* pPlayer, int slot, const char* data, size_t size, bool bIsUpdate)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL) || (data == NULL) || (size == 0) || !IsSlotValid(slot))
		return; // Quick validation - steamId is vital.

	if (bIsUpdate && (pPlayer->m_CharacterState == CHARSTATE_UNLOADED))
		return; // You cannot update your char (save) if there is no char loaded.

	charinfo_t& CharInfo = pPlayer->m_CharInfo[slot];

	if (!bIsUpdate && (CharInfo.Status == CDS_LOADING))
		return; // Busy, wait for callback!

	char pchApiUrl[REQUEST_URL_SIZE];

	if (bIsUpdate)
	{
		_snprintf(pchApiUrl, REQUEST_URL_SIZE, "/api/v2/internal/character/%s", CharInfo.Guid);

		FNShared::Print("Send update request for %llu", pPlayer->steamID64);

		if (!g_FNRequestManager.QueueRequest(new UpdateCharacterRequest(
				pPlayer->steamID64, slot, pchApiUrl, data, size)))
		{
			FNShared::Print("Failed to queue save for %llu slot %i!", pPlayer->steamID64, slot);
		}
	}
	else
	{
		_snprintf(pchApiUrl, REQUEST_URL_SIZE, "/api/v2/internal/character/");

		const auto prevStatus = CharInfo.Status;

		CharInfo.m_CachedStatus = CDS_UNLOADED;
		CharInfo.Status = CDS_LOADING;

		if (!QueueSlotRequest(new CreateCharacterRequest(
				pPlayer->steamID64, slot, pchApiUrl, data, size), CharInfo, prevStatus))
		{
			FNShared::Print("Failed to queue character creation for %llu slot %i!", pPlayer->steamID64, slot);
		}
	}
}

void FNShared::DeleteCharacter(CBasePlayer* pPlayer, int slot)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL) || !IsSlotValid(slot))
		return;

	charinfo_t& CharInfo = pPlayer->m_CharInfo[slot];

	if (CharInfo.Status == CDS_LOADING)
		return;

	const auto prevStatus = CharInfo.Status;

	char pchApiUrl[REQUEST_URL_SIZE];
	_snprintf(pchApiUrl, REQUEST_URL_SIZE, "/api/v2/internal/character/%s", CharInfo.Guid);

	CharInfo.m_CachedStatus = CDS_UNLOADED;
	CharInfo.Status = CDS_LOADING;

	if (!QueueSlotRequest(new DeleteCharacterRequest(
			pPlayer->steamID64, slot, pchApiUrl, static_cast<int>(prevStatus)),
			CharInfo, prevStatus))
	{
		FNShared::Print("Failed to queue character deletion for %llu slot %i!", pPlayer->steamID64, slot);
	}
}