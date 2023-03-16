#include "hud.h"
#include "cl_util.h"
#include "com_weapons.h"
#include "msdllheaders.h"
#include "clglobal.h"
#include "clenv.h"
#include "mscharacter.h"
#include "vgui_menudefsshared.h"
#include "player/player.h"
#include "hudmisc.h"
#include "hudscript.h"
#include "ms/vgui_hud.h"
#include "logger.h"
#include "steamhelper.h"
#include "richpresence.h"

void VGUI_Think();

msstring MSCLGlobals::AuthID;								   //My steamID
bool MSCLGlobals::OnMyOwnListenServer;						   //Did I join my own listen server?
bool MSCLGlobals::CreatingCharacter;						   //Am in the process of creating a new character?
bool MSCLGlobals::CharPanelActive;							   //Choosing a character to play with?
bool MSCLGlobals::CamThirdPerson;							   //Camera is in thirdperson?
bool MSCLGlobals::OtherPlayers = false;						   //Other players who can legally play this map are on the server
std::vector<msstring> MSCLGlobals::m_Strings;
//mslist<char *> MSCLGlobals::m_Strings;			   //All client-side globally allocated strings
mslist<CBaseEntity *> MSCLGlobals::m_ClEntites;				   //All client-side entities
mslist<cl_entity_t> MSCLGlobals::m_ClModels;				   //Extra models to be updated/animated client-side
cl_entity_t MSCLGlobals::CLViewEntities[MAX_PLAYER_HANDITEMS]; //All View entity models
mslist<mstexture_t> MSCLGlobals::Textures;					   //Custom textures, to be rendered in a unique way (reflective, blended, etc)
hudcharanims_t MSCLGlobals::DefaultHUDCharAnims;			   //Anims for the char select VGUI
hudsounds_t MSCLGlobals::DefaultHUDSounds;					   //HUD sounds
hudcoords_t MSCLGlobals::DefaultHUDCoords;					   //HUD placement coordinates
int MSCLGlobals::mapDarkenLevel = 0;						   //Map's custom bloom darkening MiB 31_DEC2010

// Local version of game .dll global variables ( time, etc. )
globalvars_t Globals;

void MSCLGlobals::AddEnt(CBaseEntity *pEntity)
{
	pEntity->pev = msnew(entvars_t);
	pEntity->pev->pContainingEntity = (edict_t *)pEntity;
	pEntity->pev->nextthink = -1;

	m_ClEntites.add(pEntity);
}
void MSCLGlobals::RemoveEnt(CBaseEntity *pEntity, bool fDelete)
{
	for (int e = 0; e < m_ClEntites.size(); e++)
		if (m_ClEntites[e] == pEntity)
		{
			m_ClEntites.erase(e);
			break;
		}
	if (fDelete)
	{
		::delete pEntity->pev;
		::delete pEntity;
	}
}

//Global one-time Initialization - called from CHud :: Init()
void MSCLGlobals::Initialize()
{
	startdbg;

	dbg("Begin");
	//Set up g_engfuncs re-directs
	gpGlobals = &Globals;
	SetupGlobalEngFuncRedirects();

	// Set up pointer
	// Fill in current time
	gpGlobals->time = gEngfuncs.GetClientTime();

	dbg("Call InitializePlayer");
	InitializePlayer();

	dbg("Call MSGlobalItemInit");
	MSGlobalItemInit();

	enddbg;
}

//Player initialization that happens every map
void MSCLGlobals::InitializePlayer()
{
	if (!player.pev)
		AddEnt(&player);

	player.CreateStats();
	player.m_CharacterState = CHARSTATE_UNLOADED;
	player.m_DisplayName = "Adventurer";
	player.PlayerHands = NULL;
	player.ClearConditions(MONSTER_OPENCONTAINER);
	player.Gear.clear();
	player.m_HP = player.m_MP = 0;

	player.m_fGameHUDInitialized = TRUE;
	//player.pbs.fMaxForwardPressTime = 0;
	player.m_SprintDelay = gpGlobals->time;
	player.m_Initialized = false;

	for (int i = 0; i < MAX_PLAYER_HANDITEMS; i++)
	{
		MSCLGlobals::CLViewEntities[i].index = MSGlobals::ClEntities[(i != 2) ? CLPERMENT_LEFTVIEW + i : CLPERMENT_LEFTVIEW];
		MSCLGlobals::CLViewEntities[i].curstate.number = MSGlobals::ClEntities[(i != 2) ? CLPERMENT_LEFTVIEW + i : CLPERMENT_LEFTVIEW];
	}
}
//Global think - sure to be called every frame
void MSCLGlobals::Think()
{
	startdbg;
	dbg("Begin");

	static float flLastThinkTime = 0;
	gpGlobals->time = gEngfuncs.GetClientTime();
	gpGlobals->frametime = gpGlobals->time - flLastThinkTime;

	//if( player.m_CharacterState == CHARSTATE_UNLOADED ) return;

	//Delete entities with |FL_KILLME
	//Count backward
	dbg("Delete entities with |FL_KILLME");
	for (int e = m_ClEntites.size() - 1; e >= 0; e--)
		if (FBitSet(m_ClEntites[e]->pev->flags, FL_KILLME))
			RemoveEnt(m_ClEntites[e]);

	//Call entity Think() functions
	dbg("Call entity Think() functions");
	for (int e = 0; e < m_ClEntites.size(); e++)
		if (flLastThinkTime <= m_ClEntites[e]->pev->nextthink && m_ClEntites[e]->pev->nextthink < gpGlobals->time)
			m_ClEntites[e]->Think();

	dbg("Update GUI");
	VGUI_Think();

	//Last task
	flLastThinkTime = gpGlobals->time;
	enddbg;
}
void MSCLGlobals::PrintAllEntites()
{
	Print("Global Items...\n");

	int items = 0;
	for (int e = 0; e < m_ClEntites.size(); e++)
		Print("Item %i: %s", items++, m_ClEntites[e]->DisplayName());
}
void MSCLGlobals::RemoveAllEntities()
{
	//Delete all entites
	if (player.m_CharacterState == CHARSTATE_LOADED)
	{
		player.Killed(NULL, 0);
		player.RemoveAllItems(false, true);
	}

	//There shouldn't be anything left here, but the player... which is skipped.
	//If any items get deleted below, conside it an error and make sure it gets
	//deleted beforehand
	//(presumably within RemoveAllItems() somewhere)
	int killed = 0;
	for (int e = 0; e < m_ClEntites.size(); e++)
	{
		CBaseEntity *pEntity = m_ClEntites[e];
		if (!pEntity) continue;

		//Skip player
		if (pEntity == &player) continue;

		//unset player hands (should happen in CBasePlayer::RemoveAllItems() first...)
		if (pEntity == (CBaseEntity *)player.PlayerHands)
			player.PlayerHands = NULL;

		Print("Cleanup Item %i: %s\n", killed++, pEntity->DisplayName());

		if (pEntity->pev)
			SetBits(pEntity->pev->flags, FL_KILLME);
	}
	//logfile << "Global Cleanup: " << killed << " unreferenced entites.\r\n";
	MSCLGlobals::Think();

	m_ClModels.clear(); //Cleanup client-side models/sprites

	//Re-initialize player
	InitializePlayer();
}
void MSCLGlobals::EndMap()
{
	ChooseChar_Interface::CentralServer = false; //Reset
}
void MSCLGlobals::DLLDetach()
{
	player.Deactivate();
}
int MSCLGlobals::GetLocalPlayerIndex()
{
	cl_entity_t *clPlayer = gEngfuncs.GetLocalPlayer();
	return clPlayer ? clPlayer->index : 1;
}

//Client versions of these functions
const char *EngineFunc::GetGameDir()
{
	return gEngfuncs.pfnGetGameDirectory();
}
void AlertMessage(ALERT_TYPE atype, char *szFmt, ...)
{
	static char string[1024];

	va_list argptr;
	va_start(argptr, szFmt);
	vsnprintf(string, sizeof(string), szFmt, argptr);
	va_end(argptr);

	ConsolePrint("cl: ");
	ConsolePrint(string);
}
char *UTIL_VarArgs(char *format, ...)
{
	static char string[1024];

	va_list argptr;	
	va_start(argptr, format);
	vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	return (char *)string;
}

Vector UTIL_VecToAngles(const Vector &vec)
{
	float rgflVecOut[3];
	AngleVectors(vec, rgflVecOut, NULL, NULL);
	return Vector(rgflVecOut);
}

string_t MSCLGlobals::AllocString(const char *pszString)
{
	if (!pszString)
		return 0;

	for (size_t s = 0; s < m_Strings.size(); s++)
		if (m_Strings[s] == pszString)
			return *m_Strings[s] - *gpGlobals->pStringBase; //Return existing string

	//uint len = strlen(pszString) + 1;
	msstring pszNewString = pszString;
	m_Strings.push_back(pszNewString);

	return *pszNewString - *gpGlobals->pStringBase;
	// if (!pszString)
	// 	return 0;

	// //Does the string already exist?
	// for (int s = 0; s < m_Strings.size(); s++)
	// 	if (FStrEq(m_Strings[s], pszString))
	// 		return m_Strings[s] - gpGlobals->pStringBase; //Return existing string

	// // we need memory leak otherwise effects don't work, wtf
	// uint len = strlen(pszString) + 1;
	// char *pszNewString = msnew(char[len]);
	// strncpy(pszNewString, pszString, len);

	// m_Strings.push_back((char *)pszNewString);

	// return pszNewString - gpGlobals->pStringBase;
}
/*char *MSCLGlobals::GetString( string_t sString )
{
	if( !sString ) return "(NULL)";
	return (char *)STRING( sString );
}*/
void MSCLGlobals::SetupGlobalEngFuncRedirects(void)
{
	// Fake functions
	g_engfuncs.pfnPrecacheModel = stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound = stub_PrecacheSound;
	g_engfuncs.pfnPrecacheEvent = stub_PrecacheEvent;
	g_engfuncs.pfnNameForFunction = stub_NameForFunction;
	g_engfuncs.pfnSetModel = stub_SetModel;
	g_engfuncs.pfnSetClientMaxspeed = HUD_SetMaxSpeed;

	// Handled locally
	g_engfuncs.pfnPlaybackEvent = HUD_PlaybackEvent;
	g_engfuncs.pfnAllocString = AllocString;
	g_engfuncs.pfnAlertMessage = AlertMessage;
	gpGlobals->pStringBase = "(NULL)";

	// Pass through to engine
	g_engfuncs.pfnPrecacheEvent = gEngfuncs.pfnPrecacheEvent;
	g_engfuncs.pfnRandomFloat = gEngfuncs.pfnRandomFloat;
	g_engfuncs.pfnRandomLong = gEngfuncs.pfnRandomLong;
}
//I've recieved all script files, I can now spawn
void CreateStoreMenus();
void ShowVGUIMenu(int iMenu);

void MSCLGlobals::SpawnIntoServer()
{
	startdbg;

	logfile << Logger::LOG_INFO << "SpawnIntoServer...";

	Cleanup(); //Clean up stuff from the previous map

	dbg("Call player.InitialSpawn");
	player.InitialSpawn();
	player.BeginRender();

	dbg("Call CreateStoreMenus");
	CreateStoreMenus();

	dbg("Call MSChar_Interface::CLInit");
	MSChar_Interface::CLInit();

	dbg("ShowVGUIMenu( MENU_NEWCHARACTER )");
	ShowVGUIMenu(MENU_NEWCHARACTER);

	logfile << "DONE\n";

	enddbg;
}

//Cleans up stuff from the previous map
void MSCLGlobals::Cleanup()
{
	startdbg;

	//Remove spell list
	player.m_SpellList.clear();

	//Kill the client-side entity list
	dbg("Call CBasePlayeR::RenderCleanup");
	player.RenderCleanup();

	// Delete all client-side entities
	dbg("Call MSCLGlobals::RemoveAllEntities( )");
	RemoveAllEntities();

	//Remove Environment Special Effects
	CRender::Cleanup();

	enddbg;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		MSGlobals::DLLAttach(hinstDLL);
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		//if( logfile.is_open() ) logfile << __FILE__ << ":" << ((int)__LINE__) << " client.dll being unloaded" << endl;
		if (logfile.is_open())
			(((logfile << Logger::LOG_INFO << __FILE__) << " client.dll being unloaded\n"));
		RichPresenceShutdown();
		MSGlobals::EndMap();
		MSCLGlobals::DLLDetach();
		MSGlobals::DLLDetach();
	}
	return TRUE;
}
