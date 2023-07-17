//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "msdllheaders.h"
#include "customentity.h"

#include "CASUtil.h"
#include "player.h"
#include "shake.h"
#include "explode.h"
#include "weapons.h"

#include "angelscript/CASContagionImport.h"

#include "angelscript/HL2ASConstants.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#include <angelscript/add_on/scriptarray.h>
#else
#include "angelscript/CASCManager.h"
#endif

// Utils
#include "CASColor.h"

/**PAGE
 *
 * Commonly used utility functions
 *
 * @page Utils
 * @category Utilities
 *
 * @type void
 *
*/

/**
 * Class name for CASUtil in scripts.
 */
#define AS_CASUTIL_CLASSNAME "CASUtil"

static CASUtil g_ASUtil;

CBasePlayer* CASUtils_ToBasePlayer( int itemslot )
{
#if !defined( ASCOMPILER )
	return (CBasePlayer*)UTIL_PlayerByIndex( itemslot );
#else
	return nullptr;
#endif
}

// JonnyBoy0719 - Must be set first, else it won't find it
CBaseEntity* CASUtils_FindEntity_Name( CBaseEntity* startEntity, const std::string& entityname )
{
	CBaseEntity *pEntity = nullptr;
#if !defined( ASCOMPILER )
	edict_t* pFind;
	pFind = FIND_ENTITY_BY_TARGETNAME( startEntity->edict(), entityname.c_str() );
	while (!FNullEnt(pFind))
	{
		CBaseEntity* pEnt = CBaseEntity::Instance( pFind );
		if ( pEnt ) return pEnt;
		pFind = FIND_ENTITY_BY_TARGETNAME( pFind, entityname.c_str() );
	}
#endif
	// Returns nullptr if we're here
	return pEntity;
}

CBaseEntity* CASUtils_FindEntity_EntIndex( int& entindex )
{
#if !defined( ASCOMPILER )
	return ContagionImport::UTIL_EntityByIndex( entindex );
#else
	return nullptr;
#endif
}

CBaseEntity* CASUtils_FindEntity_Classname( CBaseEntity* startEntity, const std::string& classname )
{
	CBaseEntity *pEntity = nullptr;
#if !defined( ASCOMPILER )
	edict_t* pFind;
	pFind = FIND_ENTITY_BY_CLASSNAME( startEntity->edict(), classname.c_str() );
	while (!FNullEnt(pFind))
	{
		CBaseEntity* pEnt = CBaseEntity::Instance( pFind );
		if ( pEnt ) return pEnt;
		pFind = FIND_ENTITY_BY_CLASSNAME( pFind, classname.c_str() );
	}
#endif
	// Returns nullptr if we're here
	return pEntity;
}

CBaseEntity* CASUtils_FindEntity_Name( const std::string& entityname )
{
	return CASUtils_FindEntity_Name( nullptr, entityname );
}

CBaseEntity* CASUtils_FindEntity_Classname( const std::string& classname )
{
	return CASUtils_FindEntity_Classname( nullptr, classname );
}

/*
CBasePlayer* CASUtils_GetPlayerByName( const std::string& playername, bool bSpecific )
{
#if !defined( ASCOMPILER )
	return ToTerrorPlayer( UTIL_PlayerByName( playername.c_str() ) );
#else
	return nullptr;
#endif
}

CBasePlayer* CASUtils_GetPlayerBySteamID( const std::string& strSteamID32 )
{
#if !defined( ASCOMPILER )
	CSteamID steamIDPlayer;
	char pszSteamID[ 128 ];
	pszSteamID[ 0 ] = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *pPlayer = PlayerByIndex( i );
		if ( !pPlayer )
			continue;

		if ( !pPlayer->GetSteamID( &steamIDPlayer ) )
			continue;

		Q_snprintf( pszSteamID, sizeof( pszSteamID ), "%llu", (unsigned long long)steamIDPlayer.ConvertToUint64() );

		SteamID steamid;
		steamid = SteamID::Parse( strSteamID32 );

		char SteamID64[ 128 ];
		SteamID64[ 0 ] = 0;

		Q_snprintf( SteamID64, sizeof( SteamID64 ), "%s", steamid[ SteamID::Formats::STEAMID64 ].c_str() );

		if ( FStrEq( pszSteamID, SteamID64 ) )
			return pPlayer;
	}
#endif
	return nullptr;
}

std::string UTIL_Steam64toSteam32( CASUtil* pThis, const std::string& input )
{
	SteamID steamid;
	steamid = SteamID::Parse( input );
	return steamid[ SteamID::Formats::STEAMID32 ];
}
*/

bool UTIL_StrEql( CASUtil* pThis, const std::string& strVar1, const std::string& strVar2 )
{
	char *value1 = (char*)strVar1.c_str();
	char *value2 = (char*)strVar2.c_str();

	if ( strcmp( value1, value2 ) == 0 )
		return true;

	return false;
}

bool UTIL_StrEqlS( CASUtil* pThis, const std::string& strVar1, const std::string& strVar2, bool bIgnoreSensitive )
{
	char *value1 = (char*)strVar1.c_str();
	char *value2 = (char*)strVar2.c_str();

	if ( bIgnoreSensitive )
	{
		if ( FStrEq( value1, value2 ) )
			return true;
	}
	else
	{
		if ( strcmp( value1, value2 ) == 0 )
			return true;
	}

	return false;
}

bool UTIL_StrContains( CASUtil* pThis, const std::string& strVar1, const std::string& strVar2 )
{
	char *value1 = (char*)strVar1.c_str();
	char *value2 = (char*)strVar2.c_str();

	if ( ContagionImport::Q_stristr(value2, value1) != 0 )
		return true;

	return false;
}

std::string UTIL_StrReplace( CASUtil* pThis, const std::string& strVar1, const std::string& strVar2, const std::string& strVar3, const bool& bCaseSensitive )
{
	char *StrOriginal = (char*)strVar1.c_str();
	char *StrFind = (char*)strVar2.c_str();
	char *StrReplace = (char*)strVar3.c_str();

	static char OutPut[ 1024 ];

	// Replace the string
	ContagionImport::V_StrSubst( StrOriginal, StrFind, StrReplace, OutPut, sizeof( OutPut ), bCaseSensitive );

	// If the string was replaced, return the output. If not, return the original string.
	if ( sizeof( OutPut ) > 0 )
		return OutPut;
	else
		return StrOriginal;
}

std::string UTIL_EscapeCharacters( CASUtil* pThis, const std::string& strValue )
{
	std::string strOuput = strValue;
#if !defined( ASCOMPILER )
	ContagionImport::UTIL_STDReplaceString( strOuput, "'", "" );
	ContagionImport::UTIL_STDReplaceString( strOuput, "´", "" );
	ContagionImport::UTIL_STDReplaceString( strOuput, "\\", "" );
	ContagionImport::UTIL_STDReplaceString( strOuput, "/", "" );
#endif
	return strOuput;
}

bool UTIL_NumbersOnly( CASUtil* pThis, const std::string& strValue )
{
	return strValue.find_first_not_of("0123456789") == std::string::npos;
}

int UTIL_StringToInt( CASUtil* pThis, const std::string& strValue )
{
	return atoi( strValue.c_str() );
}

int UTIL_FloatToInt( CASUtil* pThis, float& flValue )
{
	// Convert to interger.
	return int( flValue );
}

float UTIL_StringToFloat( CASUtil* pThis, const std::string& strValue )
{
	return atof( strValue.c_str() );
}

void UTIL_GameText( CASUtil* pThis, const std::string& strMessage, HudTextParams& params )
{
#if !defined( ASCOMPILER )
	hudtextparms_t	m_textParms;
	m_textParms.channel = params.channel;
	m_textParms.x = params.x;
	m_textParms.y = params.y;
	m_textParms.effect = params.effect;
	m_textParms.fadeinTime = params.fadeinTime;
	m_textParms.fadeoutTime = params.fadeoutTime;
	m_textParms.holdTime = params.holdTime;
	m_textParms.fxTime = params.fxTime;
	m_textParms.fadeoutTime = params.fadeoutTime;

	// Color
	m_textParms.r1 = params.r1;
	m_textParms.g1 = params.g1;
	m_textParms.b1 = params.b1;
	m_textParms.a1 = params.a1;

	// Color2
	m_textParms.r2 = params.r2;
	m_textParms.g2 = params.g2;
	m_textParms.b2 = params.b2;
	m_textParms.a2 = params.a2;

	UTIL_HudMessageAll( m_textParms, strMessage.c_str() );
#endif
}

void UTIL_Entity_EnvExplosion(
	CASUtil *pThis,
	Vector &vecOrigin,
	Vector &vecAngles,
	float &flDmg
)
{
#if !defined( ASCOMPILER )
	ExplosionCreate( vecOrigin, vecAngles, nullptr, flDmg, true );
#endif
}

void UTIL_Entity_RadiusDamage(
	CASUtil *pThis,
	Vector origin,
	CBaseEntity *inflictor,
	CBaseEntity *attacker,
	float damage,
	int iRadius,
	int classignore,
	int damagetype
)
{
#if !defined( ASCOMPILER )
	RadiusDamage( origin, inflictor->pev, attacker->pev, damage, iRadius, classignore, damagetype );
#endif
}

void UTIL_Entity_EnvExplosion_ENT(
	CASUtil *pThis,
	CBaseEntity *pEntity,
	float &flDmg
)
{
#if !defined( ASCOMPILER )
	ExplosionCreate( pEntity->Center(), pEntity->pev->angles, pEntity->edict(), flDmg, true );
#endif
}

// Copied from EnvFade.cpp
#define SF_ENV_FADE_IN				0x0000		// Fade in, not out
#define SF_ENV_FADE_OUT				0x0002		// Fade out, not in
#define SF_ENV_FADE_MODULATE		0x0004		// Modulate, don't blend
#define SF_ENV_FADE_STAYOUT			0x0008

void UTIL_Entity_ScreenFade( CASUtil *pThis, CBasePlayer *pPlayer, Color color, float flDuration, float flHoldTime, int iFlags )
{
#if !defined( ASCOMPILER )
	Vector m_vColor( color.r(), color.g(), color.b() );

	if ( iFlags & SF_ENV_FADE_OUT )
		iFlags |= FFADE_OUT;
	else
		iFlags |= FFADE_IN;

	if ( iFlags & SF_ENV_FADE_MODULATE )
		iFlags |= FFADE_MODULATE;

	if ( iFlags & SF_ENV_FADE_STAYOUT )
		iFlags |= FFADE_STAYOUT;

	if ( !pPlayer )
		UTIL_ScreenFadeAll( m_vColor, flDuration, flHoldTime, color.a(), iFlags );
	else
		UTIL_ScreenFade( pPlayer, m_vColor, flDuration, flHoldTime, color.a(), iFlags );
#endif
}

void UTIL_DoScreenShake( CASUtil *pThis, const Vector &vec, const float &flDuration, const float &flRadius, const float &flAmp, const float &flFreq )
{
#if !defined( ASCOMPILER )
	UTIL_ScreenShake( vec, flAmp, flFreq, flDuration, flRadius );
#endif
}

void UTIL_GameText_Direct( CASUtil* pThis, CBasePlayer *pPlayer, const std::string& strMessage, HudTextParams& params )
{
#if !defined( ASCOMPILER )
	if ( !pPlayer )
		return;

	hudtextparms_t	m_textParms;
	m_textParms.channel = params.channel;
	m_textParms.x = params.x;
	m_textParms.y = params.y;
	m_textParms.effect = params.effect;
	m_textParms.fadeinTime = params.fadeinTime;
	m_textParms.fadeoutTime = params.fadeoutTime;
	m_textParms.holdTime = params.holdTime;
	m_textParms.fxTime = params.fxTime;
	m_textParms.fadeoutTime = params.fadeoutTime;

	// Color
	m_textParms.r1 = params.r1;
	m_textParms.g1 = params.g1;
	m_textParms.b1 = params.b1;
	m_textParms.a1 = params.a1;

	// Color2
	m_textParms.r2 = params.r2;
	m_textParms.g2 = params.g2;
	m_textParms.b2 = params.b2;
	m_textParms.a2 = params.a2;

	UTIL_HudMessage( pPlayer, m_textParms, strMessage.c_str() );
#endif
}

/**JSON
 * Enumeration of Fade Flags.
 *
 * @type enum
 * @name FadeFlag_t
 *
 * @args fade_in # Fade in
 * @args fade_out # Fade out (not in)
 * @args fade_modulate # Modulate (don't blend)
 * @args fade_stayout # ignores the duration, stays faded out until new ScreenFade message received
 *
 */
static void RegisterScriptFadeFlags( asIScriptEngine& engine )
{
	const char* const pszObjectName = "FadeFlag_t";
	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "fade_in", SF_ENV_FADE_IN );
	engine.RegisterEnumValue( pszObjectName, "fade_out", SF_ENV_FADE_OUT );
	engine.RegisterEnumValue( pszObjectName, "fade_modulate", SF_ENV_FADE_MODULATE );
	engine.RegisterEnumValue( pszObjectName, "fade_stayout", SF_ENV_FADE_STAYOUT );
}


static void RegisterTraceStuff( asIScriptEngine& engine )
{
/**JSON
 * Enumeration of fake timer flags.
 *
 * @type enum
 * @name IGNORE_MONSTERS
 *
 * @args dont_ignore_monsters # 
 * @args ignore_monsters # 
 * @args missile # 
 *
 */
	const char* const pszObjectName = "IGNORE_MONSTERS";
	engine.RegisterEnum( pszObjectName );
	engine.RegisterEnumValue( pszObjectName, "dont_ignore_monsters", IGNORE_MONSTERS::dont_ignore_monsters );
	engine.RegisterEnumValue( pszObjectName, "ignore_monsters", IGNORE_MONSTERS::ignore_monsters );
	engine.RegisterEnumValue( pszObjectName, "missile", IGNORE_MONSTERS::missile );

/**JSON
 * Enumeration of fake timer flags.
 *
 * @type enum
 * @name IGNORE_GLASS
 *
 * @args dont_ignore_glass # 
 * @args ignore_glass # 
 *
 */
	const char* const pszObjectName2 = "IGNORE_GLASS";
	engine.RegisterEnum( pszObjectName2 );
	engine.RegisterEnumValue( pszObjectName2, "dont_ignore_glass", IGNORE_GLASS::dont_ignore_glass );
	engine.RegisterEnumValue( pszObjectName2, "ignore_glass", IGNORE_GLASS::ignore_glass );

/**JSON
 * Enumeration of fake timer flags.
 *
 * @type enum
 * @name HullNum
 *
 * @args point_hull # 
 * @args human_hull # 
 * @args large_hull # 
 * @args head_hull # 
 *
 */
	const char* const pszObjectName3 = "HullNum";
	engine.RegisterEnum( pszObjectName3 );
	engine.RegisterEnumValue( pszObjectName3, "point_hull", point_hull );
	engine.RegisterEnumValue( pszObjectName3, "human_hull", human_hull );
	engine.RegisterEnumValue( pszObjectName3, "large_hull", large_hull );
	engine.RegisterEnumValue( pszObjectName3, "head_hull", head_hull );
}

/**JSON
 * Enumeration of SteamID's that can be converted.
 *
 * @type enum
 * @name SteamIDFormat
 *
 * @args AUTO # Auto-detect format --- this also supports other unlisted formats such as full profile URLs.
 * @args STEAMID32 # Classic STEAM_x:y:zzzzzz | x = 0/1
 * @args STEAMID64 # SteamID64: 7656119xxxxxxxxxx
 * @args STEAMID3 # SteamID3 format: [U:1:xxxxxx]
 * @args S32 # Raw 32-bit SIGNED format. This is a raw steamid index that overflows into negative bitspace.<br>This is the format that SourceMod returns with GetSteamAccountID, and will always fit into a 32-bit signed variable. (e.g. a 32-bit PHP integer).
 * @args RAW # Raw index. like 64-bit minus the base value.
 *
 */
static void RegisterScriptSteamID_Flags( asIScriptEngine& engine )
{
	/*
	const char* const pszObjectName = "SteamIDFormat";
	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "AUTO", (int)SteamID::Formats::AUTO );
	engine.RegisterEnumValue( pszObjectName, "STEAMID32", (int)SteamID::Formats::STEAMID32 );
	engine.RegisterEnumValue( pszObjectName, "STEAMID64", (int)SteamID::Formats::STEAMID64 );
	engine.RegisterEnumValue( pszObjectName, "STEAMID3", (int)SteamID::Formats::STEAMID3 );
	engine.RegisterEnumValue( pszObjectName, "S32", (int)SteamID::Formats::S32 );
	engine.RegisterEnumValue( pszObjectName, "RAW", (int)SteamID::Formats::RAW );
	*/
}

static std::string SteamID_Convert( CASUtil* pThis, int idformat, const std::string& steamidformat )
{
	/*
	SteamID steamid;
	steamid = SteamID::Parse( steamidformat );

	SteamID::Formats format = SteamID::Formats::AUTO;

	switch( idformat )
	{
		default:
		case (int)SteamID::Formats::AUTO:
			format = SteamID::Formats::AUTO;
		break;
		
		case (int)SteamID::Formats::STEAMID32:
			format = SteamID::Formats::STEAMID32;
		break;
		
		case (int)SteamID::Formats::STEAMID64:
			format = SteamID::Formats::STEAMID64;
		break;
		
		case (int)SteamID::Formats::STEAMID3:
			format = SteamID::Formats::STEAMID3;
		break;
		
		case (int)SteamID::Formats::S32:
			format = SteamID::Formats::S32;
		break;
		
		case (int)SteamID::Formats::RAW:
			format = SteamID::Formats::RAW;
		break;
	}

	return steamid[ format ];
	*/
	return "";
}

void UTIL2_TraceLine( CASUtil *pThis, const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, CBaseEntity *pentIgnore, TraceResult *ptr)
{
#if !defined( ASCOMPILER )
	TRACE_LINE(vecStart, vecEnd, (igmon ? TRUE : FALSE) | (ignoreGlass ? 0x100 : 0), pentIgnore->edict(), ptr);
#endif
}

void UTIL2_TraceHull( CASUtil *pThis, const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, int hullNumber, CBaseEntity *pentIgnore, TraceResult *ptr )
{
#if !defined( ASCOMPILER )
	UTIL_TraceHull( vecStart, vecEnd, igmon, hullNumber, pentIgnore->edict(), ptr);
#endif
}

void ConstructHudParams( HudTextParams* pMemory )
{
	new ( pMemory ) HudTextParams();
}

void DestroyHudParams(HudTextParams* pMemory )
{
	pMemory->~HudTextParams();
}

void HudTextParams::SetColor( Color& color )
{
	r1 = color.r();
	g1 = color.g();
	b1 = color.b();
	a1 = color.a();
}

void HudTextParams::SetColor2( Color& color )
{
	r2 = color.r();
	g2 = color.g();
	b2 = color.b();
	a2 = color.a();
}

/**MARKDOWN
**HudTextParams** defines the effects, position etc. for the game_text.

Example:
```cpp
// Create our params
HudTextParams pParams;

// Our X pos (0-1.0)
// If -1, it will be centered
pParams.x = -1;

// Our Y pos (0-1.0)
// If -1, it will be centered
pParams.y = 0.3f;

// Our channel
pParams.channel = 1;

// Fade settings
pParams.fadeinTime = 1.5f;
pParams.fadeoutTime = 0.5f;

// Our hold time
pParams.holdTime = 5.0f;

// Our FX time
pParams.fxTime = 0.25;

// Our primary color
pParams.SetColor( Color( 232, 232, 232 ) );

// Our secondary color
pParams.SetColor2( Color( 240, 110, 0 ) );

// Print our message
Utils.GameText( "The lab is open! Retrieve the Antivirus!", pParams );
```
 * @filename GameText
 */

/**MARKDOWN
**HudTextParams** defines the effects, position etc. for the game_text.

Example:
```cpp
// Create our params
HudTextParams pParams;

// Our X pos (0-1.0)
// If -1, it will be centered
pParams.x = -1;

// Our Y pos (0-1.0)
// If -1, it will be centered
pParams.y = 0.3f;

// Our channel
pParams.channel = 1;

// Fade settings
pParams.fadeinTime = 1.5f;
pParams.fadeoutTime = 0.5f;

// Our hold time
pParams.holdTime = 5.0f;

// Our FX time
pParams.fxTime = 0.25;

// Our primary color
pParams.SetColor( Color( 232, 232, 232 ) );

// Our secondary color
pParams.SetColor2( Color( 240, 110, 0 ) );

// Print our message
Utils.GameTextPlayer( pPlayer, "The lab is open! Retrieve the Antivirus!", pParams );
```
 * @filename GameTextPlayer
 */
static void RegisterHudTextParams( asIScriptEngine &engine )
{
	const char* const pszObjectName = "HudTextParams";

	engine.RegisterObjectType(
		pszObjectName, sizeof( HudTextParams ),
		asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_CLASS_ALLFLOATS );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void HudTextParams()",
		asFUNCTION( ConstructHudParams ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void DestroyTextParams()",
		asFUNCTION( DestroyHudParams ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectProperty(
		pszObjectName, "float x",
		asOFFSET( HudTextParams, x ) );

	engine.RegisterObjectProperty(
		pszObjectName, "float y",
		asOFFSET( HudTextParams, y ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int effect",
		asOFFSET( HudTextParams, effect ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int channel",
		asOFFSET( HudTextParams, channel ) );

	engine.RegisterObjectProperty(
		pszObjectName, "float fadeinTime",
		asOFFSET( HudTextParams, fadeinTime ) );

	engine.RegisterObjectProperty(
		pszObjectName, "float fadeoutTime",
		asOFFSET( HudTextParams, fadeoutTime ) );

	engine.RegisterObjectProperty(
		pszObjectName, "float holdTime",
		asOFFSET( HudTextParams, holdTime ) );

	engine.RegisterObjectProperty(
		pszObjectName, "float fxTime",
		asOFFSET( HudTextParams, fxTime ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int r1",
		asOFFSET( HudTextParams, r1 ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int g1",
		asOFFSET( HudTextParams, g1 ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int b1",
		asOFFSET( HudTextParams, b1 ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int a1",
		asOFFSET( HudTextParams, a1 ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int r2",
		asOFFSET( HudTextParams, r2 ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int g2",
		asOFFSET( HudTextParams, g2 ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int b2",
		asOFFSET( HudTextParams, b2 ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int a2",
		asOFFSET( HudTextParams, a2 ) );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetColor(Color &in clr)",
		asMETHOD( HudTextParams, SetColor ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetColor2(Color &in clr)",
		asMETHOD( HudTextParams, SetColor2 ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "HudTextParams& opAssign(const HudTextParams& in other)",
		asMETHODPR( HudTextParams, operator=, ( const HudTextParams& ), HudTextParams& ), asCALL_THISCALL );
}

unsigned long createRGB( int r, int g, int b )
{
	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

void CASUtil::ColorToHex( Color &clr, int& iHex )
{
	iHex = createRGB( clr.r(), clr.g(), clr.b() );
}

void RegisterASUtil( asIScriptEngine &engine )
{
	RegisterHudTextParams( engine );

	const char* const pszObjectName = AS_CASUTIL_CLASSNAME;

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

/**JSON
 * Converts the color into HEX
 *
 * @desc_md true
 * @global false
 * @name ColorToHex
 * @type void
 *
 * @args %Color_in% clr # The color we want to convert to HEX
 * @args %int_out% iHex # The color in HEX
 *
*/
	engine.RegisterObjectMethod(
		pszObjectName, "void ColorToHex(Color &in clr, int &out iHex)",
		asMETHOD( CASUtil, ColorToHex ), asCALL_THISCALL );

/**JSON
 * Grabs the player entity by SteamID32.
 *
 * @type object
 * @object CBasePlayer
 * @name GetPlayerBySteamID
 * @global true
 *
 * @args %string_in% strSteamID # SteamID32
 *
 * @return Returns as a CBasePlayer@ object, which can be manipulated.
 *
 */
	/*
	engine.RegisterGlobalFunction(
		"CBasePlayer@ GetPlayerBySteamID( const string& in steamid )",
		asFUNCTIONPR( CASUtils_GetPlayerBySteamID, ( const std::string& steamid ), CBasePlayer* ), asCALL_CDECL );
	*/

/**JSON
 * Grabs the player entity by name.
 *
 * @type object
 * @object CBasePlayer
 * @name GetPlayerByName
 * @global true
 *
 * @args %string_in% strValue # Our player name, index or just their userid.
 * @args %bool_in% bSpecific # If true, it will try to make sure to find the player by the exact name, index or userid.
 *
 * @return Returns as a CBasePlayer@ object, which can be manipulated.
 *
 */
	/*
	engine.RegisterGlobalFunction(
		"CBasePlayer@ GetPlayerByName( const string& in playername, bool bSpecific )",
		asFUNCTIONPR( CASUtils_GetPlayerByName, ( const std::string& playername, bool bSpecific ), CBasePlayer* ), asCALL_CDECL );
	*/

/**JSON
 * Grabs the player entity from the player slot.
 *
 * @desc_md true
 * @type object
 * @object CBasePlayer
 * @name ToBasePlayer
 * @global true
 *
 * @args %int_in% iSlot # Our player slot (1-24)
 *
 * @return Returns as a CBasePlayer@ object, which can be manipulated.
 *
 */
	engine.RegisterGlobalFunction(
		"CBasePlayer@ ToBasePlayer(int islotindex)",
		asFUNCTIONPR( CASUtils_ToBasePlayer, ( int islotindex ), CBasePlayer* ), asCALL_CDECL );

/**JSON
 * This will search for an entity that matches the corresponding classname.
 *
 * @type object
 * @object CBaseEntity
 * @name FindEntityByEntIndex
 * @global true
 *
 * @args %int_in% entindex # The specific entity by index
 *
 * @return Returns as a CBaseEntity@ object, which can be manipulated.
 *
 */
	engine.RegisterGlobalFunction(
		"CBaseEntity@ FindEntityByEntIndex( int &in entindex )",
		asFUNCTIONPR( CASUtils_FindEntity_EntIndex, ( int& entindex ), CBaseEntity* ), asCALL_CDECL );

/**MARKDOWN
Example:
```cpp
// Find entity by classname, and then remove it
string strClassname = "weapon_sig";
CBaseEntity @pEnt = FindEntityByClassname( null, strClassname );
while( pEnt !is null )
{
	// Remove it
	pEnt.SUB_Remove();
	// Find new entity
	@pEnt = FindEntityByClassname( pEnt, strClassname );
}

// Non loop version
CBaseEntity @pEnt = FindEntityByClassname( strName );
if ( pEnt !is null )
	pEnt.SUB_Remove();
```
 * @filename Util_FindEntityByClassname
 */

/**JSON
 * This will search for an entity that matches the corresponding classname.
 *
 * @type object
 * @object CBaseEntity
 * @name FindEntityByClassname
 * @global true
 * @desc_md true
 * @desc_file Util_FindEntityByClassname
 *
 * @args %class_CBaseEntity% pEntity # The specified entity it should start from (null by default)
 * @args %string_in% strClassname # The specified classname it should search for
 *
 * @return Returns as a CBaseEntity@ object, which can be manipulated.
 *
 */
	engine.RegisterGlobalFunction(
		"CBaseEntity@ FindEntityByClassname( const string& in classname )",
		asFUNCTIONPR( CASUtils_FindEntity_Classname, ( const std::string& classname ), CBaseEntity* ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"CBaseEntity@ FindEntityByClassname( CBaseEntity@ startEntity, const string& in classname )",
		asFUNCTIONPR( CASUtils_FindEntity_Classname, ( CBaseEntity* startEntity, const std::string& classname ), CBaseEntity* ), asCALL_CDECL );

/**MARKDOWN
Example:
```cpp
// Find entity by classname, and then remove it
string strName = "my_target_name";
CBaseEntity @pEnt = FindEntityByName( null, strName );
while( pEnt !is null )
{
	// Remove it
	pEnt.SUB_Remove();
	// Find new entity
	@pEnt = FindEntityByName( pEnt, strName );
}

// Non loop version
CBaseEntity @pEnt = FindEntityByName( strName );
if ( pEnt !is null )
	pEnt.SUB_Remove();
```
 * @filename Util_FindEntityByName
 */

/**JSON
 * This will search for an entity that matches the corresponding name.
 *
 * @type object
 * @object CBaseEntity
 * @name FindEntityByName
 * @global true
 * @desc_md true
 * @desc_file Util_FindEntityByName
 *
 * @args %class_CBaseEntity% pEntity # The specified entity it should start from (null by default)
 * @args %string_in% strClassname # The specified entity name it should search for
 *
 * @return Returns as a CBaseEntity@ object, which can be manipulated.
 *
 */
	engine.RegisterGlobalFunction(
		"CBaseEntity@ FindEntityByName( const string& in entityname )",
		asFUNCTIONPR( CASUtils_FindEntity_Name, ( const std::string& entityname ), CBaseEntity* ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"CBaseEntity@ FindEntityByName( CBaseEntity@ startEntity, const string& in entityname )",
		asFUNCTIONPR( CASUtils_FindEntity_Name, ( CBaseEntity* startEntity, const std::string& entityname ), CBaseEntity* ), asCALL_CDECL );

/**JSON
 * Checks if the both strings are equal.
 *
 * @type bool
 * @name StrEql
 *
 * @args %string_in% strFind # The string we want to find
 * @args %string_in% strTarget # The string we are looking for
 * @args %bool_in% bIgnoreSensitive # If true, it will ignore sensitivity
 *
 * @return Returns true if both strings are equal
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool StrEql(const string& in strVar1, const string& in strVar2)",
		asFUNCTION( UTIL_StrEql ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool StrEql(const string& in strVar1, const string& in strVar2, bool bIgnoreSensitive)",
		asFUNCTION( UTIL_StrEqlS ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Checks if the the current string contains certain words.
 *
 * @type bool
 * @name StrContains
 *
 * @args %string_in% strFind # The string we want to find
 * @args %string_in% strTarget # The string we are looking for
 *
 * @return Returns true if it contains the string we are looking for
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool StrContains(const string& in strFind, const string& in strTarget)",
		asFUNCTION( UTIL_StrContains ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Checks if the string contains numbers.
 *
 * @type bool
 * @name NumbersOnly
 *
 * @args %string_in% strValue # The string we want to check
 *
 * @return Returns true if it only contains numbers
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool NumbersOnly(const string& in strValue)",
		asFUNCTION( UTIL_NumbersOnly ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Prints a hud message for all players
 *
 * @type void
 * @name GameText
 * @desc_md true
 *
 * @args %string_in% strMessage # The message for the game text
 * @args %HudTextParams_in% params # The params for the game text
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void GameText(const string& in strMessage, HudTextParams& in params)",
		asFUNCTION( UTIL_GameText ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Prints a hud message for the specific player
 *
 * @type void
 * @name GameTextPlayer
 * @desc_md true
 * @desc_file GameTextPlayer
 *
 * @args %class_CBasePlayer% pPlayer # The player it should display the message for
 * @args %string_in% strMessage # The message for the game text
 * @args %HudTextParams_in% params # The params for the game text
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void GameTextPlayer(CBasePlayer@ pPlayer, const string& in strMessage, HudTextParams& in params)",
		asFUNCTION( UTIL_GameText_Direct ), asCALL_CDECL_OBJFIRST );

/**MARKDOWN
The function `Utils.EnvExplosion` can also be used for [CBaseEntity@][CBaseEntity] instead of a Vector `vecOrigin`.

Example of usage:  
```cpp
Utils.EnvExplosion( pEntity, 450 );
```

 * @filename EnvExplosion
 */

/**JSON
 * Cause an explosions
 *
 * @type void
 * @desc_md true
 * @name EnvExplosion
 *
 * @args %Vector% vecOrigin # The place where we want the explosion to occur at
 * @args %float_in% flDamage # The amount of damage we want to cause
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void EnvExplosion(Vector &in vecOrigin, float &in flDamage)",
		asFUNCTION( UTIL_Entity_EnvExplosion ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void EnvExplosion(CBaseEntity@ pEntity, float &in flDamage)",
		asFUNCTION( UTIL_Entity_EnvExplosion_ENT ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Cause damage within a radius
 *
 * @type void
 * @desc_md true
 * @name RadiusDamage
 *
 * @args %Vector% vOrigin # The place where we want the damage to take place
 * @args %class_CBaseEntity% pInflictor # The object we want to ignore
 * @args %class_CBaseEntity% pAttacker # The object we want to ignore
 * @args %float_in% flDamage # The amount of damage we want to cause
 * @args %int% iRadius # The radius of the damage
 * @args %int% iClassIgnore # The class we should ignore
 * @args %int% iDamageType # The damagetype
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void RadiusDamage(Vector origin, CBaseEntity@ pInflictor, CBaseEntity@ pAttacker, float damage, int radius, int classignore, int damagetype)",
		asFUNCTION( UTIL_Entity_RadiusDamage ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Fades the screen for a specific player, or everyone on the server
 *
 * @type void
 * @name ScreenFade
 *
 * @args %class_CBasePlayer% pActivator # If not null, it will only go to the specified player. If null, all players will get the message
 * @args %Color% clr # The color of the Game Instructor message
 * @args %float_in% flDuration # How long it will take to fade in/out
 * @args %float_in% flHoldTime # The max time it will hold the fade
 * @args %enum_FadeFlag_t% eFlags # What kind of fade flags it should use
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void ScreenFade(CBasePlayer@ pActivator, Color clr, float flDuration, float flHoldTime, FadeFlag_t eFlags)",
		asFUNCTION( UTIL_Entity_ScreenFade ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Causes the screen to shake, the player is within the screen shake radius
 *
 * @type void
 * @name ScreenShake
 *
 * @args %Vector_in% vPos # The position where the shake should originate from
 * @args %float_in% flDurration # The durration of our shake
 * @args %float_in% flRadius # The radius of our shake (if set to 0 or less, it will shake everywhere)
 * @args %float_in% flAmp # Our amplitude for the shake
 * @args %float_in% flFreq # Our shake frequence
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void ScreenShake(const Vector &in vPos, const float &in flDurration, const float &in flRadius, const float &in flAmp, const float &in flFreq)",
		asFUNCTION( UTIL_DoScreenShake ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Converts the float to a interger value.
 *
 * @type int
 * @name FloatToInt
 *
 * @args %float_in% flValue # Our float to input
 *
 * @return Returns the specified value as a interger.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int FloatToInt(float& in flValue)",
		asFUNCTION( UTIL_FloatToInt ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Converts the string to a interger value.
 *
 * @type int
 * @name StringToInt
 *
 * @args %string_in% strValue # Our raw string input
 *
 * @return Returns the specified value as a interger.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int StringToInt(const string& in strValue)",
		asFUNCTION( UTIL_StringToInt ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Converts the string to a float value.
 *
 * @type float
 * @name StringToFloat
 *
 * @args %string_in% strValue # Our raw string input
 *
 * @return Returns the specified value as a float.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "float StringToFloat(const string& in strValue)",
		asFUNCTION( UTIL_StringToFloat ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Escapes the characters in a string.
 *
 * @type string
 * @name EscapeCharacters
 *
 * @args %string_in% strValue # Our raw string input
 *
 * @return Returns the escape characters.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "string EscapeCharacters(const string& in strValue) const",
		asFUNCTION( UTIL_EscapeCharacters ), asCALL_CDECL_OBJFIRST );

	// Direct approch, but Utils.Convert( STEAMID64, "steamid" ); also works
/**JSON
 * Converts a player's SteamID64 to SteamID32.
 *
 * @type string
 * @name Steam64ToSteam32
 *
 * @args %string_in% strSteamID # The steamid64 it should convert
 *
 * @return Returns the SteamID32.
 *
 */
	/*
	engine.RegisterObjectMethod(
		pszObjectName, "string Steam64ToSteam32(const string& in steamid64) const",
		asFUNCTION( UTIL_Steam64toSteam32 ), asCALL_CDECL_OBJFIRST );
	*/

/**JSON
 * Converts a player's SteamID to another ID format, such as SteamID32, 64 or SteamID3.
 *
 * @type string
 * @name Convert
 *
 * @args %enum_SteamIDFormat% iFormat # SteamID format type
 * @args %string_in% strSteamID # The SteamID it should convert
 *
 * @return Returns the specified converted SteamID of choice.
 *
 */
	/*
	engine.RegisterObjectMethod(
		pszObjectName, "string Convert( SteamIDFormat format, const string& in steamidformat )",
		asFUNCTION( SteamID_Convert ), asCALL_CDECL_OBJFIRST );
	*/

/**JSON
 * This will replace our string, to whatever we desire.
 *
 * @type string
 * @name StrReplace
 *
 * @args %string_in% strValue # Our current string of choice
 * @args %string_in% strFind # The specified string we want to find
 * @args %string_in% strReplace # The string we want to replace
 * @args %bool_in% bCaseSensitive # If true, we will only replace the string if it fully matches
 *
 * @return Returns the new replaced string. If nothing was replaced, it will return the original string.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "string StrReplace(const string& in strValue, const string& in strFind, const string& in strReplace, const bool& in bCaseSensitive) const",
		asFUNCTION( UTIL_StrReplace ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Create's a line tracer for CGameTrace.
 *
 * @type void
 * @name TraceLine
 *
 * @args %Vector_in% vecStart # Starting point for the tracer
 * @args %Vector_in% vecEnd # Ending point for the tracer
 * @args %enum_IGNORE_MONSTERS% IgnoreMonsters # Should we ignore monsters or not?
 * @args %enum_IGNORE_GLASS% IgnoreGlass # Should we ignore glass or not?
 * @args %class_CBaseEntity% pEntityIgnore # Ignores said entity, so it won't hit it
 * @args %TraceResult_out% trace # Outputs the results for the tracer
 *
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void TraceLine( const Vector& in vecStart, const Vector& in vecEnd, IGNORE_MONSTERS bIgnoreMonsters, IGNORE_GLASS bIgnoreGlass, CBaseEntity@ pEntityIgnore, TraceResult& out trace )",
		asFUNCTION( UTIL2_TraceLine ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Create's a hull tracer for CGameTrace.
 *
 * @type void
 * @name TraceHull
 *
 * @args %Vector_in% vecStart # Starting point for the tracer
 * @args %Vector_in% vecEnd # Ending point for the tracer
 * @args %Vector_in% vecMin # Minimum size of the tracer
 * @args %Vector_in% vecMax # Maximum size of the tracer
 * @args %enum_IGNORE_MONSTERS% IgnoreMonsters # Should we ignore monsters or not?
 * @args %enum_HullNum% hullnum # The hull number
 * @args %class_CBaseEntity% pEntityIgnore # Ignores said entity, so it won't hit it
 * @args %TraceResult_out% trace # Outputs the results for the tracer
 *
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void TraceHull( const Vector& in vecStart, const Vector& in vecEnd, const Vector& in vecMin, const Vector& in vecMax, IGNORE_MONSTERS IgnoreMonsters, HullNum hullnum, CBaseEntity@ pEntityIgnore, TraceResult& out trace )",
		asFUNCTION( UTIL2_TraceHull ), asCALL_CDECL_OBJFIRST );

	engine.RegisterGlobalProperty( AS_CASUTIL_CLASSNAME " Utils", &g_ASUtil );
}

void RegisterScriptUtils( asIScriptEngine& engine )
{
	RegisterScriptSteamID_Flags( engine );
	RegisterScriptFadeFlags( engine );
	RegisterTraceStuff( engine );
	RegisterASUtil( engine );
}
