#ifndef MSCHARACTER_H
#define MSCHARACTER_H

#include "msfileio.h"
#include "stats/statdefs.h"
#include "gamerules/teams.h"
#include "mscharacterheader.h"
#include "buildcontrol.h"
#include "monsters/msmonster.h"

// Char Files
enum chardatastatus_e
{
	CDS_UNLOADED,
	CDS_LOADING,
	CDS_LOADED,
	CDS_NOTFOUND
};

enum charloc_e
{
	LOC_SERVER = 1,
	LOC_CENTRAL
};

enum jointype_e
{
	JN_NOTALLOWED,
	JN_TRAVEL,
	JN_STARTMAP,
	JN_VISITED,
	JN_ELITE
};

enum charClientType
{
	CHAR_TYPE_BASE = 0,
	CHAR_TYPE_HEADER,
	CHAR_TYPE_ITEMS,
};

struct charinfo_base_t
{
	int Index; // Keep track of index, because the characters might not be loaded in order	
	int DataLen;
	char* Data;

	charinfo_base_t()
	{
		Index = 0;
		DataLen = 0;
		Data = NULL;
	}
};

#define GEARFL_COVER_HEAD (1 << 0)
#define GEARFL_COVER_TORSO (1 << 1)
#define GEARFL_COVER_ARMS (1 << 2)
#define GEARFL_COVER_LEGS (1 << 3)
#define GEARFL_WEARING (1 << 4)

struct gearinfo_t
{
	byte Flags;
	ushort Model = 0, Body = 0, Skin = 0, Anim = 0;
};

struct charinfo_t : charinfo_base_t
{
	chardatastatus_e Status, m_CachedStatus;
	jointype_e JoinType;
	charloc_e Location;

	//Char current Game Status, loaded from file header or sent from server
	int body; //MiB FEB2010a (JAN2010_27) - For sending what 'body' the char-selection model should use.
	bool IsElite;
	enum gender_e Gender;
	msstring Name, MapName, OldTrans, NextMap, NewTrans;
	char Guid[MSSTRING_SIZE];
	int Flags = 0;
	mslist<gearinfo_t> GearInfo;

	charinfo_t() : charinfo_base_t() { Data = NULL; Guid[0] = 0; }
	~charinfo_t();

	void Destroy();
#ifdef VALVE_DLL
	void AssignChar(int CharIndex, charloc_e Location, const char* Data, int DataLen, class CBasePlayer* pPlayer);
#endif
};

struct natstat_t
{
	short Value[STATPROP_TOTAL];
};
struct skillstat_t
{
	short Value[STATPROP_TOTAL];
	ulong Exp[STATPROP_TOTAL];
};
struct spellskillstat_t
{
	short Value[STATPROP_TOTAL];
	long Exp[STATPROP_TOTAL];
};

#define SAVECHAR_VERSION_MSC 11 // Legacy MS: Classic
#define SAVECHAR_VERSION_MSR 12 // MS Rebirth and up.
#define SAVECHAR_VERSION_MSGPACK 13 // msgpack format.  Further format versioning is done with CF_FORMAT, see mscharacterheader.h

#define SAVECHAR_VERSION SAVECHAR_VERSION_MSGPACK

//LEGACY: chunk types of the pre-msgpack save format.  Only used to read old save files.
//New saves use the msgpack keys in mscharacterheader.h.

enum
{
	CHARDATA_HEADER1 = 0,
	CHARDATA_MAPSVISITED1,
	CHARDATA_SKILLS1,
	CHARDATA_SPELLS1,
	CHARDATA_ITEMS1,
	CHARDATA_STORAGE1,
	CHARDATA_COMPANIONS1,
	CHARDATA_HELPTIPS1, //LEGACY: skipped on read
	CHARDATA_QUESTS1,
	CHARDATA_QUICKSLOTS1,
	CHARDATA_ITEMS2,
	CHARDATA_UNKNOWN, //If >= CHARDATA_UNKNOWN, then skip it?
};

class MSChar_Interface
{
public:
	//static Vector LastGoodPos, LastGoodAng;
	static enum jointype_e CanJoinThisMap(savedata_t &Data, msstringlist &VisitedMaps);		//Client & Server
	static enum jointype_e CanJoinThisMap(charinfo_t &CharData, msstringlist &VisitedMaps); //Client & Server
	static bool HasVisited(const char* MapName, msstringlist &VisitedMaps);				//Client & Server

#ifdef VALVE_DLL
	//Server - characters are only ever stored on the server (or central server)
	static void AutoSave(class CBasePlayer *pPlayer);
	static bool ReadCharData(void *pData, ulong Size, struct chardata_t *CharData);
	static void SaveChar(class CBasePlayer *pPlayer, savedata_t *pData = NULL);
#endif
};

#ifdef VALVE_DLL
bool DeleteChar(CBasePlayer *pPlayer, int iCharacter);
const char *GetSaveFileName(int iCharacter, CBasePlayer *pPlayer);
#endif

#define MAX_CHARSLOTS 3 //Max number of characters one person can have. This is the max the game supports.  A server operator can set less for his server via CVAR "ms_serverchar" (clamped to 1..MAX_CHARSLOTS)

struct charslot_t
{
	bool Active, //Whether this character exists and is loaded
		CanJoin, //Whether this character can enter this map
		Pending; //Central Server is currently trying to download this char

	savedata_t Data; //Character's data.  For server-side characters, only a few fields here are valid
};

class ChooseChar_Interface
{
public:
	static int ServerCharNum; //Max number of characters the server will allow
	static bool CentralServer;
	static void UpdateCharScreen();
};
#endif //MSCHARACTER_H
