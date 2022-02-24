//
// FuzzNet Data Handler - Load, Save, Update, Create characters
// Generally we want to load all three characters when player X joins the server
// Allow the player to easily switch between and use these characters.
//

#include "MSDLLHeaders.h"
#include "Player.h"
#include "FnDataHandler.h"
#include "HTTPRequestHandler.h"
#include "rapidjson/document_safe.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#define FN_TEST_URL "http://fn.msrebirth.net:27520" // TODO: Move this to a CVAR!

// Load all characters!
void FnDataHandler::LoadCharacter(CBasePlayer* pPlayer)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL)) return;

	char pchRequestUrl[REQUEST_URL_SIZE];
	_snprintf(pchRequestUrl, sizeof(pchRequestUrl), "%s/api/v1/character/%llu", FN_TEST_URL, pPlayer->steamID64);

	const JSONDocument* pDoc = HTTPRequestHandler::GetRequestAsJson(pchRequestUrl);
	if (pDoc == NULL) return;

	char printBuffer[180];
	_snprintf(printBuffer, sizeof(printBuffer), "id - %llu\n", pPlayer->steamID64);
	UTIL_ClientPrintAll(2, printBuffer);

	const JSONDocument& doc = *pDoc;
	for (const JSONValue& val : doc["data"].GetArray())
	{
		_snprintf(printBuffer, 180, "IDs: %s\n", val["steamid"].GetString());
		UTIL_ClientPrintAll(2, printBuffer);
	}

	HTTPRequestHandler::PrintJSONDocument(pDoc);
	delete pDoc;
}

// Save character profile X for SteamID y.
void FnDataHandler::SaveCharacter(CBasePlayer* pPlayer)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL) || (pPlayer->m_CharacterState == CHARSTATE_UNLOADED)) return;

	// TODO - Use Rapid JSON document writer -> Get the document as string then use this string as the body.
	const char* body =
		"{ \"steamid\": \"76561198092541770\", \"slot\":3, \"name\": \"Cancifer\", \"gender\":1, \"race\":\"human\", \"flags\":\"{}\", \"quickslots\":\"{}\", \"quests\":\"{}\", \"skills\":\"{}\", \"pets\":\"{}\", \"health\":500, \"mana\": 500, \"equipped\":\"{}\", \"spells\":\"{}\", \"spellbook\":\"{}\", \"bags\":\"{}\", \"sheaths\":\"{}\" }";

	HTTPRequestHandler::PostRequest("api/v1/character/", body);
}

// Create a new character, if possible.
void FnDataHandler::CreateCharacter(CBasePlayer* pPlayer, unsigned char profile, const char* race, unsigned char gender, unsigned char* type)
{
	if ((pPlayer == NULL) || (pPlayer->steamID64 == 0ULL)) return;

	// Create default char + send back to plr!
	// TODO
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