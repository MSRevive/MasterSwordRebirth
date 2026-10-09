#ifndef MSCHARACTERHEADER_H
#define MSCHARACTERHEADER_H

#ifdef _WIN32
#include <pshpack4.h>
#endif

//Legacy on-disk character header.  Old (pre-msgpack) save files store this struct raw,
//so its layout is FROZEN - never add, remove or reorder anything in here.
//Add new header fields to savedata_t below instead.
struct savedata_legacy_t
{
	//Player info
	int Version;
	char Name[32],
		Race[16],
		MapName[16],
		NextMap[32],
		OldTrans[32],
		NewTrans[32],
		SteamID[32],
		Party[12];
	byte IsElite;
	int Gold;
	short MaxHP, MaxMP, HP, MP;
	Vector Origin, Angles;
	byte Gender;
	ulong PartyID; //Unique ID for party.  If I join a server that has a party with the same name, I won't
		//automatically join their party

	short PlayerKills;
	float TimeWaitedToForgetKill;  //Counts up... when reachs a certain number, decrement PlayerKills
	float TimeWaitedToForgetSteal; //Counts up... when reachs a certain number, player is not considered a thief
};

#ifdef _WIN32
#include <poppack.h>
#endif

//In-memory character header.  New header fields go here (and get a charfield_e key).
struct savedata_t : savedata_legacy_t
{
	char PartyName[13]; //Full party name (MAX_TEAMNAME_LEN + 1).  Use this instead of Party, which only fits 11 chars
};

//
// Character save format (msgpack)
// ===============================
// A save is "MSRC" followed by a msgpack map of { charfield_e -> value }.
// Sub-records (items, storages, companions) are maps keyed by their own enums below.
//
// Rules for changing the format:
//  - Add a field:    append a NEW key to the right enum, write it in SaveChar and read it in the reader.
//                    Older builds ignore keys they don't know.  Saves without the key leave the field zeroed.
//  - Remove a field: stop writing/reading it and mark the key "// RETIRED".  Never reuse or renumber a key.
//  - Only bump CHARFMT_VERSION when an existing key changes meaning.  Readers refuse saves
//    with a newer format, and can branch on CF_FORMAT to convert older ones.
//
// Reading and writing live in sv_character.cpp (server only).
//

#define CHARFMT_VERSION 1

//Top-level keys
enum charfield_e
{
	CF_FORMAT = 0,			//int - CHARFMT_VERSION the save was written with
	CF_NAME = 1,			//str
	CF_MAPNAME = 2,			//str
	CF_NEXTMAP = 3,			//str
	CF_OLDTRANS = 4,		//str
	CF_NEWTRANS = 5,		//str
	CF_STEAMID = 6,			//str
	CF_PARTY = 7,			//str
	CF_PARTYID = 8,			//uint
	CF_ISELITE = 9,			//int
	CF_GOLD = 10,			//int
	CF_MAXHP = 11,			//int
	CF_MAXMP = 12,			//int
	CF_HP = 13,				//int
	CF_MP = 14,				//int
	CF_GENDER = 15,			//int
	CF_PLAYERKILLS = 16,	//int
	CF_TIMEFORGETKILL = 17,	//float

	CF_VISITEDMAPS = 32,	//[str, ...]
	CF_SKILLS = 33,			//[ stat: [ substat: [value, exp], ... ], ... ]
	CF_SPELLS = 34,			//[str, ...]
	CF_ITEMS = 35,			//[item, ...]
	CF_STORAGES = 36,		//[storage, ...]
	CF_COMPANIONS = 37,		//[companion, ...]
	CF_QUESTS = 38,			//[ [name, data], ... ]
	CF_QUICKSLOTS = 39,		//[ nil | [type, id], ... ]
};

//Item keys
enum charitemfield_e
{
	CI_NAME = 0,		//str
	CI_PROPERTIES = 1,	//uint
	CI_LOCATION = 2,	//int
	CI_HAND = 3,		//int
	CI_ID = 4,			//uint - item ID at last save (used by quickslots)
	CI_QUALITY = 5,		//int - perishable/drinkable only
	CI_MAXQUALITY = 6,	//int - perishable/drinkable only
	CI_QUANTITY = 7,	//int - groupable only
	CI_CONTENTS = 8,	//[item, ...] - containers only
};

//Storage keys
enum charstoragefield_e
{
	CS_NAME = 0,	//str
	CS_ITEMS = 1,	//[item, ...]
};

//Companion keys
enum charcompanionfield_e
{
	CC_SCRIPT = 0,	//str
	CC_VARS = 1,	//[ [name, value], ... ]
};

#endif //MSCHARACTERHEADER_H
