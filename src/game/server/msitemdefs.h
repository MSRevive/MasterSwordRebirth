//Definitions specific to Master Sword that are used in various places

#ifndef MSITEMDEFS
#define MSITEMDEFS

//#define DG_DEBUG //REM this line out when testing MS with others
//#define DG_WEAPONBEAMS //Show weapon tracelines

//Expiretime was 60, upped
constexpr int MSITEM_TIME_EXPIRE = 120; //Time until an item on the ground disappears

enum item_flags
{
	METHOD_NONE,
	METHOD_PICKEDUP,		//Sometimes do or don't do something based on whether you just picked it up
	METHOD_DROPPED,			//..based on whether you just dropped it
	METHOD_SWITCHEDHANDS,	//..based on whether you just switched hands
	METHOD_REMOVEDPACK,		//..based on whether you just removed a pack
	METHOD_REMOVEDFROMPACK, //..based on whether you just removed it from a pack
	METHOD_PUTINPACK,		//..based on whether you just put it in a pack
	METHOD_WEAR				//..based on whether you just wore it
};

enum
{				   //Counter Effects
	CE_HITMONSTER, //Item hit an npc (weapon or projectile)
	CE_HITWORLD,   //Hit the world
	CE_SHIELDHIT,  //A shield was hit
};

//Menus
enum
{
	MENU_USERDEFINED = 1,
};

//Actions
enum
{
	ACTION_SIT,
	ACTION_STAND,
};

//MS Object movetypes
enum
{
	MOVETYPE_NORMAL,
	MOVETYPE_ARROW,
	MOVETYPE_STUCKARROW,
};

//MS-Specifiv client-side effects for studio-model-based entities, sent through pev->playerclass
//Don't use for brush models because the engine uses playerclass on brushes to specify that it can be decaled
enum {
	ENT_EFFECT_FOLLOW_ROTATE = (1 << 0),		//An entity that follows another on the client.  Rotates with the host entity.  pev->basevelocity.x specifies distance out from the host
	ENT_EFFECT_FOLLOW_ALIGN_BOTTOM = (1 << 1)//The follow entity is aligned to the bottom of the host's bounding box (in players, the bottom is 36 units lower)
	//ENT_EFFECT_FOLLOW_FACE_HOST = (1<<2)		//The follow entity always faces the host (pitch ignored)
};

// ***** Player *****

constexpr int PLAYER_GIB_HEATH = -40;

//These are for CBasePlayer::MoreBTNSDown

enum {
	BTN_SPECIAL = (IN_SCORE * 2),
	BTN_ATTACKLEFT = (BTN_SPECIAL * 2),
	BTN_ATTACKRIGHT = (BTN_ATTACKLEFT * 2)
};

//These are for CBasePlayer::m_StatusFlags

enum {
	PLAYER_MOVE_RUNNING = (1 << 0),
	PLAYER_MOVE_ATTACKING = (1 << 1),
	PLAYER_MOVE_JUMPING = (1 << 2),
	PLAYER_MOVE_SWIMMING = (1 << 3) ,//Set by script.. really just an indicator for the client that it should rotate the model by pitch
	PLAYER_MOVE_NORUN = (1 << 3),
	PLAYER_MOVE_NOJUMP = (1 << 4),
	PLAYER_MOVE_NODUCK = (1 << 5),
	PLAYER_MOVE_NOATTACK = (1 << 6),
	PLAYER_MOVE_NOMOVE = (1 << 7),
	PLAYER_MOVE_SITTING = (1 << 8), //JAN2010_09 Thothie - Attempting to allow inventory access while sittin again
	PLAYER_MOVE_STOPRUN = (1 << 9)
};

//#define PLAYER_MOVE_SPECTATE		(1<<3)
//#define PLAYER_MOVE_SPECTATE_MOVE	(1<<4)
//#define PLAYER_MOVE_STUNNED		(1<<3)

//***** Item properties *****

enum {
	ITEM_WEARABLE = (1 << 0),
	ITEM_SHIELD = (1 << 1),
	ITEM_ARMOR = (1 << 2),
	ITEM_GENERIC = (1 << 3),
	ITEM_GROUPABLE = (1 << 4),
	ITEM_CONTAINER = (1 << 5),
	ITEM_PROJECTILE = (1 << 6),
	ITEM_DRINKABLE = (1 << 7),
	ITEM_PERISHABLE = (1 << 8),
	ITEM_SPELL = (1 << 9),
	ITEM_MS_BODYPART = (1 << 10),
	ITEM_MS_NPC = (1 << 11),
	ITEM_MS_PLAYER = (1 << 12),
	ITEM_MS_CORPSE = (1 << 13),
	ITEM_MS_SHIELD = (1 << 14),
	ITEM_NOPICKUP = (1 << 15) //MIB FEB2010_13 [thothie: 1<<10 is taken below - switched to 1<<15]
};

//Shield Sounds
constexpr const char* SOUND_IRONSHIELD_HIT1 = "weapons/cbar_hit1.wav"; //global sounds
constexpr const char* SOUND_IRONSHIELD_HIT2 = "weapons/cbar_hit2.wav";

//Corpse
constexpr const char* CLASS_CORPSE = "corpse";

#endif //MSITEMDEFS