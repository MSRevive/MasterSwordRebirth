/*

	MSMonster.cpp - Extended functionality for Master Sword monsters.

	*included by both dlls*

*/

#include "msdllheaders.h"
#include "hl/monsters.h"
#include "animation.h"
#include "soundent.h"
#include "script.h"
#include "stats/stats.h"
#include "stats/races.h"
#include "store.h"
#include "msitemdefs.h"
#include "stats/statdefs.h"
#include "magic.h"
#include "weapons/genericitem.h"
#include "syntax/syntax.h"
#include "ms/vgui_hud.h"
#include "logger.h"

#ifdef VALVE_DLL

BOOL SameTeam(CBaseEntity *pObject1, CBaseEntity *pObject2);
void AlignToNormal(/*In*/ Vector &vNormal, /*Out*/ Vector &vAngles);
CGenericItem *FindParryWeapon(CMSMonster *pMonster, /*out*/ int &iPlayerHand, /*out*/ int &iAttackNum);
extern int g_SummonedMonsters;
extern int g_netmsg[NETMSG_NUM];
#define MOVE_STEP_SIZE 1
//#define MOVE_STEP_SIZE 32

LINK_ENTITY_TO_CLASS(ms_npc, CMSMonster);
LINK_ENTITY_TO_CLASS(msnpc_human1, CMSMonster);

LINK_ENTITY_TO_CLASS(msmonster_orcwarrior, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_orcarcher, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_orcranger, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_orcberserker, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_bat, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_giantbat, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_boar, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_hawk, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_giantrat, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_giantspider, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_skeleton, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_troll, CMSMonster);
//NOV2014_20 Thothie - adding some new entries for ease of view in Hammer [begin]
LINK_ENTITY_TO_CLASS(msmonster_goblin, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_hobgoblin, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_dwarf, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_wizard, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_bigboar, CMSMonster);
LINK_ENTITY_TO_CLASS(msmonster_giantboar, CMSMonster);
//NOV2014_20 Thothie - adding some new entries for ease of view in Hammer [end]
LINK_ENTITY_TO_CLASS(msmonster_random, CMSMonster); //NOV2014_20 - Thothie msmonster_random
LINK_ENTITY_TO_CLASS(msworlditem_treasure, CMSMonster);

#endif

extern int iBeam;

int CMSMonster::Classify()
{
	return CLASS_NPC;
}

int CMSMonster::IRelationship(CBaseEntity *pTarget)
{
	if (!m_Race[0] || !pTarget || !pTarget->IsMSMonster())
		return RELATIONSHIP_NO;

	CMSMonster *pMonster = (CMSMonster *)pTarget;
	if (!pMonster->m_Race[0])
		return RELATIONSHIP_NO;

	return (int)CRaceManager::Relationship(m_Race, pMonster->m_Race);
}
bool CMSMonster::CanDamage(CBaseEntity *pOther) //Can I damage this entity?
{
	return (CBaseEntity::CanDamage(pOther)) &&		   //The entity can take damage
		   (IRelationship(pOther) <= RELATIONSHIP_NE); //I have a bad or nonexistant releationship with the entity
}

//Set up this monster to spawn at a spawn area
void CMSMonster::Activate()
{
	if (!m_iszMonsterSpawnArea.len())
		return;

	//Thothie JUN2007a
	//- No changes, just note: maybe can fix bug where mon becomes invincible + inifinite XP
	//- If can verify existance of monster spawn here, or here after, somehow
	//-- yes, MiB fixed this, somehow

	edict_t *peSpawnArea = NULL,
			*peFirstArea = NULL;
	CBaseEntity *pSpawnAreaList[255];
	int SpawnsFound = 0;

	while ((peSpawnArea = FIND_ENTITY_BY_TARGETNAME(peSpawnArea, m_iszMonsterSpawnArea)) && SpawnsFound < 255)
	{
		if (peSpawnArea == peFirstArea)
			break;
		else if (!peFirstArea)
			peFirstArea = peSpawnArea;
		CBaseEntity *pSpawnArea = CBaseEntity::Instance(peSpawnArea);
		if (pSpawnArea && (FStrEq(STRING(pSpawnArea->pev->classname), "msarea_monsterspawn") || FStrEq(STRING(pSpawnArea->pev->classname), "ms_monsterspawn")))
			pSpawnAreaList[SpawnsFound++] = pSpawnArea;
	}

	if (SpawnsFound)
	{
		CBaseEntity *pSpawnArea = pSpawnAreaList[RANDOM_LONG(0, SpawnsFound - 1)];
		pSpawnArea->m_pGoalEnt = this;
		pSpawnArea->Activate();
		pev->owner = pSpawnArea->edict(); //Only used for spawn_on_trigger monsters
	}
	else
	{
		ALERT(at_console, "ERROR: msarea_monsterspawn named %s NOT FOUND\n", m_iszMonsterSpawnArea.c_str());
	}

	if (!m_fSpawnOnTrigger)
		SUB_Remove(); //REMOVE_ENTITY( edict() );
}
void CMSMonster::Spawn()
{
	m_OldGold = 0;

	//startdbg;
	//SUB_Remove( ); return;
	//DelayedRemove( );

	//dbg( "Precache" );

	//NOV2014_20 - Thothie msmonster_random [begin]
	if (m_nRndMobs > 0)
	{
		//precache all random mob scripts
		string_i orig_scriptName = m_ScriptName;
		for (int i = 0; i < m_nRndMobs; i++)
		{
			logfile << UTIL_VarArgs("DEBUG: msmonster_random precache #%i / %i as %s\n", i, m_nRndMobs, random_monsterdata[i].m_ScriptName.c_str());
			CScript TempScript;
			TempScript.Spawn(random_monsterdata[i].m_ScriptName, this, this->GetScripted(), true);
			//TempScript.RunScriptEventByName( "game_precache" ); //skipping this, and hoping it behaves, as I'm not sure if it'll remove when done or how to do so

			//dis didn't work
			//m_ScriptName = random_monsterdata[i].m_ScriptName;
			//Precache( );
			//RunScriptEvents( );
		}
		m_ScriptName = orig_scriptName;
	}
	//NOV2014_20 - Thothie msmonster_random [end]

	Precache();

	//dbg( "Setup flags" );

	if (!m_Brush)
	{
		pev->flags |= FL_MONSTER | FL_MONSTERCLIP;
		pev->movetype = MOVETYPE_STEP;
		pev->solid = SOLID_BBOX;
	}
	pev->deadflag = DEAD_NO;
	pev->takedamage = DAMAGE_AIM;
	//pev->solid			= SOLID_SLIDEBOX;
	if (!m_MaxHP)
		pev->health = m_HP = pev->max_health = m_MaxHP = 1;
	m_pAnimHandler = NULL;
	m_Activity = ACT_IDLE;
	m_SpeedMultiplier = 1.0; //Script settable
	m_MaxGroundSlope = .7;
	m_hEnemy = NULL;
	m_LastEnemy = NULL;
	m_LookTime = 0;
	m_NodeCancelTime = 0;
	m_NextNodeTime = 0;
	m_RoamDelay = 2.0f;
	m_StepSize = 18;
	m_TurnRate = .3;
	m_Framerate = 1.0f;
	m_Framerate_Modifier = 1.0f;
	pev->gravity = 1;

	if (!m_ScriptName)
		m_ScriptName = "NPCs/default_human";

	//dbg( "Setup Various" );
	if (pev->targetname)
		m_DisplayName = STRING(pev->targetname); //Default name, overridden by script
	else
		m_DisplayName = "Npc";
	if (!m_Lives)
		m_Lives = -1; //zero == infinite lives
	if (!m_SpawnChance)
		m_SpawnChance = 100.0; //Wasn't specified in map
	if (m_SpawnChance == -1)
		m_SpawnChance = 0.0;
	SetUse(&CMSMonster::Used);

	//dbg( "Create Script" );
	//This loads the script file and precaches all models/sounds it uses
	bool fScriptSpawned = Script_Add(m_ScriptName, this) ? true : false;

	if (!fScriptSpawned)
	{
		SUB_Remove();
		return;
	}

	//dbg( "Delete After Precache" );
	if (g_fInPrecache)
	{
		//This monster was only created to precache its resources
		RunScriptEvents(); //Run scriptevents once to execute any 'precachefile' commands
		SUB_Remove();
		return;
	}
	if (m_iszMonsterSpawnArea.len())
	{
		SetBits(pev->effects, EF_NODRAW);
		return; //wait until Activate(), when everything is spawned
	}

	//dbg( "Create Stats" );
	CreateStats();

	//dbg( "Call Script Event: \"Spawn\"" );
	CScriptedEnt::Spawn();

	//dbg( "Setup BBox" );
	if (!m_Brush)
		if (pev->model)
		{
			UTIL_SetSize(pev, Vector(-(m_Width / 2), -(m_Width / 2), 0), Vector((m_Width / 2), (m_Width / 2), m_Height));
			InitBoneControllers();
			SetAnimation(MONSTER_ANIM_WALK, m_IdleAnim);
		}

	//	TurnRate = .8;//.7
	pev->view_ofs = Vector(0, 0, m_Height);
	m_CurrentHand = LEFT_HAND;
	LastEvent.event = 0;

	////dbg( "Setup Float/fly status" );
	//If I'm floating, slowly lower me to the water surface
	//if( FBitSet(pev->flags, FL_FLOAT) ) { pev->movetype = MOVETYPE_FLY; pev->gravity = 0.5; }
	//if( FBitSet(pev->flags, FL_FLY) ) pev->movetype = MOVETYPE_FLY;
	//FloatTime = gpGlobals->time + 0.1;

	pev->nextthink = gpGlobals->time + 0.1;

	//dbg( "Use Spawn targets" );

	//Thothie OCT2007a - hoping to add this conditional to cut down on null fireevents
	if (this->pev->target)
		SUB_UseTargets(this, USE_TOGGLE, 0);

	//Thothie AUG2007 & SEP2007 - add custom name, hp, and damage multipleirs
	//Defaults
	//dbg( "Set Custom Title" );
	if (m_title.len() <= 0)
		m_title.append("default");
	if (m_addparams.len() <= 0)
		m_addparams.append("none");
	//dbg( "Set Custom DMG Multi" );
	if (m_DMGMulti == 0)
		m_DMGMulti = 1.0;
	//dbg( "Set Custom HP Multi" );
	if (m_HPMulti == 0)
		m_HPMulti = 1.0;

	//ALERT( at_console, "Postspawn Adj Reads: dmg%f hp%f \n", m_DMGMulti, m_HPMulti);

	//dbg( "Set Custom Params" );
	msstringlist Parameters;
	Parameters.add(m_title);
	Parameters.add(FloatToString(m_DMGMulti));
	Parameters.add(FloatToString(m_HPMulti));
	Parameters.add(m_addparams);

	CallScriptEvent("game_postspawn", &Parameters);
	//enddbg;
}
void CMSMonster::Precache()
{
	CBaseMonster::Precache();
}

void CMSMonster::KeyValue(KeyValueData *pkvd)
{
	msstring randomdata = pkvd->szKeyName ? pkvd->szKeyName : "null"; //NOV2014_20 - Thothie msmonster_random

	if (FStrEq(pkvd->szKeyName, "killtarget"))
	{
		m_iszKillTarget = pkvd->szValue;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "perishtarget"))
	{
		m_iszPerishTarget = pkvd->szValue;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnarea"))
	{
		m_iszMonsterSpawnArea = pkvd->szValue;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "delaylow"))
	{
		m_SpawnDelayLow = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "delayhigh"))
	{
		m_SpawnDelayHigh = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lives"))
	{
		m_Lives = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnchance"))
	{
		//m_SpawnChance = min( 100, max( 0, atof( pkvd->szValue ) ) );
		if (0 > atof(pkvd->szValue))
			m_SpawnChance = 0;
		else if (100 < atof(pkvd->szValue))
			m_SpawnChance = 100;
		else
			m_SpawnChance = atof(pkvd->szValue);

		if (!m_SpawnChance)
			m_SpawnChance = -1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnstart"))
	{
		m_fSpawnOnTrigger = (atoi(pkvd->szValue)) ? true : false;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "nplayers"))
	{
		//Thothie - AUG2007a - adding option to only spawn monster if # players present
		m_ReqPlayers = (atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "reqhp"))
	{
		//Thothie - AUG2007a - adding option to only spawn monster if server's total HP equals
		//m_HPReq = (atoi(pkvd->szValue));
		msstringlist reqhp_stringlist;
		TokenizeString((pkvd->szValue), reqhp_stringlist);
		m_HPReq_min = 0;
		m_HPReq_max = 0;
		m_HPReq_useavg = false;
		if (reqhp_stringlist.size() > 0)
			m_HPReq_min = atoi(reqhp_stringlist[0].c_str());
		if (reqhp_stringlist.size() > 1 && !reqhp_stringlist[1].contains("avg"))
		{
			m_HPReq_max = atoi(reqhp_stringlist[1].c_str());
			if (m_HPReq_max < m_HPReq_min)
			{
				m_HPReq_max = 0;
				logfile << "MAP_ERROR: " << m_Scripts[0]->m.ScriptFile.c_str() << " - max reqhp set higher than min.\n";
			}
			//else if ( m_HPReq_min == 0 ) m_HPReq_min = 1; //NOV2014_20 - this may fux with things if all players are flagged AFK - fixed in msarea_monsterspawn
		}
		if (reqhp_stringlist.size() > 2 || reqhp_stringlist[1].contains("avg")) 
			m_HPReq_useavg = true; //Thothie OCT2015_28 - allow use average when calculating HP req, if token 2-3 is "avg"
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "title"))
	{
		//Thothie - AUG2007b - adding option to change monster name
		m_title = pkvd->szValue;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "params"))
	{
		//Thothie - DEC2007b - adding option to send params to script
		m_addparams = pkvd->szValue;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "hpmulti"))
	{
		//Thothie - SEP2007a - multiply hp by this amount
		float thoth_inhealth = atof(pkvd->szValue);
		if (thoth_inhealth > 1)
		{
			m_HPMulti = thoth_inhealth;
			pkvd->fHandled = TRUE;
		}
	}
	else if (FStrEq(pkvd->szKeyName, "dmgmulti"))
	{
		//Thothie - SEP2007a - multiply damage by this amount
		float thoth_invar = atof(pkvd->szValue);
		if (thoth_invar > 1)
		{
			m_DMGMulti = thoth_invar;
			pkvd->fHandled = TRUE;
		}
	}
	else if (FStrEq(pkvd->szKeyName, "scriptfile") ||
			 (FStrEq(pkvd->szKeyName, "defscriptfile") && !m_ScriptName))
	{
		m_ScriptName = pkvd->szValue;
		pkvd->fHandled = TRUE;
	}
	//NOV2014_20 - Thothie msmonster_random [begin]
	else if (randomdata.starts_with("random_")) //optimize this - dis is stupid lazy
	{
		//Some of this requires the mapper not to fug up, and thus doesn't error check well

		//get mob idx we're adding to based on property name
		//not the most efficient way to do this...
		static msstringlist Tokens;
		Tokens.clearitems();
		TokenizeString(randomdata, Tokens, "_"); //not sure if we can use semi-colons in properties - would look icky anyways
		int idx = atoi(Tokens[1].c_str()) - 1;
		msstring rndproperty = Tokens[2];
		if (random_monsterdata.size() == 0)
		{
			//random_monster_t tmp;
			for (int i = 0; i < 32; i++)
			{
				random_monsterdata.add_blank();
			};
			logfile << UTIL_VarArgs("DEBUG: msmonster_random spaces for new mob idx %i - array size after %i\n", idx, (int)random_monsterdata.size());
		}
		if (rndproperty == "title")
			random_monsterdata[idx].m_title = pkvd->szValue;
		if (rndproperty == "params")
			random_monsterdata[idx].m_addparams = pkvd->szValue;
		if (rndproperty == "scriptfile")
		{
			//is required, and only one of each, so add count here
			random_monsterdata[idx].m_ScriptName = pkvd->szValue;
			if (m_nRndMobs == 0)
				m_nRndMobs = 1;
			else
				++m_nRndMobs;
			logfile << UTIL_VarArgs("DEBUG: msmonster_random added new mob #%i = %s \n", idx, random_monsterdata[idx].m_ScriptName.c_str());
		}
		if (rndproperty == "hpmulti")
			random_monsterdata[idx].m_HPMulti = atof(pkvd->szValue);
		if (rndproperty == "dmgmulti")
			random_monsterdata[idx].m_DMGMulti = atof(pkvd->szValue);
		//if ( rndproperty == "lives" ) random_monsterdata[idx].m_Lives = atoi(pkvd->szValue);
		if (rndproperty == "nplayers")
			random_monsterdata[idx].m_ReqPlayers = atoi(pkvd->szValue);
		//sticky bit
		if (rndproperty == "reqhp")
		{
			msstringlist reqhp_stringlist;
			TokenizeString((pkvd->szValue), reqhp_stringlist);
			int mrand_m_HPReq_min = 0;
			int mrand_m_HPReq_max = 0;
			bool rand_m_HPReq_useavg = false; //Thothie OCT2015_28 - allow use average when calculating HP req, if token 2-3 is "avg"
			if (reqhp_stringlist.size() > 0)
				mrand_m_HPReq_min = atoi(reqhp_stringlist[0].c_str());
			if (reqhp_stringlist.size() > 1 && !reqhp_stringlist[1].contains("avg"))
			{
				mrand_m_HPReq_max = atoi(reqhp_stringlist[1].c_str());
				if (mrand_m_HPReq_max < mrand_m_HPReq_min)
				{
					mrand_m_HPReq_max = 0;
					logfile << Logger::LOG_WARN << "MAP_ERROR: " << STRING(random_monsterdata[idx].m_ScriptName) << " - max reqhp set higher than min.\n";
				}
				//else if ( mrand_m_HPReq_min == 0 ) mrand_m_HPReq_min = 1; //NOV2014_20 - this may fux with things if all players are flagged AFK - fixed in msarea_monsterspawn
			}

			if (reqhp_stringlist.size() > 2 || reqhp_stringlist[1].contains("avg")) 
				rand_m_HPReq_useavg = true; //Thothie OCT2015_28 - allow use average when calculating HP req, if token 2-3 is "avg"

			random_monsterdata[idx].m_HPReq_min = mrand_m_HPReq_min;
			random_monsterdata[idx].m_HPReq_max = mrand_m_HPReq_max;
			random_monsterdata[idx].m_HPReq_useavg = rand_m_HPReq_useavg;
		}
		//Gotta use the logfile here, dernitall
		logfile << UTIL_VarArgs("DEBUG: msmonster_random added rndproperty #%i / tot %i - %s %s\n", idx, m_nRndMobs, rndproperty.c_str(), pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	//NOV2014_20 - Thothie msmonster_random [end]
	else
		CBaseMonster::KeyValue(pkvd);
}

//
//	Think
//	�����
void CMSMonster ::Think()
{
	pev->vuser3.x = MaxHP();
	pev->vuser3.y = (IsAlive() ? pev->health : 0);
	pev->vuser3.z = 0.0f;

	//dbg( "Start" );
	pev->ltime = gpGlobals->time;
	pev->nextthink = gpGlobals->time + 0.1;

	//if( pev->framerate > 1 )

	float flInterval = 0;
	//dbg( "StudioFrameAdvance" );
	if (IsAlive())
		flInterval = StudioFrameAdvance();

	startdbg; //MAR2008b - Skipping StudioFrameAdvance errors for now

	dbg("m_pfnThink");

	if (m_pfnThink)
	{
		CBaseEntity::Think();
		return;
	}

	dbg("!IsAlive()");

	if (!IsAlive())
		return;

	dbg("pev->button");

	pev->button = 0;

	//Ignore normal script events when possessed by an NPC script ent
	dbg("Run script events");
	if (!HasConditions(MONSTER_NOAI))
		RunScriptEvents();
	//Only run timed named events when this flag is set
	else
		RunScriptEvents(true);

	if (!pev->model)
	{
		return;
	}

	Act();

	dbg("Look");
	if (!HasConditions(MONSTER_BLIND))
		Look();

	if (!HasConditions(MONSTER_NOAI))
	{
		dbg("ListenForSound");
		ListenForSound();
	}

	dbg("Trade");
	Trade();

	dbg("SetSpeed");
	SetSpeed();

	dbg("SetMoveDest");
	SetMoveDest();

	dbg("SetWanderDest");
	SetWanderDest();

	//Clear out single-frame events
	switch (LastEvent.event)
	{
	case 600:
		LastEvent.event = 0;
	}
	dbg("DispatchAnimEvents");
	DispatchAnimEvents();

	dbg("Play Movement Anim");

	//NPCScripts override the monster animation
	if (m_MonsterState != MONSTERSTATE_SCRIPT)
	{
		if (HasConditions(MONSTER_HASMOVEDEST)) //I have a movement destination, play my movement anim
			SetAnimation(MONSTER_ANIM_WALK, m_MoveAnim);
		else if (m_IdleAnim.len())
			//if( m_pAnimHandler == &gAnimWalk ) pev->frame = 0; //Repeat the idle
			SetAnimation(MONSTER_ANIM_WALK, m_IdleAnim);

		else if (m_OldActivity != m_Activity || m_fSequenceFinished)
		{
			m_pAnimHandler = NULL;
			SetActivity(ACT_IDLE);
		}

		m_OldActivity = m_Activity;
	}

	dbg("Move");
	Move(flInterval);

	//Need to check if I'm alive again, because the above MoveExecute will
	//sometimes call the touch function of a projectile that kills me.
	if (!IsAlive())
		return;

	if (m_hEnemy != NULL)
	{
		if (!FVisible(m_hEnemy))
			m_hEnemy = NULL;
	}

	Float(); //Special velocity for floating monsters

	dbg("Talk");
	Talk();
	int TempConditions = FrameConditions;
	FrameConditions = 0;
	if (TempConditions & FC_AVOID)
		FrameConditions |= FC_AVOID;

	enddbgprt(m_ScriptName.c_str());
}
/*int CMSMonster :: MoveExecute ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist, bool fTestMove )
{
	Vector	vecStartPos,// Record monster's position before trying the move
			vecMove;    // Direction to move
	float	flDist, flStep, stepSize;
	int		iReturn;//, iMoveType = fTestMove ? WALKMOVE_CHECKONLY : WALKMOVE_NORMAL;
	bool	fIsFlying = IsFlying() ? true : false;
	float   flTotalMoved = 0;

	vecStartPos = pev->origin;
	
	//if( fIsFlying )
		vecMove = (vecEnd - vecStart).Normalize();
	//else flYaw = UTIL_VecToYaw ( vecEnd - vecStart );// build a yaw that points to the goal.
	flDist = ( vecEnd - vecStart ).Length();// get the distance.
	iReturn = LOCALMOVE_VALID;// assume everything will be ok.

	// move the monster to the start of the local move that's to be checked.
	if( fTestMove ) UTIL_SetOrigin( pev, vecStart );

	int iFlags = pev->flags;      //Make all monsters flying temporarily
	SetBits( pev->flags, FL_FLY );//so that they will drop off cliffs
	int MoveType = pev->movetype;
	pev->movetype = MOVETYPE_FLY;

	// Take single steps to the goal.
	for ( flStep = 0 ; flStep < flDist ; flStep += stepSize )
	{
		stepSize = (flStep + MOVE_STEP_SIZE) < flDist ? MOVE_STEP_SIZE : (flDist - flStep);

		bool fMoveSuccess;
		Vector vBeforeMove = pev->origin;
		UTIL_MoveToOrigin( edict(), pev->origin + vecMove * flDist, stepSize, MOVE_STRAFE );
		fMoveSuccess = (vBeforeMove - pev->origin).Length() ? true : false;

		//See if I can step up onto this surface
		if( !fMoveSuccess && !fIsFlying && vecMove.z > -1 ) 
		{
			Vector FlyStartOrigin = pev->origin;
			Vector FlyDestOrigin = pev->origin + Vector(0,0,m_StepSize);
			UTIL_MoveToOrigin( edict(), FlyDestOrigin, m_StepSize, MOVE_STRAFE );	//Check if I hit the roof on the way up
			float move = (FlyDestOrigin - pev->origin).Length2D();
			fMoveSuccess = move ? false : true;
			if( fMoveSuccess )
			{
				Vector UpStartMove = pev->origin;
				//UTIL_SetOrigin( pev, pev->origin );
				UTIL_MoveToOrigin( edict(), pev->origin + vecMove * flDist, stepSize, MOVE_STRAFE );
				move = (UpStartMove - pev->origin).Length2D();
				fMoveSuccess = (move >= stepSize * 0.5)  ? true : false;
				//Print( "%.2f\n", move );
				if( fMoveSuccess )
				{
					Vector BeforeAngles = pev->angles;
					FaceForward( );

					//Print( "%.2f\n", m_GroundNormal.z );
					if( m_GroundNormal.z > m_MaxGroundSlope )
						DROP_TO_FLOOR( edict() );
					else
					{
						fMoveSuccess = false;
						pev->origin = vBeforeMove;
						pev->angles = BeforeAngles;
					}
				}
				else 
					pev->origin = vBeforeMove;
			}
			else
				pev->origin = vBeforeMove;
		}

		flTotalMoved += (vBeforeMove - pev->origin).Length();

		//if ( !fMoveSuccess )
		if ( !fMoveSuccess && fabs(flDist-flTotalMoved) > flDist * 0.1 ) //Not within 10% of target
		{// can't take the next step, fail!

			if ( pTarget && pTarget->edict() == gpGlobals->trace_ent )
			{
				// if this step hits target ent, the move is legal.
				iReturn = LOCALMOVE_VALID;
				break;
			}
			else if( pTarget )
			{
				// If we're going toward an entity, and we're almost getting there, it's OK.
				if ( pTarget && fabs( flDist - flStep ) < MOVE_STEP_SIZE )
					iReturn = LOCALMOVE_VALID;
				else iReturn = LOCALMOVE_INVALID;
				break;
			}

			iReturn = LOCALMOVE_INVALID;
			break;
		}
	}
	
	// since we've actually moved the monster during the check, undo the move.
	pev->flags = iFlags;
	pev->movetype = MoveType;
	if( fTestMove ) UTIL_SetOrigin( pev, vecStartPos );
	if( pflDist ) *pflDist = flTotalMoved;

	return iReturn;
}*/
bool CMSMonster ::MoveExecute(moveexec_t &MoveExec)
{
	Vector OrigOrigin = pev->origin;
	Vector OrigAngles = pev->angles;
	int OrigFlags = pev->flags;
	int OrigMovetype = pev->movetype;
	Vector CurrentPos = OrigOrigin;
	Vector PrevPos = CurrentPos;
	float TotalDist = (MoveExec.vecEnd - MoveExec.vecStart).Length();
	float TotalDistThreshold = TotalDist * 0.1; //Must move to within 10% of goal
	TraceResult tr;
	//SetBits( pev->flags, FL_FLY );
	//pev->movetype = MOVETYPE_FLY;
	bool fContinue = false, fSuccess = false;
	int LoopTimes = 0;
	bool fIsFlying = IsFlying() ? true : false;

	MoveExec.Return_DistMoved = 0;
	MoveExec.Return_HitTargetEnt = false;
	MoveExec.Return_HitStep = false;

	do
	{
		LoopTimes++;
		fContinue = false; //Set this each loop
		PrevPos = CurrentPos;
		Vector MoveDir = (MoveExec.vecEnd - CurrentPos).Normalize();
		TRACE_MONSTER_HULL(edict(), CurrentPos, MoveExec.vecEnd, dont_ignore_monsters, edict(), &tr);

		bool HitTargetEnt = (MoveExec.pTarget && MSInstance(tr.pHit) == MoveExec.pTarget);

		if ((tr.pHit || tr.flFraction < 1) && !HitTargetEnt)
		{
			fSuccess = false;

			if (MoveExec.fAllowStep)
			{
				//CurrentPos += MoveDir * tr.flFraction * 0.9;

				//See if I can step up onto this surface
				TraceResult FlyTr;
				FlyTr.pHit = NULL;
				Vector FlyOrigin = CurrentPos + Vector(0, 0, m_StepSize);
				TRACE_MONSTER_HULL(edict(), CurrentPos, FlyOrigin, dont_ignore_monsters, edict(), &FlyTr);
				if (FlyTr.flFraction >= 1 && !FlyTr.pHit)
				{
					Vector FlyDest = MoveExec.vecEnd + Vector(0, 0, m_StepSize);
					TRACE_MONSTER_HULL(edict(), FlyOrigin, FlyDest, dont_ignore_monsters, edict(), &FlyTr);
					if (FlyTr.flFraction >= 1 && !FlyTr.pHit)
					{
						Vector BeforeAngles = pev->angles;
						Vector BeforeNormal = m_GroundNormal;
						pev->origin = FlyDest;
						FaceForward();

						//Print( "%.2f\n", m_GroundNormal.z );
						if (m_GroundNormal.z > m_MaxGroundSlope)
						{
							//Stepped up onto surface.
							MoveExec.Return_HitStep = true;
							CurrentPos = FlyDest;
							fSuccess = true;
							//fContinue = true;
						}
						else
						{
							//Slope too high
							m_GroundNormal = BeforeNormal;
							pev->angles = BeforeAngles;
							pev->origin = OrigOrigin;
							fSuccess = false; //Blocked and stepped up onto the object but new ground slope is too high
						}
					}
					else
						fSuccess = false; //Blocked, moved up, but couldn't move forward, on top of the object
				}
				else
					fSuccess = false; //Blocked, and couldn't move up to step over it

				//}
				//else
				//Move was within threshold.  Consider it a success
				//	fSuccess = true;
			}
		}
		else
		{
			// Made it all the way to goal, or hit target entity
			// If I hit the entity, then don't move, but return success
			if (!HitTargetEnt)
			{
				CurrentPos = MoveExec.vecEnd;
				MoveExec.Return_DistMoved = (MoveExec.vecEnd - MoveExec.vecStart).Length();
			}
			else
				MoveExec.Return_HitTargetEnt = true;

			fSuccess = true;
		}
	} while (fContinue && LoopTimes < 4 && !fSuccess); //restored DO/While MAR2012

	pev->flags = OrigFlags;
	pev->movetype = OrigMovetype;

	if (fSuccess && !MoveExec.fTestMove)
	{
		pev->origin = CurrentPos;
		UTIL_SetOrigin(pev, pev->origin);

		if (MoveExec.Return_HitStep)
			DROP_TO_FLOOR(edict());
	}

	return fSuccess;
}
BOOL CMSMonster::CanSetVelocity() { return TRUE; }

void CMSMonster::Float()
{
	/*#ifdef VALVE_DLL
	if( !FBitSet(pev->flags, FL_FLOAT ) ) return;
	if( gpGlobals->time < FloatTime ) return;

	FloatTime = gpGlobals->time + 2; //default

	CBaseEntity *pWater = UTIL_FindEntityByClassname( NULL, "func_water" );
	while( pWater ) {
		if( Intersects( pWater ) ) {
			if( Center().z - Height/4 < pWater->pev->absmax.z ) {
				pev->velocity.z = 30;
				pev->movetype = MOVETYPE_FLY;
				FloatTime = gpGlobals->time + 0.1;
			}
			else {
				pev->velocity.z = 0;
				FloatTime = gpGlobals->time + 3;
			}
		}
		pWater = UTIL_FindEntityByClassname( pWater, "func_water" );
	}
#endif*/
}

void CMSMonster ::Touch(CBaseEntity *pOther)
{
	//	if( FBitSet(pev->flags,FL_FLOAT) ) ALERT( at_console, "Touching %s\n", STRING(pOther->pev->classname) );
	CBaseEntity ::Touch(pOther);
	if (m_HandleTouch && (pOther->IsPlayer() || pOther->IsMSMonster()))
	{
		msstringlist Parameters;
		Parameters.add(EntToString(pOther));
		CallScriptEvent("game_touch", &Parameters);
	}
}
void CMSMonster::Look()
{

	if (gpGlobals->time < m_LookTime)
		return;

	// Look around for the best target
	CBaseMonster ::Look(1024); //was 512

	CBaseEntity *pEnt = m_pLink; //Save seen targets
	m_EnemyListNum = 0;
	while (pEnt && m_EnemyListNum < MAX_ENEMYLIST)
	{
		m_hEnemyList[m_EnemyListNum] = pEnt;
		m_EnemyListNum++;
		pEnt = pEnt->m_pLink;
	}
	m_LookTime = gpGlobals->time + 0.5; //1.0
}
void CMSMonster::ListenForSound()
{
	startdbg;

	if (gpGlobals->time < m_ListenTime)
		return;

	CBaseMonster ::Listen();

	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		int iThisSound = m_iAudibleList;
		CSound *pSound = NULL;
		msstringlist Parameters;

		while (iThisSound != SOUNDLIST_EMPTY)
		{
			pSound = CSoundEnt::SoundPointerForIndex(iThisSound);

			if ((pSound != NULL) && (bool)pSound->m_SrcEntity)
			{
				StoreEntity(pSound->m_SrcEntity.Entity(), ENT_LASTHEARD);
				Parameters.clearitems();
				Parameters.add(pSound->m_Type);
				Parameters.add(VecToString(pSound->m_vecOrigin));
				Parameters.add(UTIL_VarArgs("%.2f", pSound->m_DangerRadius));
				CallScriptEvent("game_heardsound", &Parameters);
			}

			if (pSound)
				iThisSound = pSound->m_iNextAudible;
		}

		/*if ( iBestSound >= 0 )
		{
			pSound = CSoundEnt::SoundPointerForIndex( iBestSound );
			return pSound;
		}
	#if _DEBUG
		ALERT( at_error, "NULL Return from PBestSound\n" );
	#endif
		return NULL;*/
		/*CSound *pSound;
		
		if( pSound = PBestSound( ) )
		{
			bool fCheckSound = true;

			//if( CRaceManager::Relationship( m_Race, pSound->m_Race ) != RELATIONSHIP_NO )
			//	fCheckSound = false;

			if( m_hEnemy != NULL ) fCheckSound = false;

			if( fCheckSound ) 
			{
				//Create a temporary entity for the script so it can react to the sound properly
				relationship_e RelationShip = CRaceManager::Relationship( m_Race, pSound->m_Race );
				msstring_ref pszRelationShip = "neutral";
				if( RelationShip > RELATIONSHIP_NO ) pszRelationShip = "ally";
				else if( RelationShip < RELATIONSHIP_NO ) pszRelationShip = "enemy";
				SetScriptVar( "game.monster.lastsound.relationship", pszRelationShip );

				CBaseEntity *pSoundEnt = GetClassPtr( (CBaseEntity *)NULL );
				pSoundEnt->pev->origin = pSound->m_vecOrigin;

				StoreEntity( pSoundEnt, ENT_LASTHEARD );
				msstringlist Parameters;
				Parameters.add( );
				CallScriptEvent( "game_heardsound" );
				pSoundEnt->SUB_Remove( );

				//m_MoveDest.Origin = pSound->m_vecOrigin;
				//m_MoveDest.Proximity = 9999.9f;
				//m_NodeCancelTime = 0.5;
				//LookTime = 0;
			}
		}*/
	}

	m_ListenTime = gpGlobals->time + 1.0;

	enddbg;
}
void CMSMonster::SetMoveDest()
{
	if (!HasConditions(MONSTER_HASMOVEDEST))
		return;
	if (m_MonsterState == MONSTERSTATE_SCRIPT && m_Activity != ACT_WALK)
		return;
	if (m_Activity == ACT_SPECIAL_ATTACK1 || m_Activity == ACT_MELEE_ATTACK1)
		return;
	if (pev->movetype == MOVETYPE_FOLLOW || pev->movetype == MOVETYPE_NONE)
		return;

	Vector m_vecTarget;

	//Might have to put this visibility check back
	/*if( !FVisible( m_vecEnemyLKP ) ) {
		//ALERT( at_console, "%s: Move dest NOT VISIBLE!\n", STRING(DisplayName) );
		ClearConditions( MONSTER_HASMOVEDEST );
		m_NextNodeTime = gpGlobals->time + 2.0; //wait 2 seconds before wandering
		return;
	}*/

	m_vecTarget = m_MoveDest.Origin - EyePosition();
	Vector ExactAngle = UTIL_VecToAngles(m_vecTarget);

	//Are we close enough?
	if ((IsFlying() ? m_vecTarget.Length() : m_vecTarget.Length2D()) <= m_MoveDest.Proximity)
	{
		//		ALERT( at_console, "%s: Within Prox(%f) of target.\n", STRING(DisplayName), MoveProximity );
		pev->v_angle = ExactAngle;
		pev->v_angle.x *= -1;
		pev->angles.y = pev->v_angle.y;
		//if( IsFlying() ) pev->angles.x = pev->v_angle.x;
		//FaceDirection( UTIL_VecToAngles(m_vecEnemyLKP - EyePosition()) );
		StopWalking();
		CallScriptEvent("game_reached_dest");
		return;
	}

	//TODO: Turn the correct way (right or left) based on shortest turn
	if (!FBitSet(FrameConditions, FC_AVOID))
	{
		float FullTurn = UTIL_AngleDiff(ExactAngle.y, pev->angles.y);
		if (fabs(UTIL_AngleDiff(ExactAngle.y, pev->angles.y)) > 6)
		{
			float PartialTurn = FullTurn * m_TurnRate;
			pev->angles.y += PartialTurn;
			if (fabs(UTIL_AngleDiff(ExactAngle.y, pev->angles.y)) <= 6)
				pev->angles.y = ExactAngle.y;
		}
	}
	pev->v_angle.x = -ExactAngle.x;
	pev->v_angle.y = pev->angles.y;
	pev->v_angle.z = 0;
	m_Activity = ACT_WALK;
	//if( IsFlying() ) pev->angles.x = pev->v_angle.x;
	static msstringlist Parameters; //Static, for speed
	Parameters.clearitems();
	Parameters.add(VecToString(ExactAngle));
	CallScriptEvent("game_movingto_dest", &Parameters);
}
void CMSMonster::SetWanderDest()
{
	startdbg;
	//No enemy, walk around randomly (here m_vecEnemyLKP is the random place to walk)
	if (!HasConditions(MONSTER_ROAM))
		return;

	//	if( gpGlobals->time < MoveTime ) return;

	Vector m_vecTarget, VecDest;
	TraceResult tr;

	dbg("Check m_NodeCancelTime");
	//Canel a movedest if we've been trying for too long
	if (!m_NextNodeTime && gpGlobals->time >= m_NodeCancelTime)
		m_NextNodeTime = gpGlobals->time + m_RoamDelay;

	dbg("Check m_NextNodeTime");
	if (m_NextNodeTime && gpGlobals->time >= m_NextNodeTime)
	{
		//Find another random spot to walk to
		int count = 0;
		float Proximity = GetDefaultMoveProximity();
		bool FoundPath = false;
		float NewYaw;
		m_hEnemy = NULL;
		Vector vForward;
		dbg("Check 15 random move dirs");
		while (count < 15)
		{
			NewYaw = UTIL_AngleMod(pev->angles.y + RANDOM_FLOAT(-130, 130));
			UTIL_MakeVectorsPrivate(Vector(0, NewYaw, 0), vForward, NULL, NULL);
			VecDest = pev->origin + vForward * (Proximity * 3);
			UTIL_TraceLine(EyePosition(), VecDest, dont_ignore_monsters, edict(), &tr);
			if (tr.flFraction >= 1.0)
			{
				//AUG2013_22 Thothie - Fishies stay in water
				if (pev->flags & (FL_SWIM))
				{
					if (EngineFunc::Shared_PointContents(VecDest) == CONTENTS_WATER)
					{
						FoundPath = true;
						break;
					}
				}
				else
				{
					FoundPath = true;
					break;
				}
			}
			count++;
		}
		if (!FoundPath)
		{
			dbg("Check 360 sequential move dirs");
			//Couldn't randomly find an angle, so brute force one
			for (count = 0; count < 360; count++)
			{
				NewYaw = count;
				UTIL_MakeVectorsPrivate(Vector(0, NewYaw, 0), vForward, NULL, NULL);
				VecDest = Center() + vForward * (Proximity * 3);
				UTIL_TraceLine(EyePosition(), VecDest, dont_ignore_monsters, edict(), &tr);
				if (tr.flFraction >= 1.0)
				{
					//AUG2013_22 Thothie - Fishies stay in water
					if (pev->flags & (FL_SWIM))
					{
						if (EngineFunc::Shared_PointContents(VecDest) == CONTENTS_WATER)
						{
							FoundPath = true;
							break;
						}
					}
					else
					{
						FoundPath = true;
						break;
					}
				}
			}
			if (!FoundPath)
			{
				//ALERT( at_console, "Monster: %s is UNFIXABLY STUCK!\n", STRING(DisplayName) );
				m_NextNodeTime = gpGlobals->time + 10.0; //big delay
			}
		}
		dbg("Check if path found");
		if (FoundPath)
		{
			//pev->angles.x = 0; pev->angles.z = 0;
			dbg("Setup wander variables");
			m_MoveDest.Origin = EyePosition() + vForward * RANDOM_FLOAT(300, 6000);
			m_MoveDest.Proximity = GetDefaultMoveProximity();
			m_Wandering = true;
			m_NodeCancelTime = gpGlobals->time + 7.0;
			SetConditions(MONSTER_HASMOVEDEST);
			m_NextNodeTime = 0;
			m_Activity = ACT_WALK;
			dbg("CallEvent game_wander");
			CallScriptEvent("game_wander");
		}
	}
	enddbg;
}

Vector StartAng;
void CMSMonster::Move(float flInterval)
{
	startdbg;
	int Side[2] = {1, -1};

	if (pev->movetype == MOVETYPE_FOLLOW || pev->movetype == MOVETYPE_NONE)
		return;

	//Get the desired direction
	m_MoveDir = (m_MoveDest.Origin - pev->origin).Normalize();

	//Turn to face my ideal direction
	if (HasConditions(MONSTER_HASMOVEDEST))
	{
		if (!IsFlying())
			pev->angles.y = UTIL_VecToAngles(m_MoveDir).y;
		else
		{
			Vector Angle = UTIL_VecToAngles(m_MoveDir);
			pev->angles = Angle;
			pev->v_angle = Angle;
			pev->v_angle.x = -pev->v_angle.x;
		}
	}

	if (pev->angles.y != m_LastYaw //I turned or moved, now check the ground under me for my proper pitch/roll orientation
		|| m_LastOrigin != pev->origin)
		FaceForward();

	dbg("Set Movement velocity");
	if (m_flGroundSpeed)
	{
		if (CanSetVelocity())
		{
			Vector vTargetAng = IsFlying() ? pev->v_angle : Vector(-pev->angles.x, pev->angles.y, pev->angles.z);
			UTIL_MakeVectorsPrivate(vTargetAng, m_MoveDir, NULL, NULL);

			float ScriptMultiplier = 1.0f;
			if (pev->maxspeed)
				ScriptMultiplier = pev->maxspeed / 100.0f;

			float flTotal = m_flGroundSpeed * pev->framerate * flInterval * m_SpeedMultiplier * ScriptMultiplier;
			float flMoved = 0;

			moveexec_t MoveExec;
			MoveExec.fTestMove = false;
			MoveExec.vecStart = pev->origin;
			MoveExec.vecEnd = pev->origin + m_MoveDir * flTotal;
			MoveExec.pTarget = m_MoveDest.MoveTarget.Entity();
			MoveExec.fAllowStep = true;

			//MIB MAR2012 game_stuck (fail, false positives)
			if (!MoveExecute(MoveExec) && !MoveExec.Return_HitTargetEnt)
			{
				AvoidFrontObject(flTotal); //Turn to avoid object
				Vector vTargetAng = IsFlying() ? pev->v_angle : Vector(-pev->angles.x, pev->angles.y, pev->angles.z);
				UTIL_MakeVectorsPrivate(vTargetAng, m_MoveDir, NULL, NULL);
				MoveExec.vecEnd = pev->origin + m_MoveDir * flTotal;
				MoveExecute(MoveExec);

				//Do move again
				/*
				if ( !MoveExecute( MoveExec ) )
				{
					// If it STILL didn't work, apparently we have a problem. Likely stuck in something
					// Alert the scripts
					CallScriptEvent( "game_stuck" );
				}
	            */

				// MiB - This is redundant, as MoveExecute does it. Saving a very tiny bit of processing.
				//UTIL_SetOrigin( pev, pev->origin );
			}

			//if( pev->gravity == 1 )
			//	Print( "%.2f %.2f %.2f %.2f %i\n", m_flGroundSpeed, pev->origin.x, pev->origin.y, flMoved, FBitSet(pev->flags,FL_ONGROUND) );
		}
	}

	//Gravity has to be done manually...
	dbg("Gravity");
	if (!IsFlying() && !FBitSet(FrameConditions, FC_STEP))
	{
		bool fAddGravity = true;

		//Run this to set the proper FL_ONGROUND. The NPC doesn't move
		//fAddGravity = WALK_MOVE( ENT(pev), pev->angles.y, 1, WALKMOVE_NORMAL ) ? true : false;
		//pev->velocity.z += 0.0001f;

		if (1) //Manual onground check
		{
			int FallCheckAmt = 3;
			//int oldflags = pev->flags;
			//pev->flags |= FL_FLY;

			/*float flMoved;
			MoveExecute( pev->origin, pev->origin + Vector( 0, 0, -FallCheckAmt ), NULL, &flMoved, true );
			//pev->flags = oldflags;

			//if( flMoved < FallCheckAmt ) fAddGravity = false;
			//if( flMoved < FallCheckAmt ) SetBits( pev->flags, FL_ONGROUND );
			if( flMoved >= FallCheckAmt ) ClearBits( pev->flags, FL_ONGROUND );*/

			ClearBits(pev->flags, FL_ONGROUND);
		}

		//if( !FBitSet(pev->flags,FL_ONGROUND) )
		//if( fAddGravity )
		//{
		//	pev->velocity.z -= CVAR_GET_FLOAT("sv_gravity") * 0.5
		//						* (pev->gravity ? pev->gravity : 1)
		//						* (gpGlobals->frametime);
		//}
	}

	m_LastYaw = pev->angles.y;
	m_LastOrigin = pev->origin;

	enddbg;
}
void CMSMonster::AvoidFrontObject(float MoveAmt)
{
	static int Side[2] = {1, -1};

	float flmdist = 0;
	ClearBits(FrameConditions, FC_AVOID);
	if (m_Activity == ACT_WALK && HasConditions(MONSTER_HASMOVEDEST) && m_flGroundSpeed)
	{
		//Run avoidance AI before stopping

		bool fShouldStop = true;

		Vector vOriginSave = pev->origin;
		float flYawSave = StartAng.y;

		//Use the angle on either side for which i have to make the smallest turn
#define TURN_INCREMENT 15
		int Increment = TURN_INCREMENT;
		for (int ang = 0; ang <= 170; ang += Increment) //, Increment *= 4 )   //MiB MAR2012_19 - Doing a solid 15 degree increment. We were getting false positives with small angles
		{

			Vector vAng, vAvoidDir, SideAng[2];
			float Dist[2];
			for (int i = 0; i < 2; i++) //Try each side
			{
				//Finds the forward vector of the avoidance direction
				vAng.y = UTIL_AngleMod(m_LastYaw + ang * -Side[i]);
				AlignToNormal(m_GroundNormal, vAng);
				UTIL_MakeVectorsPrivate(vAng, vAvoidDir, NULL, NULL);

				//Check for avoidance space
				moveexec_t MoveExec;
				MoveExec.fTestMove = true;
				MoveExec.vecStart = pev->origin;
				MoveExec.vecEnd = pev->origin + vAvoidDir * MoveAmt;
				MoveExec.pTarget = m_MoveDest.MoveTarget.Entity();
				MoveExec.fAllowStep = true;

				bool moved = MoveExecute(MoveExec) ? true : false;

				//if( fltestmovedist > flmdist )
				if (moved)
				{
					//Dist[i] = MoveExec.Return_DistMoved;
					Dist[i] = (m_MoveDest.Origin - MoveExec.vecEnd).Length();
					SideAng[i] = vAng;
				}
				else
					Dist[i] = -1;
			}

			float flNewYaw = 0;
			if (Dist[0] > -1 && (Dist[1] < 0 || Dist[0] < Dist[1]))
			{
				fShouldStop = false;
				pev->angles = SideAng[0];
			}
			else if (Dist[1] > -1 && (Dist[0] < 0 || Dist[1] < Dist[0]))
			{
				fShouldStop = false;
				pev->angles = SideAng[1];
			}

			if (!fShouldStop)
			{
				//Break all the way out
				SetBits(FrameConditions, FC_AVOID);
				break;
			}

			//else ALERT( at_console, "Avoid, but blocked\n" );
			pev->origin = vOriginSave;
			pev->angles.y = flYawSave;
		}

		if (fShouldStop) //I'm blocked, stop
		{
			pev->angles.y = UTIL_VecToAngles(m_MoveDir).y;
			StopWalking();
		}
	}
}

void CMSMonster::FaceForward()
{
	if (!IsFlying())
	{
		//Get the ground normal
		TraceResult tr;
		//UTIL_TraceLine( EyePosition(), EyePosition() - Vector(0,0,4096), ignore_monsters, edict(), &tr );
		//UTIL_TraceHull( EyePosition(), EyePosition() - Vector(0,0,4096), ignore_monsters, 3, edict(), &tr );
		TRACE_MONSTER_HULL(edict(), pev->origin, pev->origin - Vector(0, 0, m_Height), ignore_monsters, edict(), &tr);

		//if( HasConditions( MONSTER_HASMOVEDEST ) )
		//	pev->angles.y = UTIL_VecToAngles(m_MoveDir).y;

		if (tr.flFraction < 1)
		{
			//Aligns the monster to the ground slope
			m_GroundNormal = tr.vecPlaneNormal;
			//AlignToNormal( m_GroundNormal, pev->angles );

			Vector vTestAng = Vector(0, pev->angles.y, 0);
			AlignToNormal(m_GroundNormal, vTestAng);
			pev->angles = vTestAng;

			UTIL_MakeVectorsPrivate(vTestAng, m_MoveDir, NULL, NULL);
		}
		else
		{
			m_GroundNormal = Vector(0, 0, 1);
		}

		//Finds the forward vector (of the target direction, not neccesarily in front)
		//Vector vAng;
		//vAng.y = UTIL_VecToAngles(vDir).y;
		//AlignToNormal( tr.vecPlaneNormal, vAng );
	}
}

void CMSMonster::StopWalking()
{
	if (!pev->model)
		return; //Just in case
	//if( m_Activity == ACT_MELEE_ATTACK1 && !m_fSequenceFinished ) return;
	//pev->velocity = g_vecZero;

	ClearConditions(MONSTER_HASMOVEDEST);
	m_NextNodeTime = gpGlobals->time + m_RoamDelay; //wait 2 seconds before wandering
	m_Activity = ACT_IDLE;
	if (!HasConditions(MONSTER_TRADING))
		m_hEnemy = NULL; //Thothie FEB2011_22 - try not to break trade menus when looking around via setmovedest
	m_Wandering = true;
	CallScriptEvent("game_stopmoving");
}

//
// Attack
// ������

void CMSMonster::Act()
{
	/*if( m_Activity == ACT_SPECIAL_ATTACK1 )
	{
		//Action Animation
		if( m_fSequenceFinished ) 
		{
			StopWalking( );
		}
		else {
			SetAnimation( MONSTER_ANIM_ONCE, (char *)STRING(ActionAnim) );
		}
	}
	else {
		//Attack with the current hand
		CGenericItem *CurrentHand = ActiveItem( );
		if( CurrentHand ) {
			if( CurrentHand->CurrentAttack ) 
			{
				m_Activity = ACT_MELEE_ATTACK1;
				
				if( m_fSequenceFinished ) {
					//m_Hand->CurrentAttack->tStart = -65534;
					//m_Hand->CurrentAttack->fCanCancel = TRUE;
					StopWalking( );
					CurrentHand->CancelAttack( );
					return;
				}
				else CurrentHand->CurrentAttack->fCanCancel = FALSE;
			
				if( LastEvent.event == 600 ) //call attack this frame
				{
					CurrentHand->CurrentAttack->fAttackThisFrame = true;
					pev->iuser4 = -1;
				}
				else CurrentHand->CurrentAttack->fAttackThisFrame = FALSE; //basically never
			}

			//m_Hand->AttackButtonDown( );
		}
	}*/
}

void CMSMonster::HandleAnimEvent(MonsterEvent_t *pEvent)
{
	LastEvent = *pEvent;
	float z;
	//ALERT( at_console, "event %i\n", LastEvent.event );
	switch (LastEvent.event)
	{
	case 400: //Animation Event 400 - Jump
		UTIL_MakeVectors(pev->angles);
		pev->velocity = gpGlobals->v_forward * 250;
		pev->velocity.z = 450;
		break;
	case 401: //Animation Event 401 - Move forward in Jump
		z = pev->velocity.z;
		UTIL_MakeVectors(pev->angles);
		pev->velocity = gpGlobals->v_forward * 250;
		pev->velocity.z = z;
		break;
	case 450: //Animation Event 450 - Stop forward/backward movement
		pev->velocity.x = 0;
		pev->velocity.y = 0;
		break;
	case 500: //Animation Event 500 - Call any script event
		if (pEvent->options)
			CallScriptEvent(pEvent->options);
		break;
	case 600: //Animation Event 600 - Call Attack() on my held weapon
		//... or do damage if its an anim type attack
		if (pEvent->options)
			CallScriptEvent(pEvent->options);
		break;
	default:
		CBaseAnimating::HandleAnimEvent(pEvent);
		break;
	}
}

/*
void CMSMonster :: Jump( ) {
#ifdef VALVE_DLL
	//jump onto things
	if( !JumpHeight || FrameConditions&FC_STEP ) return;
	if( m_Activity == ACT_HOP ) {
		if( m_fSequenceFinished ) m_Activity = ACT_IDLE;
		return;
	}
	Vector VecDest;
	TraceResult tr;
	UTIL_MakeVectors( pev->angles );
	VecDest = pev->origin + gpGlobals->v_forward * (MoveProximity*1.5);
	gpGlobals->trace_flags |= FTRACE_SIMPLEBOX;
	UTIL_TraceLine( pev->origin, VecDest, dont_ignore_monsters, edict(), &tr );
	if( tr.flFraction < 1.0 ) {
		//Don't jump on your enemy
		CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
		if( pHit && m_hEnemy != NULL && pHit->edict() == m_hEnemy->edict() ) return;

		VecDest = VecDest + Vector(0,0,JumpHeight);
		gpGlobals->trace_flags |= FTRACE_SIMPLEBOX;
		UTIL_TraceLine( pev->origin+ Vector(0,0,JumpHeight), VecDest, dont_ignore_monsters, edict(), &tr );
		if( tr.flFraction >= 1.0 )
			m_Activity = ACT_HOP;
	}
#endif
}*/
//
//   Say - Have the Monster say something...
//   ���
void CMSMonster ::Say(msstring_ref Sound, float fDuration)
{

	//Thothie JUN2007b
	//- wracking my brain trying to stop this thing from sending non-waves to players
	msstring thoth_pissed = Sound;
	if (!thoth_pissed.contains("RND_SAY"))
	{
		word_t NewWord;
		NewWord.Soundfile = Sound;
		NewWord.Duration = fDuration;
		NewWord.Spoken = false;

		m_Words.add(NewWord);
	}
}
void CMSMonster ::Talk()
{
	if (!m_Words.size())
		return;

	word_t &Word = m_Words[0];
	float TimeEndWord = m_TimeLastSpoke + Word.Duration;
	if (!Word.Spoken)
	{
		//Thothie JUN2007b - trying to remove wave error with var based say commands
		msstring thoth_filter = Word.Soundfile.c_str();
		if (thoth_filter.contains(".wav"))
		{
			if (strlen(Word.Soundfile))
				EMIT_SOUND(edict(), CHAN_VOICE, Word.Soundfile, SndVolume, ATTN_NORM);
		}
		m_TimeLastSpoke = gpGlobals->time;
		Word.Spoken = true;
	}
	else if (gpGlobals->time < TimeEndWord)
	{
		//Still speaking this word
		float MouthAmt = (TimeEndWord - gpGlobals->time) * 80;
		if (MouthAmt > 20)
			MouthAmt = 20;
		else if (MouthAmt < 0)
			MouthAmt = 0;

		SetBoneController(1, MouthAmt);
	}
	else
	{
		//Done speaking this word
		m_Words.erase(0);
		if (!m_Words.size())
			m_Words.clear();
	}
}
//
// Speak - Text speech for both NPCs and players
// �����
void CMSMonster ::Speak(char *pszSentence, speech_type SpeechType)
{

	if (!pszSentence || !pszSentence[0])
		return;

	// remove quotes if present
	if (*pszSentence == '"')
	{
		if (strlen(pszSentence) > 1)
		{
			pszSentence++;
			if (pszSentence[strlen(pszSentence) - 1] == '"')
				pszSentence[strlen(pszSentence) - 1] = 0;
		}
	}

	// make sure the text has content
	char *pc = NULL;
	for (pc = pszSentence; pc != NULL && *pc != 0; pc++)
	{
		if (isprint(*pc) && !isspace(*pc))
		{
			pc = NULL; // we've found an alphanumeric character,  so text is valid
			break;
		}
	}
	if (pc != NULL)
		return; // no character found, so say nothing

	//ALERT( at_console, "PrevSentence: %s\n", pszSentence );

	char FinalSentence[4096] = "";

	saytext_e SayTextType = SAYTEXT_GLOBAL;
	switch (SpeechType)
	{
	case SPEECH_GLOBAL:
		strcat(FinalSentence, "[global] ");
		SayTextType = SAYTEXT_GLOBAL;
		break;
	case SPEECH_PARTY:
		strcat(FinalSentence, "[party] ");
		SayTextType = SAYTEXT_PARTY;
		break;
	case SPEECH_LOCAL:
		char cTemp[4096];
		 _snprintf(cTemp, sizeof(cTemp),  "%s says,  \"%s\"\n",  DisplayName(),  pszSentence );
		strcat(FinalSentence, cTemp);
		SayTextType = IsPlayer() ? SAYTEXT_LOCAL : SAYTEXT_NPC;
		break;
	}

	if (SpeechType != SPEECH_LOCAL)
	{
		strcat(FinalSentence, DisplayName());
		strcat(FinalSentence, ": ");

		//ALERT( at_console, "PrevSentence: %s\n", pszSentence );

		strcat(FinalSentence, pszSentence);
		strcat(FinalSentence, "\n");
		//ALERT( at_console, "FinalSentence: %s\n", FinalSentence );
	}

	CBaseEntity *pList[255], *pEnt = NULL;
	// Fill pList with a all the monsters and players on the level
	int count = UTIL_EntitiesInBox(pList, 255, Vector(-6000, -6000, -6000), Vector(6000, 6000, 6000), FL_MONSTER | FL_CLIENT);

	// Now try to speak to each one
	for (int i = 0; i < count; i++)
	{
		pEnt = pList[i];

		if (!pEnt->pev || FNullEnt(pEnt->edict()))
			continue;

		if (pEnt->edict() == edict())
		{
			//Thothie FEB2008b - admin_gag
			CBasePlayer *pPlayer = (CBasePlayer *)pEnt;
			if (pPlayer)
			{
				if (pPlayer->m_Gagged)
				{
					int rnd_muffle = RANDOM_LONG(1, 4);
					if (rnd_muffle == 1)
						 _snprintf(FinalSentence, sizeof(FinalSentence),  "[muted] %s: %s\n",  DisplayName(),  "Hmmmf... Mmmm! MMmmmmf!" );
					else if (rnd_muffle == 2)
						 _snprintf(FinalSentence, sizeof(FinalSentence),  "[muted] %s: %s\n",  DisplayName(),  "Mmmmm! Mmmmmmmmmf!" );
					else if (rnd_muffle == 3)
						 _snprintf(FinalSentence, sizeof(FinalSentence),  "[muted] %s: %s\n",  DisplayName(),  "Ffffffmmmmffff!!!" );
					else if (rnd_muffle == 4)
						 _snprintf(FinalSentence, sizeof(FinalSentence),  "[muted] %s: %s\n",  DisplayName(),  "MMmmmmf! HMmmmmmf!" );
				}
			}

			if (pEnt->IsNetClient())
			{
				// print to the sending client
				MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_HUDMSG], NULL, edict());
				WRITE_BYTE(4);
				WRITE_BYTE(SayTextType);
				WRITE_STRING_LIMIT(FinalSentence, WRITE_STRING_MAX);
				MESSAGE_END();
			}
			continue;
		}

		// Local speech
		//Thothie SEP2007 - removing "|| !FMVisible( pEnt )" condition - causes issues
		if (SpeechType == SPEECH_LOCAL)
			if ((pEnt->Center() - Center()).Length2D() > m_SayTextRange)
				continue;

		// Party speech
		if ((SpeechType == SPEECH_PARTY && IsPlayer()) && !SameTeam(pEnt, this))
			continue;

		if (pEnt->IsNetClient())
		{
			MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_HUDMSG], NULL, pEnt->edict());
			WRITE_BYTE(4);
			WRITE_BYTE(SayTextType);
			WRITE_STRING_LIMIT(FinalSentence, WRITE_STRING_MAX);
			MESSAGE_END();
		}

		//MiB DEC2007a
		if (SpeechType == SPEECH_LOCAL && IsPlayer() && pEnt->IsMSMonster())
		{
			StoreEntity(this, ENT_LASTSPOKE); //Thothie - not working :(
			msstringlist Params;

			Params.clear();
			Params.add(stripBadChars(pszSentence));
			Params.add(EntToString(this)); //Thothie - Workaround
			((CMSMonster *)pEnt)->CallScriptEvent("game_heardtext", &Params);
		}

		//This has to be called after the text msgs are sent out
		if (SpeechType == SPEECH_LOCAL && IsPlayer() && pEnt->IsMSMonster())
			((CMSMonster *)pEnt)->HearPhrase(this, pszSentence);
	}

	// echo to server console
	if (IsPlayer())
		g_engfuncs.pfnServerPrint(FinalSentence);
}

//Called when a player talks (normal mode)
void CMSMonster ::HearPhrase(CMSMonster *pSpeaker, const char *phrase)
{
	char cTemp1[256];

	if (HasConditions(MONSTER_NOAI))
		return;
	if (!IsAlive())
		return;

	 strncpy(cTemp1,  phrase, sizeof(cTemp1) );
	_strlwr(cTemp1); //lower case comparison

	listenphrase_t *BestPhrase = NULL;
	float BestMatchedRatio = 0;

	for (int i = 0; i < m_Phrases.size(); i++)
	{
		listenphrase_t &Phrase = m_Phrases[i];
		for (int p = 0; p < Phrase.Phrases.size(); p++)
		{
			msstring_ref CheckPhrase = Phrase.Phrases[p];
			msstring_ref SubPhrase = NULL;
			if (SubPhrase = strstr(cTemp1, CheckPhrase))
			{
				int Matched = 0;
				int len = strlen(CheckPhrase);
				for (int x = 0; x < len; x++)
					if (SubPhrase[x] == CheckPhrase[x])
						Matched++;

				float ratio = (float)Matched / strlen(cTemp1);
				if (ratio > BestMatchedRatio)
				{
					BestMatchedRatio = ratio;
					BestPhrase = &Phrase;
				}

				break;
			}
		}
	}

	if (BestPhrase)
	{
		//Heard a phrase, store the talker and run the event
		StoreEntity(pSpeaker, ENT_LASTSPOKE);
		CallScriptEvent(BestPhrase->ScriptEvent);
	}
}

void CMSMonster ::Used(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
	{
		if (!m_fSpawnOnTrigger)
			return;
		if (pev->owner)
			CBaseEntity::Instance(pev->owner)->MSQuery((int)pev);
		return;
	};
	if (HasConditions(MONSTER_NOAI))
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pActivator;

	StoreEntity(pActivator, ENT_LASTUSED);

	if (m_Menu_Autoopen) //Auto open menu when player does +use on me
		OpenMenu(pPlayer);

	//Using pev->euser1 is the old way, phase it out
	pev->euser1 = pActivator->edict();

	msstringlist Parameters;
	Parameters.add(EntToString(pActivator));
	CallScriptEvent("game_playerused", &Parameters);

	pev->euser1 = NULL;
}
//
// Trade - Manage trading with others
// �����
void CMSMonster ::Trade()
{
	if (!HasConditions(MONSTER_TRADING))
		return;

	if (m_hEnemy != NULL && m_hEnemy->IsAlive())
	{
		CBaseMonster *pEnemy = (CBaseMonster *)(CBaseEntity *)m_hEnemy;
		if (pEnemy->m_hEnemy == this &&
			(pEnemy->Center() - Center()).Length() <= 128)
			return;
	}

	//End trade
	if (m_TradeCallBackEvent.len())
		CallScriptEvent(UTIL_VarArgs("%s_done", m_TradeCallBackEvent.c_str()));
	ClearConditions(MONSTER_TRADING);
	m_hEnemy = NULL;
	OpenStore = NULL;

	/*
	CBaseEntity *pTarget = m_hEnemy;
	CBasePlayer *pPlayer = pTarget ? (pTarget->IsPlayer() ? (CBasePlayer *)pTarget : NULL) : NULL;
	if ( pPlayer ) pPlayer->InMenu = false;
	*/
}
tradeinfo_t *CMSMonster::TradeItem(tradeinfo_t *ptiTradeInfo)
{
	if (!HasConditions(MONSTER_TRADING) || m_hEnemy == NULL ||
		!ptiTradeInfo || !OpenStore)
		return NULL;

	//If I'm the vendor...
	if (ptiTradeInfo->pCustomer != this)
	{
		static tradeinfo_t tiTradeInfo;
		memset(&tiTradeInfo, 0, sizeof(tradeinfo_t));

		storeitem_t *psiStoreItem = NULL;

		tiTradeInfo.ItemName = ptiTradeInfo->ItemName;
		tiTradeInfo.pCustomer = ptiTradeInfo->pCustomer;
		tiTradeInfo.pStore = OpenStore;

		if (ptiTradeInfo->iStatus == TRADE_BUY)
		{
			//If gold, then just give all the gold
			if (!strcmp(ptiTradeInfo->ItemName, "GOLD"))
			{
				tiTradeInfo.iStatus = ptiTradeInfo->iStatus;
				tiTradeInfo.pItem = NULL;
				tiTradeInfo.iPrice = m_Gold;
				tiTradeInfo.psiStoreItem = NULL;
			}
			//If not gold, then match the item name to its store item
			else if ((psiStoreItem = OpenStore->GetItem(ptiTradeInfo->ItemName)) &&
					 psiStoreItem->Quantity >= psiStoreItem->iBundleAmt)
			{
				tiTradeInfo.iStatus = ptiTradeInfo->iStatus;
				tiTradeInfo.pItem = NewGenericItem(tiTradeInfo.ItemName);
				tiTradeInfo.pItem->iQuantity = psiStoreItem->iBundleAmt;
				tiTradeInfo.iPrice = psiStoreItem->iCost;
				tiTradeInfo.psiStoreItem = psiStoreItem;
			}
			else
				return NULL;
		}
		else if (ptiTradeInfo->iStatus == TRADE_SELL)
		{
			//Find the item on the customer.  Item must be owned by the customer
			ptiTradeInfo->pItem = MSUtil_GetItemByID(atoi(ptiTradeInfo->ItemName));
			if (!ptiTradeInfo->pItem || ptiTradeInfo->pItem->Owner() != ptiTradeInfo->pCustomer)
				return NULL;

			//Find the item in my store.  Item must exist in my store
			if (!(psiStoreItem = OpenStore->GetItem(ptiTradeInfo->pItem->ItemName.c_str())))
				return NULL;

			tiTradeInfo.iStatus = ptiTradeInfo->iStatus;
			tiTradeInfo.pItem = ptiTradeInfo->pItem;
			tiTradeInfo.iPrice = int(psiStoreItem->iCost * psiStoreItem->flSellRatio);
			tiTradeInfo.psiStoreItem = psiStoreItem;
		}
		return &tiTradeInfo;
	}

	return NULL;
}
//
// AcceptOffer - Accept an offer from a player or monster
// �����������
bool CMSMonster ::AcceptOffer()
{
	bool fRecievedItem = false;

	CBaseEntity *pEnt = (CBaseEntity *)GET_PRIVATE(INDEXENT(m_OfferInfo.SrcMonsterIDX));
	if (pEnt && pEnt == m_OfferInfo.pSrcMonster && pEnt->MyMonsterPointer())
	{
		CMSMonster *pMonster = (CMSMonster *)pEnt;

		if (m_OfferInfo.ItemType == ITEM_NORMAL)
		{
			CBaseEntity *pItemEnt = (CBaseEntity *)GET_PRIVATE(INDEXENT((int)m_OfferInfo.pItemData));
			if (pItemEnt &&
				pItemEnt == m_OfferInfo.pItemData2 &&
				FBitSet(pItemEnt->MSProperties(), ITEM_GENERIC) &&
				((CGenericItem *)pItemEnt)->m_pOwner == m_OfferInfo.pSrcMonster)
			{
				CGenericItem *pItem = (CGenericItem *)pItemEnt;
				if (IsPlayer())
					fRecievedItem = pItem->GiveTo(this) ? true : false;
				else if (pItem->m_pOwner)
				{
					pItem->m_pOwner->RemoveItem(pItem);
					pItem->SUB_Remove();
					fRecievedItem = true;
				}
			}
		}
		else
		{
			int iGoldAmt = max(min((int)m_OfferInfo.pItemData, pMonster->m_Gold), 0);
			m_Gold += iGoldAmt;
			pMonster->m_Gold -= iGoldAmt;
			fRecievedItem = true;
		}

		if (fRecievedItem)
			StoreEntity(pMonster, ENT_LASTGAVEITEM);

		m_OfferInfo.SrcMonsterIDX = 0;
	}

	return fRecievedItem;
}

float CMSMonster::Give(givetype_e Type, float Amt)
{
	float *Current = NULL, Max;

	switch (Type)
	{
	case GIVE_HP:
		Current = &m_HP;
		Max = MaxHP();
		break;
	case GIVE_MP:
		Current = &m_MP;
		Max = MaxMP();
		break;
	default:
		return 0;
		//case GIVE_GOLD: Current = &m; Max = MaxMP( ); break;
	}

	float AddAmount = min(Max - *Current, Amt); //Max amount that can be added
	AddAmount = max(-*Current, AddAmount);		//Max health that can be taken

	if (AddAmount < 0 && FBitSet(pev->flags, FL_GODMODE))
		AddAmount = 0;

	*Current += AddAmount;
	if (Type == GIVE_HP)
		pev->health = *Current;

	return AddAmount;
}

// Set the activity based on an event or current state
void CMSMonster::SetAnimation(MONSTER_ANIM AnimType, const char *pszAnimName, void *vData)
{
	startdbg;

	if (m_Brush)
		return;

	if (!m_pAnimHandler)
		m_pAnimHandler = &gAnimWalk;

	//if( IsActing( ) ) return;

	m_pAnimHandler->m_pOwner = this;
	pszCurrentAnimName = pszAnimName;
	dbg("Check for new animation");
	//Thothie - JUN2007b
	//- The code is causing monsters to break anims at weird moments
	//- if you 'dance' around an affected monster, he can never attack, as his swing anims break
	//- trying to stop that by making sure only scripts can call anim breaks
	if (m_pAnimHandler->CanChangeTo(AnimType, vData) && pszAnimName)
	{
		//Thothie - JUN2007b
		//- Its impossible to tell what animation a monster is currently using
		//- Since some anim changes are internal
		//- Setting an event here to capture script side
		//- Move to playanim if we dont need it, prob lags here
		//- Undoing - not helping with related problem
		/*
		msstringlist Parameters;
		bool thoth_abort_break = false;
		if ( pszAnimName ) Parameters.add( pszAnimName );
		if ( pszAnimName == "break" && AnimType == MONSTER_ANIM_BREAK ) Parameters.add( "break_script" );
		if ( !pszAnimName && AnimType == MONSTER_ANIM_BREAK )
		{
			Parameters.add( "break_msdll" );
			thoth_abort_break = true;
		}
		CallScriptEvent( "game_anim_new", &Parameters );
		//[/thothie]
		*/

		dbg("New Animation");

		/*if( AnimType == MONSTER_ANIM_BREAK ) //Special case: PLAYER_BREAK
		{
			if( vData ) AnimType = (MONSTER_ANIM)(long)vData;
			else {
				AnimType = MONSTER_ANIM_WALK;
				pszCurrentAnimName = NULL;
			}
			vData = NULL;
			m_pAnimHandler = NULL;
		}
		else
		{*/
		m_pAnimHandler = m_pAnimHandler->ChangeTo(AnimType);
		if (m_pAnimHandler)
		{
			m_pAnimHandler->m_pOwner = this;
			m_pAnimHandler->Initialize(vData);
		}
		//}
	}

	//Run current animation
	if (m_pAnimHandler)
	{
		m_pAnimHandler->m_pOwner = this;

		dbg("Animate");
		m_pAnimHandler->Animate();

		dbg("PostAnimate");
		m_pAnimHandler->PostAnimate();

		pev->framerate = m_Framerate;
		if (m_Framerate_Modifier)
			pev->framerate *= m_Framerate_Modifier;
	}

	enddbgprt((IsPlayer() ? "(Monster)" : "(PLAYER)"));
}
void CMSMonster::BreakAnimation(MONSTER_ANIM AnimType, const char *pszAnimName, void *vData)
{
	//Thothie - Attempting to stop msdll from breaking anims

	startdbg;

	if (m_Brush)
		return;

	if (!m_pAnimHandler)
		m_pAnimHandler = &gAnimWalk;

	m_pAnimHandler->m_pOwner = this;
	pszCurrentAnimName = pszAnimName;
	dbg("Check for new animation");

	dbg("Break Animation");

	if (AnimType == MONSTER_ANIM_BREAK) //Special case: PLAYER_BREAK
	{
		if (vData)
			AnimType = (MONSTER_ANIM)(long)vData;
		else
		{
			AnimType = MONSTER_ANIM_WALK;
			pszCurrentAnimName = NULL;
		}
		vData = NULL;
		m_pAnimHandler = NULL;
	}
	else
	{
		m_pAnimHandler = m_pAnimHandler->ChangeTo(AnimType);
		if (m_pAnimHandler)
		{
			m_pAnimHandler->m_pOwner = this;
			m_pAnimHandler->Initialize(vData);
		}
	}

	//Run current animation
	if (m_pAnimHandler)
	{
		m_pAnimHandler->m_pOwner = this;

		dbg("Animate");
		m_pAnimHandler->Animate();

		dbg("PostAnimate");
		m_pAnimHandler->PostAnimate();

		pev->framerate = m_Framerate;
		if (m_Framerate_Modifier)
			pev->framerate *= m_Framerate_Modifier;
	}

	enddbgprt((IsPlayer() ? "(Monster)" : "(PLAYER)"));
}
void CMSMonster::Attacked(CBaseEntity *pAttacker, damage_t &Damage)
{
	//Thothie FEB2011_22 - not used by any script, save the call
	/*
	msstringlist Parameters;
	Parameters.add( EntToString(pAttacker) );
	Parameters.add( UTIL_VarArgs("%f",Damage.flDamage) );
	Parameters.add( Damage.sDamageType );
	CallScriptEvent( "game_attacked", &Parameters );
	*/
}

//MiB MAR2008a multiple changes
float CMSMonster::TraceAttack(damage_t &Damage)
{
	startdbg;

	//CBaseMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );

	// Handle parrying
	// Attacker - Rolls from [AccuracyRoll - 130       ]
	// Defender - Rolls from [0            - ParryValue]
	// Attacker gets that extra 30 so attacks will get through more often
	dbg("Begin");

	//This is a last-chance check, to make sure that somebody doesn't find an obscure way to hurt another player
	//The relationship status is first checked in DoDamage()
	//---UNDONE: This check should be done before hand.  If TraceAttack is called, we have to accept the damage regardless of who caused it
	//if( Damage.pevAttacker && CBaseEntity::Instance(Damage.pevAttacker)->CanDamage( this ) )
	//	return 0;

	bool thoth_did_parry = false;

	CBaseEntity *pAttacker = Damage.pAttacker;
	if (IsAlive() && pAttacker && pAttacker->IsMSMonster())
	{
		CMSMonster *pMonster = (CMSMonster *)pAttacker;

		//PROBLEM: Players get INSANE XP bonus when monsters parry,
		//- maybe internalize Parry for monsters?
		//- Thothie note of FEB2008a

		int ParryValue = GetSkillStat("parry", STATPROP_SKILL);

		if (ParryValue > 60)
			ParryValue = 60; //Thothie JAN2010_22 - cap out parry (shhh)

		int AccRoll = RANDOM_LONG(Damage.AccuracyRoll, 100), ParryRoll = RANDOM_LONG(0, ParryValue);

		//Using a weapon that can parry (shield, some swords) gives a parry bonus
		int iAttackNum = 0, iHand = 0;
		;
		CGenericItem *pHandItem = FindParryWeapon(this, iHand, iAttackNum);
		//NOV2014_09 - this is handled by setting the parry skill now, oh dear
		/*
		if( pHandItem )
		{
			attackdata_t *pAttack = &pHandItem->m_Attacks[iAttackNum];
			ParryRoll *= 1 + (pAttack->flAccuracyDefault / 100.f);		//If 100% bonus, the roll gets multiplied by 2
		}
		*/

		//Thothie comments: Problems with this system
		//Awareness + (0-Parry Skill) is basically your parry roll
		//Total caps at 80
		//mob just rolls 1d100 - (accuracy<100)

		//Awareness gives a small parry bonus
		ParryRoll += GetNatStat(NATURAL_AWR);

		//Thothie - do not parry certain damage types
		msstring thoth_dmgtype = Damage.sDamageType;
		if (thoth_dmgtype.starts_with("target"))
			ParryRoll = 0;
		if (thoth_dmgtype.starts_with("fire"))
			ParryRoll = 0;
		if (thoth_dmgtype.starts_with("poison"))
			ParryRoll = 0;
		if (thoth_dmgtype.starts_with("lightning"))
			ParryRoll = 0;
		if (thoth_dmgtype.starts_with("cold"))
			ParryRoll = 0;
		if (thoth_dmgtype.contains("effect"))
			ParryRoll = 0; //do not parry DOT attacks
		if (thoth_dmgtype.starts_with("magic"))
			ParryRoll = 0;
		if (Damage.flDamage == 0)
			ParryRoll = 0; //do not parry 0 damage atks
		if (ParryRoll > 80)
			ParryRoll = 80; //always allow at least 20% chance to be hit

		if (ParryRoll > AccRoll)
		{
			//thothie attempting to pass parry vars
			msstringlist ParametersB;
			ParametersB.add(pAttacker ? EntToString(pAttacker) : "none");
			ParametersB.add(UTIL_VarArgs("%f", Damage.flDamage));
			ParametersB.add(Damage.sDamageType);
			ParametersB.add(UTIL_VarArgs("%i", ParryRoll));
			ParametersB.add(UTIL_VarArgs("%i", AccRoll));
			ParametersB.add(UTIL_VarArgs("%i", ParryValue));
			CallScriptEvent("game_parry", &ParametersB);
			if (pHandItem)
				pHandItem->CallScriptEvent("game_parry", &ParametersB);
			Damage.flDamage = -1; // -1 means the monster dodged the attack
			ClearMultiDamage();
			thoth_did_parry = true;
			/* MiB JUL2010_02 - Remove Parry as a levellable stat
			//Learn parry skill from  a successful parry
			if( !pAttacker->IsPlayer() )  //Can't learn from being attacked by players
				if ( GetSkillStat( SKILL_PARRY, 0 ) < CHAR_LEVEL_CAP )
					LearnSkill ( SKILL_PARRY, STATPROP_SKILL, max( 0 , pMonster->m_SkillLevel * 2 ) );
			//MiB Jul2008a (JAN2010_15) - Parry was capped at 40. What say we stop exp gain?

				/*if (pMonster->m_SkillLevel * 2 > 0)
					LearnSkill (SKILL_PARRY, STATPROP_SKILL, pMonster->m_SkillLevel * 2 );
				else
					LearnSkill (SKILL_PARRY, STATPROP_SKILL, 0 );
				//LearnSkill( SKILL_PARRY, STATPROP_SKILL, max(pAttacker->m_SkillLevel * 2,0) );*/
		}
	}

	//this was moved to where experience is calculated in GIAttack
	//Damage Modifiers ( takedmg xxx )
	dbg("Damage Modifiers");
	Damage.flDamage *= m.GenericTDM;
	if (Damage.sDamageType)
		for (int i = 0; i < m.TakeDamageModifiers.size(); i++)
		{
			takedamagemodifier_t &TDM = m.TakeDamageModifiers[i];
			//msstring thoth_my_dmgtype = TDM.DamageType;
			msstring thoth_inc_dmgtype = Damage.sDamageType.c_str();
			if (thoth_inc_dmgtype.starts_with(TDM.DamageType))
			{
				//ALERT( at_console, "Damage modified!");// %.2f --> %.2f ( %s )\n", Damage.flDamage, Damage.flDamage * TDM.modifier, Damage.sDamageType );
				Damage.flDamage *= TDM.modifier;
			}
		}

	//Allow scripts to affect the damage
	msstringlist Parameters;
	Parameters.add(pAttacker ? EntToString(pAttacker) : "none");
	Parameters.add(UTIL_VarArgs("%f", Damage.flDamage));
	Parameters.add(Damage.sDamageType); //Thothie attempting to add damage type to game_damaged
	Parameters.add(UTIL_VarArgs("%i", Damage.AccuracyRoll));
	Parameters.add(Damage.pInflictor ? EntToString(Damage.pInflictor) : "none");

	//[begin] Thothie DEC2014_13 - return skill used (WEAPON_SKILL not being reliable)
	CStat *pStat = FindStat(Damage.ExpStat);
	if (pStat)
	{
		msstring out_skill = pStat->m_Name;
		if (out_skill.contains("spell"))
		{
			out_skill.append(".");
			char toconv[256];
			strncpy(toconv, SpellTypeList[Damage.ExpProp], sizeof(toconv));
			out_skill.append(msstring(_strlwr(toconv)));
		}
		Parameters.add(out_skill);
	}
	else
	{
		Parameters.add("none"); //Thothie OCT2019_11 - MiB Sanity Check
	}
	//[end] Thothie DEC2014_13 - return skill used (WEAPON_SKILL not being reliable)

	CallScriptEvent("game_damaged", &Parameters);

	if (m_ReturnData.len())
	{
		//Each script sets a ratio of damage you should take.  Factor each one into the damage
		msstringlist DamageRatios;
		TokenizeString(m_ReturnData, DamageRatios);
		for (int i = 0; i < DamageRatios.size(); i++)
			Damage.flDamage *= atof(DamageRatios[i]); //Script can reject the damage with "returndata"
	}

	Parameters.clearitems();
	Parameters.add(pAttacker ? EntToString(pAttacker) : "none");
	Parameters.add(UTIL_VarArgs("%f", Damage.flDamage));
	CallScriptEvent("game_damaged_end", &Parameters); //Allow post-parsing of damage, after all the scripts have messed with it

	dbg("StruckSound/Bleed");
	if (Damage.flDamage > 0)
	{
		//if( IsAlive() ) StruckSound( CBaseEntity::Instance(Damage.pevInflictor), CBaseEntity::Instance(Damage.pevAttacker), Damage.flDamage, &Damage.outTraceResult, Damage.iDamageType );

		if (pev->health > 0 && pev->takedamage && !FBitSet(pev->flags, FL_GODMODE))
		{
			SpawnBlood(Damage.outTraceResult.vecEndPos, BloodColor(), Damage.flDamage); // a little surface blood.
			TraceBleed(Damage.flDamage, (Damage.vecEnd - Damage.vecSrc).Normalize(), &Damage.outTraceResult, Damage.iDamageType);
		}
	}

	dbg("AddMultiDamage");
	if (!thoth_did_parry)
		AddMultiDamage(Damage.pAttacker ? Damage.pAttacker->pev : NULL, this, Damage.flDamage, Damage.iDamageType);

	return Damage.flDamage;

	enddbg;

	return 0;
}

int CMSMonster::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	//	if( m_hEnemy == NULL || m_hEnemy.Get() != pevAttacker->pContainingEntity )
	//		m_hEnemy.Set( pevAttacker->pContainingEntity );

	//if( pevAttacker == pev ) return fTookDamage;
	//startdbg;
	//dbg( "Begin" );
	CBaseEntity *pInflictor = NULL, *pAttacker = NULL;
	if (!FNullEnt(pevInflictor))
		pInflictor = CBaseEntity ::Instance(pevInflictor);
	if (!FNullEnt(pevAttacker))
		pAttacker = CBaseEntity ::Instance(pevAttacker);

	//--UNDONE:  If Damage is given, we must accept it regardless of the entity that caused it
	//if( pAttacker && IRelationship(pAttacker) >= RELATIONSHIP_NO )
	//	return 0;

	//dbg( "TakeDamageEffect" );
	TakeDamageEffect(pInflictor, pAttacker, flDamage, bitsDamageType);

	if (pevInflictor)
		m_vecEnemyLKP = pevInflictor->origin;
	else
		m_vecEnemyLKP = pev->origin - (g_vecAttackDir * 64);

	if (IsAlive())
	{
		//Attacked, store the attacker and run the struck event
		//dbg( "StoreEntity" );
		StoreEntity(pAttacker, ENT_LASTSTRUCK);

		//dbg( "CallScriptEvent->game_struck" );
		msstringlist Parameters;
		Parameters.add(UTIL_VarArgs("%f", flDamage));
		CallScriptEvent("game_struck", &Parameters);

		//AUG2013_21 Thothie - This is an old function we don't use, and it doesn't report damage proper
		/*
		if( HasConditions( MONSTER_REFLECTIVEDMG ) && pAttacker && pAttacker != this )
			pAttacker->TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
		*/

		//dbg( "GiveHP" );
		GiveHP(-flDamage);
		if (pev->health <= 0)
		{
			if (FBitSet(bitsDamageType, DMG_NOKILL))
			{
				//dbg( "Zero_Health^1" );
				pev->health = m_HP = 1;
			}
			else
			{
				//dbg( "Killed" );
				Killed(pevAttacker, GIB_NEVER);
			}
		}
	}
	else
	{
		//dbg( "AttackCorpse" );
		//Attack a dead body
		//Skinning disabled
		/*if( Skin && pAttacker && pAttacker->MyMonsterPointer() &&
			!FBitSet(bitsDamageType,DMG_NOSKIN) )
		{
			//Skin a dead body
			if( ((CMSMonster *)pAttacker)->SkinMonster( this ) )
			{   //Skining success, disappear
				SetThink( SUB_Remove );
				pev->nextthink = gpGlobals->time + 0.1;
				return 0;
			}
			else { //Skining failed
				Skin = NULL;
				//fTookDamage = CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
			}
		}*/
		GiveHP(-flDamage);
		if (pev->health <= 0)
			pev->health = 0;
	}

	//enddbg;

	return flDamage;
}
void CMSMonster::CounterEffect(CBaseEntity *pInflictor, int iEffect, void *pExtraData)
{
	if (!pInflictor || !pInflictor->IsMSItem())
		return;

	//Call hitliving event on the item that struck me

	pInflictor->CounterEffect(this, CE_HITMONSTER, this);
}

void CMSMonster::Killed(entvars_t *pevAttacker, int iGib)
{
	startdbg;
	dbg("Begin");
	BOOL DeleteMe = TRUE;

	//NOV2014_21 Thothie - script side XP management option [begin]
	bool xp_dump = (strcmp(GetFirstScriptVar("NPC_DUMP_XP"), "1") == 0);	 //dump hits to array
	bool xp_custom = (strcmp(GetFirstScriptVar("NPC_CUSTOM_XP"), "1") == 0); //don't add XP code side
	msstring skillname;

	if (xp_dump)
	{
		//this maybe called post-death, ya may have to swap this for the GM ent
		Util_ScriptArray(this, "create", "ARRAY_XP_PLAYERS", "0");
		Util_ScriptArray(this, "create", "ARRAY_XP_SKILLS", "0");
		Util_ScriptArray(this, "create", "ARRAY_XP_AMTS", "0");
	}
	//NOV2014_21 Thothie - script side XP management option [end]

	//MiB JUL2010_28 Anti-Gib
	if (pev->deadflag == DEAD_NO)
	{
		//Thothie OCT2015_24 - make sure we remove any scripted client effects before we do anything else
		CBaseEntity* pTarget = RetrieveEntity(ENT_ME);
		IScripted* pScripted = (pTarget ? pTarget->GetScripted() : NULL);
		if (pScripted)
		{
			for (int i = 0; i < pScripted->m_Scripts.size(); i++) // Check each 
			{
				if (pScripted->m_Scripts[i]->VarExists("game.effect.id")) //This is an effect 
				{
					pScripted->m_Scripts[i]->RunScriptEventByName("effect_die"); //Call this effect's die function
					pScripted->m_Scripts[i]->m.RemoveNextFrame = true;
				}
			}
		}

		//Award players with exp proportional to the damage they did
		dbg("AwardXP");
		for (int i = 1; i <= MAXPLAYERS; i++)
		{
			CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(i); // MiB MAR2019_22 [SLOT_EXP] - Need this to be a CBasePlayer, not sure why it wasn't
			if (!pPlayer)
				continue;

			msstring vsId = pPlayer->AuthID() + "_" + pPlayer->m_CharacterNum;
			if (!FStrEq(vsId, m_PlayerDamage[i - 1].msId))
				continue; // MiB MAR2019_22 [SLOT_EXP] - Skip if the id doesn't match

			float xpsend = 0.0;
			// MiB JUN2010_19 - Decreases the exp-damage ratio if monster was overkilled
			float mult = min(1, m_MaxHP / m_PlayerDamage[i - 1].dmgInTotal);
			for (int n = SKILL_FIRSTSKILL; n < SKILL_MAX_ATTACK; n++)
			{
				for (int r = 0; r < STATPROP_ALL_TOTAL; r++)
				{
					float dmg = m_PlayerDamage[i - 1].dmg[n][r];
					if (dmg > 0)
					{
						float xp = (m_SkillLevel * ((dmg * mult) / MaxHP()));
						xp = xp - floor(xp) >= 0.5 ? ceil(xp) : floor(xp); //Round the value. If it seems people are getting screwed on exp, lower the 0.5

						//MiB - Removed
						//if ( xpsend < m_SkillLevel )
						xpsend += xp; //Thothie added if ( xpsend < m_SkillLevel ) condition - don't give more than the monsters worth just cuz you hit it a lot!
						//if( xpsend > 0 ) ALERT( at_console, "Gained XP: %f", xpsend );  //Thothie returns XP
						//if( xpsend > 0 ) ClientPrint( pPlayer->pev, at_console, "Gained XP: %f", xpsend );
						dbg("pPlayer->LearnSkill");
						if (!xp_custom)
							pPlayer->LearnSkill(n, r, xp); //NOV2014_21 Thothie - script side XP management option

						//NOV2014_21 Thothie - script side XP management option [begin]
						if (xp_dump)
						{
							Util_ScriptArray(this, "add", "ARRAY_XP_PLAYERS", EntToString(pPlayer));

							skillname = SkillStatList[n - SKILL_FIRSTSKILL].Name;
							skillname.append(".");
							if (skillname.contains("Spell"))
								skillname.append(SpellTypeList[r]);
							else if (r < 3)
								skillname.append(SkillTypeList[r]);

							Util_ScriptArray(this, "add", "ARRAY_XP_SKILLS", skillname.c_str());
							Util_ScriptArray(this, "add", "ARRAY_XP_AMTS", FloatToString(xp));
						}
						//NOV2014_21 Thothie - script side XP management option [end]
					}
				}
			}
			dbg("SendXP");
			if (xpsend > 0 && !xp_custom)
			{
				msstringlist Parameters;
				Parameters.add(UTIL_VarArgs("%f", xpsend));
				Parameters.add(EntToString(this)); //Thothie APR2018_03
				pPlayer->CallScriptEvent("game_xpgain", &Parameters);
			}
		}

		//Stop attacking
		CancelAttack();

		//Slience my weapon
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);

		//Callback for msarea_monsterspawn
		//MAR2008b - this used to be after ShouldGibMonster( iGib ) - moving before
		if (pev->owner)
		{
			CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
			// MiB AUG2010_01 - pOwner - If the msarea_monsterspawn has been deleted, pev->owner will be "true", but it will null-pointer here
			//				  - DEAD_NO redundancy is in case game_death changes it (happens on skeletons)
			//if ( pOwner && pev->deadflag != DEAD_NO )
			if (pOwner)
				pOwner->DeathNotice(pev); //Thothie DEC2010_22 - check that pOwner exists
		}

		//MAR2008b fire targets here instead of in death fade, in case gibs
		if (m_iszKillTarget.len() > 0)
			FireTargets(m_iszKillTarget, this, this, USE_TOGGLE, 0);

		//Gib now, if neccesary
		if (ShouldGibMonster(iGib))
			CallGibMonster();

		//Thothie SEP2007 - attempting to remove inifnite xp sploit from missing monsterspawn
		//- maybe causing issues - prob not - test further when have time (see if actually fixes inf xp)
		//bool thoth_spawner_alive = CBaseEntity::Instance(pev->owner)->IsAlive;
		//if( !thoth_spawner_alive ) CallGibMonster();

		CallScriptEvent("game_predeath"); //Thothie MAY2016_14 - allows fixing of death fx on some mobs

		//I'm dying, create my body
		if (!FBitSet(pev->effects, EF_NODRAW))
		{
			Activity DeathAct = GetDeathActivity();
			pev->sequence = LookupActivity(DeathAct);
			ResetSequenceInfo();
			pev->frame = 0;
			pev->deadflag = DEAD_DYING;
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_TOSS; //So I'll fall to the ground
			ClearBits(pev->flags, FL_ONGROUND);
			UTIL_SetOrigin(pev, pev->origin); //Thothie - do we need this?, messses with fliers
			SetThink(&CMSMonster::DieThink);
			//			ALERT( at_console, "m_flFrameRate: %f\n", m_flFrameRate );
			//pev->nextthink = gpGlobals->time + ((256 / m_flFrameRate) * pev->framerate);
			pev->health = pev->max_health / 2; //Thothie - Dunno what this is about
			DeleteMe = FALSE;				   //live until the body's dead

			if (FStrEq(STRING(pev->classname), "msmonster_summoned"))
				g_SummonedMonsters--;

			//ALERT( at_console, "called game_death\n" );
			CallScriptEvent("game_death");
			m_SkillLevel = 0; //Thothie APR2011_06 - attempt to make sure monster does not give additional skill post mortem
		}
		else
			DeleteMe = TRUE; // Gibbed, destroy this entity
	}
	if (DeleteMe)
	{
		if (!IsPlayer())
			DropAllItems(); //Be sure to drop items
		GibMonster();
	}
	enddbg;
}
void CMSMonster ::SUB_Remove()
{
	CBaseEntity::SUB_Remove();
}
void CMSMonster ::DieThink()
{
	StudioFrameAdvance();
	DispatchAnimEvents(); //Have to handle this, even when dead
	if (!IsPlayer())
		DropAllItems();

	//Thothie APR2008a - massive commenting out
	//- changes made here so monsters fade away faster and dependably

	//Thothie comment [dated]: I think some models are not firing this event
	//- we may have to rig it so that the monster is removed eventually
	//- even if this never happens
	/*if( !m_fSequenceFinished )
	{
		DropAllItems( );
		//APR2008a - trying to apply above idea
		m_fSequenceFinished = true;
		UTIL_SetOrigin( pev, pev->origin ); //APR2008b ditto
		pev->nextthink = gpGlobals->time + ( (256/m_flFrameRate) * pev->framerate );
		//pev->nextthink = gpGlobals->time + 0.1;
		return;
	}*/

	pev->deadflag = DEAD_DEAD; //I am now officially dead
							   //pev->solid			= SOLID_NOT;		//Bodies aren't solid anymore

	/*	Vector vOrigin = pev->origin;
	GetBonePosition( 1, pev->origin, Vector(0,0,0) );
	UTIL_SetOrigin(pev, pev->origin );

	pev->maxs = pev->maxs; pev->maxs.z = 16;
	UTIL_SetSize(pev, pev->mins, pev->maxs );

	pev->origin = vOrigin;*/

	//pev->maxs.z = 16;
	//UTIL_SetSize(pev, pev->mins, pev->maxs );

	//Make the BBox HUGE, so it covers the whole body.
	/*	pev->mins.x = min(pev->mins.x,-128);
	pev->mins.y = min(pev->mins.y,-128);
	pev->mins.z = min(pev->mins.z,-128);
	pev->maxs.x = max(pev->maxs.x,128);
	pev->maxs.y = max(pev->maxs.y,128);
	pev->maxs.z = max(pev->maxs.z,128);*/

	float z = pev->mins.z;
	ExtractBbox(pev->sequence, pev->mins, pev->maxs);
	pev->mins.z = z;

	/*float CheckRange = 1024;
	Vector delta = Vector( CheckRange, CheckRange, CheckRange );
	CBaseEntity *pEnt[100], *pCorpse = NULL;
	int count = UTIL_EntitiesInBox( pEnt, 100, pev->origin - delta, pev->origin + delta, NULL );
	for ( int i = 0; i < count; i++ )
	{
		//Cut every corpse's expire time in half (except players)
		pCorpse = pEnt[i];
		if( !pCorpse || pCorpse == this || !pCorpse->MyMonsterPointer() || pCorpse->IsAlive() || pCorpse->m_pfnThink != &CBaseEntity::SUB_StartFadeOut ) continue;
		float flExpireDelay = pCorpse->pev->nextthink - pCorpse->pev->ltime;
		flExpireDelay *= 0.5;
		pCorpse->pev->nextthink = gpGlobals->time + pCorpse->pev->ltime + flExpireDelay;
	}*/

	SetThink(&CBaseEntity::SUB_StartFadeOut);
	SetTouch(NULL);
	//MAR2008b Reducing remove after death delay, larger creatures linger longer
	//sometimes monsters not going away at all with this? :/
	//float fade_delay = 10.0;
	//if ( MaxHP() > 100 ) fade_delay += ( MaxHP() / 100 );
	//if ( fade_delay > 100 ) fade_delay = 100;
	//Print("fade_delay: %f", fade_delay);
	//( (MaxHP()>0)? 0 : MaxHP() / 100 ) - no can do
	pev->nextthink = gpGlobals->time + 20.0; //MSITEM_TIME_EXPIRE;
	pev->ltime = gpGlobals->time;			 //save the time you died
}

void CMSMonster::ReportAIState()
{
	Print("Name: %s\n", DisplayName());
	Print("Script: %s\n", m_ScriptName.len() ? m_ScriptName.c_str() : "None");
}

/*
	LearnSkill - Called after certain actions to increase your skill stats.  
	����������   Caller gets EnemySkillLevel experience pts and then
				 it checks whether it should advance his stats
*/

bool CMSMonster::LearnSkill(int iStat, int iStatType, int EnemySkillLevel)
{
	float LearnMultiplier = 1.0;

	CStat *pStat = FindStat(iStat);
	if (!pStat)
		return false;

	if (iStatType >= (signed)pStat->m_SubStats.size())
		return false;

	//if ( pStat->Value() >= CHAR_LEVEL_CAP ) return false; //Thoth DEC2008a level cap

	CSubStat &SubStat = pStat->m_SubStats[iStatType];

	//MiB JAN2010_15 Global Level Cap [BEGIN]
	if (SubStat.Value >= CHAR_LEVEL_CAP)
	{
		//Don't go looking for a better option if it's a spell-stat or parry
		if (pStat->m_SubStats.size() == 1 || pStat->m_SubStats.size() > 3)
			return false;

		//Look through the other SubStats for one that's not at the cap, yet.
		//Not random, but good enough.
		for (int i = 0; i < 1; i++)
		{
			iStatType = (iStatType + 1) % 3;
			CSubStat &SubStat = pStat->m_SubStats[iStatType];
			if (SubStat.Value < CHAR_LEVEL_CAP)
				break;
		}

		if (SubStat.Value >= CHAR_LEVEL_CAP)
			return false;
	}
	//MiB JAN2010_15 Global Level Cap [/END]

	SubStat.Exp += max(int(max(EnemySkillLevel, 0) * LearnMultiplier), 0);
	//ALERT( at_console, "Exp: %i %i\n", pStat->Exp[iStatType],  GetSkillStat( sStatName, iStatType )  );
	//ALERT( at_console, "Exp: %f \n", SubStat.Exp ); //thothie attempting to get total XP

	int OldVal = SubStat.Value;
	long double ExpNeeded = GetExpNeeded(OldVal);
	//ALERT( at_console, "Exp: %i/%i\n", pStat->Exp[iStatType],  ExpNeeded  );
	//Must use long double variables because a typecast to it won't work correctly
	long double ExpLeft = SubStat.Exp - ExpNeeded;

	//Shuriken - Message to send the Exp because it gets f'd up at high levels.
	//Thothie AUG2007a - this causes crash when monster kills monster (message not client)
	//- attempting to add isplayer() conditionals
	if (OldVal > 25 && ExpLeft < 0)
	{
		if (IsPlayer())
		{
			MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_EXP], NULL, pev);
			WRITE_BYTE(iStat - SKILL_FIRSTSKILL);
			WRITE_BYTE(iStatType);
			WRITE_LONG(SubStat.Exp);
			MESSAGE_END();
		}
	}

	if (ExpLeft < 0)
		return false;

	SubStat.Value += 1;
	if (SubStat.Value > STATPROP_MAX_VALUE)
		SubStat.Value = STATPROP_MAX_VALUE;
	SubStat.Exp = 0;
	//Thothie - why's this here twice? I dont see where oldval could change.
	if (OldVal > 25)
	{
		if (IsPlayer())
		{
			MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_EXP], NULL, pev);
			WRITE_BYTE(iStat - SKILL_FIRSTSKILL);
			WRITE_BYTE(iStatType);
			WRITE_LONG(0);
			MESSAGE_END();
		}
	}

	return true;
}

float CMSMonster::GetBodyDist(Vector Point, float Radius)
{
	return CBaseMonster::GetBodyDist(Point, Radius) - GetDefaultMoveProximity();
}
bool CMSMonster::IsLootable(CMSMonster *pLooter)
{
	return pLooter->CanDamage(this) && !IsAlive();
}
void CMSMonster::SetSpeed()
{
	pev->maxspeed = 0.0f; //Zero means normal speed

	float SpeedPercent = 100.0f;
	m_Framerate_Modifier = 1.0;

	for (int i = 0; i < m_Scripts.size(); i++)
	{
		if (!m_Scripts[i]->VarExists("game.effect.id"))
			continue;

		if (m_Scripts[i]->VarExists("game.effect.movespeed"))
		{
			float EffectPercent = atof(m_Scripts[i]->GetVar("game.effect.movespeed"));
			SpeedPercent *= (EffectPercent / 100.0f);
		}

		if (m_Scripts[i]->VarExists("game.effect.anim.framerate"))
		{
			float EffectPercent = atof(m_Scripts[i]->GetVar("game.effect.anim.framerate"));
			m_Framerate_Modifier *= EffectPercent;
		}
	}

	if (SpeedPercent != 100.0f)
		pev->maxspeed = SpeedPercent;

	//Can't move while a spell is preparing
	//Thothie/Orochi APR2011_04 - undone
	/*
	 for (int i = 0; i < MAX_NPC_HANDS; i++) 
		if( Hand(i) && !Hand(i)->Spell_CanAttack() )
			SpeedPercent = 0;  //Percentage of normal speed
	*/

	if (!SpeedPercent)
		SetBits(m_StatusFlags, PLAYER_MOVE_NOMOVE);
	//g_engfuncs.pfnSetClientMaxspeed( edict(), fSpeed );
}
//Opens interact menu from server
void CMSMonster::OpenMenu(CBasePlayer *pPlayer)
{
	startdbg;
	m_MenuCurrentOptions = NULL;

	dbg("PrepClient");

	//Thothie SEP2011_07 - prevent menus to players with full inv
	if (pPlayer->NumItems() >= THOTH_MAX_ITEMS)
	{
		pPlayer->SendEventMsg(HUDEVENT_UNABLE, "Cannot use menus while inventory is full.");
		return;
	}

	MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pPlayer->pev);
	WRITE_BYTE(25);
	WRITE_LONG(entindex());
	WRITE_STRING_LIMIT(DisplayName(), WRITE_STRING_MAX);
	MESSAGE_END();

	dbg("Read:game_menu_getoptions");

	static msstringlist Params;
	Params.clearitems();
	Params.add(EntToString(pPlayer));

	m_MenuCurrentOptions = &m_MenuOptions[pPlayer->entindex()];
	mslist<menuoption_t> &Menuoptions = *m_MenuCurrentOptions;
	Menuoptions.clearitems();

	CallScriptEvent("game_menu_getoptions", &Params);
	pPlayer->CallScriptEvent("ext_remove_afk"); //NOV2014_15 - flag player as non-afk when first uses a menu

	m_MenuCurrentOptions = NULL;

	dbg("SendMenu");

	for (int i = 0; i < Menuoptions.size(); i++)
	{
		menuoption_t &MenuOption = Menuoptions[i];
		if (MenuOption.Access != MOA_ALL)
			continue;

		MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pPlayer->pev);
		WRITE_BYTE(26);
		WRITE_BYTE(MenuOption.Access);
		WRITE_STRING_LIMIT(MenuOption.Title, 92);
		WRITE_BYTE(MenuOption.Type);
		WRITE_STRING_LIMIT(MenuOption.Data, 92);
		MESSAGE_END();
	}
	pPlayer->InMenu = true;
	enddbg;
}
void CMSMonster::UseMenuOption(CBasePlayer *pPlayer, int Option)
{
	pPlayer->InMenu = false;
	mslist<menuoption_t> &Menuoptions = m_MenuOptions[pPlayer->entindex()];

	//Thothie JAN2008a - need a way of dealing with canceled menus
	if (Option == -1)
	{
		static msstringlist Params;
		Params.clearitems();
		Params.add(EntToString(pPlayer));
		CallScriptEvent("game_menu_cancel", &Params);
	}

	if (Option < 0 || Option >= (signed)Menuoptions.size())
		return;

	menuoption_t &MenuOption = Menuoptions[Option];

	bool PlayerCanPay = true;

	if (MenuOption.Type == MOT_SAY)
	{
		pPlayer->Speak(MenuOption.Data, SPEECH_LOCAL);
	}
	else if (MenuOption.Type == MOT_PAYMENT)
	{
		static msstringlist Payments;
		Payments.clearitems();

		TokenizeString(MenuOption.Data, Payments);

		int TotalGold = 0;
		static mslist<CGenericItem *> TotalFoundItems;
		TotalFoundItems.clearitems();

		for (int i = 0; i < Payments.size(); i++)
		{
			msstring &Payment = Payments[i];
			if (Payment.starts_with("gold"))
				TotalGold += atoi(Payment.substr(5));
			else
			{
				//Find x number of specified item
				int Amount = 1;
				if (Payment.contains(":"))
				{
					Amount = atoi(Payment.substr(Payment.findchar(":") + 1));
					if (!Amount)
						Amount = 1;
				}

				msstring ItemName = Payment.thru_char(":");

				ulong LastItem = 0;
				ulong FirstItem = 0;
				CGenericItem *pItem = NULL;
				static mslist<CGenericItem *> FoundItems;
				FoundItems.clearitems();
				while ((pItem = pPlayer->GetItemInInventory(LastItem, false, true, true)) && ((signed)FoundItems.size() < Amount))
				{
					if (pItem->ItemName == ItemName)
						FoundItems.add(pItem);

					LastItem = pItem->m_iId;

					if (FirstItem == 0)
						FirstItem = LastItem;
					else if (LastItem == FirstItem)
						break;
				}

				if ((signed)FoundItems.size() < Amount)
				{
					msstring ItemDispName = CGenericItemMgr::GetItemDisplayName(ItemName, false, true, Amount);

					if (!MenuOption.SilentPayment)
						pPlayer->SendEventMsg(HUDEVENT_UNABLE, msstring("You can't afford the payment of ") + ItemDispName);
					PlayerCanPay = false;
					break;
				}

				for (int x = 0; x < FoundItems.size(); x++)
					TotalFoundItems.add(FoundItems[x]);
			}
		}

		int PlayerGold = pPlayer->m_Gold;
		if (PlayerGold < TotalGold)
		{
			if (!MenuOption.SilentPayment)
				pPlayer->SendEventMsg(HUDEVENT_UNABLE, msstring("You can't afford the payment of ") + TotalGold + " Gold");
			PlayerCanPay = false;
		}

		if (PlayerCanPay)
		{
			pPlayer->m_Gold -= TotalGold;
			for (int i = 0; i < TotalFoundItems.size(); i++)
				TotalFoundItems[i]->SUB_Remove(); //MIB JUN2010_14 (original line commented below)
												  //pPlayer->RemoveItem( TotalFoundItems[i] );
		}
	}

	//Handle callback
	static msstringlist Params;
	Params.clearitems();
	Params.add(EntToString(pPlayer));
	Params.add(MenuOption.Data); //Thothie - reg.mitem.data function wasn't returning as PARAM2 in type callback as described by docs, so I tried this, seems to work

	msstring CallbackEvent = MenuOption.CB_Name;

	if (MenuOption.Type == MOT_PAYMENT)
	{
		if (!PlayerCanPay)
			CallbackEvent = MenuOption.CB_Failed_Name;
	}

	if (CallbackEvent.len())
	{
		CallScriptEvent(CallbackEvent, &Params);
	}
	Menuoptions.clearitems();
}

void AlignToNormal(/*In*/ Vector &vNormal, /*In - yaw must be set | Out - Sets pitch and roll*/ Vector &vAngles)
{
	Vector VNormalAngles = UTIL_VecToAngles(vNormal);
	float yDiff = UTIL_AngleDiff(VNormalAngles.y, vAngles.y);
	float yDiffMult = yDiff > 0 ? 1 : -1;
	float yoffset = -((fabs(yDiff) / 90) - 1);

	vAngles.x = (VNormalAngles.x - 90) * yoffset;
	vAngles.z = (VNormalAngles.x - 90) * (1 - fabs(yoffset)) * yDiffMult;
}