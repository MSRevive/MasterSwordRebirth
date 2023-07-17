//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "cbase.h"
#include "CASTrace.h"

#include <stdstring.h>

#include "angelscript/HL2ASConstants.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#else
#include "angelscript/CASCManager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#include "memdbgoff.h"

/**PAGE
 *
 * Manipulates the CGameTrace object entity, and it's values.
 *
 * @page CGameTrace
 * @category Entities
 *
 * @desc_md false
 * @type class
 *
*/

void ConstructTrace( CGameTrace* pMemory )
{
	new ( pMemory ) CGameTrace();
}

#include "memdbgon.h"

void DestroyTracer( CGameTrace* pMemory )
{
	pMemory->~CGameTrace();
}

/**JSON
 * Enumeration of mask types.
 *
 * @type enum
 * @name MASK
 *
 * @args MASK_ALL # checks everything
 * @args MASK_SOLID # everything that is normally solid
 * @args MASK_NPCSOLID # blocks npc movement
 * @args MASK_WATER # water physics in these contents
 * @args MASK_OPAQUE # everything that blocks lighting
 * @args MASK_VISIBLE # everything that blocks line of sight for players
 * @args MASK_SHOT # bullets see these as solid
 * @args MASK_SHOT_HULL # non-raycasted weapons see this as solid (includes grates)
 * @args MASK_SHOT_PORTAL # hits solids (not grates) and passes through everything else
 * @args MASK_SOLID_BRUSHONLY # everything normally solid, except monsters (world+brush only)
 * @args MASK_PLAYERSOLID_BRUSHONLY # everything normally solid for player movement, except monsters (world+brush only)
 * @args MASK_SPLITAREAPORTAL # These are things that can split areaportals
 * @args MASK_CURRENT # any moving water
 * @args MASK_DEADSOLID # everything that blocks corpse movement
 * @args MASK_L4D_VISION # everything that blocks line of sight in Left4Dead (players and bots all use this, and it hits other players and bots)
 * @args MASK_CONTAGION_VISION # Same as MASK_L4D_VISION, except, we don't want to ignore OPAQUE, since some models uses it.
 *
 */


/**JSON
 * Enumeration of collision group types.
 *
 * @type enum
 * @name COLLISIONGROUP
 *
 * @args COLLISION_GROUP_NONE # No collision
 * @args COLLISION_GROUP_DEBRIS # Debris
 * @args COLLISION_GROUP_DEBRIS_TRIGGER # Debris that can interact with triggers
 * @args COLLISION_GROUP_INTERACTIVE_DEBRIS # Debris that can be interacted by players
 * @args COLLISION_GROUP_INTERACTIVE # Interactive by players
 * @args COLLISION_GROUP_PLAYER # Players
 * @args COLLISION_GROUP_BREAKABLE_GLASS # Breakable glass planes
 * @args COLLISION_GROUP_PLAYER_MOVEMENT # Players (When not standing still)
 * @args COLLISION_GROUP_WEAPON # Weapons
 * @args COLLISION_GROUP_PROJECTILE # Projectiles
 * @args COLLISION_GROUP_DOOR_BLOCKER # Door blocker
 * @args COLLISION_GROUP_PASSABLE_DOOR # Passable doors
 * @args COLLISION_GROUP_DISSOLVING # Dissolving entities
 *
 */
static void RegisterScriptTracer_Utils( asIScriptEngine& engine )
{
	// MASKING
	const char* const pszObjectName = "MASK";
	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "MASK_ALL", (int)MASK_ALL );
	engine.RegisterEnumValue( pszObjectName, "MASK_SOLID", (int)MASK_SOLID );
	engine.RegisterEnumValue( pszObjectName, "MASK_NPCSOLID", (int)MASK_NPCSOLID );
	engine.RegisterEnumValue( pszObjectName, "MASK_WATER", (int)MASK_WATER );
	engine.RegisterEnumValue( pszObjectName, "MASK_OPAQUE", (int)MASK_OPAQUE );
	engine.RegisterEnumValue( pszObjectName, "MASK_VISIBLE", (int)MASK_VISIBLE );
	engine.RegisterEnumValue( pszObjectName, "MASK_SHOT", (int)MASK_SHOT );
	engine.RegisterEnumValue( pszObjectName, "MASK_SHOT_HULL", (int)MASK_SHOT_HULL );
	engine.RegisterEnumValue( pszObjectName, "MASK_SHOT_PORTAL", (int)MASK_SHOT_PORTAL );
	engine.RegisterEnumValue( pszObjectName, "MASK_SOLID_BRUSHONLY", (int)MASK_SOLID_BRUSHONLY );
	engine.RegisterEnumValue( pszObjectName, "MASK_SPLITAREAPORTAL", (int)MASK_SPLITAREAPORTAL );
	engine.RegisterEnumValue( pszObjectName, "MASK_CURRENT", (int)MASK_CURRENT );
	engine.RegisterEnumValue( pszObjectName, "MASK_DEADSOLID", (int)MASK_DEADSOLID );
	engine.RegisterEnumValue( pszObjectName, "MASK_L4D_VISION", (int)MASK_L4D_VISION );
	engine.RegisterEnumValue( pszObjectName, "MASK_CONTAGION_VISION", (int)MASK_CONTAGION_VISION );
	
	// CONTENTS
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_EMPTY", (int)CONTENTS_EMPTY );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_SOLID", (int)CONTENTS_SOLID );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_WINDOW", (int)CONTENTS_WINDOW );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_AUX", (int)CONTENTS_AUX );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_GRATE", (int)CONTENTS_GRATE );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_SLIME", (int)CONTENTS_SLIME );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_WATER", (int)CONTENTS_WATER );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_BLOCKLOS", (int)CONTENTS_BLOCKLOS );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_OPAQUE", (int)CONTENTS_OPAQUE );
	engine.RegisterEnumValue( pszObjectName, "LAST_VISIBLE_CONTENTS", (int)LAST_VISIBLE_CONTENTS );
	engine.RegisterEnumValue( pszObjectName, "ALL_VISIBLE_CONTENTS", (int)ALL_VISIBLE_CONTENTS );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_TESTFOGVOLUME", (int)CONTENTS_TESTFOGVOLUME );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_TEAM1", (int)CONTENTS_TEAM1 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_TEAM2", (int)CONTENTS_TEAM2 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_IGNORE_NODRAW_OPAQUE", (int)CONTENTS_IGNORE_NODRAW_OPAQUE );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_MOVEABLE", (int)CONTENTS_MOVEABLE );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_AREAPORTAL", (int)CONTENTS_AREAPORTAL );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_PLAYERCLIP", (int)CONTENTS_PLAYERCLIP );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_MONSTERCLIP", (int)CONTENTS_MONSTERCLIP );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_0", (int)CONTENTS_CURRENT_0 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_90", (int)CONTENTS_CURRENT_90 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_180", (int)CONTENTS_CURRENT_180 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_270", (int)CONTENTS_CURRENT_270 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_UP", (int)CONTENTS_CURRENT_UP );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_DOWN", (int)CONTENTS_CURRENT_DOWN );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_MONSTER", (int)CONTENTS_MONSTER );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_DEBRIS", (int)CONTENTS_DEBRIS );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_DETAIL", (int)CONTENTS_DETAIL );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_TRANSLUCENT", (int)CONTENTS_TRANSLUCENT );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_LADDER", (int)CONTENTS_LADDER );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_HITBOX", (int)CONTENTS_HITBOX );

	// COLLISION GROUP
	const char* const pszObjectName2 = "COLLISIONGROUP";
	engine.RegisterEnum( pszObjectName2 );

	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_NONE", (int)COLLISION_GROUP_NONE );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_DEBRIS", (int)COLLISION_GROUP_DEBRIS );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_DEBRIS_TRIGGER", (int)COLLISION_GROUP_DEBRIS_TRIGGER );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_INTERACTIVE_DEBRIS", (int)COLLISION_GROUP_INTERACTIVE_DEBRIS );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_INTERACTIVE", (int)COLLISION_GROUP_INTERACTIVE );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_PLAYER", (int)COLLISION_GROUP_PLAYER );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_BREAKABLE_GLASS", (int)COLLISION_GROUP_BREAKABLE_GLASS );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_PLAYER_MOVEMENT", (int)COLLISION_GROUP_PLAYER_MOVEMENT );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_WEAPON", (int)COLLISION_GROUP_WEAPON );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_PROJECTILE", (int)COLLISION_GROUP_PROJECTILE );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_DOOR_BLOCKER", (int)COLLISION_GROUP_DOOR_BLOCKER );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_PASSABLE_DOOR", (int)COLLISION_GROUP_PASSABLE_DOOR );
	engine.RegisterEnumValue( pszObjectName2, "COLLISION_GROUP_DISSOLVING", (int)COLLISION_GROUP_DISSOLVING );
}

void RegisterASTrace( asIScriptEngine &engine )
{
	RegisterScriptTracer_Utils( engine );

	const char* const pszObjectName = "CGameTrace";

	engine.RegisterObjectType(
		pszObjectName, sizeof( CGameTrace ),
		asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_CLASS_ALLFLOATS );

/**JSON
 * Constructs a CGameTrace class.
 *
 * @type class
 * @name CGameTrace
 *
 * @return Returns CGameTrace class which can be manipulated
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void CGameTrace()",
		asFUNCTION( ConstructTrace ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Destroys the CGameTrace
 *
 * @type void
 * @name DestroyTracer
 * @child CGameTrace
 * @isfunc true
 *
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void DestroyTracer()",
		asFUNCTION( DestroyTracer ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the amount of time
 *
 * @type float
 * @child CGameTrace
 * @name fraction
 *
 * @return Returns the time completed, 1.0 = didn't hit anything
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float fraction",
		asOFFSET( CGameTrace, fraction ) );

/**JSON
 * Grabs the amount of time subce we left a solid
 *
 * @type float
 * @child CGameTrace
 * @name fractionleftsolid
 *
 * @return Returns the time we left a solid, only valid if we started in solid
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float fractionleftsolid",
		asOFFSET( CGameTrace, fractionleftsolid ) );

/**JSON
 * Grabs the contents from the hit surface if we hit something
 *
 * @type int
 * @child CGameTrace
 * @name contents
 *
 * @return Returns the contents on other side of surface hit
 */
	engine.RegisterObjectProperty(
		pszObjectName, "int contents",
		asOFFSET( CGameTrace, contents ) );

/**JSON
 * Grabs the hitgroup if we hit something
 *
 * @type int
 * @child CGameTrace
 * @name hitgroup
 *
 * @return Returns the hitgroup, 0 == generic, non-zero is specific body part
 */
	engine.RegisterObjectProperty(
		pszObjectName, "int hitgroup",
		asOFFSET( CGameTrace, hitgroup ) );

/**JSON
 * If hEnt points at the world entity, then this is the static prop index. Otherwise, this is the hitbox index.
 *
 * @type int
 * @child CGameTrace
 * @name hitbox
 *
 * @return Returns the hitbox index
 */
	engine.RegisterObjectProperty(
		pszObjectName, "int hitbox",
		asOFFSET( CGameTrace, hitbox ) );

/**JSON
 * Checks if the plane is fully solid or not
 *
 * @type bool
 * @child CGameTrace
 * @name allsolid
 *
 * @return Returns true, plane is not valid
 */
	engine.RegisterObjectProperty(
		pszObjectName, "bool allsolid",
		asOFFSET( CGameTrace, allsolid ) );

/**JSON
 * Checks if the plane is solid or not
 *
 * @type bool
 * @child CGameTrace
 * @name startsolid
 *
 * @return Returns true, the initial point was in a solid area
 */
	engine.RegisterObjectProperty(
		pszObjectName, "bool startsolid",
		asOFFSET( CGameTrace, startsolid ) );

/**JSON
 * Grabs the start position of the tracer
 *
 * @type Vector
 * @child CGameTrace
 * @name startpos
 *
 * @return Returns Vector class which can be manipulated
 */
	engine.RegisterObjectProperty(
		pszObjectName, "Vector startpos",
		asOFFSET( CGameTrace, startpos ) );

/**JSON
 * Grabs the end position of the tracer
 *
 * @type Vector
 * @child CGameTrace
 * @name endpos
 *
 * @return Returns Vector class which can be manipulated
 */
	engine.RegisterObjectProperty(
		pszObjectName, "Vector endpos",
		asOFFSET( CGameTrace, endpos ) );

/**JSON
 * Grabs the plane normal of the trace
 *
 * @type Vector
 * @name TracePlaneNormal
 * @child CGameTrace
 * @isfunc true
 *
 * @return Returns Vector class which can be manipulated
 */
	engine.RegisterObjectMethod(
		pszObjectName, "Vector TracePlaneNormal()",
		asMETHOD( CGameTrace, TracePlaneNormal ), asCALL_THISCALL );

/**JSON
 * Grabs the entity, if it's valid
 *
 * @type object
 * @object CBaseEntity
 * @name m_pEnt
 *
 * @return Returns CBaseEntity@ class which can be manipulated
 */
	engine.RegisterObjectProperty(
		pszObjectName, "CBaseEntity@ m_pEnt",
		asOFFSET( CGameTrace, m_pEnt ) );

/**JSON
 * Checks if the tracer hit the world or not
 *
 * @type bool
 * @name DidHitWorld
 * @child CGameTrace
 * @isfunc true
 *
 * @return Returns true if the tracer hits the world
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool DidHitWorld() const",
		asMETHOD( CGameTrace, DidHitWorld ), asCALL_THISCALL );

/**JSON
 * Checks if the tracer hit a non world entity or not
 *
 * @type bool
 * @name DidHitNonWorldEntity
 * @child CGameTrace
 * @isfunc true
 *
 * @return Returns true if the tracer hit an entity object that isn't the world
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool DidHitNonWorldEntity() const",
		asMETHOD( CGameTrace, DidHitNonWorldEntity ), asCALL_THISCALL );

/**JSON
 * Grabs the index from the object, if it's valid
 *
 * @type int
 * @name GetEntityIndex
 * @child CGameTrace
 * @isfunc true
 *
 * @return Returns the index of the entity object
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int GetEntityIndex() const",
		asMETHOD( CGameTrace, GetEntityIndex ), asCALL_THISCALL );

/**JSON
 * Checks if the tracer hit something
 *
 * @type bool
 * @name DidHit
 * @child CGameTrace
 * @isfunc true
 *
 * @return Returns true if the tracer hit something
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool DidHit() const",
		asMETHOD( CGameTrace, DidHit ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CGameTrace& opAssign(const CGameTrace& in other)",
		asMETHODPR( CGameTrace, operator=, ( const CGameTrace& ), CGameTrace& ), asCALL_THISCALL );
}
