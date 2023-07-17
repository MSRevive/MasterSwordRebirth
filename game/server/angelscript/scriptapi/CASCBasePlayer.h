#ifndef GAME_SHARED_ANGELSCRIPT_SCRIPTAPI_ENTITIES_ASCBASEPLAYER_H
#define GAME_SHARED_ANGELSCRIPT_SCRIPTAPI_ENTITIES_ASCBASEPLAYER_H

#include "CASCBaseEntity.h"
#include "player.h"

/**
*	Registers constants related to players.
*	@param engine Script engine.
*/
void RegisterScriptPlayerDependencies( asIScriptEngine& engine );

/**
*	Class name for CBasePlayer in scripts.
*/
#define AS_CBASEPLAYER_NAME "CBasePlayer"

/**
*	@see RegisterScriptBaseEntity( asIScriptEngine& engine, const char* const pszObjectName )
*/
template<typename CLASS>
void RegisterScriptCBasePlayer( asIScriptEngine& engine, const char* const pszObjectName );

inline void RegisterScriptCBasePlayer( asIScriptEngine& engine )
{
	RegisterScriptCBasePlayer<CBasePlayer>( engine, AS_CBASEPLAYER_NAME );
}

#endif //GAME_SHARED_ANGELSCRIPT_SCRIPTAPI_ENTITIES_ASCBASEPLAYER_H