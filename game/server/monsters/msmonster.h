#ifndef _MSMONSTER
#define _MSMONSTER

#include "monsteranimation.h"
#include "genitemlist.h"
#include "stats/stats.h"

class CStore;
class CMSMonster;
class CBaseBody;
class CScript;
class CGenericItem;
struct storeitem_t;
struct playerdamage_t;

struct dest_t //Destination
{
	Vector Origin;			 //Location
	float Proximity;		 //How close I need to be to 'arrive'
	entityinfo_t MoveTarget; //Target entity (if moving to entity)
	bool operator==(dest_t &a) { return Origin == a.Origin && Proximity == a.Proximity; }
	bool operator!=(dest_t &a) { return !operator==(a); }
};

struct word_t
{ //For talking with mouth syncronization
	string_i Soundfile;
	float Duration;
	bool Spoken;
};
struct listenphrase_t
{ //For listening for player-typed messages
	mslist<string_i> Phrases;
	string_i ScriptEvent;
};

enum
{ //Trade status
	TRADE_BUY,
	TRADE_SELL,
	TRADE_OFFERPRICE,
	TRADE_REJECTPRICE
};
struct tradeinfo_t
{
	char *ItemName;
	int iStatus, iPrice;
	CMSMonster *pCustomer;
	CGenericItem *pItem;
	storeitem_t *psiStoreItem;
	CStore *pStore;
};

enum itemtype_e
{ //Offer item type
	ITEM_NORMAL,
	ITEM_GOLD,
};
struct offerinfo_t
{ //For offers to other players/monsters
	int SrcMonsterIDX;
	CMSMonster *pSrcMonster;
	itemtype_e ItemType;
	void *pItemData, *pItemData2;
};

enum npc_attack
{
	NPCATT_ANIM,
	NPCATT_HAND
};

enum speech_type
{
	SPEECH_GLOBAL,
	SPEECH_LOCAL,
	SPEECH_PARTY
};
#define SPEECH_LOCAL_RANGE 300

enum gender_e
{
	GENDER_MALE,
	GENDER_FEMALE,
	GENDER_UNKNOWN
};

struct damage_t
{
	damage_t() { clrmem(*this); }
	//Set before the attack
	CBaseEntity *pInflictor;	   //Could be monster, could be weapon or arrow
	CBaseEntity *pAttacker;		   //Always CMSMonster or NULL
	CBaseEntity *pDirectDmgTarget; //If doing direct damage, always hit this entity
	Vector vecSrc;
	Vector vecEnd;
	float flOriginalDamage;		  //Original damage
	float flDamage;				  //Adjusted damage (for armor, npc takedamagemodifiers, parry, etc.)
	int iDamageType;			  //Coded damage type
	int iHitGroup;				  //Player hitgroup that was hit
	bool ExpUseProps;			  //Give exp to the following stat/prop
	int ExpStat, ExpProp;		  //Stat and prop that receives exp from this attack
	string_i sDamageType;		  //Custom damage type from script
	msstring *ItemCallBackPrefix; //Item callback prefix, if this attack was caused by an item
	float flHitPercentage;
	float flDamageRange;
	float flRange;			 //Range of normal attack.  If 0, use traceline
	float flAOERange;		 //Area of effect range //Range of normal attack
	float flAOEAttn;		 //Area of effect attenuation
	msstring dodamage_event; //Thothe OCT2011_04 - allow alternative game_dodamage event for xdodamage
	//Calulated during the attack
	int AccuracyRoll;
	bool AttackHit; //Whether the roll cuased a hit or miss
	class CGenericItem *pParryItem;
	TraceResult outTraceResult;
	bool nodecal; //Thothie MAR2012_16 non-bullet/noise damage
};

struct moveexec_t
{
	Vector vecStart, vecEnd;
	CBaseEntity *pTarget;

	bool fTestMove,
		fAllowStep;

	float Return_DistMoved;
	bool Return_HitTargetEnt,
		Return_HitStep;
};

enum menuoptionaccess_e
{
	MOA_ALL,
	MOA_PLAYER,
	MOA_PARTY
};
enum menuoptiontype_e
{
	MOT_CALLBACK,
	MOT_PAYMENT,
	MOT_DISABLED,
	MOT_GIVEITEM,
	MOT_SAY,
	MOT_FORGIVE,
	MOT_DESC,
	MOT_GREEN
}; //Thothie AUG2013_12 - new green type for completed tally
struct menuoption_t
{
	msstring ID;			   //Script-defined ID
	int RestrictedPlayerIdx;   //Only this player can use this menuitem
	msstring RestrictedParty;  //Only this party can use this menuitem
	menuoptionaccess_e Access; //Access level of this menuitem
	bool SilentPayment;		   //If you can't afford the payment, the game won't tell you what you're missing
	int Priority;			   //Priority - higher = more
	msstring Title;			   //Title
	menuoptiontype_e Type;	   //Type of menuitem (require payment, require item transfer, open trade window, etc)
	msstring Data;			   //Data specific to the Type
	msstring CB_Name;		   //Script eventname for callback when the action succeeds (default if there is no fail condition)
	msstring CB_Failed_Name;   //Script eventname for callback when the action fails
};

struct getitem_t
{
	msstring Name;				 //Name to check for
	bool IgnoreHands;			 //Check inside player hands
	bool IgnoreWornItems;		 //Check items player is wearing
	bool IgnoreInsideContainers; //Check inside containers
	bool CheckPartialName;		 //Name only needs to appear somewhere within the Item Name

	bool retFound;				//Was item found?
	CGenericItem *retItem;		//The item that was found
	CGenericItem *retContainer; //The container the item was in
};

//NOV2014_20 - Thothie msmonster_random [begin]
//seems we're wasting memory on the client, since it doesn't use this, but ifndef'ing kills the compile on the server side
struct random_monster_t
{
	msstring m_title, //title
		m_addparams;  //params

	string_i m_ScriptName; //scriptfile

	float m_HPMulti, //hpmulti
		m_DMGMulti;	 //dmgmulti

	int				  //m_Lives, //lives (undone, difficult to apply in spawner)
		m_HPReq_min,  //reqhp (min;)
		m_HPReq_max,  //reqhp (;max)
		m_ReqPlayers, //nplayers
		m_HPReq_useavg;
};
//NOV2014_20 - Thothie msmonster_random [end]

//Frame conditions
#define FC_STEP (1 << 1)  //Monster walked up a step this frame
#define FC_JUMP (1 << 2)  //Monster began a jump this frame
#define FC_AVOID (1 << 3) //Monster is avoiding an object this frame

//General Conditions (Added to those defined in schedule.h)
#define MONSTER_ROAM (1 << 23)			// Monster should roam around
#define MONSTER_HASMOVEDEST (1 << 24)	// vMoveDest is valid
#define MONSTER_TRADING (1 << 25)		// NPC is trading with a player
#define MONSTER_REFLECTIVEDMG (1 << 26) // Damage is reflected back to attacker
#define MONSTER_NOAI (1 << 27)			// Don't normal script events
#define MONSTER_OPENCONTAINER (1 << 28) // Player is looking inside a pack
#define MONSTER_BLIND (1 << 29)			// MAR2008b - Monster is blind
#define MONSTER_INVISIBLE (1 << 30)		// MAR2008b - Monster is invisible

#define MAX_ENEMYLIST 12
#define MAX_NPC_HANDS 3

class CMSMonster : public CBaseMonster
{
public:
	float m_StepSize,	  //How high I can step
		m_NodeCancelTime, //Time I should quit trying to get to the current node
		m_NextNodeTime,	  //Time I'll be on my way to the next node
		m_RoamDelay,
		//FloatTime,		//Next time I should try to rise to the surface
		m_SpeedMultiplier,	  //Multiply my speed times this
		m_TurnRate,			  //Turn ratio (1.0 == full turn instantly 0.0 = never turn)
		m_LastYaw,			  //Yaw last frame.  If pev->angles.y changes from this, recalculate my ground orientation
		m_HearingSensitivity, //Sensitivity to sounds

		m_LookTime,	  //Time I'll next check around for targets
		m_ListenTime, //Time I'll next listen for targets
		m_AvoidTime,  //Time I'll try to avoid what's in front of me
		//MoveTime,			//Time I'll update my movement path

		//AttackTime,		//Time I can next attack
		//AttackLandTime,	//Time my attack actually tracelines and hits

		m_SpawnDelayLow,  //My respawn is a random value between these
		m_SpawnDelayHigh, //two values
		m_SpawnChance,	  //% chance I'll spawn

		m_Width,		//Width for bounding box
		m_Height,		//Height for bounding box
		m_HITMulti,		//Thothie FEB2009_18 - multiply hit chances by this amount
		m_SayTextRange; //Max range for text speech

	float m_HP, //Health, mana
		m_MP,
		m_MaxHP, //m_MaxHP/m_MaxMP should not be read for determining max amounts.  Use MaxHP() and MaxHP()
		m_MaxMP; //The functions factor in status ailments, and player skills.  Players dont even use m_MaxHP/m_MaxMP

	int m_Gold;	// Amount of gold I'm carrying
	int m_OldGold; // Last Gold Value - for updating

	byte m_Gender;			//My Gender (default male)
	bool m_fSpawnOnTrigger, //Spawn only when triggered?
		m_UseExpStat;		//Whether m_ExpStat & m_ExpProp are valid

	int m_CurrentHand, //Current hand
		//ItemModelBodyOffset, //Neccesary hack for using the same p_ models for player and monsters
		m_Lives,   //I can only die this many times
		m_ExpStat, //The stat/prop that is given exp when the script command dodamage is called
		m_ExpProp,
		m_HPReq_min,  //Thothie AUG2007a - adding optional req total hp on server to spawn monster
		m_HPReq_max,  //Thothie FEB2011_22 - adding option for "min;max" hpreq
		m_ReqPlayers, //Thothie AUG2007a - adding optional REQ players to spawn monster
		m_nRndMobs,	  //NOV2014_20 - Thothie msmonster_random
		m_HPReq_useavg;

	float m_HPMulti, //Thothie SEP2007a - multiply HP by this amount
		m_DMGMulti;	 //Thothie SEP2007a - multiply DMG by this amount

	bool m_nopush; //Thothie MAR2008a - immune to push

	float Stamina; //Your strength left in combat
				   //WorldVolume;   //Sound that other monsters can hear

	msstring m_IdleAnim, //Idle animation
		m_MoveAnim,		 //Movement animation
		m_title,		 //Thothie AUG2007b - allow dynamic naming of monsters from map entity
		m_spawnedby,	 //Thothie DEC2007a - Entity that made me (used to check for existance of monsterspawn)
		m_addparams,	 //Thothie DEC2007a - send these paramaters to script from map entity
		//ActionAnim,	//Action animation (Attacks, etc.)

		m_iszMonsterSpawnArea, //targetname of my monster spawn ent
		m_iszKillTarget,	   //Fire upon death
		m_iszPerishTarget,	   //Fire upon perish (died with 0 lives left)

		m_TradeCallBackEvent; //Prefix for the trade callbacks
							  // Skin;			//Name of the skin I become when skinned
	string_i m_ScriptName;	  //Name of my main script

	mslist<random_monster_t> random_monsterdata; //NOV2014_20 - Thothie msmonster_random

	char m_szAnimExtention[32];
	const char *pszCurrentAnimName;
	CAnimation *m_pAnimHandler, *m_pLastAnimHandler;
	dest_t m_MoveDest;		  //Current move destination
	Vector m_MoveDir;		  //Current move direction (accounts for slopes)
	Vector m_GroundNormal;	  //Current normal of ground I'm standing on
	Vector m_LastOrigin;	  //Origin last frame.  If this changes, recalculate my ground orientation
	float m_MaxGroundSlope,	  //Steepest ground slope that the monster can traverse
		m_Framerate,		  //Current framerate
		m_Framerate_Modifier; //Dynamic framerate modifier - set by script
	bool m_Wandering,		  //This mode dest is just a wander, and not important
		m_StatsCreated,		  //Stats have been created and initialized
		m_Menu_Autoopen;	  //Auto open interact menu when player does +use on me

	MonsterEvent_t LastEvent;
	int FrameConditions;	//Things I've has already done this frame (Step, Jump, etc.)
	int m_StatusFlags;		//Multi-frame conditions (burning, stunned, etc.)
	Activity m_OldActivity; //What the monster was doing last frame (animation)

	// Store the enemies you see around you (includes anything alive with
	//a relationship better than R_NO)
	EHANDLE m_hEnemyList[MAX_ENEMYLIST],
		m_LastEnemy;
	int m_EnemyListNum;

	CBaseBody *Body;								//If this monster uses body parts
	CItemList Gear;									//This monster's gear
	mslist<menuoption_t> m_MenuOptions[MAXPLAYERS]; //The current menu options for each player
	mslist<menuoption_t> *m_MenuCurrentOptions;		//Only set during OpenMenu - Used in script operations

	CStore *OpenStore;					   //Current store I'm offering to someone
	offerinfo_t m_OfferInfo;			   //The current offer from another player/monster
	mslist<playerdamage_t> m_PlayerDamage; //Stats on who has damaged me
	virtual void MarkDamage(CBasePlayer * pPlayer, CGenericItem * pItem, float vAmount);
	virtual void MarkDamage(CBasePlayer * pPlayer, msstring vsStat, float vAmount);
	virtual void MarkDamage(CBasePlayer * pPlayer, int vStat, int vProp, float vAmount);

	struct takedamagemodifier_t
	{
		string_i DamageType;
		float modifier;
	};

	struct props
	{
		props() { GenericTDM = 1.0f; }
		mslist<takedamagemodifier_t> TakeDamageModifiers; //Damage modifiers for specific damage types
		float GenericTDM;								  //Damage modifier for any damage
	} m;

	static void DynamicPrecache(); //Precaches monsters specified in the ms_dynamicnpc CVAR.

	CMSMonster();
	~CMSMonster();

	virtual void Deactivate(); //Called when the server shuts down
	virtual bool IsMSMonster(void) { return true; }
	virtual float MaxHP() { return m_MaxHP; }
	virtual float MaxMP() { return m_MaxMP; }
	virtual float GetDefaultMoveProximity() { return m_Width * 1.1; }

	BOOL IsAlive() { return pev ? pev->deadflag == DEAD_NO : FALSE; }
	virtual void CancelAttack();
	//virtual void Step( );
	//virtual void Jump( );

	virtual CGenericItem *GetContainer(const char *pNameSubstring);
	virtual CGenericItem *GetContainer(ULONG ID);
	virtual CGenericItem *GetGearItem(ULONG ID);
	virtual CGenericItem *GetItem(const char *pAmmoName, CGenericItem **pPack = NULL);
	virtual bool GetItem(getitem_t &ItemDesc);

	virtual CGenericItem *GetItemInInventory(uint StartID, bool WeaponOnly, bool CheckHands, bool CheckWorn, msstring SearchName = "");
	CGenericItem *FindItem(ulong ID) { return FindItem(ID, true, true, true); }
	virtual CGenericItem *FindItem(ulong ID, bool CheckHands, bool CheckWorn, bool CheckPacks);
	virtual CGenericItem *Hand(int Handidx);

	virtual int NewItemHand(CGenericItem *pItem, bool CheckWeight, bool bVerbose, bool FreeHands = false, char *pszErrorString = NULL);
	virtual bool CanHold(CGenericItem *pItem, bool bVerbose = true, char *pszErrorString = NULL);
	virtual bool AddItem(CGenericItem *pItem, bool ToHand, bool CheckWeight, int ForceHand = -1);
	virtual bool RemoveItem(CGenericItem *pItem);
	virtual void DropAllItems();
	virtual CGenericItem *ActiveItem();
	virtual bool SwitchHands(int iHand, bool bVerbose = true); //Switch to a held item
	virtual bool SwitchToBestHand();
	virtual float Weight();

	virtual float WalkSpeed(bool bParseSpeed = true);
	virtual float RunSpeed(bool bParseSpeed = true);

	virtual bool IsActing();																   //Am I attacking or charging an attack?
	virtual bool IsShielding();																   //Am I holding a shield up?
	virtual bool IsFlying() { return FBitSet(pev->flags, (FL_FLY | FL_SWIM)) ? true : false; } //Don't align to ground and don't need to be onground to move

	//Script functions
	//static void SpeechPrecache( );
	mslist<word_t> m_Words;
	float m_TimeLastSpoke;
	mslist<listenphrase_t> m_Phrases;

//IScripted
#ifdef VALVE_DLL
	void Script_Setup();																				  //Ties m_pScriptActions and m_pScriptConditions to
	bool Script_ExecuteCmd(CScript *Script, SCRIPT_EVENT &Event, scriptcmd_t &Cmd, msstringlist &Params); //Runs a single command
	int Script_ParseLine(CScript *Script, msstring_ref pszCommandLine, scriptcmd_t &Cmd);
	bool GetScriptVar(msstring &ParserName, msstringlist &Params, CScript *BaseScript, msstring &Return);
	msstring_ref GetProp(CBaseEntity *pTarget, msstring &FullParams, msstringlist &Params);
#endif

	static scriptcmdname_list m_ScriptCommands; //Global monster script commands

	//Stats
	char m_Race[64];
	statlist m_Stats;		  //All stats.  Natural stats and Skill-based stats
	msstringlist m_SpellList; //Known spells (scriptnames)
	float m_SkillLevel;		  //How much experience I'm worth

	virtual bool CreateStats();
	virtual void DeleteStats();
	int GetNatStat(int iStatName) { return GetStat(iStatName, 0); }
	int GetSkillStat(int iStatIdx) { return GetStat(iStatIdx, 1); }
	int GetSkillStat(int iStatIdx, int StatProperty);
	int GetSkillStat(msstring_ref StatName, int StatProperty);
	int GetSkillStatCount();
	CStat *FindStat(int idx);			//False = Natural Stat / True = Skill Stat
	CStat *FindStat(msstring_ref Name); //Find any stat
	int GetStat(int iStat, int iStatType);
	int GiveGold(int iAmount, bool fVerbose = true);

	//Overridden
	int MSProperties();
	float HearingSensitivity(void) { return m_HearingSensitivity; }
	int ObjectCaps(void) { return CBaseMonster ::ObjectCaps() | FCAP_IMPULSE_USE; }
	BOOL HasHumanGibs() { return TRUE; }
	BOOL ShouldFadeOnDeath(void) { return FALSE; }

#ifdef VALVE_DLL
	virtual float GiveHP(float flAmount)
	{
		return Give(GIVE_HP, flAmount);
	}
	virtual float GiveMP(float flAmount) { return Give(GIVE_MP, flAmount); }
	float Give(givetype_e Type, float Amt); //Give/Take Health, MP, Gold, etc
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	virtual float TraceAttack(struct damage_t &Damage);
	void CounterEffect(CBaseEntity *pInflictor, int iEffect, void *pExtraData);
	float GetBodyDist(Vector Point, float Radius);

	virtual bool LearnSkill(int iStat, int iStatType, int EnemySkillLevel);
	virtual bool LearnSkill(int iStat, int EnemySkillLevel) { return false; }

	virtual BOOL CanSetVelocity();
	virtual void SetAnimation(MONSTER_ANIM AnimType, const char *pszAnimName = NULL, void *vData = NULL);
	virtual void BreakAnimation(MONSTER_ANIM AnimType, const char *pszAnimName = NULL, void *vData = NULL); //Thothie - JUN2007b
	virtual void FaceForward();																				//Face forward, accounting for the ground slope

	virtual void Speak(char *pszSentence, speech_type SpeechType);
	virtual void Used(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual void Trade();												 // Handle trading with others
	virtual bool AcceptOffer();											 // Accept an offer from a player or monster
	virtual BOOL SkinMonster(CMSMonster *pDeadMonster) { return FALSE; } //Skin other monsters
	virtual tradeinfo_t *TradeItem(tradeinfo_t *tiTradeInfo);			 // Trade an item
	virtual void Attacked(CBaseEntity *pAttacker, damage_t &Damage);
	virtual void Seen(CMSMonster *pMonster) {}
	virtual void DieThink();
	virtual bool IsInAttackStance() { return false; }

	void Activate();
	void KeyValue(KeyValueData *pkvd);
	void Spawn(msstring_ref ScriptName)
	{
		m_ScriptName = ScriptName;
		Spawn();
	}
	void Spawn();
	void Precache();
	virtual void Act();												   //Handle actions, like attacks
	virtual void Say(msstring_ref Sound, float fDuration);			   //Stores the next word if already playing one
	virtual void HearPhrase(CMSMonster *pSpeaker, const char *phrase); //Called when a player talks (normal mode)
	virtual void Talk();											   // Called every frame, Talk if it's time to speak up
	void Touch(CBaseEntity *pOther);
	virtual void HandleAnimEvent(MonsterEvent_t *pEvent);

	virtual bool CanDamage(CBaseEntity *pOther);  //Can I damage this entity?
	virtual void Look();						  //Look around for targets
	virtual void ListenForSound();				  //Listen for noises
	virtual void Float();						  //Apply special float velocity
	virtual void SetMoveDest();					  //Follow m_MoveDest
	virtual void SetWanderDest();				  //Wander idly
	virtual void Move(float flInterval);		  //Move forward, fall, align to ground, go up steps
	virtual void AvoidFrontObject(float MoveAmt); //Turn to avoid the object in front of me

	//virtual int  MoveExecute( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist, bool fTestMove = false );
	virtual bool MoveExecute(moveexec_t &MoveExec);
	virtual void StopWalking();

	int Classify();
	int IRelationship(CBaseEntity *pTarget);
	void ReportAIState(void);

	void Think();
	void Killed(entvars_t *pevAttacker, int iGib);
	void SUB_Remove();

	virtual bool IsLootable(CMSMonster *pLooter);
	virtual void SetSpeed(); //Determine proper speed

	virtual void OpenMenu(CBasePlayer *pPlayer);
	virtual void UseMenuOption(CBasePlayer *pPlayer, int Option);
#endif
};

extern DLL_GLOBAL bool g_fInPrecache;
long double GetExpNeeded(int StatValue);
void AlignToNormal(/*In*/ Vector &vNormal, /*In - yaw must be set | Out - Sets pitch and roll*/ Vector &vAngles);

#endif //_MSMONSTER
