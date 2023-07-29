#include "cbase.h"
#include <angelscript/ScriptAPI/CASCustomEntity_Weapon.h>

#include "angelscript/HL2ASConstants.h"
#include "angelscript/ScriptAPI/CASCBaseEntity.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#include "terror/weapons/Bullet.h"
#include "TerrorPlayer.h"
#include "NextBotInterface.h"
#include "NextBotManager.h"
#else
#include "angelscript/CASCManager.h"
#include "angelscript/compile_fix/act_table.h"
#include "angelscript/compile_fix/terrorweapon.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/**PAGE
 *
 * Manipulates the CTerrorWeapon object entity, and it's values.
 *
 * @page CTerrorWeapon
 * @subpage CBaseEntity
 * @category Entities
 *
 * @desc_md false
 * @type void
 *
*/

/**
 * Class name for CTerrorWeapon in scripts.
 */
#define AS_CASCTERRORWEAPON_CLASSNAME "CTerrorWeapon"

#include "memdbgoff.h"

void ConstructWeaponInfo( WeaponInfo* pMemory )
{
	new ( pMemory ) WeaponInfo();
}

#include "memdbgon.h"

void DestructWeaponInfo( WeaponInfo* pMemory )
{
	pMemory->~WeaponInfo();
}

/**JSON
 * Contains weapon information for custom weapons
 *
 * @type class
 * @name WeaponInfo
 * @desc_md true
 * @desc_file WeaponInfo
 *
 */

/**MARKDOWN
Example:
```cpp
void GetWeaponInformation( WeaponInfo& out info )
{
	info.szResFile				= "weapon_glock";											// Our weapon resource file
	info.szPrintName			= "GLOCK 18C";												// Our weapon name
	info.szIconName				= "glock18c";												// Our weapon icon (read from the res file)
	info.szIconNameSB			= "glock18c_sb";											// Our weapon icon for Scoreboard (read from the res file)
	info.szWeaponModel_V		= "models/weapons/v_glock18c/v_glock18c.mdl";				// Our weapon model (view)
	info.szWeaponModel_W		= "models/weapons/w_models/w_glock18c/w_glock18c.mdl";		// Our weapon model (world)
	// Particles
	info.szMuzzleFlash_V		= "weapon_muzzle_flash_pistol_FP";
	info.szMuzzleFlash_W		= "weapon_muzzle_flash_pistol";
	info.szEjectBrass			= "weapon_shell_casing_9mm";
	// Melee sound events
	info.szSndMelee				= "Weapon_M1911_MeleeMiss";
	info.szSndMeleeHit			= "Weapon_M1911_MeleeHit";
	info.szSndMeleeHitWorld		= "Weapon_M1911_MeleeHitWorld";
}
```
 * @filename WeaponInfo
 */
static void RegisterWeaponInfo_Base( asIScriptEngine& engine )
{
	const char* const pszObjectName = "WeaponInfo";

	// Register it
	engine.RegisterObjectType( pszObjectName, sizeof( WeaponInfo ), asOBJ_VALUE | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_CDAK );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void WeaponInfo()",
		asFUNCTION( ConstructWeaponInfo ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void DestructWeaponInfo()",
		asFUNCTION( DestructWeaponInfo ), asCALL_CDECL_OBJFIRST );

	// Struct for WeaponInfo
	engine.RegisterObjectProperty( pszObjectName, "string szPath", asOFFSET(WeaponInfo, szPath) );
	engine.RegisterObjectProperty( pszObjectName, "string szClassname", asOFFSET(WeaponInfo, szClassname) );
	engine.RegisterObjectProperty( pszObjectName, "string szWeaponModel_V", asOFFSET(WeaponInfo, szWeaponModel_V) );
	engine.RegisterObjectProperty( pszObjectName, "string szWeaponModel_W", asOFFSET(WeaponInfo, szWeaponModel_W) );
	engine.RegisterObjectProperty( pszObjectName, "string szPrintName", asOFFSET(WeaponInfo, szPrintName) );
	engine.RegisterObjectProperty( pszObjectName, "string szIconName", asOFFSET(WeaponInfo, szIconName) );
	engine.RegisterObjectProperty( pszObjectName, "string szIconNameSB", asOFFSET(WeaponInfo, szIconNameSB) );
	engine.RegisterObjectProperty( pszObjectName, "string szMuzzleFlash_V", asOFFSET(WeaponInfo, muzzleflash_v) );
	engine.RegisterObjectProperty( pszObjectName, "string szMuzzleFlash_W", asOFFSET(WeaponInfo, muzzleflash_w) );
	engine.RegisterObjectProperty( pszObjectName, "string szEjectBrass", asOFFSET(WeaponInfo, ejectbrass) );
	engine.RegisterObjectProperty( pszObjectName, "string szResFile", asOFFSET(WeaponInfo, szResFile) );
	engine.RegisterObjectProperty( pszObjectName, "string szSndMelee", asOFFSET(WeaponInfo, szSndMelee) );
	engine.RegisterObjectProperty( pszObjectName, "string szSndMeleeHit", asOFFSET(WeaponInfo, szSndMeleeHit) );
	engine.RegisterObjectProperty( pszObjectName, "string szSndMeleeHitWorld", asOFFSET(WeaponInfo, szSndMeleeHitWorld) );
}

/**JSON
 * Enumeration of the weapon attachments
 *
 * @type enum
 * @name WeaponAttachment
 *
 * @args k_eNone # 
 * @args k_eFlashlight # 
 * @args k_eSilencer # 
 * @args k_eRailMod # 
 * @args k_eSightDef # 
 * @args k_eSightDefFront # 
 * @args k_eSightRedDot # 
 * @args k_eSightHolo # 
 * @args k_eSightScope # 
 * @args k_eLaserSight # 
 * @args k_eArrowSet # 
 * @args k_eVertGrip # 
 * @args k_eExtendedMag # 
 *
 */
static void RegisterWeaponInfo_Attachments( asIScriptEngine& engine )
{
	const char* const pszObjectName = "WeaponAttachment";
	engine.RegisterEnum( pszObjectName );
	engine.RegisterEnumValue( pszObjectName, "k_eNone", WeaponAttachmentApplyModification::Attachment_none );
	engine.RegisterEnumValue( pszObjectName, "k_eFlashlight", WeaponAttachmentApplyModification::Attachment_flashlight );
	engine.RegisterEnumValue( pszObjectName, "k_eSilencer", WeaponAttachmentApplyModification::Attachment_silencer );
	engine.RegisterEnumValue( pszObjectName, "k_eRailMod", WeaponAttachmentApplyModification::Attachment_railattachment );
	engine.RegisterEnumValue( pszObjectName, "k_eSightDef", WeaponAttachmentApplyModification::Attachment_defsight );
	engine.RegisterEnumValue( pszObjectName, "k_eSightDefFront", WeaponAttachmentApplyModification::Attachment_defsight_front );
	engine.RegisterEnumValue( pszObjectName, "k_eSightHolo", WeaponAttachmentApplyModification::Attachment_holosight );
	engine.RegisterEnumValue( pszObjectName, "k_eSightRedDot", WeaponAttachmentApplyModification::Attachment_reddot );
	engine.RegisterEnumValue( pszObjectName, "k_eSightScope", WeaponAttachmentApplyModification::Attachment_scope );
	engine.RegisterEnumValue( pszObjectName, "k_eLaserSight", WeaponAttachmentApplyModification::Attachment_lasersight );
	engine.RegisterEnumValue( pszObjectName, "k_eArrowSet", WeaponAttachmentApplyModification::Attachment_arrowset );
	engine.RegisterEnumValue( pszObjectName, "k_eVertGrip", WeaponAttachmentApplyModification::Attachment_vertgrip );
	engine.RegisterEnumValue( pszObjectName, "k_eExtendedMag", WeaponAttachmentApplyModification::Attachment_extendedmag );
}

/**JSON
 * Enumeration of the weapon upgrades
 *
 * @type enum
 * @name WeaponUpgrade
 *
 * @args k_eNone # 
 * @args k_eExplosive # 
 * @args k_eDissolve # 
 * @args k_eDoubleDamage # 
 * @args k_eFireDamage # 
 * @args k_eFlashlight # 
 * @args k_eMoneyBoost # 
 *
 */
static void RegisterWeaponUpgrade( asIScriptEngine& engine )
{
	const char* const pszObjectName = "WeaponUpgrade";
	engine.RegisterEnum( pszObjectName );
	engine.RegisterEnumValue( pszObjectName, "k_eNone", CWeaponUpgradeModule::upgrade_none );
	engine.RegisterEnumValue( pszObjectName, "k_eExplosive", CWeaponUpgradeModule::upgrade_explosive );
	engine.RegisterEnumValue( pszObjectName, "k_eDissolve", CWeaponUpgradeModule::upgrade_dissolve );
	engine.RegisterEnumValue( pszObjectName, "k_eDoubleDamage", CWeaponUpgradeModule::upgrade_dmgboost );
	engine.RegisterEnumValue( pszObjectName, "k_eGoldenBullet", CWeaponUpgradeModule::upgrade_goldenbullet );
	engine.RegisterEnumValue( pszObjectName, "k_eFireDamage", CWeaponUpgradeModule::upgrade_firebullet );
	engine.RegisterEnumValue( pszObjectName, "k_eFlashlight", CWeaponUpgradeModule::upgrade_flashlight );
	engine.RegisterEnumValue( pszObjectName, "k_eMoneyBoost", CWeaponUpgradeModule::upgrade_moneyboost );
}

enum WeaponUpgradeInfo
{
	SET = 0,
	HAS,
	REM
};
/**JSON
 * Enumeration of the weapon upgrades information
 *
 * @type enum
 * @name WeaponUpgradeInfo
 *
 * @args SET # Applies the upgrade module
 * @args HAS # Checks if it has the upgrade module
 * @args REM # Removes the upgrade moduke
 *
 */
static void RegisterWeaponUpgradeInfo( asIScriptEngine& engine )
{
	const char* const pszObjectName = "WeaponUpgradeInfo";
	engine.RegisterEnum( pszObjectName );
	engine.RegisterEnumValue( pszObjectName, "SET", WeaponUpgradeInfo::SET );
	engine.RegisterEnumValue( pszObjectName, "HAS", WeaponUpgradeInfo::HAS );
	engine.RegisterEnumValue( pszObjectName, "REM", WeaponUpgradeInfo::REM );
}

/**JSON
 * Enumeration of the WeaponInfo ammo
 *
 * @type enum
 * @name WeaponInfo_AmmoDef
 *
 * @args AMMO_NONE # If set, it will read the iAmmoSizeMax from WeaponInfo
 * @args AMMO_REVOLVER # Tells the weapon to use revolver ammo
 * @args AMMO_PISTOL # Tells the weapon to use pistol ammo
 * @args AMMO_RIFLE # Tells the weapon to use rifle ammo
 * @args AMMO_SHOTGUN # Tells the weapon to use shotgun ammo
 * @args AMMO_SNIPER # Tells the weapon to use sniper ammo
 * @args AMMO_ARROW # Tells the weapon to use arrows
 * @args AMMO_FLAME # Tells the weapon to use flame ammo
 * @args AMMO_GLAUNCHER # Tells the weapon to use grenade ammo
 * @args AMMO_45ACP # Tells the weapon to use 45acp ammo
 * @args AMMO_50AE # Tells the weapon to use 50ae ammo
 *
 */

static void RegisterWeaponInfo_Ammo( asIScriptEngine& engine )
{
	const char* const pszObjectName = "WeaponInfo_AmmoDef";
	engine.RegisterEnum( pszObjectName );
	engine.RegisterEnumValue( pszObjectName, "AMMO_NONE", WeaponInfo_AmmoDef::AMMO_NONE );
	engine.RegisterEnumValue( pszObjectName, "AMMO_REVOLVER", WeaponInfo_AmmoDef::AMMO_REVOLVER );
	engine.RegisterEnumValue( pszObjectName, "AMMO_PISTOL", WeaponInfo_AmmoDef::AMMO_PISTOL );
	engine.RegisterEnumValue( pszObjectName, "AMMO_RIFLE", WeaponInfo_AmmoDef::AMMO_RIFLE );
	engine.RegisterEnumValue( pszObjectName, "AMMO_SHOTGUN", WeaponInfo_AmmoDef::AMMO_SHOTGUN );
	engine.RegisterEnumValue( pszObjectName, "AMMO_SNIPER", WeaponInfo_AmmoDef::AMMO_SNIPER );
	engine.RegisterEnumValue( pszObjectName, "AMMO_ARROW", WeaponInfo_AmmoDef::AMMO_ARROW );
	engine.RegisterEnumValue( pszObjectName, "AMMO_FLAME", WeaponInfo_AmmoDef::AMMO_FLAME );
	engine.RegisterEnumValue( pszObjectName, "AMMO_GLAUNCHER", WeaponInfo_AmmoDef::AMMO_GLAUNCHER );
	engine.RegisterEnumValue( pszObjectName, "AMMO_45ACP", WeaponInfo_AmmoDef::AMMO_45ACP );
	engine.RegisterEnumValue( pszObjectName, "AMMO_50AE", WeaponInfo_AmmoDef::AMMO_50AE );
}

static void RegisterWeaponInfo_Sound( asIScriptEngine& engine )
{
	const char* const pszObjectName = "WeaponInfo_Sounds";
	engine.RegisterEnum( pszObjectName );
	engine.RegisterEnumValue( pszObjectName, "SINGLE", WeaponSound_t::SINGLE );
	engine.RegisterEnumValue( pszObjectName, "SINGLE_EMPTY", WeaponSound_t::EMPTY );
	engine.RegisterEnumValue( pszObjectName, "HOLSTER", WeaponSound_t::HOLSTER );
	engine.RegisterEnumValue( pszObjectName, "DRAW", WeaponSound_t::DRAW );
	engine.RegisterEnumValue( pszObjectName, "MELEE_MISS", WeaponSound_t::MELEE_MISS );
	engine.RegisterEnumValue( pszObjectName, "MELEE_HIT", WeaponSound_t::MELEE_HIT );
	engine.RegisterEnumValue( pszObjectName, "MELEE_HIT_WORLD", WeaponSound_t::MELEE_HIT_WORLD );
	engine.RegisterEnumValue( pszObjectName, "RELOAD", WeaponSound_t::RELOAD );
	engine.RegisterEnumValue( pszObjectName, "RELOAD_EMPTY", WeaponSound_t::UNLOAD );
}


void CTerrorWeapon_SetModel( CTerrorWeapon* pThis, const std::string& szWorld, const std::string& szView )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return;
	pThis->SetModel( szWorld.c_str() );
	int modelIndex = modelinfo->GetModelIndex( szView.c_str() );
	pThis->SetViewModelIndex( modelIndex );
#endif
}

bool CTerrorWeapon_UpgradeModule( CTerrorWeapon* pThis, WeaponUpgradeInfo inf, CWeaponUpgradeModule eUpgrades )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return false;

	bool bRet = false;
	switch ( inf )
	{
		case WeaponUpgradeInfo::SET:
		{
			if ( !pThis->HasUpgradeModule( eUpgrades ) )
			{
				pThis->AddUpgradeModules( eUpgrades );
				bRet = true;
			}
		}
		break;
		case WeaponUpgradeInfo::HAS:
			bRet = pThis->HasUpgradeModule( eUpgrades );
		break;
		case WeaponUpgradeInfo::REM:
		{
			if ( pThis->HasUpgradeModule( eUpgrades ) )
			{
				pThis->RemoveUpgradeModule( eUpgrades );
				bRet = true;
			}
		}
		break;
	}

	return bRet;
#endif
	return false;
}

void CTerrorWeapon_SetAttachment( CTerrorWeapon* pThis, WeaponAttachmentApplyModification attachment, int state )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return;
	int iState = state ? 0 : 1;
	if ( !pThis->FindAndSetAttachmentValue( attachment, iState ) )
		pThis->AddToRandomAttachment( iState, attachment );
#endif
}

bool CTerrorWeapon_HasAttachment( CTerrorWeapon* pThis, WeaponAttachmentApplyModification attachment, bool &bAttached )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return false;
	bool state = pThis->FindAttachment( attachment );
	if ( state ) bAttached = pThis->HasAttachmentMod( attachment );
	return state;
#else
	bAttached = false;
	return false;
#endif
}

int CTerrorWeapon_GetAmmoCount( CTerrorWeapon* pThis )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return 0;
	CBaseCombatCharacter *pOwner = pThis->GetOwner();
	if ( !pOwner ) return 0;
	return pOwner->GetAmmoCount( pThis->m_iPrimaryAmmoType );
#else
	return 0;
#endif
}

void CTerrorWeapon_SendWeaponAnim1( CTerrorWeapon* pThis, Activity iAct )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return;
	pThis->AS_SendWeaponAnim( iAct );
#endif
}

void CTerrorWeapon_SendWeaponAnim2( CTerrorWeapon* pThis, PlayerAnimEvent_t iEvent, Activity iAct )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return;
	pThis->AS_SendWeaponAnim( iEvent, iAct );
#endif
}

void CTerrorWeapon_SetWeaponIdleTime( CTerrorWeapon* pThis, float flIdle )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return;
	pThis->SetWeaponIdleTime( flIdle );
#endif
}

void CTerrorWeapon_WeaponSound( CTerrorWeapon* pThis, int iSound, float flTime )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return;
	pThis->WeaponSound( (WeaponSound_t)iSound, flTime );
#endif
}

void CTerrorWeapon_WeaponSound2( CTerrorWeapon* pThis, const std::string& szSound )
{
#if !defined( ASCOMPILER )
	if ( !pThis ) return;
	IWSoundEngine()->EmitSoundEvent( szSound.c_str(), pThis->entindex() );
#endif
}


void RegisterASCTerrorWeapon( asIScriptEngine& engine )
{
	RegisterWeaponInfo_Base( engine );
	RegisterWeaponInfo_Attachments( engine );
	RegisterWeaponInfo_Ammo( engine );
	RegisterWeaponInfo_Sound( engine );
	RegisterWeaponUpgrade( engine );
	RegisterWeaponUpgradeInfo( engine );

	const char* const pszObjectName = AS_CASCTERRORWEAPON_CLASSNAME;

	// Makes CBaseEntity our base, so we can use it's stuff
	as::RegisterCasts<CTerrorWeapon, CBaseEntity>( engine, pszObjectName, "CBaseEntity", &as::Cast_UpCast, &as::Cast_DownCast );

	RegisterASCBaseEntity( pszObjectName, engine );

/**JSON
 * Grabs the current clip
 *
 * @type int
 * @name m_iClip
 *
 */
	engine.RegisterObjectProperty(
		pszObjectName, "int m_iClip",
		asOFFSET( CTerrorWeapon, m_iClip1 ) );

/**JSON
 * Grabs the current fire state
 *
 * @type int
 * @name m_iIsFiring
 *
 */
	engine.RegisterObjectProperty(
		pszObjectName, "int m_iIsFiring",
		asOFFSET( CTerrorWeapon, m_iIsFiring ) );

/**JSON
 * Grabs the current primary attack time
 *
 * @type float
 * @name m_flNextPrimaryAttack
 *
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float m_flNextPrimaryAttack",
		asOFFSET( CTerrorWeapon, m_flNextPrimaryAttack ) );

/**JSON
 * Grabs the current secondary attack time
 *
 * @type float
 * @name m_flNextSecondaryAttack
 *
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float m_flNextSecondaryAttack",
		asOFFSET( CTerrorWeapon, m_flNextSecondaryAttack ) );

/**JSON
 * Grabs the firerate of the weapon
 *
 * @type float
 * @name GetFireRate
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "float GetFireRate()",
		asMETHOD( CTerrorWeapon, GetFireRate ), asCALL_THISCALL );

/**JSON
 * Checks if the idle time has elapsed
 *
 * @type bool
 * @name HasWeaponIdleTimeElapsed
 * @isfunc true
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool HasWeaponIdleTimeElapsed()",
		asMETHOD( CTerrorWeapon, HasWeaponIdleTimeElapsed ), asCALL_THISCALL );

/**JSON
 * Switches to another weapon
 *
 * @type void
 * @name SwitchAwayFromThis
 * @isfunc true
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SwitchAwayFromThis()",
		asMETHOD( CTerrorWeapon, SwitchAwayFromThis ), asCALL_THISCALL );

/**JSON
 * When the weapon gets created
 *
 * @type void
 * @name OnCreate
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void OnCreate()",
		asMETHOD( CTerrorWeapon, OnCreate ), asCALL_THISCALL );

/**JSON
 * When the weapon gets destroyed
 *
 * @type void
 * @name OnDestroy
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void OnDestroy()",
		asMETHOD( CTerrorWeapon, OnDestroy ), asCALL_THISCALL );

/**JSON
 * Our weapon information
 *
 * @type void
 * @name GetWeaponInformation
 * @isfunc true
 *
 * @args %WeaponInfo_out% info # The weapon information
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void GetWeaponInformation( WeaponInfo &out info )",
		asMETHOD( CTerrorWeapon, GetWeaponInformation ), asCALL_THISCALL );

/**JSON
 * Play a weapon animation
 *
 * @type bool
 * @name SendWeaponAnim
 * @isfunc true
 *
 * @args %Activity% iAct # The animation we want to play
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 * @return Returns true if the animation exists
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SendWeaponAnim( Activity iAct )",
		asFUNCTION( CTerrorWeapon_SendWeaponAnim1 ), asCALL_CDECL_OBJFIRST );
	engine.RegisterObjectMethod(
		pszObjectName, "void SendWeaponAnim( PlayerAnimEvent_t iEvent, Activity iAct )",
		asFUNCTION( CTerrorWeapon_SendWeaponAnim2 ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void WeaponSound( int iSound, float soundtime = 0.0f )",
		asFUNCTION( CTerrorWeapon_WeaponSound ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void WeaponSound( const string &in szSound )",
		asFUNCTION( CTerrorWeapon_WeaponSound2 ), asCALL_CDECL_OBJFIRST );

/**JSON
 * When the weapon should switch back to idle
 *
 * @type void
 * @name SetWeaponIdleTime
 *
 * @args %float% flIdle # When we should switch back to idle animation
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetWeaponIdleTime( float flIdle = 0.0f )",
		asFUNCTION( CTerrorWeapon_SetWeaponIdleTime ), asCALL_CDECL_OBJFIRST );

/**JSON
 * When primary attack gets fired
 *
 * @type void
 * @name PrimaryAttack
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void PrimaryAttack()",
		asMETHOD( CTerrorWeapon, PrimaryAttack ), asCALL_THISCALL );

/**JSON
 * When secondary attack gets fired
 *
 * @type void
 * @name SecondaryAttack
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SecondaryAttack()",
		asMETHOD( CTerrorWeapon, SecondaryAttack ), asCALL_THISCALL );

/**JSON
 * When melee attack gets fired
 *
 * @type void
 * @name MeleeAttack
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void MeleeAttack()",
		asMETHOD( CTerrorWeapon, MeleeAttack ), asCALL_THISCALL );

/**JSON
 * Grabs the owner of the weapon
 *
 * @type object
 * @object CTerrorPlayer
 * @name GrabOwner
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "CTerrorPlayer@ GrabOwner()",
		asMETHOD( CTerrorWeapon, GrabOwner ), asCALL_THISCALL );

/**JSON
 * Checks if the weapon is within the world
 *
 * @type bool
 * @name IsInWorld
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 * @return Returns true if the weapon is within the world
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool IsInWorld()",
		asMETHOD( CTerrorWeapon, IsInWorld ), asCALL_THISCALL );

/**JSON
 * Sets the view and world model for the weapon
 *
 * @type void
 * @name SetModel
 *
 * @args %string_in% szWorld # Sets the world model
 * @args %string_in% szView # Sets the view model
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetModel(const string &in szWorld, const string &in szView)",
		asFUNCTION( CTerrorWeapon_SetModel ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Apply, remove or check if the weapon has upgrades.
 *
 * @type void
 * @name UpgradeModule
 *
 * @args %enum_WeaponUpgradeInfo% UpgradeSet # The Upgrade we want to apply
 * @args %enum_WeaponUpgrade% UpgradeMod # The Upgrade we want to apply
 *
 * @return Returns true if we could apply, remove or if it has said upgrade
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool UpgradeModule( WeaponUpgradeInfo UpgradeSet, WeaponUpgrade UpgradeMod )",
		asFUNCTION( CTerrorWeapon_UpgradeModule ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Apply weapon attachments for the weapon.
 *
 * @type void
 * @name SetAttachment
 * @desc_md true
 * @desc_file SetAmmoAttachmentInfo
 *
 * @args %enum_WeaponAttachment% Attachment # The Attachment ID
 * @args %int% state # If true, it will set the specified bodygroup 0. If false, it will set it to 1
 *
 */
/**MARKDOWN
_**Note: The weapon requires the bodygroup (view model and world model) for it to work. You can find examples within `contagion_sdk/weapons/viewmodels` and `contagion_sdk/weapons/worldmodels`**_

Attachment Enum         | Bodygroup Name
----------------------- | ----------------------------
k_eFlashlight           | light_attachment
k_eSilencer             | silencer_attachment
k_eRailMod              | rail_attachment
k_eSightDef             | defaultsight_attachment
k_eSightDefFront        | frontsight_attachment
k_eSightRedDot          | reddot_attachment
k_eSightHolo            | holosight_attachment
k_eSightScope           | scope_attachment
k_eLaserSight           | lasersight_attachment
k_eArrowSet             | attachment_arrowset
k_eVertGrip             | vertgrip_attachment
k_eExtendedMag          | extendedmag_attachment
 * @filename SetAmmoAttachmentInfo
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetAttachment( WeaponAttachment Attachment, bool state )",
		asFUNCTION( CTerrorWeapon_SetAttachment ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Checks if the weapon has said attachment
 *
 * @type bool
 * @name HasAttachment
 *
 * @args %enum_WeaponAttachment% Attachment # The Attachment ID
 * @args %bool_out% bAttached # Returns true if it's attached
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 * @return Returns true if the attachment exist
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool HasAttachment( WeaponAttachment Attachment, bool &out bAttached )",
		asFUNCTION( CTerrorWeapon_HasAttachment ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the current ammo count
 *
 * @type int
 * @name GetAmmoCount
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 * @return Returns the amount of ammo left
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int GetAmmoCount()",
		asFUNCTION( CTerrorWeapon_GetAmmoCount ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Our max clip amount
 *
 * @type int
 * @name GetMaxClip
 * @isfunc true
 *
 * @return Returns the max clip amount
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int GetMaxClip()",
		asMETHOD( CTerrorWeapon, GetMaxClip1 ), asCALL_THISCALL );

/**JSON
 * Fires our bullets
 *
 * @type void
 * @name FireBullets
 * @isfunc true
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	// 1, vecSrc, vecAiming, VECTOR_CONE_2DEGREES, 9216, DMG_BULLET, iDamage
	engine.RegisterObjectMethod(
		pszObjectName, "void FireBullets()",
		asMETHOD( CTerrorWeapon, AS_FireBullets ), asCALL_THISCALL );

/**JSON
 * Set the ammotype this weapon should use. Check [WeaponInfo_AmmoDef] for available ammotypes
 *
 * @type void
 * @name SetAmmoType
 *
 * @args %int% AmmoType # The ammotype ID
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetAmmoType( int AmmoType )",
		asMETHOD( CTerrorWeapon, AS_SetAmmoType ), asCALL_THISCALL );
	engine.RegisterObjectMethod(
		pszObjectName, "void SetAmmoType( WeaponInfo_AmmoDef AmmoType )",
		asMETHOD( CTerrorWeapon, AS_SetAmmoType ), asCALL_THISCALL );

/**JSON
 * Set the melee damage for this weapon
 *
 * @type void
 * @name SetMeleeDamage
 *
 * @args %int% MeleeDamage # The damage amount
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetMeleeDamage( int MeleeDamage )",
		asMETHOD( CTerrorWeapon, AS_SetMeleeDamage ), asCALL_THISCALL );

/**JSON
 * Set the clip size for this weapon
 *
 * @type void
 * @name SetClipSize
 *
 * @args %int% ClipSize # The clip size
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetClipSize( int ClipSize )",
		asMETHOD( CTerrorWeapon, AS_SetClipSize ), asCALL_THISCALL );

/**JSON
 * Allow this weapon to fire underwater?
 *
 * @type void
 * @name SetAllowUnderWater
 *
 * @args %bool% ClipSize # If true, this weapon will be able to shoot underwater
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetAllowUnderWater( bool state )",
		asMETHOD( CTerrorWeapon, AS_SetAllowUnderWater ), asCALL_THISCALL );

/**JSON
 * Is this weapon heavy?
 *
 * @type void
 * @name SetIsHeavy
 *
 * @args %bool% state # If true, this weapon will do heavy staggers
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetIsHeavy( bool state )",
		asMETHOD( CTerrorWeapon, AS_SetIsHeavy ), asCALL_THISCALL );

/**JSON
 * Should this weapon allow attachments?
 *
 * @type void
 * @name SetAllowAttachmentDrop
 *
 * @args %bool% state # If true, this weapon will allow weapon drops
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc Use this function override inside your weapon script
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void SetAllowAttachmentDrop( bool state )",
		asMETHOD( CTerrorWeapon, AS_SetAllowAttachmentDrop ), asCALL_THISCALL );
}
