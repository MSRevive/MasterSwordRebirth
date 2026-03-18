// eventscripts.h
#if !defined(EVENTSCRIPTSH)
#define EVENTSCRIPTSH

// defaults for clientinfo messages
constexpr int DEFAULT_VIEWHEIGHT = 28;
constexpr int VEC_DUCK_VIEW = 12;

enum {
	FTENT_FADEOUT = 0x00000080
};

enum {
	DMG_GENERIC = 0,            // generic damage was done
	DMG_CRUSH = (1 << 0),       // crushed by falling or moving object
	DMG_BULLET = (1 << 1),      // shot
	DMG_SLASH = (1 << 2),       // cut, clawed, stabbed
	DMG_BURN = (1 << 3),        // heat burned
	DMG_FREEZE = (1 << 4),      // frozen
	DMG_FALL = (1 << 5),        // fell too far
	DMG_BLAST = (1 << 6),       // explosive blast damage
	DMG_CLUB = (1 << 7),        // crowbar, punch, headbutt
	DMG_SHOCK = (1 << 8),       // electric shock
	DMG_SONIC = (1 << 9),       // sound pulse shockwave
	DMG_ENERGYBEAM = (1 << 10), // laser or other high energy beam
	DMG_NEVERGIB = (1 << 12),   // with this bit OR'd in, no damage type will be able to gib victims upon death
	DMG_ALWAYSGIB = (1 << 13),  // with this bit OR'd in, any damage type can be made to gib victims upon death.

	// time-based damage
	//mask off TF-specific stuff too
	DMG_TIMEBASED = (~(0xff003fff)), // mask for time-based damage
	DMG_DROWN = (1 << 14), // Drowning
	DMG_FIRSTTIMEBASED = DMG_DROWN,
	DMG_PARALYZE = (1 << 15),     // slows affected creature down
	DMG_NERVEGAS = (1 << 16),     // nerve toxins, very bad
	DMG_POISON = (1 << 17),       // blood poisioning
	DMG_RADIATION = (1 << 18),    // radiation exposure
	DMG_DROWNRECOVER = (1 << 19), // drowning recovery
	DMG_ACID = (1 << 20),         // toxic chemicals or acid burns
	DMG_SLOWBURN = (1 << 21),     // in an oven
	DMG_SLOWFREEZE = (1 << 22),   // in a subzero freezer
	DMG_MORTAR = (1 << 23),       // Hit by air raid (done to distinguish grenade from mortar)
	DMG_IGNITE = (1 << 24),       // Players hit by this begin to burn
	DMG_RADIUS_MAX = (1 << 25),   // Radius damage with this flag doesn't decrease over distance
	DMG_RADIUS_QUAKE = (1 << 26), // Radius damage is done like Quake. 1/2 damage at 1/2 radius.
	DMG_IGNOREARMOR = (1 << 27),  // Damage ignores target's armor
	DMG_AIMED = (1 << 28),        // Does Hit location damage
	DMG_WALLPIERCING = (1 << 29), // Blast Damages ents through walls
	DMG_CALTROP = (1 << 30),
	DMG_HALLUC = (1 << 31)
};

// Some of these are HL/TFC specific?
void EV_EjectBrass(float *origin, float *velocity, float rotation, int model, int soundtype);
void EV_GetGunPosition(struct event_args_s *args, float *pos, float *origin);
void EV_GetDefaultShellInfo(struct event_args_s *args, float *origin, float *velocity, float *ShellVelocity, float *ShellOrigin, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale);
qboolean EV_IsLocal(int idx);
qboolean EV_IsPlayer(int idx);
void EV_CreateTracer(float *start, float *end);

struct cl_entity_s *GetEntity(int idx);
struct cl_entity_s *GetViewEntity(void);
void EV_MuzzleFlash(void);

#endif // EVENTSCRIPTSH
