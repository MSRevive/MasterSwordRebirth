//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASENGINE_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASENGINE_H

#if defined( SOURCE )
#include <stdstring.h>
#else
#include <string.h>
#include <vector.h>
#endif

class asIScriptEngine;
class CScriptArray;
class CBaseEntity;

// Engine related enums
enum PRECACHE_TYPE
{
	E_MODEL,
	E_SOUND,
	E_GENERIC,
	E_HUD
};

struct HammerEvent_t
{
public:
	int				m_iEntIndex;
	std::string		m_strEnt;
	float			m_flDelay;
};

class IASHammerEvents
{
public:
	virtual void Register( CBaseEntity *pEntity, const float& EntDelay ) = 0;
	virtual void Register( const std::string& EntName, const float& EntDelay ) = 0;
	virtual void Update() = 0;
	virtual void Destroy() = 0;
};
extern IASHammerEvents *gASHammerEvents;

class CASEngine final
{
public:
	CASEngine() = default;
	~CASEngine() = default;

	void Ent_Fire( const std::string& EntName );
	void Ent_Fire_D( const std::string& EntName, const float& EntDelay );

	void Ent_Fire_Array( CScriptArray* pEnts );
	void Ent_Fire_Array_D( CScriptArray* pEnts, const float& EntDelay );

	void Ent_Fire_Ent( CBaseEntity *pEntity );
	void Ent_Fire_D_Ent( CBaseEntity *pEntity, const float& EntDelay );

	void EmitSoundEntity( CBaseEntity *pEntity, const std::string& strSoundFile, const int& iChannel, const float& flVol = 1.0f, const int& iPitch = 100 );
	void EmitSoundPosition( int index, const std::string &strSoundFile, Vector &vecOrigin, const float& flVol = 1.0f, const int& iPitch = 100 );

	void RunConsoleCommand( const std::string& strConsoleCommand );

	int PrecacheFile( PRECACHE_TYPE iValue, const std::string& strFileName );

private:
	CASEngine( const CASEngine & ) = delete;
	CASEngine& operator=( const CASEngine & ) = delete;
};

void RegisterASEngine( asIScriptEngine &engine );

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASENGINE_H
