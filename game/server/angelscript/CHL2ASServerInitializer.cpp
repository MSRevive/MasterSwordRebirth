//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

//#include "entities/plats/CPathTrack.h"

#include "msdllheaders.h"
#include "angelscript/CHL2ASManager.h"

// Exception
//#include "angelscript/add_on/scripthelper.h"

#include "angelscript/ScriptAPI/CASEngine.h"
#include "angelscript/ScriptAPI/CASSchedule.h"
#include "angelscript/ScriptAPI/CASCBaseEntity.h"
//#include "angelscript/ScriptAPI/CASCBasePlayer.h"
#include "angelscript/ScriptAPI/CASUtil.h"
#include "angelscript/ScriptAPI/CASColor.h"
//#include "angelscript/ScriptAPI/CASChat.h"
#include "angelscript/ScriptAPI/CASEntityCreator.h"

#include "CHL2ASServerInitializer.h"

#define REGISTER_GLOBAL_INT_DEFINE_PROPERTY( _FUNC, _VAL ) \
static int static_##_VAL = _VAL; \
engine.RegisterGlobalProperty( "const int " _FUNC, &static_##_VAL );

bool CHL2ASServerInitializer::RegisterCoreAPI( CASManager& manager )
{
	CHL2ASBaseInitializer::RegisterCoreAPI( manager );

	auto& engine = *manager.GetEngine();

	// Required stuff
	RegisterASColor( engine );

	// Entities
	RegisterScriptEntityDependencies( engine );
	RegisterScriptBaseEntity( engine );
	//RegisterASCBasePlayer( engine );

	//RegisterASChat( engine );

	RegisterASEngine( engine );

	RegisterScriptUtils( engine );

	RegisterASEntityCreator( engine );

	// Exception
	//RegisterExceptionRoutines( &engine );

	return true;
}

bool CHL2ASServerInitializer::AddEvents( CASManager& manager, CASEventManager& eventManager )
{
	CHL2ASBaseInitializer::AddEvents( manager, eventManager );
	return true;
}
