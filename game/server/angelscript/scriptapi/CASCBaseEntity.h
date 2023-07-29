#ifndef GAME_SERVER_ANGELSCRIPT_SCRIPTAPI_ENTITIES_ASCBASEENTITY_H
#define GAME_SERVER_ANGELSCRIPT_SCRIPTAPI_ENTITIES_ASCBASEENTITY_H

#include <string>

#include <angelscript.h>

#include <AngelscriptUtils/util/ASUtil.h>

#include "CTakeDamageInfo.h"

/**
*	Registers types, constants, globals, etc used by entities.
*	@param engine Script engine.
*/
void RegisterScriptEntityDependencies( asIScriptEngine& engine );

/**
*	Class name for CBaseEntity in scripts.
*/
#define AS_CBASEENTITY_NAME "CBaseEntity"

/**
*	Wrapper so scripts can call KeyValue.
*/
template<typename CLASS>
bool CBaseEntity_KeyValue( CLASS* pThis, const std::string& szKeyName, const std::string& szValue )
{
	KeyValueData kvd;

	char buf[128];
	_snprintf(buf, sizeof(buf),  "%s",  szKeyName.c_str() );
	kvd.szKeyName		= buf;

	buf[0] = 0;
	_snprintf(buf, sizeof(buf),  "%s", szValue.c_str() );
	kvd.szValue			= buf;

	buf[0] = 0;
	_snprintf(buf, sizeof(buf),  "%s", STRING( pThis->pev->classname ) );
	kvd.szClassName		= buf;

	kvd.fHandled		= false;

	pThis->KeyValue( &kvd );

	return kvd.fHandled != 0;
}

std::string CBaseEntity_GetClassname( const CBaseEntity* pThis );

bool CBaseEntity_ClassnameIs( CBaseEntity* pThis, const std::string& szClassname );

std::string CBaseEntity_GetGlobalName( const CBaseEntity* pThis );

void CBaseEntity_SetGlobalName( CBaseEntity* pThis, const std::string& szGlobalName );

std::string CBaseEntity_GetTargetname( const CBaseEntity* pThis );

void CBaseEntity_SetTargetname( CBaseEntity* pThis, const std::string& szTargetname );

std::string CBaseEntity_GetTarget( const CBaseEntity* pThis );

void CBaseEntity_SetTarget( CBaseEntity* pThis, const std::string& szTarget );

std::string CBaseEntity_GetNetName( const CBaseEntity* pThis );

void CBaseEntity_SetNetName( CBaseEntity* pThis, const std::string& szNetName );

bool CBaseEntity_HasTarget( CBaseEntity* pThis, const std::string& szTarget );

std::string CBaseEntity_TeamID( CBaseEntity* pThis );

std::string CBaseEntity_GetNoise( const CBaseEntity* pThis );

void CBaseEntity_SetNoise( CBaseEntity* pThis, const std::string& szNoise );

std::string CBaseEntity_GetNoise1( const CBaseEntity* pThis );

void CBaseEntity_SetNoise1( CBaseEntity* pThis, const std::string& szNoise );

std::string CBaseEntity_GetNoise2( const CBaseEntity* pThis );

void CBaseEntity_SetNoise2( CBaseEntity* pThis, const std::string& szNoise );

std::string CBaseEntity_GetNoise3( const CBaseEntity* pThis );

void CBaseEntity_SetNoise3( CBaseEntity* pThis, const std::string& szNoise );

std::string CBaseEntity_GetMessage( const CBaseEntity* pThis );

void CBaseEntity_SetMessage( CBaseEntity* pThis, const std::string& szMessage );

std::string CBaseEntity_GetModelName( const CBaseEntity* pThis );

void CBaseEntity_SetModel( CBaseEntity* pThis, const std::string& szModelName );

std::string CBaseEntity_GetViewModelName( const CBaseEntity* pThis );

void CBaseEntity_SetViewModelName( CBaseEntity* pThis, const std::string& szViewModelName );

std::string CBaseEntity_GetWeaponModelName( const CBaseEntity* pThis );

void CBaseEntity_SetWeaponModelName( CBaseEntity* pThis, const std::string& szWeaponModelName );


/**
*	Registers CBaseEntity methods and properties.
*	Uses templates to avoid virtual function calls in scripts whenever possible.
*	Remember to add new entities to the list of forward declared types in ASCBaseEntity.cpp
*	@param engine Script engine.
*	@param pszObjectName Name of the class to register object data for.
*	@tparam CLASS Concrete C++ class being registered.
*/
template<typename CLASS>
inline void RegisterScriptCBaseEntity( asIScriptEngine& engine, const char* const pszObjectName )
{
	//Register casts to convert between entity types.
	as::RegisterCasts<CBaseEntity, CLASS>( engine, AS_CBASEENTITY_NAME, pszObjectName, &as::Cast_UpCast, &as::Cast_DownCast );

	engine.RegisterObjectProperty(
		pszObjectName, "CBaseEntity@ m_pLink",
		asOFFSET( CLASS, m_pLink ) );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetClassname() const",
		asFUNCTION( CBaseEntity_GetClassname ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool ClassnameIs(const string& in szClassname) const",
		asFUNCTION( CBaseEntity_ClassnameIs ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasGlobalName() const",
		asMETHOD( CBaseEntity, HasGlobalName ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetGlobalName() const",
		asFUNCTION( CBaseEntity_GetGlobalName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetGlobalName(const string& in szGlobalName)",
		asFUNCTION( CBaseEntity_SetGlobalName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearGlobalName()",
		asMETHOD( CBaseEntity, ClearGlobalName ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasTargetname() const",
		asMETHOD( CBaseEntity, HasTargetname ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetTargetname() const",
		asFUNCTION( CBaseEntity_GetTargetname ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetTargetname(const string& in szTargetname)",
		asFUNCTION( CBaseEntity_SetTargetname ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearTargetname()",
		asMETHOD( CBaseEntity, ClearTargetname ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasTarget() const",
		asMETHODPR( CLASS, HasTarget, () const, bool ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetTarget() const",
		asFUNCTION( CBaseEntity_GetTarget ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetTarget(const string& in szTarget)",
		asFUNCTION( CBaseEntity_SetTarget ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearTarget()",
		asMETHOD( CBaseEntity, ClearTarget ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasNetName() const",
		asMETHOD( CBaseEntity, HasNetName ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNetName() const",
		asFUNCTION( CBaseEntity_GetNetName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetNetName(const string& in szNetName)",
		asFUNCTION( CBaseEntity_SetNetName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearNetName()",
		asMETHOD( CBaseEntity, ClearNetName ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetAbsOrigin() const",
		asMETHOD( CBaseEntity, GetAbsOrigin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAbsOrigin(const Vector& in vecOrigin)",
		asMETHOD( CBaseEntity, SetAbsOrigin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetOldOrigin() const",
		asMETHOD( CBaseEntity, GetOldOrigin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetOldOrigin(const Vector& in vecOrigin)",
		asMETHOD( CBaseEntity, SetOldOrigin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetAbsVelocity() const",
		asMETHOD( CBaseEntity, GetAbsVelocity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAbsVelocity(const Vector& in vecVelocity)",
		asMETHOD( CBaseEntity, SetAbsVelocity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetBaseVelocity() const",
		asMETHOD( CBaseEntity, GetBaseVelocity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetBaseVelocity(const Vector& in vecVelocity)",
		asMETHOD( CBaseEntity, SetBaseVelocity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetMoveDir() const",
		asMETHOD( CBaseEntity, GetMoveDir ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetMoveDir(const Vector& in vecMoveDir)",
		asMETHOD( CBaseEntity, SetMoveDir ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetAbsAngles() const",
		asMETHOD( CBaseEntity, GetAbsAngles ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAbsAngles(const Vector& in vecAngles)",
		asMETHOD( CBaseEntity, SetAbsAngles ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetAngularVelocity() const",
		asMETHOD( CBaseEntity, GetAngularVelocity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAngularVelocity(const Vector& in vecVelocity)",
		asMETHOD( CBaseEntity, SetAngularVelocity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetPunchAngle() const",
		asMETHOD( CBaseEntity, GetPunchAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetPunchAngle(const Vector& in vecPunchAngle)",
		asMETHOD( CBaseEntity, SetPunchAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetViewAngle() const",
		asMETHOD( CBaseEntity, GetViewAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetViewAngle(const Vector& in vecViewAngle)",
		asMETHOD( CBaseEntity, SetViewAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetIdealPitch() const",
		asMETHOD( CBaseEntity, GetIdealPitch ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetIdealPitch(const float flIdealPitch)",
		asMETHOD( CBaseEntity, SetIdealPitch ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetPitchSpeed() const",
		asMETHOD( CBaseEntity, GetPitchSpeed ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetPitchSpeed(const float flPitchSpeed)",
		asMETHOD( CBaseEntity, SetPitchSpeed ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetIdealYaw() const",
		asMETHOD( CBaseEntity, GetIdealYaw ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetIdealYaw(const float flIdealYaw)",
		asMETHOD( CBaseEntity, SetIdealYaw ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetYawSpeed() const",
		asMETHOD( CBaseEntity, GetYawSpeed ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetYawSpeed(const float flYawSpeed)",
		asMETHOD( CBaseEntity, SetYawSpeed ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetModelIndex() const",
		asMETHOD( CBaseEntity, GetModelIndex ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetModelIndex(const int iModelIndex)",
		asMETHOD( CBaseEntity, SetModelIndex ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasModel() const",
		asMETHOD( CBaseEntity, HasModel ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetModelName() const",
		asFUNCTION( CBaseEntity_GetModelName ), asCALL_CDECL_OBJFIRST );

	//Not exposed: SetModelName. The engine handles it, if it's needed add a special handler for it. - Solokiller

	engine.RegisterObjectMethod(
		pszObjectName, "void SetModel(const string& in szModelName)",
		asFUNCTION( CBaseEntity_SetModel ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearModel()",
		asMETHOD( CBaseEntity, ClearModel ), asCALL_THISCALL );

#ifdef VALVE_DLL
	engine.RegisterObjectMethod(
		pszObjectName, "string GetViewModelName() const",
		asFUNCTION( CBaseEntity_GetViewModelName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetViewModelName(const string& in szViewModelName)",
		asFUNCTION( CBaseEntity_SetViewModelName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearViewModelName()",
		asMETHOD( CBaseEntity, ClearViewModelName ), asCALL_THISCALL );
#endif

	engine.RegisterObjectMethod(
		pszObjectName, "string GetWeaponModelName() const",
		asFUNCTION( CBaseEntity_GetWeaponModelName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetWeaponModelName(const string& in szWeaponModelName)",
		asFUNCTION( CBaseEntity_SetWeaponModelName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearWeaponModelName()",
		asMETHOD( CBaseEntity, ClearWeaponModelName ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetWeaponAnim() const",
		asMETHOD( CBaseEntity, GetWeaponAnim ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetWeaponAnim(const int iWeaponAnim)",
		asMETHOD( CBaseEntity, SetWeaponAnim ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsDucking() const",
		asMETHOD( CBaseEntity, IsDucking ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetDucking(const bool bDucking)",
		asMETHOD( CBaseEntity, SetDucking ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetStepSoundTime() const",
		asMETHOD( CBaseEntity, GetStepSoundTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetStepSoundTime(const int iTime)",
		asMETHOD( CBaseEntity, SetStepSoundTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetSwimSoundTime() const",
		asMETHOD( CBaseEntity, GetSwimSoundTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetSwimSoundTime(const int iTime)",
		asMETHOD( CBaseEntity, SetSwimSoundTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetDuckTime() const",
		asMETHOD( CBaseEntity, GetDuckTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetDuckTime(const int iTime)",
		asMETHOD( CBaseEntity, SetDuckTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsStepLeft() const",
		asMETHOD( CBaseEntity, IsStepLeft ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetStepLeft(const bool bStepLeft)",
		asMETHOD( CBaseEntity, SetStepLeft ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetFallVelocity() const",
		asMETHOD( CBaseEntity, GetFallVelocity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetFallVelocity(const float flFallVelocity)",
		asMETHOD( CBaseEntity, SetFallVelocity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetAbsMin() const",
		asMETHOD( CBaseEntity, GetAbsMin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAbsMin(const Vector& in vecMin)",
		asMETHOD( CBaseEntity, SetAbsMin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetAbsMax() const",
		asMETHOD( CBaseEntity, GetAbsMax ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAbsMax(const Vector& in vecMax)",
		asMETHOD( CBaseEntity, SetAbsMax ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetRelMin() const",
		asMETHOD( CBaseEntity, GetRelMin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetRelMin(const Vector& in vecMin)",
		asMETHOD( CBaseEntity, SetRelMin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetRelMax() const",
		asMETHOD( CBaseEntity, GetRelMax ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetRelMax(const Vector& in vecMax)",
		asMETHOD( CBaseEntity, SetRelMax ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetBounds() const",
		asMETHOD( CBaseEntity, GetBounds ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetSize(const Vector& in vecSize)",
		asMETHODPR( CBaseEntity, SetSize, ( const Vector& ), void ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetSize(const Vector& in vecMin, const Vector& in vecMax)",
		asMETHODPR( CBaseEntity, SetSize, ( const Vector&, const Vector& ), void ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetLastThink() const",
		asMETHOD( CBaseEntity, GetLastThink ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetLastThink(const float flLastThink)",
		asMETHOD( CBaseEntity, SetLastThink ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetNextThink() const",
		asMETHOD( CBaseEntity, GetNextThink ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetNextThink(const float flNextThink)",
		asMETHOD( CBaseEntity, SetNextThink ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetSkin() const",
		asMETHOD( CBaseEntity, GetSkin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetSkin(const int iSkin)",
		asMETHOD( CBaseEntity, SetSkin ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetBody() const",
		asMETHOD( CBaseEntity, GetBody ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetBody(const int iBody)",
		asMETHOD( CBaseEntity, SetBody ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetGravity() const",
		asMETHOD( CBaseEntity, GetGravity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetGravity(const float flGravity)",
		asMETHOD( CBaseEntity, SetGravity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetFriction() const",
		asMETHOD( CBaseEntity, GetFriction ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetFriction(const float flFriction)",
		asMETHOD( CBaseEntity, SetFriction ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetSequence() const",
		asMETHOD( CBaseEntity, GetSequence ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetSequence(const int iSequence)",
		asMETHOD( CBaseEntity, SetSequence ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetGaitSequence() const",
		asMETHOD( CBaseEntity, GetGaitSequence ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetGaitSequence(const int iGaitSequence)",
		asMETHOD( CBaseEntity, SetGaitSequence ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetFrame() const",
		asMETHOD( CBaseEntity, GetFrame ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetFrame(const float flFrame)",
		asMETHOD( CBaseEntity, SetFrame ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetAnimTime() const",
		asMETHOD( CBaseEntity, GetAnimTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAnimTime(const float flAnimTime)",
		asMETHOD( CBaseEntity, SetAnimTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetFrameRate() const",
		asMETHOD( CBaseEntity, GetFrameRate ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetFrameRate(const float flFrameRate)",
		asMETHOD( CBaseEntity, SetFrameRate ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetScale() const",
		asMETHOD( CBaseEntity, GetScale ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetScale(const float flScale)",
		asMETHOD( CBaseEntity, SetScale ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetRenderAmount() const",
		asMETHOD( CBaseEntity, GetRenderAmount ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetRenderAmount(const float flRenderAmount)",
		asMETHOD( CBaseEntity, SetRenderAmount ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetRenderColor() const",
		asMETHOD( CBaseEntity, GetRenderColor ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetRenderColor(const Vector& in vecColor)",
		asMETHOD( CBaseEntity, SetRenderColor ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetHealth() const",
		asMETHOD( CBaseEntity, GetHealth ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetHealth(const float flHealth)",
		asMETHOD( CBaseEntity, SetHealth ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetMaxHealth() const",
		asMETHOD( CBaseEntity, GetMaxHealth ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetMaxHealth(const float flMaxHealth)",
		asMETHOD( CBaseEntity, SetMaxHealth ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetArmorAmount() const",
		asMETHOD( CBaseEntity, GetArmorAmount ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetArmorAmount(const float flArmorAmount)",
		asMETHOD( CBaseEntity, SetArmorAmount ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetArmorType() const",
		asMETHOD( CBaseEntity, GetArmorType ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetArmorType(const float flArmorType)",
		asMETHOD( CBaseEntity, SetArmorType ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetFrags() const",
		asMETHOD( CBaseEntity, GetFrags ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetFrags(const float flFrags)",
		asMETHOD( CBaseEntity, SetFrags ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "const Vector& GetViewOffset() const",
		asMETHOD( CBaseEntity, GetViewOffset ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector& GetMutableViewOffset()",
		asMETHOD( CBaseEntity, GetMutableViewOffset ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetViewOffset(const Vector& in vecViewOffset)",
		asMETHOD( CBaseEntity, SetViewOffset ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetImpulse() const",
		asMETHODPR( CLASS, GetImpulse, () const, int ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetImpulse(const int iImpulse)",
		asMETHODPR( CLASS, SetImpulse, ( const int ), void ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetColorMap() const",
		asMETHODPR( CLASS, GetColorMap, () const, int ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetColorMap(const int iColorMap)",
		asMETHODPR( CLASS, SetColorMap, ( const int ), void ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void GetColorMap( int& out iTopColor, int& out iBottomColor ) const",
		asMETHODPR( CLASS, GetColorMap, ( int&, int& ) const, void ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetColorMap(const int iTopColor, const int iBottomColor)",
		asMETHODPR( CLASS, SetColorMap, ( const int, const int ), void ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetTeamID() const",
		asMETHOD( CBaseEntity, GetTeamID ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetTeamID(const int iTeamID)",
		asMETHOD( CBaseEntity, SetTeamID ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetPlayerClass() const",
		asMETHOD( CBaseEntity, GetPlayerClass ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetPlayerClass(const int iPlayerClass)",
		asMETHOD( CBaseEntity, SetPlayerClass ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasMessage() const",
		asMETHOD( CBaseEntity, HasMessage ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetMessage() const",
		asFUNCTION( CBaseEntity_GetMessage ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetMessage(const string& in szMessage)",
		asFUNCTION( CBaseEntity_SetMessage ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearMessage()",
		asMETHOD( CBaseEntity, ClearMessage ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetSpeed() const",
		asMETHOD( CBaseEntity, GetSpeed ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetSpeed(const float flSpeed)",
		asMETHOD( CBaseEntity, SetSpeed ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetMaxSpeed() const",
		asMETHOD( CBaseEntity, GetMaxSpeed ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetMaxSpeed(const float flMaxSpeed)",
		asMETHOD( CBaseEntity, SetMaxSpeed ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetAirFinishedTime() const",
		asMETHOD( CBaseEntity, GetAirFinishedTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAirFinishedTime(const float flime)",
		asMETHOD( CBaseEntity, SetAirFinishedTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetPainFinishedTime() const",
		asMETHOD( CBaseEntity, GetPainFinishedTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetPainFinishedTime(const float flime)",
		asMETHOD( CBaseEntity, SetPainFinishedTime ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetFOV() const",
		asMETHOD( CBaseEntity, GetFOV ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetFOV(const float flFOV)",
		asMETHOD( CBaseEntity, SetFOV ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetDamage() const",
		asMETHOD( CBaseEntity, GetDamage ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetDamage(const float flDamage)",
		asMETHOD( CBaseEntity, SetDamage ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ GetOwner() const",
		asMETHODPR( CLASS, GetOwner, () const, CBaseEntity* ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ GetOwner()",
		asMETHODPR( CLASS, GetOwner, (), CBaseEntity* ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetOwner(CBaseEntity@ pOwner)",
		asMETHOD( CBaseEntity, SetOwner ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ GetGroundEntity()",
		asMETHOD( CBaseEntity, GetGroundEntity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetGroundEntity(CBaseEntity@ pGroundEntity)",
		asMETHOD( CBaseEntity, SetGroundEntity ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ GetChain()",
		asMETHOD( CBaseEntity, GetChain ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetChain(CBaseEntity@ pEntity)",
		asMETHOD( CBaseEntity, SetChain ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasNoise() const",
		asMETHOD( CBaseEntity, HasNoise ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNoise() const",
		asFUNCTION( CBaseEntity_GetNoise ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetNoise(const string& in szNoise)",
		asFUNCTION( CBaseEntity_SetNoise ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearNoise()",
		asMETHOD( CBaseEntity, ClearNoise ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasNoise1() const",
		asMETHOD( CBaseEntity, HasNoise1 ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNoise1() const",
		asFUNCTION( CBaseEntity_GetNoise1 ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetNoise1(const string& in szNoise)",
		asFUNCTION( CBaseEntity_SetNoise1 ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearNoise1()",
		asMETHOD( CBaseEntity, ClearNoise1 ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasNoise2() const",
		asMETHOD( CBaseEntity, HasNoise2 ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNoise2() const",
		asFUNCTION( CBaseEntity_GetNoise2 ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetNoise2(const string& in szNoise)",
		asFUNCTION( CBaseEntity_SetNoise2 ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearNoise2()",
		asMETHOD( CBaseEntity, ClearNoise2 ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasNoise3() const",
		asMETHOD( CBaseEntity, HasNoise3 ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNoise3() const",
		asFUNCTION( CBaseEntity_GetNoise3 ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetNoise3(const string& in szNoise)",
		asFUNCTION( CBaseEntity_SetNoise3 ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void ClearNoise3()",
		asMETHOD( CBaseEntity, ClearNoise3 ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void KeyValue(KeyValueData@ pkvd)",
		asMETHOD( CBaseEntity, KeyValue ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool KeyValue(const string& in szKeyName, const string& in szValue)",
		asFUNCTION( CBaseEntity_KeyValue<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "int ObjectCaps() const",
		asMETHOD( CBaseEntity, ObjectCaps ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetObjectCollisionBox()",
		asMETHOD( CBaseEntity, SetObjectCollisionBox ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ Respawn()",
		asMETHOD( CBaseEntity, Respawn ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int entindex() const",
		asMETHOD( CBaseEntity, entindex ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void Think()",
		asMETHOD( CBaseEntity, Think ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void Touch(CBaseEntity@ pOther)",
		asMETHOD( CBaseEntity, Touch ), asCALL_THISCALL );

	//Give flValue a default so people don't always have to type it.
	engine.RegisterObjectMethod(
		pszObjectName, "void Use(CBaseEntity@ pActivator, CBaseEntity@ pCaller, USE_TYPE useType, float flValue = 0)",
		asMETHOD( CBaseEntity, Use ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void Blocked(CBaseEntity@ pOther)",
		asMETHOD( CBaseEntity, Blocked ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "EntityClassification_t Classify()",
		asMETHOD( CBaseEntity, Classify ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int BloodColor() const",
		asMETHOD( CBaseEntity, BloodColor ), asCALL_THISCALL );

	//Pointers and references are equivalent, so we don't need to update CBaseEntity's methods just yet. - Solokiller
	engine.RegisterObjectMethod(
		pszObjectName, "void TraceAttack(const CTakeDamageInfo& in info, Vector vecDir, TraceResult& in tr)",
		asMETHOD( CBaseEntity, TraceAttack_New ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void TraceBleed(const CTakeDamageInfo& in info, Vector vecDir, TraceResult& in tr)",
		asMETHOD( CBaseEntity, TraceBleed_New ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void Killed(const CTakeDamageInfo& in info, GibAction gibAction)",
		asMETHOD( CBaseEntity, Killed_New ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsTriggered(const CBaseEntity@ pActivator) const",
		asMETHOD( CBaseEntity, IsTriggered ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseMonster@ MyMonsterPointer()",
		asMETHOD( CBaseEntity, MyMonsterPointer ), asCALL_THISCALL );

	//TODO: MySquadMonsterPointer probably won't exist for much longer so don't expose it. - Solokiller

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsMoving() const",
		asMETHOD( CBaseEntity, IsMoving ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void OverrideReset()",
		asMETHOD( CBaseEntity, OverrideReset ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int DamageDecal(int bitsDamageType) const",
		asMETHOD( CBaseEntity, DamageDecal ), asCALL_THISCALL );

	//TODO: OnControls may be replaced with a better way so don't expose it. - Solokiller

	engine.RegisterObjectMethod(
		pszObjectName, "bool OnControls(CBaseEntity@ pTest) const",
		asMETHOD( CBaseEntity, OnControls ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsAlive() const",
		asMETHOD( CBaseEntity, IsAlive ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsBSPModel() const",
		asMETHOD( CBaseEntity, IsBSPModel ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool ReflectGauss() const",
		asMETHOD( CBaseEntity, ReflectGauss ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasTarget(const string& in szTarget) const",
		asFUNCTION( CBaseEntity_HasTarget ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsInWorld() const",
		asMETHOD( CBaseEntity, IsInWorld ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsPlayer() const",
		asMETHOD( CBaseEntity, IsPlayer ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsNetClient() const",
		asMETHOD( CBaseEntity, IsNetClient ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ GetNextTarget()",
		asMETHOD( CBaseEntity, GetNextTarget ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SUB_Remove()",
		asMETHOD( CBaseEntity, SUB_Remove ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SUB_DoNothing()",
		asMETHOD( CBaseEntity, SUB_DoNothing ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SUB_StartFadeOut()",
		asMETHOD( CBaseEntity, SUB_StartFadeOut ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SUB_FadeOut()",
		asMETHOD( CBaseEntity, SUB_FadeOut ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SUB_CallUseToggle()",
		asMETHOD( CBaseEntity, SUB_CallUseToggle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool ShouldToggle(USE_TYPE useType, const bool bCurrentState) const",
		asMETHOD( CBaseEntity, ShouldToggle ), asCALL_THISCALL );

#if 0
	engine.RegisterObjectMethod(
		pszObjectName, "void FireBullets("
		"const uint uiShots,"
		"Vector vecSrc, Vector vecDirShooting, Vector vecSpread,"
		"float flDistance, int iBulletType,"
		"int iTracerFreq = 4, int iDamage = 0, CBaseEntity@ pAttacker = null)",
		asMETHOD( CBaseEntity, FireBullets ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void FireBulletsPlayer("
		"const uint uiShots,"
		"Vector vecSrc, Vector vecDirShooting, Vector vecSpread,"
		"float flDistance, int iBulletType,"
		"int iTracerFreq = 4, int iDamage = 0, CBaseEntity@ pAttacker = null, int shared_rand = 0)",
		asMETHOD( CBaseEntity, FireBulletsPlayer ), asCALL_THISCALL );
#endif

	engine.RegisterObjectMethod(
		pszObjectName, "void SUB_UseTargets(CBaseEntity@ pActivator, USE_TYPE useType, float flValue)",
		asMETHOD( CBaseEntity, SUB_UseTargets ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool Intersects(CBaseEntity@ pOther) const",
		asMETHOD( CBaseEntity, Intersects ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void MakeDormant()",
		asMETHOD( CBaseEntity, MakeDormant ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsDormant() const",
		asMETHOD( CBaseEntity, IsDormant ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsLockedByMaster() const",
		asMETHOD( CBaseEntity, IsLockedByMaster ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void DeathNotice(CBaseEntity@ pChild)",
		asMETHOD( CBaseEntity, DeathNotice ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector Center() const",
		asMETHOD( CBaseEntity, Center ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector EyePosition() const",
		asMETHOD( CBaseEntity, EyePosition ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector EarPosition() const",
		asMETHOD( CBaseEntity, EarPosition ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector BodyTarget(const Vector& in vecPosSrc) const",
		asMETHOD( CBaseEntity, Center ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int Illumination() const",
		asMETHOD( CBaseEntity, Illumination ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int FVisible(const CBaseEntity@ pEntity) const",
		asMETHODPR( CLASS, FVisible, ( CBaseEntity* ), BOOL ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int FVisible(const Vector& in vecOrigin) const",
		asMETHODPR( CLASS, FVisible, ( const Vector& ), BOOL ), asCALL_THISCALL );
}

inline void RegisterScriptCBaseEntity( asIScriptEngine& engine )
{
	RegisterScriptCBaseEntity<CBaseEntity>( engine, AS_CBASEENTITY_NAME );
}

template<typename CLASS>
void BaseEntity_OnCreate( CLASS* pThis )
{
	pThis->CLASS::OnCreate();
}

template<typename CLASS>
void BaseEntity_OnDestroy( CLASS* pThis )
{
	pThis->CLASS::OnDestroy();
}

template<typename CLASS>
void BaseEntity_UpdateOnRemove( CLASS* pThis )
{
	pThis->CLASS::UpdateOnRemove();
}

template<typename CLASS>
void BaseEntity_KeyValue( CLASS* pThis, KeyValueData* pkvd )
{
	pThis->CLASS::KeyValue( pkvd );
}

/**
*	Wrapper so scripts can call KeyValue.
*/
template<typename CLASS>
bool BaseEntity_KeyValue( CLASS* pThis, const std::string& szKeyName, const std::string& szValue )
{
	KeyValueData kvd;

	char buf[128];
	_snprintf(buf, sizeof(buf),  "%s",  szKeyName.c_str() );
	kvd.szKeyName		= buf;

	buf[0] = 0;
	_snprintf(buf, sizeof(buf),  "%s", szValue.c_str() );
	kvd.szValue = buf;

	buf[0] = 0;
	_snprintf(buf, sizeof(buf),  "%s", STRING( pThis->pev->classname ) );
	kvd.szClassName		= buf;

	kvd.fHandled = false;

	pThis->CLASS::KeyValue( &kvd );

	return kvd.fHandled != 0;
}

template<typename CLASS>
void BaseEntity_Precache( CLASS* pThis )
{
	pThis->CLASS::Precache();
}

template<typename CLASS>
void BaseEntity_Spawn( CLASS* pThis )
{
	pThis->CLASS::Spawn();
}

template<typename CLASS>
void BaseEntity_Activate( CLASS* pThis )
{
	pThis->CLASS::Activate();
}

template<typename CLASS>
int BaseEntity_ObjectCaps( CLASS* pThis )
{
	return pThis->CLASS::ObjectCaps();
}

template<typename CLASS>
void BaseEntity_SetObjectCollisionBox( CLASS* pThis )
{
	pThis->CLASS::SetObjectCollisionBox();
}

template<typename CLASS>
CBaseEntity* BaseEntity_Respawn( CLASS* pThis )
{
	return pThis->CLASS::Respawn();
}

template<typename CLASS>
void BaseEntity_Think( CLASS* pThis )
{
	pThis->CLASS::Think();
}

template<typename CLASS>
void BaseEntity_Touch( CLASS* pThis, CBaseEntity* pOther )
{
	pThis->CLASS::Touch( pOther );
}

template<typename CLASS>
void BaseEntity_Use( CLASS* pThis, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue )
{
	pThis->CLASS::Use( pActivator, pCaller, useType, flValue );
}

template<typename CLASS>
void BaseEntity_Blocked( CLASS* pThis, CBaseEntity* pOther )
{
	pThis->CLASS::Blocked( pOther );
}

template<typename CLASS>
int BaseEntity_Classify( CLASS* pThis )
{
	return pThis->CLASS::Classify();
}

template<typename CLASS>
int BaseEntity_BloodColor( CLASS* pThis )
{
	return pThis->CLASS::BloodColor();
}

template<typename CLASS>
void BaseEntity_TraceAttack( CLASS* pThis, const CTakeDamageInfo& info, Vector vecDir, TraceResult& tr )
{
	pThis->CLASS::TraceAttack_New( info, vecDir, tr );
}

template<typename CLASS>
void BaseEntity_TraceBleed( CLASS* pThis, const CTakeDamageInfo& info, Vector vecDir, TraceResult& tr )
{
	pThis->CLASS::TraceBleed_New( info, vecDir, tr );
}

template<typename CLASS>
void BaseEntity_OnTakeDamage( CLASS* pThis, const CTakeDamageInfo& info )
{
	pThis->CLASS::OnTakeDamage( info );
}

template<typename CLASS>
void BaseEntity_Killed( CLASS* pThis, const CTakeDamageInfo& info, int gibAction )
{
	pThis->CLASS::Killed_New( info, gibAction );
}

template<typename CLASS>
bool BaseEntity_IsTriggered( CLASS* pThis, CBaseEntity* pActivator )
{
	return (bool)pThis->CLASS::IsTriggered( pActivator );
}

//TODO: MyMonsterPointer - Solokiller

template<typename CLASS>
bool BaseEntity_IsMoving( CLASS* pThis )
{
	return (bool)pThis->CLASS::IsMoving();
}

template<typename CLASS>
void BaseEntity_OverrideReset( CLASS* pThis )
{
	pThis->CLASS::OverrideReset();
}

template<typename CLASS>
int BaseEntity_DamageDecal( CLASS* pThis, int bitsDamageType )
{
	return pThis->CLASS::DamageDecal( bitsDamageType );
}

template<typename CLASS>
bool BaseEntity_OnControls( CLASS* pThis, CBaseEntity* pTest )
{
	return (bool)pThis->CLASS::OnControls( pTest->pev );
}

template<typename CLASS>
bool BaseEntity_IsAlive( CLASS* pThis )
{
	return (bool)pThis->CLASS::IsAlive();
}

template<typename CLASS>
bool BaseEntity_IsBSPModel( CLASS* pThis )
{
	return pThis->CLASS::IsBSPModel();
}

template<typename CLASS>
bool BaseEntity_ReflectGauss( CLASS* pThis )
{
	return (bool)pThis->CLASS::ReflectGauss();
}

template<typename CLASS>
bool BaseEntity_HasTarget( CLASS* pThis, const std::string& targetname )
{
	return (bool)pThis->CLASS::HasTarget( MAKE_STRING( targetname.c_str() ) );
}

template<typename CLASS>
bool BaseEntity_IsInWorld( CLASS* pThis )
{
	return (bool)pThis->CLASS::IsInWorld();
}

template<typename CLASS>
std::string BaseEntity_TeamID( CLASS* pThis )
{
	return pThis->CLASS::TeamID();
}

template<typename CLASS>
CBaseEntity* BaseEntity_GetNextTarget( CLASS* pThis )
{
	return pThis->CLASS::GetNextTarget();
}

template<typename CLASS>
bool BaseEntity_IsLockedByMaster( CLASS* pThis )
{
	return (bool)pThis->CLASS::IsLockedByMaster();
}

template<typename CLASS>
void BaseEntity_DeathNotice( CLASS* pThis, CBaseEntity* pChild )
{
	pThis->CLASS::DeathNotice( pChild->pev );
}

template<typename CLASS>
Vector BaseEntity_Center( CLASS* pThis )
{
	return pThis->CLASS::Center();
}

template<typename CLASS>
Vector BaseEntity_EyePosition( CLASS* pThis )
{
	return pThis->CLASS::EyePosition();
}

template<typename CLASS>
Vector BaseEntity_EarPosition( CLASS* pThis )
{
	return pThis->CLASS::EarPosition();
}

template<typename CLASS>
Vector BaseEntity_BodyTarget( CLASS* pThis, const Vector& posSrc )
{
	return pThis->CLASS::BodyTarget( posSrc );
}

template<typename CLASS>
int BaseEntity_Illumination( CLASS* pThis )
{
	return pThis->CLASS::Illumination();
}

template<typename CLASS>
bool BaseEntity_FVisible( CLASS* pThis, CBaseEntity* pEntity )
{
	return (bool)pThis->CLASS::FVisible( pEntity );
}

template<typename CLASS>
bool BaseEntity_FVisible( CLASS* pThis, Vector& vecOrigin )
{
	return (bool)pThis->CLASS::FVisible( vecOrigin );
}

/**
*	Registers CBaseEntity methods for the BaseClass type for CLASS.
*	@param engine Script engine.
*	@param pszObjectName Name of the class to register.
*/
template<typename CLASS>
void RegisterScriptBaseEntity( asIScriptEngine& engine, const char* const pszObjectName )
{
	engine.RegisterObjectType(
		pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	engine.RegisterObjectMethod(
		pszObjectName, "void OnCreate()",
		asFUNCTION( BaseEntity_OnCreate<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void OnDestroy()",
		asFUNCTION( BaseEntity_OnDestroy<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void UpdateOnRemove()",
		asFUNCTION( BaseEntity_UpdateOnRemove<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void KeyValue(KeyValueData@ pkvd)",
		asFUNCTIONPR( BaseEntity_KeyValue<CLASS>, ( CLASS*, KeyValueData* ), void ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool KeyValue(const string& in szKey, const string& in szValue)",
		asFUNCTIONPR( BaseEntity_KeyValue<CLASS>, ( CLASS*, const std::string&, const std::string& ), bool ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void Precache()",
		asFUNCTION( BaseEntity_Precache<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void Spawn()",
		asFUNCTION( BaseEntity_Spawn<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void Activate()",
		asFUNCTION( BaseEntity_Activate<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "int ObjectCaps()",
		asFUNCTION( BaseEntity_ObjectCaps<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetObjectCollisionBox()",
		asFUNCTION( BaseEntity_SetObjectCollisionBox<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ Respawn()",
		asFUNCTION( BaseEntity_Respawn<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void Think()",
		asFUNCTION( BaseEntity_Think<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void Touch(CBaseEntity@ pOther)",
		asFUNCTION( BaseEntity_Touch<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void Use(CBaseEntity@ pActivator, CBaseEntity@ pCaller, USE_TYPE useType, float flValue)",
		asFUNCTION( BaseEntity_Use<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void Blocked(CBaseEntity@ pOther)",
		asFUNCTION( BaseEntity_Blocked<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "int Classify()",
		asFUNCTION( BaseEntity_Classify<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "int BloodColor()",
		asFUNCTION( BaseEntity_BloodColor<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void TraceAttack(const CTakeDamageInfo& in info, Vector vecDir, TraceResult& in tr)",
		asFUNCTION( BaseEntity_TraceAttack<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void TraceBleed(const CTakeDamageInfo& in info, Vector vecDir, TraceResult& in tr)",
		asFUNCTION( BaseEntity_TraceBleed<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void OnTakeDamage(const CTakeDamageInfo& in info)",
		asFUNCTION( BaseEntity_OnTakeDamage<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void Killed(const CTakeDamageInfo& in info, GibAction gibAction)",
		asFUNCTION( BaseEntity_Killed<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsTriggered(const CBaseEntity@ pActivator)",
		asFUNCTION( BaseEntity_IsTriggered<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsMoving()",
		asFUNCTION( BaseEntity_IsMoving<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void OverrideReset()",
		asFUNCTION( BaseEntity_OverrideReset<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "int DamageDecal(int bitsDamageType)",
		asFUNCTION( BaseEntity_DamageDecal<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool OnControls(const CBaseEntity@ pTest)",
		asFUNCTION( BaseEntity_OnControls<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsAlive()",
		asFUNCTION( BaseEntity_IsAlive<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsBSPModel()",
		asFUNCTION( BaseEntity_IsBSPModel<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool ReflectGauss()",
		asFUNCTION( BaseEntity_ReflectGauss<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool HasTarget(const string& in targetname)",
		asFUNCTION( BaseEntity_HasTarget<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsInWorld()",
		asFUNCTION( BaseEntity_IsInWorld<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "string TeamID()",
		asFUNCTION( BaseEntity_TeamID<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ GetNextTarget()",
		asFUNCTION( BaseEntity_GetNextTarget<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsLockedByMaster()",
		asFUNCTION( BaseEntity_IsLockedByMaster<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void DeathNotice(CBaseEntity@ pChild)",
		asFUNCTION( BaseEntity_DeathNotice<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector Center()",
		asFUNCTION( BaseEntity_Center<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector EyePosition()",
		asFUNCTION( BaseEntity_EyePosition<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector EarPosition()",
		asFUNCTION( BaseEntity_EarPosition<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector BodyTarget(const Vector& in posSrc)",
		asFUNCTION( BaseEntity_BodyTarget<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "int Illumination()",
		asFUNCTION( BaseEntity_Illumination<CLASS> ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool FVisible(CBaseEntity@ pEntity)",
		asFUNCTIONPR( BaseEntity_FVisible<CLASS>, ( CLASS*, CBaseEntity* ), bool ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool FVisible(Vector& in vecOrigin)",
		asFUNCTIONPR( BaseEntity_FVisible<CLASS>, ( CLASS*, Vector& ), bool ), asCALL_CDECL_OBJFIRST );
}

/**
*	Registers the BaseClass type for CBaseEntity: BaseEntity.
*/
inline void RegisterScriptBaseEntity( asIScriptEngine& engine )
{
	RegisterScriptBaseEntity<CBaseEntity>( engine, AS_CBASEENTITY_NAME + 1 );
}

#endif //GAME_SERVER_ANGELSCRIPT_SCRIPTAPI_ENTITIES_ASCBASEENTITY_H