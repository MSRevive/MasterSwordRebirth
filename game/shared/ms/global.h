#include "votedefs.h"

//The client calls this once, on DLL load
//The server calls this every map change, at CWorld::Precache
void MSGlobalItemInit();

#ifndef _WINDEF_
typedef int HANDLE;
typedef HANDLE HINSTANCE;
#define HMODULE void *
#endif

enum gametype_e
{
	GAMETYPE_ADVENTURE, //Normal map, saving allowed
	GAMETYPE_CHALLENGE, //Special scenario.  Can be a custom map. Only played for fun... character is never saved.
	GAMETYPE_NUM,
};

enum clperments_e
{
	CLPERMENT_INSET,
	CLPERMENT_LEFTVIEW,
	CLPERMENT_RIGHTVIEW,
	CLPERMENT_TOTAL,
};

class MSGlobals
{
public:
	static bool PKAllowed,					 //Pk and stealing allowed outside of town
		PKAllowedinTown,					 //PK and stealing allowed anywhere
		IsLanGame,							 //Lan game
		CentralEnabled,						 //Thothie attempting to fix FN upload sploit
		DevModeEnabled,						 //MiB JUL2010_13 - Making it so dev-mode makes it so you can't save.
		CanCreateCharOnMap,					 //Can create character on this map
		ServerSideChar,						 //Whether characters are saved/loaded server-side
		InvertTownAreaPKFlag,				 //Town areas work opposite - PK is only allowed _in_ the townarea
		IsServer,							 //This is the server (not client)
		HasConnected,							// We managed to connect to the FN server, use this for other functions if we connected etc.
		InPrecache;							 //Dll is loading and precaching (The client doesn't precache, but this prevents spawn from being called on items)
	static gametype_e GameType;				 //Regular game (allows saving) or custom map (no saving allowed)
	static vote_t CurrentVote;				 //Current vote
	static msstring ServerName;				 //Client - Server name
	static msstring MapName;				 //Client - Map name
	static msstring MapTitle;				 //Thothie SEP2007a
	static msstring MapDesc;				 //Thothie SEP2007a
	static msstring map_addparams;			 //Thothie DEC2014_17 - global addparams
	static msstring map_flags;				 //Thothie DEC2014_17 - global map flags

	static msstring MapWeather;				//Thothie SEP2007a
	static msstring HPWarn;					//Thothie SEP2007a
	static float maxviewdistance;			//Thothie JAN2010_23
	//static int FakeHP;					//DEC2007a - AUG2011_17 Thothie - Switched to cvar
	static msstring AbsGamePath;			//GameDirPath Absolute
	static int DefaultGold,					//Starting gold
		FXLimit;							//Server FX Limit
	static msstringlist DefaultWeapons;		//Default starting weapons
	static msstringlist DefaultFreeItems;	//Free items that come with a new character
	static string_i DefaultSpawnBoxModel;	//Model surrounding the player when he first spawns in
	static class IScripted *GameScript;		//Controls global events, like weather, etc
	static char Buffer[32768];				//A huge buffer for text or anything else
	static int ClEntities[CLPERMENT_TOTAL]; //Index of the permanent client entities.  Must be allocated on server & client
	static int SpawnLimit; //Thothie OCT2016_18 spawnlimiter

	//Vars for managing music, will be used for playing music to players who just connect. Should help them keep up with players who have triggered stuff further in a map
	static std::string AllMusic; //Last track played to all players
	static int AllMusicMode; //Mode of the last track played to all players

	static void DLLAttach(HINSTANCE hinstDLL); //Client & Server - Called on dll load
	static void DLLDetach();				   //Client & Server - Called on new unload
	static void NewMap();					   //Client & Server - Called on new level
	static void SharedThink();				   //Client & Server - Called each frame
	static void EndMap();					   //Client & Server - Called when map ends
	static int gSoundPrecacheCount;
};

class MemMgr
{
public:
	static int m_TotalAllocations;
	static int m_HighestAllocations;
	static float m_TimePrintAllocations;

	static void NewAllocation(void *pAddr, size_t size);
	static void NewDeallocation(void *pAddr);
	static void Think();
	static void EndMap();
};

enum MusicType {
	MUSIC_AREA,
	MUSIC_COMBAT,
	MUSIC_SYSTEM,
	MUSIC_STOP_COMBAT,//For scripts, stops combat music
	MUSIC_STOP //For map triggers, stops combat & area music
};