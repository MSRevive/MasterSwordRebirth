//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "msdllheaders.h"
#include "customentity.h"
#include "CASEngine.h"

#include "angelscript/CASContagionImport.h"

#include "angelscript/HL2ASConstants.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#include <angelscript/add_on/scriptarray.h>
#else
#include "angelscript/CASCManager.h"
#endif

#if !defined( ASCOMPILER )
#include "util.h"
#if defined( SPDLOG )
	#include "spdlog_main.h"
#endif
#endif

#define ArrayGrab( _ARR, _ITR ) for ( asUINT _ITR = 0; _ITR < _ARR->GetSize(); _ITR++ )

/**PAGE
 *
 * Class used for engine related purposes.
 *
 * @page Engine
 * @category Engine
 *
 * @desc_md false
 * @type void
 *
*/

class CASHammerEvents : public IASHammerEvents
{
public:
	virtual void Register( const std::string& EntName, const float& EntDelay );
	virtual void Register( CBaseEntity *pEntity, const float& EntDelay );
	virtual void Update();
	virtual void Destroy();

protected:
	// list of our sources
	std::list<HammerEvent_t> m_Events;

	void FireEvent( const HammerEvent_t& event );
};

static CASHammerEvents s_ASHammerEvents;
IASHammerEvents* gASHammerEvents = &s_ASHammerEvents;


int CASEngine::PrecacheFile( PRECACHE_TYPE iValue, const std::string& strFileName )
{
#if !defined( ASCOMPILER )
	// Empty? Then don't do anything
	if ( strFileName.empty() ) return 0;
	switch ( iValue )
	{
		// What did we even try to preache?
		default:
#if defined( SPDLOG )
			SpdLog( false, "PrecacheFile", UTIL_LogFolder( LOG_ANGELSCRIPT_PLUGINS ), LOGTYPE_WARN, UTIL_VarArgs( "%i is not a valid value!", iValue ) );
#else
			ALERT( at_console, "%i is not a valid value!", iValue );
#endif
		break;

		// Audio
		case E_SOUND:
			return PRECACHE_SOUND( strFileName.c_str() );
		break;

		// Generic
		case E_GENERIC:
		{
			char buf[128];
			_snprintf(buf, sizeof(buf), "%s", strFileName.c_str() );
			PRECACHE_GENERIC( buf );
			return 1;
		}
		break;

		// Model
		case E_MODEL:
		{
			return PRECACHE_MODEL( strFileName.c_str() );
		}
		break;
	}
#endif
	return 0;
}


void CASEngine::Ent_Fire(const std::string& EntName)
{
	Ent_Fire_D( EntName, 0.0f );
}


void CASEngine::Ent_Fire_D(const std::string& EntName, const float& EntDelay)
{
#if !defined( ASCOMPILER )
	if ( !gASHammerEvents ) return;
	gASHammerEvents->Register( EntName, EntDelay );
#endif
}


void CASEngine::Ent_Fire_Array( CScriptArray* pEnts )
{
	Ent_Fire_Array_D( pEnts, 0.0f );
}

void CASEngine::Ent_Fire_Array_D( CScriptArray* pEnts, const float& EntDelay )
{
#if !defined( ASCOMPILER )
	asIScriptContext *ctx = asGetActiveContext();
	if ( !ctx ) return;

	asIScriptEngine *engine = ctx->GetEngine();
	if ( !engine ) return;

	asITypeInfo* pArray_Ents = pEnts->GetArrayObjectType();
	asITypeInfo* t = engine->GetTypeInfoByDecl( "array<string>" );

	if ( t != pArray_Ents )
	{
		ALERT( at_console,
			"{red}Warning{default}:\n" \
			"\t\"{yellow}Engine.Ent_Fire_Array{default}\" tried to use an array that is not a {green}string{default}!\n" \
			"\t{cyan}Engine{default}.{yellowgreen}Ent_Fire{default}( {lime}CBaseEntity{default}@ {forestgreen}pCaller{default}, {lime}CBaseEntity{default}@ {forestgreen}pActivator{default}, {blue}array{default}<{lime}string{default}> {forestgreen}aEnts{default}, {blue}array{default}<{lime}string{default}> {forestgreen}aActions{default}, {blue}const{default} {lime}string{default}& in {forestgreen}EntValue{default}, {blue}const{default} {lime}string{default}& in {forestgreen}EntDelay{default} );\n"
		);
		return;
	}

	// Now go trough the ents we want to send the stuff too
	ArrayGrab( pEnts, i )
	{
		std::string strEnt( ((std::string*)pEnts->At( i ))->c_str() );
		Ent_Fire_D( strEnt, EntDelay );
	}
#endif
}

void CASEngine::Ent_Fire_Ent(CBaseEntity *pEntity)
{
	Ent_Fire_D_Ent( pEntity, 0.0f );
}

void CASEngine::Ent_Fire_D_Ent(CBaseEntity *pEntity, const float& EntDelay)
{
	if ( !gASHammerEvents ) return;
	gASHammerEvents->Register( pEntity, EntDelay);
}

void CASEngine::EmitSoundEntity( CBaseEntity *pEntity, const std::string &strSoundFile, const int& iChannel, const float& flVol, const int& iPitch )
{
#if !defined( ASCOMPILER )
	if ( !pEntity ) return;
	//pEntity->EmitSound( strSoundFile.c_str() );
	EMIT_SOUND_DYN( ENT(pEntity->pev), iChannel, strSoundFile.c_str(), flVol, ATTN_NORM, 1.0, iPitch );
#endif
}

void CASEngine::EmitSoundPosition( int index, const std::string &strSoundFile, Vector &vecOrigin, const float& flVol, const int& iPitch )
{
#if !defined( ASCOMPILER )
	CBaseEntity *pEntity = ContagionImport::UTIL_EntityByIndex( index );
	if ( !pEntity ) return;
	UTIL_EmitAmbientSound( ENT(pEntity->pev), vecOrigin, strSoundFile.c_str(), flVol, ATTN_NORM, 1.0, iPitch );
#endif
}

std::string& LTrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

void CASEngine::RunConsoleCommand( const std::string& strConsoleCommand )
{
#if !defined( ASCOMPILER )
	std::string szActualCommand( strConsoleCommand );
	LTrim( szActualCommand );

	if ( szActualCommand.empty() ) return;

	const char cEnd = szActualCommand.back();

	// The engine only accepts commands if they end with either of these, so make it easy on script writers and do it for them. - Solokiller
	if ( cEnd != '\n' && cEnd != ';' )
	{
		szActualCommand += '\n';
	}

	g_engfuncs.pfnServerCommand( UTIL_VarArgs( "%s\n", strConsoleCommand.c_str() ) );
#endif
}

/**
*	Class name for CASEngine in scripts.
*/
#define AS_CENGINE_CLASSNAME "CASEngine"

static CASEngine g_ASEngine;

/**JSON
 * Enumeration of precache types.
 *
 * @type enum
 * @name PRECACHE_TYPE
 *
 * @args soundbank # Used to precache soundbanks
 * @args sound # Used to precache sounds
 * @args hud # Used to precache hud elements
 * @args model # Used to precache models and / or sprites
 * @args material # Used to precache materials (VMT only)
 * @args particle # Used to precache particle files
 *
 */
static void RegisterScriptPRECACHE_TYPE( asIScriptEngine& engine )
{
	const char* const pszObjectName = "PRECACHE_TYPE";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "sound", E_SOUND );
	engine.RegisterEnumValue( pszObjectName, "hud", E_HUD );
	engine.RegisterEnumValue( pszObjectName, "model", E_MODEL );
}

void RegisterASEngine( asIScriptEngine &engine )
{
	RegisterScriptPRECACHE_TYPE( engine );

	const char* const pszObjectName = AS_CENGINE_CLASSNAME;

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

/**MARKDOWN
The function `Ent_Fire` also accepts string arrays for the entity classnames, targetnames and functions. Do note that if using classnames, all instances are impacted.

Examples:  
```cpp
// Method 1
array<string> doors = { "doorA", "doorB" };
Engine.Ent_Fire( doors );

// Method 2
Engine.Ent_Fire( doors, 3.0f );

// Method 3
Engine.Ent_Fire( "doorC", 5.0f );
```

 * @filename Ent_Fire
 */

/**JSON
 * Fires an IO value to a specific entity.
 *
 * @type void
 * @name Ent_Fire
 * @desc_md true
 *
 * @args %string_in% strName # The targeted entity name
 * @args %string_in% strAction # The action it should send
 * @args %string_in% strValue # The value we want to set it as, otherwise its empty (optional)
 * @args %string_in% strDelay # When it should be fired, otherwise it's fired instantly (optional)
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void Ent_Fire(const string& in EntName)", 
		asMETHOD( CASEngine, Ent_Fire ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void Ent_Fire(const string& in EntName, const float& in EntDelay)", 
		asMETHOD( CASEngine, Ent_Fire_D ), asCALL_THISCALL );

	// Array Variant 1
	engine.RegisterObjectMethod(
		pszObjectName, "void Ent_Fire(array<string> @aEnts)", 
		asMETHOD( CASEngine, Ent_Fire_Array ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void Ent_Fire(array<string> @aEnts,  const float& in EntDelay)", 
		asMETHOD( CASEngine, Ent_Fire_Array_D ), asCALL_THISCALL );

/**JSON
 * Fires an IO value to a specific entity.
 *
 * @type void
 * @name Ent_Fire_Ent
 *
 * @args %class_CBaseEntity% pEntity # The targeted entity
 * @args %string_in% strDelay # When it should be fired, otherwise it's fired instantly (optional)
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void Ent_Fire_Ent(CBaseEntity@ pEntity)", 
		asMETHOD( CASEngine, Ent_Fire_Ent ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void Ent_Fire_Ent(CBaseEntity@ pEntity, const float& in EntDelay)", 
		asMETHOD( CASEngine, Ent_Fire_D_Ent), asCALL_THISCALL );

/**JSON
 * Emits a sound from the specified entity
 *
 * @type void
 * @name EmitSoundEntity
 *
 * @args %class_CBaseEntity% pEntity # The entity where the sound will play from
 * @args %string_in% strSoundFile # The file we want to play
 * @args %int_in% iChannel # The channel for the sound
 * @args %float_in% flVol # The volume of the sound
 * @args %int_in% iPitch # The pitch of the sound (default is 100)
 *
 * @infobox_enable true
 * @infobox_type warning
 * @infobox_desc 32-Bit version of Contagion does not support Steam Audio! You need use WWise soundevents for strSoundFile.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void EmitSoundEntity(CBaseEntity@ pEntiy, const string& in strSoundFile, const int& in iChannel)", 
		asMETHOD( CASEngine, EmitSoundEntity ), asCALL_THISCALL );
	engine.RegisterObjectMethod(
		pszObjectName, "void EmitSoundEntity(CBaseEntity@ pEntiy, const string& in strSoundFile, const int& in iChannel, const float& in flVol)", 
		asMETHOD( CASEngine, EmitSoundEntity ), asCALL_THISCALL );
	engine.RegisterObjectMethod(
		pszObjectName, "void EmitSoundEntity(CBaseEntity@ pEntiy, const string& in strSoundFile, const int& in iChannel, const float& in flVol, const int& in iPitch)", 
		asMETHOD( CASEngine, EmitSoundEntity ), asCALL_THISCALL );

/**JSON
 * Emits a sound from the specified location
 *
 * @type void
 * @name EmitSoundPosition
 *
 * @args %int% index # The index that plays this sound (if 0, then the sound is coming from the world)
 * @args %string_in% strSoundFile # The file we want to play
 * @args %Vector_in% vecOrigin # The position where the sound will play from
 * @args %float_in% flVol # The volume of the sound
 * @args %int_in% iPitch # The pitch of the sound (default is 100)
 *
 * @infobox_enable true
 * @infobox_type warning
 * @infobox_desc 32-Bit version of Contagion does not support Steam Audio! You need use WWise soundevents for strSoundFile.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void EmitSoundPosition(int index, const string& in strSoundFile, Vector &in vecOrigin)", 
		asMETHOD( CASEngine, EmitSoundPosition ), asCALL_THISCALL );
	engine.RegisterObjectMethod(
		pszObjectName, "void EmitSoundPosition(int index, const string& in strSoundFile, Vector &in vecOrigin, const float& in flVol)", 
		asMETHOD( CASEngine, EmitSoundPosition ), asCALL_THISCALL );
	engine.RegisterObjectMethod(
		pszObjectName, "void EmitSoundPosition(int index, const string& in strSoundFile, Vector &in vecOrigin, const float& in flVol, const int& in iPitch)", 
		asMETHOD( CASEngine, EmitSoundPosition ), asCALL_THISCALL );

/**JSON
 * Precaches the files, so the client and / or server doesn't crash when spawning said value.
 *
 * @type int
 * @name PrecacheFile
 *
 * @args %enum_PRECACHE_TYPE% iValue # Determines what we should precache
 * @args %string_in% strFileName # The file that should be precached
 *
 * @return Returns returns the index of the precached value (if model), else it will return 1 if it was successful.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int PrecacheFile(PRECACHE_TYPE iValue, const string& in strFileName)", 
		asMETHOD( CASEngine, PrecacheFile ), asCALL_THISCALL );

/**JSON
 * Executes a server console command.
 *
 * @type void
 * @name RunConsoleCommand
 *
 * @args %string_in% strConsoleCommand # The server command that will be executed.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void RunConsoleCommand(const string& in strConsoleCommand)", 
		asMETHOD( CASEngine, RunConsoleCommand ), asCALL_THISCALL );

	engine.RegisterGlobalProperty( AS_CENGINE_CLASSNAME " Engine", &g_ASEngine );
}

void CASHammerEvents::Register( const std::string& EntName, const float& EntDelay )
{
#if !defined( ASCOMPILER )
	HammerEvent_t hHammerEvent;
	hHammerEvent.m_iEntIndex = -1;
	hHammerEvent.m_strEnt = EntName;
	hHammerEvent.m_flDelay = EntDelay;
	m_Events.push_back( hHammerEvent );
#endif
}

void CASHammerEvents::Register( CBaseEntity *pEntity, const float &EntDelay )
{
#if !defined( ASCOMPILER )
	if ( !pEntity ) return;
	if ( FClassnameIs( pEntity->edict(), "worldspawn"))
		return;
	/*
	else if ( Q_stristr(pEntity->GetClassname(), "random_") != 0
		|| Q_stristr(pEntity->GetClassname(), "weapon_") != 0
		|| Q_stristr(pEntity->GetClassname(), "item_") != 0
		|| Q_stristr(pEntity->GetClassname(), "ammo_") != 0 )
		return;
	*/
	HammerEvent_t hHammerEvent;
	hHammerEvent.m_iEntIndex = pEntity->entindex();
	hHammerEvent.m_strEnt = "";
	hHammerEvent.m_flDelay = EntDelay;
	m_Events.push_back( hHammerEvent );
#endif
}

void CASHammerEvents::Update()
{
#if !defined( ASCOMPILER )
	if ( m_Events.size() == 0 ) return;
	std::list< HammerEvent_t >::iterator entEvent = m_Events.begin();
	while ( entEvent != m_Events.end() )
	{
		// Fire it, and delete it from our list
		if ( entEvent->m_flDelay <= gpGlobals->time )
		{
			FireEvent( *entEvent );
			m_Events.erase( entEvent++ );
		}
	}
#endif
}

void CASHammerEvents::Destroy()
{
	m_Events.clear();
}

void CASHammerEvents::FireEvent( const HammerEvent_t& event )
{
#if !defined( ASCOMPILER )
	CBaseEntity *pEntity = ContagionImport::UTIL_EntityByIndex( event.m_iEntIndex );
	if ( pEntity )
		pEntity->Use( nullptr, nullptr, USE_TOGGLE, 0 );
	else
	{
		// If targetname, then use that instead
		edict_t* pFind;
		pFind = FIND_ENTITY_BY_TARGETNAME( NULL, event.m_strEnt.c_str() );
		while (!FNullEnt(pFind))
		{
			CBaseEntity* pEnt = CBaseEntity::Instance( pFind );
			if ( pEnt )
				pEnt->Use( nullptr, nullptr, USE_TOGGLE, 0 );
			pFind = FIND_ENTITY_BY_TARGETNAME( pFind, event.m_strEnt.c_str() );
		}
		// If classname, then use that instead
		pFind = FIND_ENTITY_BY_CLASSNAME( NULL, event.m_strEnt.c_str() );
		while (!FNullEnt(pFind))
		{
			CBaseEntity* pEnt = CBaseEntity::Instance( pFind );
			if ( pEnt )
				pEnt->Use( nullptr, nullptr, USE_TOGGLE, 0 );
			pFind = FIND_ENTITY_BY_CLASSNAME( pFind, event.m_strEnt.c_str() );
		}
	}
#endif
}
