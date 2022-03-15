//
// FuzzNet Data Handler - Load, Save, Update, Create characters
// Generally we want to load all three characters when player X joins the server
// Allow the player to easily switch between and use these characters.
//

#include "MSDLLHeaders.h"
#include "Player.h"
#include "Global.h"
#include "FnDataHandler.h"
#include "HTTPRequestHandler.h"
#include "rapidjson/document_safe.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "base64/base64.h"

using namespace rapidjson;

static bool IsSlotValid(int slot) { return ((slot >= 0) && (slot < MAX_CHARSLOTS)); }

static const char* GetFnUrl(char* fmt, ...)
{
	static char requestUrl[REQUEST_URL_SIZE], string[REQUEST_URL_SIZE];

	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	_snprintf(requestUrl, REQUEST_URL_SIZE, "%s/", CVAR_GET_STRING("ms_central_addr"));
	strncat(requestUrl, string, REQUEST_URL_SIZE - strlen(requestUrl) - 1);

	return requestUrl;
}

// Load single char details.
static bool LoadCharacter(CBasePlayer* pPlayer, const JSONValue& val)
{
	const int slot = val["slot"].GetInt();
	const int size = val["size"].GetInt();

	if (!IsSlotValid(slot)) return false; // Invalid slot!

	charinfo_t& CharInfo = pPlayer->m_CharInfo[slot];
	strncpy(CharInfo.Guid, val["id"].GetString(), MSSTRING_SIZE);
	CharInfo.AssignChar(slot, LOC_CENTRAL, (char*)base64_decode(val["data"].GetString()).c_str(), size, pPlayer);

	return true;
}

// Load all characters!
void FnDataHandler::LoadCharacter(CBasePlayer* pPlayer)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL)) return;

	for (int i = 0; i < MAX_CHARSLOTS; i++)
	{
		pPlayer->m_CharInfo[i].m_CachedStatus = CDS_UNLOADED;
		pPlayer->m_CharInfo[i].Status = CDS_NOTFOUND;
	}

	const JSONDocument* pDoc = HTTPRequestHandler::GetRequestAsJson(GetFnUrl("api/v1/character/%llu", pPlayer->steamID64));
	if (pDoc == NULL) return;

	const JSONDocument& doc = *pDoc;
	for (const JSONValue& val : doc["data"].GetArray()) // Iterate through the characters returned, if any.	
		LoadCharacter(pPlayer, val);

	delete pDoc;
}

// Load a specific character!
void FnDataHandler::LoadCharacter(CBasePlayer* pPlayer, int slot)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL) || !IsSlotValid(slot)) return;

	pPlayer->m_CharInfo[slot].m_CachedStatus = CDS_UNLOADED;
	pPlayer->m_CharInfo[slot].Status = CDS_NOTFOUND;

	const JSONDocument* pDoc = HTTPRequestHandler::GetRequestAsJson(GetFnUrl("api/v1/character/%llu/%i", pPlayer->steamID64, slot));
	if (pDoc == NULL) return;

	const JSONDocument& doc = *pDoc;
	for (const JSONValue& val : doc["data"].GetArray()) // Iterate through the characters returned, if any.
	{
		if (LoadCharacter(pPlayer, val))
			break;
	}

	delete pDoc;
}

// Create or Update character.
void FnDataHandler::CreateOrUpdateCharacter(CBasePlayer* pPlayer, int slot, const char* data, int size, bool bIsUpdate)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL) || (data == NULL) || (size <= 0) || !IsSlotValid(slot)) return; // Quick validation - steamId is vital.

	if (bIsUpdate && (pPlayer->m_CharacterState == CHARSTATE_UNLOADED)) return; // You cannot update your char (save) if there is no char loaded.

	StringBuffer s;
	Writer<StringBuffer> writer(s);

	writer.StartObject();

	writer.Key("steamid");
	writer.String(pPlayer->steamID64String);

	writer.Key("slot");
	writer.Int(slot);

	writer.Key("size");
	writer.Int(size);

	writer.Key("data");
	writer.String(base64_encode((const unsigned char*)data, size).c_str());

	writer.EndObject();

	if (bIsUpdate)
	{
		HTTPRequestHandler::PutRequest(GetFnUrl("api/v1/character/%s", pPlayer->m_CharInfo[slot].Guid), s.GetString());
		return;
	}

	JSONDocument* pResponse = HTTPRequestHandler::PostRequestAsJson(GetFnUrl("api/v1/character/"), s.GetString());
	if (pResponse)
	{
		charinfo_t& CharInfo = pPlayer->m_CharInfo[slot];
		strncpy(CharInfo.Guid, (*pResponse)["data"]["id"].GetString(), MSSTRING_SIZE);
		CharInfo.AssignChar(slot, LOC_CENTRAL, data, size, pPlayer);
	}

	delete pResponse;
}

void FnDataHandler::DeleteCharacter(CBasePlayer* pPlayer, int slot)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL) || !IsSlotValid(slot)) return;

	HTTPRequestHandler::DeleteRequest(GetFnUrl("api/v1/character/%s", pPlayer->m_CharInfo[slot].Guid));

	pPlayer->m_CharInfo[slot].Status = CDS_NOTFOUND;
	pPlayer->m_CharInfo[slot].m_CachedStatus = CDS_UNLOADED;
}

// Handle thinking, if necessary?
// TODO: Add proper multi threading - check queue system here later?
void FnDataHandler::Think(void)
{
}

bool FnDataHandler::IsEnabled(void)
{
	return (MSGlobals::CentralEnabled && !MSGlobals::IsLanGame && MSGlobals::ServerSideChar);
}

// We store 64-bit SteamIDs, convert from old 32-bit string based ID to 64-bit numeric.
// See https://developer.valvesoftware.com/wiki/SteamID for help.
// EX input: STEAM_0:0:7019991 = STEAM_X:Y:Z
// Formula: 
// V = 76561197960265728
// ID = Z*2+V+Y
unsigned long long FnDataHandler::GetSteamID64(const char* id)
{
	if (!id || !id[0]) return 0ULL;

	char pchSteamID[32];
	strncpy(pchSteamID, id, sizeof(pchSteamID));

	const char* pStart = pchSteamID;
	size_t size = strlen(pchSteamID);

	for (size_t i = 0; i < size; i++)
	{
		if (pchSteamID[i] == ':') // Skip the first part.
		{
			pStart += (i + 1);
			strncpy(pchSteamID, pStart, sizeof(pchSteamID)); // Removes STEAM_X:, WE ONLY CARE ABOUT Y:Z part!
			break;
		}
	}

	char pchArg1[16], pchArg2[16];
	pStart = pchSteamID;
	size = strlen(pchSteamID);

	for (size_t i = 0; i < size; i++)
	{
		if (pchSteamID[i] == ':') // split
		{
			strncpy(pchArg1, pStart, sizeof(pchArg1));
			pchArg1[i] = '\0';

			pStart += (i + 1);
			strncpy(pchArg2, pStart, sizeof(pchArg2));
			break;
		}
	}

	return (atoi(pchArg1) + 2ULL * atoi(pchArg2) + 76561197960265728ULL);
}