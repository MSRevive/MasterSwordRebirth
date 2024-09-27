/*
	Global.cpp - Shared global initialization functions
*/

#include "inc_weapondefs.h"
#include "script.h"
#include "titles.h"
#include "scriptedeffects.h"
#include "logger.h"
#ifndef VALVE_DLL
#include "hud.h"
#include "cl_util.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "event_api.h"
#include "clglobal.h"
#else
#include "global.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

//#define EXTENSIVE_LOGGING		//Causes EXTENSIVE logging of every dbg operation

unsigned int g_iNumBytesWritten = 0;
int g_iUserMessageType = -1;

void LogMemoryUsage(msstring_ref Title);

//Global Declarations.  Both Client & Server
int MSGlobals::FXLimit = 20;
bool MSGlobals::PKAllowed = false;
bool MSGlobals::PKAllowedinTown = false;
bool MSGlobals::CentralEnabled = false; //Thothie attempting to fix FN upload sploit
bool MSGlobals::DevModeEnabled = false; //Thothie JUL2010_22 attempting to fix "Mapper Scripts.rtf"
bool MSGlobals::IsLanGame = false;
bool MSGlobals::CanCreateCharOnMap = false;
bool MSGlobals::ServerSideChar = false;
bool MSGlobals::InvertTownAreaPKFlag = false;
bool MSGlobals::IsServer =
#ifdef VALVE_DLL
	true;
#else
	false;
#endif
bool MSGlobals::InPrecache = false;
gametype_e MSGlobals::GameType = GAMETYPE_ADVENTURE;
vote_t MSGlobals::CurrentVote;
msstring MSGlobals::MapName;
msstring MSGlobals::map_addparams = "";				//Thothie DEC2014_17 - global addparams
msstring MSGlobals::map_flags = "";					//Thothie DEC2014_17 - map flags
msstring MSGlobals::MapTitle;						//Thothie SEP2007a
msstring MSGlobals::MapDesc;						//Thothie SEP2007a
msstring MSGlobals::MapWeather;						//Thothie SEP2007a
msstring MSGlobals::HPWarn;							//Thothie SEP2007a
int MSGlobals::SpawnLimit=0; //Thothie OCT2016_18 spawnlimiter
float MSGlobals::maxviewdistance;					//Thothie JAN2010_23
//int MSGlobals::FakeHP;	//DEC2007a (AUG2011_17 - switched to cvar)
msstring MSGlobals::ServerName;
msstring MSGlobals::AbsGamePath;
msstringlist MSGlobals::DefaultWeapons;
msstringlist MSGlobals::DefaultFreeItems; //Free items that come with a new character
int MSGlobals::DefaultGold = 10;		  //Starting gold
string_i MSGlobals::DefaultSpawnBoxModel;

IScripted *MSGlobals::GameScript = nullptr;
char MSGlobals::Buffer[32768]; //A huge buffer for text or anything else
int MSGlobals::ClEntities[CLPERMENT_TOTAL] = {500, 501, 502};
int MSGlobals::gSoundPrecacheCount = 0;

std::string MSGlobals::AllMusic = "";
int MSGlobals::AllMusicMode = 0;

msstringlist vote_t::VotesTypes;		//All The vote types
msstringlist vote_t::VotesTypesAllowed; //All The vote types allowed

//The client calls this once, on DLL load
//The server calls this every map change, at CWorld::Precache
void MSGlobalItemInit()
{
	startdbg;
	dbg("Begin");
	MSGlobals::InPrecache = true;

	//Delete the previous titles
	CTitleManager::DeleteAllTitles();

	//Delete the scripted effects
	CGlobalScriptedEffects::DeleteEffects();

	// Load the scripts from file
	CGenericItemMgr::GenericItemPrecache();

	//since we're on steam now and can control what is shipped,
	//the save directory should already be there.
	//MSChar_Interface::CreateSaveDir();

	//Load the global.script file
	//This sets up various un-changing values and game relationships
	//If this script uses any commands that refer to the script owner, the game will crash
	CScript GameGlobals;
	GameGlobals.Spawn("global", NULL, NULL);
	GameGlobals.RunScriptEvents(false);

	vote_t::VotesTypes.clearitems();
	vote_t::VotesTypes.add("kick");
	vote_t::VotesTypes.add("advtime");

	MSGlobals::InPrecache = false;

	enddbg;
}

//Called on client & server when a new map is loaded
void MSGlobals::NewMap()
{
	Log("[NewMap]: %s", MSGlobals::MapName.c_str());
	if (MSGlobals::GameScript)
	{
		//GameScript should have been deleted by EndMap() before NewMap() is called
		MSErrorConsoleText("MSGlobals::NewMap", "MSGlobals::GameScript already allocated!");
	}
	
#if SPAWN_GLOBAL_ITEMS
	for(int i = 0; i < CGenericItemMgr::ItemCount(); i++)
	{
		GenItem_t * pGenItem = CGenericItemMgr::Item(i);
		if (pGenItem) // Paranoid
		{
				CGenericItemMgr::GetGlobalGenericItemByName(pGenItem->Name, true);
		}
	}
#endif
	
	//This global script controls all continuous global events
	MSGlobals::GameScript = msnew IScripted;
#ifdef VALVE_DLL
	MSGlobals::GameScript->Script_Add("world", CBaseEntity::Instance(INDEXENT(0)));
#else
	MSGlobals::GameScript->Script_Add("world", NULL);
#endif
	if (MSGlobals::GameScript)
	{
		MSGlobals::GameScript->RunScriptEvents(false);

		msstringlist Params;
		Params.add(MSGlobals::MapName);
		MSGlobals::GameScript->CallScriptEvent("game_newlevel", &Params); //Be sure to call this before spawn, so stuff gets precached

		MSGlobals::GameScript->CallScriptEvent("game_precache");
		MSGlobals::GameScript->CallScriptEvent("game_spawn");
	}
}

//Called on client & server when an old map ends
//Client: Called from the INIT Msg or from When cl DLL is detached
//Server: Called from ServerDeactivate()
void MSGlobals::EndMap()
{
#ifndef VALVE_DLL
	MSCLGlobals::EndMap();
#endif

	if (MSGlobals::GameScript)
	{
		//MSGlobals::GameScript->CallScriptEvent("game_end"); //Thothie - this does not get called due to unknown error (See SVGlobals.cpp line 228)
		MSGlobals::GameScript->Deactivate();
		delete MSGlobals::GameScript;
		MSGlobals::GameScript = NULL;
	}

	//LogMemoryUsage( "[End Map Remaining Memory Allocations]" );

	MSGlobals::GameType = GAMETYPE_ADVENTURE; //Default to adventure game for next level

	//Reset vars for music
	AllMusic.clear();
	AllMusicMode = 0;

	MemMgr::EndMap();
}

void MSGlobals::SharedThink()
{
	startdbg;

	dbg("Call MSGlobals->GameScript->Think");
	if (MSGlobals::GameScript)
		MSGlobals::GameScript->RunScriptEvents(false);

	MemMgr::Think();

	enddbg;
}

//Called on client & server when the dll is loaded
void MSGlobals::DLLAttach(HINSTANCE hinstDLL)
{
#ifdef _WIN32
	msstring DllFileName;
	GetModuleFileName(hinstDLL, DllFileName.c_str(), MSSTRING_SIZE);

	// Remove cl_dlls or dlls (plus slash) from the path!
#ifdef VALVE_DLL
	size_t nullterm = 5;
#else
	size_t nullterm = 8;
#endif

	int len = DllFileName.len();
	for (int i = 0; i < len; i++)
	{
		if (DllFileName[len - 1 - i] == '\\' || DllFileName[len - 1 - i] == '/')
		{
			AbsGamePath = DllFileName.substr(0, len - i - 1);
			break;
		}
	}

	DllFileName = AbsGamePath;
	AbsGamePath = DllFileName.substr(0, DllFileName.len() - nullterm);
#else	
	char szTempPath[MSSTRING_SIZE];
	getcwd(szTempPath, MSSTRING_SIZE);
	AbsGamePath = msstring(szTempPath) + msstring("/msr");
#endif

	OpenLogFiles();
}

//Called on client & server when the dll is unloaded
void MSGlobals::DLLDetach()
{
	//Delete the titles
	CTitleManager::DeleteAllTitles();

	//Delete the scripted effects
	CGlobalScriptedEffects::DeleteEffects();

	CScript::m_gVariables.clear();

	LogMemoryUsage("[DLL Unload Remaining Memory Allocations]");
}

void CBaseEntity::DelayedRemove()
{
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = BaseThinkTime() + 0.1;
}

genericitem_t::genericitem_t(class CGenericItem *pItem)
{
	ID = pItem->m_iId;
	memcpy(&Name, &pItem->ItemName, sizeof(pItem->ItemName)); //Use Memcpy instead of = to perfectly sync all 256 bytes.  This is memcmp'd later
	Properties = pItem->MSProperties();
	Quantity = pItem->iQuantity;
	Quality = pItem->Quality;
	MaxQuality = pItem->MaxQuality;
}
genericitem_t::operator class CGenericItem *()
{
	//Creates an Item
	//Don't copy the ID
	CGenericItem *pItem = NewGenericItem(Name);
	if (!pItem)
		return NULL;
	//pItem->Properties = Properties;
	pItem->iQuantity = Quantity;
	pItem->Quality = Quality;
	pItem->MaxQuality = MaxQuality;
	pItem->m_OldID = ID;

	return pItem;
}

genericitem_full_t::genericitem_full_t(CGenericItem *pItem) : genericitem_t(pItem)
{
	Hand = pItem->m_Hand;
	Location = pItem->m_Location;
	Spell_TimePrepare = pItem->Spell_TimePrepare;
	Spell_CastSuccess = pItem->Spell_CastSuccess;
	if (FBitSet(pItem->MSProperties(), ITEM_CONTAINER))
		for (int i = 0; i < pItem->Container_ItemCount(); i++)
			ContainerItems.add(pItem->Container_GetItem(i));
}
genericitem_full_t::operator class CGenericItem *()
{
	CGenericItem *pItem = genericitem_t::operator CGenericItem *();
	if (!pItem)
		return NULL;

	pItem->m_Location = Location;
	pItem->Spell_TimePrepare = Spell_TimePrepare;
	pItem->Spell_CastSuccess = Spell_CastSuccess ? true : false;
	pItem->m_Hand = Hand;
	if (FBitSet(pItem->MSProperties(), ITEM_CONTAINER))
		for (int i = 0; i < ContainerItems.size(); i++)
			pItem->Container_AddItem(ContainerItems[i]);

	return pItem;
}

const char *g_EntTypeByName[ENT_TYPE_TOTAL] =
	{
		"ent_lastseen",
		"ent_lastheard",
		"ent_lastspoke",
		"ent_lastoffered",
		"ent_lastgave",
		"ent_laststole",
		"ent_lastused",
		"ent_laststruck",
		"ent_laststruckbyme",
		"ent_lastprojectile",
		"ent_lastcreated",
		"ent_me",
		"ent_owner",
		"ent_creationowner",
		"ent_target",
		"ent_expowner",
		"ent_localplayer",
		"ent_currentplayer",
};
int EntityNameToType(const char *pszName)
{
	for (int i = 0; i < ENT_TYPE_TOTAL; i++)
		if (!_stricmp(pszName, g_EntTypeByName[i]))
			return i;
	return -1;
}

entityinfo_t::entityinfo_t(CBaseEntity *pEntity)
{
	entindex = pEntity->entindex();
	pvPrivData = pEntity;
}

entityinfo_t &entityinfo_t::operator=(CBaseEntity *pEntity)
{
	entindex = (pEntity ? pEntity->entindex() : 0);
	pvPrivData = pEntity;
	return *this;
}

entityinfo_t::operator bool()
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(INDEXENT(entindex));
	return pvPrivData == pEntity;
}

CBaseEntity *entityinfo_t::Entity()
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(INDEXENT(entindex));
	return pvPrivData == pEntity ? pEntity : NULL;
}

void CBaseEntity::StoreEntity(CBaseEntity *pEntity, enttype_e EntType)
{
#ifdef VALVE_DLL
	m_EntityList[EntType].entindex = pEntity ? pEntity->entindex() : 0;
	m_EntityList[EntType].pvPrivData = pEntity ? pEntity : NULL;
#endif
}

CBaseEntity *CBaseEntity::RetrieveEntity(enttype_e EntType)
{
#ifdef VALVE_DLL
	if (!m_EntityList[EntType].pvPrivData)
		return NULL;

	CBaseEntity *pEntity = MSInstance(INDEXENT(m_EntityList[EntType].entindex));
	if (pEntity && pEntity == m_EntityList[EntType].pvPrivData)
		return pEntity;
#endif
	return NULL;
}

CBaseEntity *CBaseEntity::RetrieveEntity(const char *pszName)
{
	CBaseEntity *pEntity = StringToEnt(pszName);
	if (pEntity)
		return pEntity;

#ifdef VALVE_DLL
	int EntType = EntityNameToType(pszName);
	if (EntType == -1)
		return NULL;

	pEntity = MSInstance(INDEXENT(m_EntityList[EntType].entindex));
	if (pEntity && pEntity == m_EntityList[EntType].pvPrivData)
		return pEntity;
#endif
	return NULL;
}

Vector CBaseEntity::DetermineOrigin(msstring & vsOrigin)
{
	if (!vsOrigin.len())
	{
			return Vector();
	}
	
#ifdef VALVE_DLL
	CBaseEntity * pEntity = RetrieveEntity(vsOrigin);
	if (pEntity) 
		return pEntity->pev->origin;
#else
	int vEntIndx = vsOrigin[0] == '(' ? -1 : atoi(vsOrigin);
	cl_entity_t * pEntity = vEntIndx > -1 ? gEngfuncs.GetEntityByIndex( vEntIndx ) : NULL;
	if (pEntity) 
		return pEntity->origin;
#endif
	return StringToVec(vsOrigin);
}

//	CScriptedEnt - Any entity with a script

static msstringlist Parameters; //made static, for speed

void CScriptedEnt::Spawn()
{
	startdbg;
	dbg("Begin");
	StoreEntity(this, ENT_ME);
	m_HandleThink = true;

	edict_t *pEdict = edict();
	CBaseEntity::Spawn();
	if (!pEdict->free)
	{
		dbg("Call game_spawn");
		CallScriptEvent("spawn");	   //old
		CallScriptEvent("game_spawn"); //not called by players (dunno about monsters)
	}

	enddbg;
}
void CScriptedEnt::Think()
{
	startdbg;
	dbg("CScriptedEnt::Think - Begin");

	edict_t *pEdict = edict();
	CBaseEntity::Think();

	if (pEdict->free)
		return;

	RunScriptEvents();

	if (!m_HandleThink)
		return;

	CallScriptEvent("game_think");

	enddbg;
}

void CScriptedEnt::Touch(CBaseEntity *pOther)
{
	CBaseEntity::Touch(pOther);

	if (!m_HandleTouch)
		return;

	msstring msTouchTargetName = STRING(pev->targetname);
	msstring msTouchTarget = STRING(pev->target);

	Parameters.clearitems();
	Parameters.add(EntToString(pOther));
	Parameters.add(msTouchTargetName);
	Parameters.add(msTouchTarget);

	CallScriptEvent("game_touch", &Parameters);
}
void CScriptedEnt::Blocked(CBaseEntity *pOther)
{
	if (!m_HandleBlocked)
		return;

	Parameters.clearitems();
	Parameters.add(EntToString(pOther));

	CallScriptEvent("game_blocked", &Parameters);
}
void CScriptedEnt::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	startdbg;
	dbg("Begin");

	if (m_pfnUse)
		(this->*m_pfnUse)(pActivator, pCaller, useType, value);

	Parameters.clearitems();
	Parameters.add(EntToString(pActivator));
	Parameters.add(EntToString(pCaller));
	Parameters.add(UTIL_VarArgs("%i", useType));
	Parameters.add(UTIL_VarArgs("%f", value));
	CallScriptEvent("game_used", &Parameters);
	enddbg;
}
void CScriptedEnt::KeyValue(KeyValueData *pkvd)
{
	startdbg;
	dbg("Begin");

	if (!pkvd->fHandled && !strcmp(pkvd->szKeyName, "scriptname"))
	{
		Script_Add(pkvd->szValue, this);
		RunScriptEvents(); //For precachefile
		pkvd->fHandled = TRUE;
	}
	else
		pkvd->fHandled = FALSE;

	enddbg;
}
void CScriptedEnt::Deactivate()
{
	startdbg;
	dbg("CScriptedEnt::Deactivate - Begin");

	CBaseEntity::Deactivate();
	IScripted::Deactivate();

	enddbg;
}
/*
======================
AlertMessage

Print debug messages to console
======================
*/
void Print(const char *szFmt, ...)
{
	static char string[1024];

	va_list argptr;
	va_start(argptr, szFmt);
	vsnprintf(string, sizeof(string), szFmt, argptr);
	va_end(argptr);

	ALERT(at_console, "%s", string);
}

void Log(const char *szFmt, ...)
{
	if (!logfile.is_open())
		return;

	static char string[1024];

	va_list argptr;	
	va_start(argptr, szFmt);
	vsnprintf(string, sizeof(string), szFmt, argptr);
	va_end(argptr);

	logfile << string << "\n";
}

void LogExtensive(msstring_ref Text)
{
#ifdef EXTENSIVE_LOGGING
	LogCurrentLine(Text);
#endif
}

void DbgLog(char *szFmt, ...)
{
#ifdef EXTENSIVE_LOGGING
	if (!logfile.is_open())
		return;

	static char string[1024];

	va_list argptr;	
	va_start(argptr, szFmt);
	vsnprintf(string, sizeof(string), szFmt, argptr);
	va_end(argptr);

	logfile << string << "\n";
#endif
}

msstring_ref EngineFunc::GetString(int string)
{
	return STRING(string);
}
int EngineFunc::AllocString(msstring_ref String)
{
	return ALLOC_STRING(String);
}
void EngineFunc::MakeVectors(const Vector &vecAngles, float *p_vForward, float *p_vRight, float *p_vUp)
{
#ifdef VALVE_DLL
	g_engfuncs.pfnAngleVectors(vecAngles, p_vForward, p_vRight, p_vUp);
#else
	AngleVectors(vecAngles, p_vForward, p_vRight, p_vUp);
#endif
}
float EngineFunc::CVAR_GetFloat(msstring_ref Cvar)
{
	return CVAR_GET_FLOAT(Cvar);
}
msstring_ref EngineFunc::CVAR_GetString(msstring_ref Cvar)
{
	return CVAR_GET_STRING(Cvar);
}
#ifndef VALVE_DLL
cvar_s *EngineFunc::CVAR_Create(msstring_ref Cvar, msstring_ref Value, const int Flags)
{
	return CVAR_CREATE(Cvar, Value, Flags);
}
#else
void EngineFunc::CVAR_Create(cvar_t &Cvar)
{
	CVAR_REGISTER(&Cvar);
}
#endif

void EngineFunc::CVAR_SetFloat(msstring_ref Cvar, float Value)
{
#ifdef VALVE_DLL
	CVAR_SET_FLOAT(Cvar, Value);
#else
	gEngfuncs.Cvar_SetValue((char *)Cvar, Value);
#endif
}

void EngineFunc::CVAR_SetString(msstring_ref Cvar, msstring_ref Value)
{
#ifdef VALVE_DLL
	CVAR_SET_STRING(Cvar, Value);
#else
	cvar_t *pCVar = gEngfuncs.pfnGetCvarPointer("lightgamma");
	if (pCVar)
		pCVar->string = (char *)STRING(ALLOC_STRING(Value)); //Allocate mem for the cvar
#endif
}

void EngineFunc::Shared_TraceLine(const Vector &vecStart, const Vector &vecEnd, int ignore_monsters, sharedtrace_t &Tr, int CLFlags, int CLIgnoreFlags, edict_t *SVIgnoreEnt)
{
#ifdef VALVE_DLL
	TraceResult tr;
	IGNORE_GLASS ig = FBitSet(ignore_monsters, missile) ? ignore_glass : dont_ignore_glass; //missile is a hack to indicate ignore_glass
	ClearBits(ignore_monsters, missile);

	UTIL_TraceLine(vecStart, vecEnd, (IGNORE_MONSTERS)ignore_monsters, ig, SVIgnoreEnt, &tr);
	Tr.Allsolid = tr.fAllSolid ? true : false;
	Tr.EndPos = tr.vecEndPos;
	Tr.Fraction = tr.flFraction;
	Tr.HitEnt = tr.pHit ? ENTINDEX(tr.pHit) : 0;
	Tr.Hitgroup = tr.iHitgroup;
	Tr.Inopen = tr.fInOpen ? true : false;
	Tr.Inwater = tr.fInWater ? true : false;
	Tr.Startsolid = tr.fStartSolid ? true : false;
	Tr.PlaneNormal = tr.vecPlaneNormal;
	Tr.PlaneDist = tr.flPlaneDist;
#else
	//SEP2011_16 Thothie's special clworld trace
	//- "worldonly" traces seem to treat collision boxes (monsters) as part of the map hull
	//- this avoids this, but it also ignores solid map entities, such as func_walls
	if (CLIgnoreFlags == 32)
	{
		pmtrace_t pmtr;
		gEngfuncs.pEventAPI->EV_PlayerTrace((float *)&vecStart[0], (float *)&vecEnd[0], PM_WORLD_ONLY, -1, &pmtr);
		Tr.Allsolid = pmtr.allsolid ? true : false;
		Tr.EndPos = pmtr.endpos;
		Tr.Fraction = pmtr.fraction;
		Tr.HitEnt = pmtr.ent;
		Tr.Hitgroup = pmtr.hitgroup;
		Tr.Inopen = pmtr.inopen ? true : false;
		Tr.Inwater = pmtr.inwater ? true : false;
		Tr.Startsolid = pmtr.startsolid ? true : false;
		Tr.PlaneNormal = pmtr.plane.normal;
		Tr.PlaneDist = pmtr.plane.dist;
	}
	else
	{
		pmtrace_s &pmtr = *gEngfuncs.PM_TraceLine((float *)&vecStart[0], (float *)&vecEnd[0], CLFlags, 2, CLIgnoreFlags);
		Tr.Allsolid = pmtr.allsolid ? true : false;
		Tr.EndPos = pmtr.endpos;
		Tr.Fraction = pmtr.fraction;
		Tr.HitEnt = pmtr.ent;
		Tr.Hitgroup = pmtr.hitgroup;
		Tr.Inopen = pmtr.inopen ? true : false;
		Tr.Inwater = pmtr.inwater ? true : false;
		Tr.Startsolid = pmtr.startsolid ? true : false;
		Tr.PlaneNormal = pmtr.plane.normal;
		Tr.PlaneDist = pmtr.plane.dist;
	}

#endif
}

int EngineFunc::Shared_PointContents(const Vector &Origin)
{
#ifdef VALVE_DLL
	return UTIL_PointContents(Origin);
#else
	return gEngfuncs.PM_PointContents((float *)&Origin[0], NULL);
#endif
}

float EngineFunc::Shared_GetWaterHeight(const Vector &Origin, float minz, float maxz)
{
	//Use this after you've already determined that the entity is in water (with waterlevel)

	Vector midUp = Origin;
	minz += Origin.z;
	maxz += Origin.z;
	midUp.z = minz;

	if (EngineFunc::Shared_PointContents(midUp) != CONTENTS_WATER)
		return minz;

	midUp.z = maxz;
	if (EngineFunc::Shared_PointContents(midUp) == CONTENTS_WATER)
		return maxz;

	float diff = maxz - minz;
	while (diff > 1.0)
	{
		midUp.z = minz + diff / 2.0;
		if (EngineFunc::Shared_PointContents(midUp) == CONTENTS_WATER)
		{
			minz = midUp.z;
		}
		else
		{
			maxz = midUp.z;
		}
		diff = maxz - minz;
	}

	return midUp.z;
}
float EngineFunc::AngleDiff(float destAngle, float srcAngle)
{
	float delta;

	delta = destAngle - srcAngle;
	if (destAngle > srcAngle)
	{
		if (delta >= 180)
			delta -= 360;
	}
	else
	{
		if (delta <= -180)
			delta += 360;
	}
	return delta;
}
//Play sound independent of an entity
void EngineFunc::Shared_PlaySound3D(msstring_ref Sound, float Volume, const Vector &Origin, float Attn)
{
#ifdef VALVE_DLL
	//UTIL_EmitAmbientSound( ENT(0), Origin, Sound, Volume, ATTN_NORM, 0, 100 );
	UTIL_EmitAmbientSound(ENT(0), Origin, Sound, Volume, Attn, 0, 100); //attn added
#else
	//gEngfuncs.pfnPlaySoundByNameAtLocation( (char *)Sound, Volume, *(Vector *)&Origin );
	gEngfuncs.pEventAPI->EV_PlaySound(0, *(Vector *)&Origin, CHAN_AUTO, Sound, Volume, Attn, 0, 100); //attn added
#endif
}

//Thothie MAR2012 - As above, plus channel control
//- prob didn't need a new command, but didn't want to risk conflicts
void EngineFunc::Shared_PlaySound3D2(msstring_ref Sound, float Volume, const Vector &Origin, float Attn, int Chan, float sPitch)
{
#ifdef VALVE_DLL
	//UTIL_EmitAmbientSound( ENT(0), Origin, Sound, Volume, ATTN_NORM, 0, 100 );
	UTIL_EmitAmbientSound(ENT(0), Origin, Sound, Volume, Attn, Chan, sPitch); //attn added
#else
	//gEngfuncs.pfnPlaySoundByNameAtLocation( (char *)Sound, Volume, *(Vector *)&Origin );
	gEngfuncs.pEventAPI->EV_PlaySound(0, *(Vector *)&Origin, Chan, Sound, Volume, Attn, 0, sPitch); //attn added
#endif
}

//Thothie AUG2010_03
//Return true if sky texture above this point
//FindSkyHeight could be used for this, but is fooled by sloppy brushwork due to the trace up and down method
bool UnderSky(Vector Origin)
{
	Vector StartOrigin = Origin;
	Vector EndOrigin = StartOrigin + Vector(0, 0, 8092);
	Vector CurrentOrigin = StartOrigin;
	sharedtrace_t Tr;
	EngineFunc::Shared_TraceLine(StartOrigin, EndOrigin, ignore_monsters, Tr, 0, PM_GLASS_IGNORE | PM_WORLD_ONLY, NULL);

	if (!Tr.Allsolid)
	{
		if (EngineFunc::Shared_PointContents(Tr.EndPos) == CONTENTS_SKY)
			return true;
	}

	return false;
}

bool FindSkyHeight(Vector Origin, float &SkyHeight)
{
	Vector StartOrigin = Origin;
	Vector EndOrigin = StartOrigin + Vector(0, 0, 8092);
#define CHECKSTEP 32

	Vector CurrentOrigin = StartOrigin;
	sharedtrace_t Tr;
	EngineFunc::Shared_TraceLine(StartOrigin, EndOrigin, ignore_monsters, Tr, 0, PM_GLASS_IGNORE | PM_WORLD_ONLY, NULL);
	bool Direction = true; //true == up
	if (!Tr.Allsolid)
	{
		if (Tr.Inwater && (EngineFunc::Shared_PointContents(Tr.EndPos) == CONTENTS_SKY))
		{
			CurrentOrigin = Tr.EndPos;
			Direction = false; //Trace back down until there's no sky
		}

		//Find the beginning or end of the sky.  If the traceline didn't work, trace up. If it did, trace down
		int CheckStep = Direction ? CHECKSTEP : -CHECKSTEP;

		for (int i = 0; i < 64; i++)
		{
			int Contents = EngineFunc::Shared_PointContents(CurrentOrigin);
			if ((Direction && Contents == CONTENTS_SKY) ||
				(!Direction && Contents != CONTENTS_SKY))
			{
				SkyHeight = CurrentOrigin.z - CheckStep;
				return true;
			}
			CurrentOrigin.z += CheckStep;
		}
	}

	return false;
}

/*int	MSAllocateModel( char *pszModel )
{
	int idx = g_engfuncs.pfnPrecacheModel( pszModel );
	logfile << "Loaded Model #" << idx << " (" << pszModel << ")" << endl;
	return idx;
}*/

bool g_MemWarningActive = false;
int MemMgr::m_TotalAllocations = 0;
int MemMgr::m_HighestAllocations = 0;
float MemMgr::m_TimePrintAllocations = 0;

void MemMgr::NewAllocation(void *pAddr, size_t size)
{
	m_TotalAllocations++;

	if (g_MemWarningActive)
	{
		if (m_TotalAllocations > m_HighestAllocations)
		{
			g_MemWarningActive = false;
		}
	}
}

void MemMgr::NewDeallocation(void *pAddr)
{
	m_TotalAllocations--;
}

void MemMgr::Think()
{
	float DebugMemValue = EngineFunc::CVAR_GetFloat("ms_debug_mem");
	if (DebugMemValue > 0)
	{
		if (gpGlobals->time > m_TimePrintAllocations)
		{
			Print("Mem Allocations: %i\n", m_TotalAllocations);
			m_TimePrintAllocations = gpGlobals->time + DebugMemValue;
		}
	}

	if (m_TotalAllocations == m_HighestAllocations)
		g_MemWarningActive = true;

	m_HighestAllocations = m_TotalAllocations;
}

void MemMgr::EndMap()
{
	m_TimePrintAllocations = 0;
}