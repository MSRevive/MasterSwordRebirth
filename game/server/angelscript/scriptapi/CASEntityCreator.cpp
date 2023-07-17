//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "msdllheaders.h"
#include "decals.h"
#include "explode.h"
#include "qangle.h"

#include "CASEntityCreator.h"
#include "angelscript/ScriptAPI/CASEngine.h"

#ifdef SPDLOG
#include "spdlog_main.h"
#endif

// Allow AS Custom classes
#define ALLOW_CUSTOM_AS_CLASSES 1

#include <AngelscriptUtils/util/StringUtils.h>
#include <AngelscriptUtils/util/ASUtil.h>
#include <AngelscriptUtils/CASModule.h>
#include <AngelscriptUtils/wrapper/ASCallable.h>

#include "angelscript/HL2ASConstants.h"
#include "angelscript/CASClassWriter.h"

#include "CASCustomEntity_Entity.h"
//#include <angelscript/ScriptAPI/CASCustomEntity_Weapon.h>

#if ALLOW_CUSTOM_AS_CLASSES
#include "CASCustomEntityUtils.h"
#endif
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#include <angelscript/CASLoggerInterface.h>
#else
#include "angelscript/CASCManager.h"
#endif

class CEntityDataObject final
{
public:
	CEntityDataObject(CEntityData* data);
	~CEntityDataObject() = default;

	std::vector<KeyValueData>				m_dInput;
	bool m_bCustomFlag;
	int m_nFlag;

private:
	CEntityDataObject(const CEntityDataObject& vdata);
};

CASCustomEntities g_CustomEntities;

void UTIL_DestructEntity( CBaseEntity* pEntity )
{
	ASSERT( pEntity );
	pEntity->OnDestroy();
	pEntity->~CBaseEntity();
}

void UTIL_RemoveNow( CBaseEntity* pEntity )
{
	if ( !pEntity ) return;
#ifdef SERVER_DLL
	//Let UTIL_Remove's stuff happen even when removing right away. - Solokiller
	UTIL_RemoveCleanup( pEntity );

	REMOVE_ENTITY( pEntity->edict() );
#else
	UTIL_DestructEntity( pEntity );

	//On the client, entities are allocated using byte arrays. - Solokiller
	delete[] reinterpret_cast<byte*>( pEntity );
#endif
}

CBaseEntity* GET_CBASEENTITY( edict_t* pent )
{
	if ( pent )
		return reinterpret_cast<CBaseEntity*>( pent->pvPrivateData );
	return nullptr;
}

/**PAGE
 *
 * Creates new entities trough angelscript
 *
 * @page Entity Creator
 * @category Entities
 *
 * @desc_md false
 * @type void
 *
*/

void CEntityData_AddInput( CEntityData* pThis, const std::string& strInput, const std::string& strValue )
{
	if ( !pThis ) return;
	pThis->AddInput( strInput, strValue );
}

CBaseEntity *CASEntityCreator_CreateEntity_Full( const std::string& strEntityName, Vector vecOrigin, QAngle qAngle, CEntityData *data )
{
#if !defined( ASCOMPILER )
	if ( !data ) return nullptr;
	CEntityDataObject *pNet = new CEntityDataObject( data );
	if ( !pNet ) return nullptr;

	// Create our custom entity
	bool bCustomWeapon = false;
	CBaseEntity *pEntity = g_CustomEntities.CreateCustomEntity( strEntityName, bCustomWeapon );
	bool bCustomEntity = pEntity ? true : false;

	// If not a custom ent, create the noraml one
	if ( !pEntity )
	{
		int istr = MAKE_STRING( strEntityName.c_str() );
		edict_t* pent = CREATE_NAMED_ENTITY( istr );
		if (FNullEnt(pent))
		{
			ALERT( at_console, "NULL Ent in CreateEntity!\n" );
			return nullptr;
		}
		pEntity = GET_CBASEENTITY( pent );
	}

	// If custom entity
	if ( bCustomEntity )
	{
		// If custom weapon, do the weapon info overrides
#if 0
		auto pWep = dynamic_cast<ScriptBase_Weapon*>( pEntity );
		if ( pWep )
		{
			// Grab our weapon information
			WeaponInfo info;
			pWep->GetWeaponInformation( info );
			info.szClassname = strEntityName;
			pWep->SaveWeaponInformation( info );
		}
#endif

		// Add the values that should be added before the spawn
		for ( int i = 0; i < pNet->m_dInput.size(); i++ )
			pEntity->KeyValue( &pNet->m_dInput[i] );

		// Custom spawnflags
		if ( pNet->m_bCustomFlag )
			pEntity->pev->spawnflags = pNet->m_nFlag;

		pEntity->Precache();

		DispatchSpawn( pEntity->edict() );
		pEntity->pev->classname = MAKE_STRING( strEntityName.c_str() );

		// GoldSource uses Vector instead of the Source Engine QAngle. So let's just convert our QAngle to Vector
		Vector angles( qAngle.x, qAngle.y, qAngle.z );
		pEntity->pev->origin = vecOrigin;
		pEntity->pev->angles = angles;
	}
	else
	{
		if ( pEntity )
		{
			// Add the values that should be added before the spawn
			for ( int i = 0; i < pNet->m_dInput.size(); i++ )
				pEntity->KeyValue( &pNet->m_dInput[i] );

			// Custom spawnflags
			if ( pNet->m_bCustomFlag )
				pEntity->pev->spawnflags = pNet->m_nFlag;

			// Precache it
			pEntity->Precache();

			// Spawn it
			DispatchSpawn( pEntity->edict() );

			// Place the entity down
			// GoldSource uses Vector instead of the Source Engine QAngle. So let's just convert our QAngle to Vector
			Vector angles( qAngle.x, qAngle.y, qAngle.z );
			pEntity->pev->origin = vecOrigin;
			pEntity->pev->angles = angles;

			// Activate it
			pEntity->Activate();
		}
	}

	return pEntity;
#else
	return nullptr;
#endif
}

CEntityData *CASEntityCreator_EntityData()
{
	return new CEntityData();
}

CBaseEntity *CASEntityCreator_CreateEntity( const std::string& strEntityName, Vector vecOrigin, QAngle qAngle )
{
	return CASEntityCreator_CreateEntity_Full( strEntityName, vecOrigin, qAngle, new CEntityData() );
}

#if ALLOW_CUSTOM_AS_CLASSES
void CASEntityCreator_UnregisterCustomEntity( const std::string& strClassName, const std::string& strClass )
{
#if !defined( ASCOMPILER )
	g_CustomEntities.UnregisterCustomEntity( strClassName, strClass );
#endif
}

void CASEntityCreator_RegisterCustomWeapon( const std::string& strClassName, const std::string& strClass )
{
#if !defined( ASCOMPILER )
	g_CustomEntities.RegisterCustomEntity( strClassName, strClass, CustomEntityTypes_t::k_eCustomWeapon );
#endif
}

void CASEntityCreator_RegisterCustomEntity( CustomEntityTypes_t eClassType, const std::string& strClassName, const std::string& strClass )
{
#if !defined( ASCOMPILER )
	g_CustomEntities.RegisterCustomEntity( strClassName, strClass, eClassType );
#endif
}
#endif

/**JSON
 * Enumeration of custom entity types
 *
 * @type enum
 * @name CustomEntityTypes_t
 *
 * @args k_eCustomWeapon # Create custom weapons
 * @args k_eCustomEntity # Clients will be able to interact and see this entity
 *
 */

static void RegisterScriptCustomEntityTypes_t( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CustomEntityTypes_t";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "k_eCustomWeapon", k_eCustomWeapon );
	engine.RegisterEnumValue( pszObjectName, "k_eCustomEntity", k_eCustomEntity );
}

void RegisterASEntityCreator( asIScriptEngine &engine )
{
	const char *pszObjectName = "CEntityData";

	RegisterScriptCustomEntityTypes_t( engine );

	// Register the objects
	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

/**MARKDOWN
Example of usage:  
```cpp
CEntityData@ inputdata = EntityCreator::EntityData();
inputdata.Add( "targetname", "foobar" );
inputdata.Add( "spawnflags", "8192" );
inputdata.Add( "skin", "2" );
inputdata.Add( "model", "models/props_doors/door_metal.mdl" );

// After it is spawned, let's call some input values
inputdata.Add( "SetBreakable", "1", true );
inputdata.Add( "SetHealth", "100", true, "1.0" );

// Spawn our entity
CBaseEntity @pEntity = EntityCreator::Create( "prop_door_rotating", Vector( 0, 0, 0 ), QAngle( 0, 0, 0 ), inputdata );
```

 * @filename CEntityData_AddInput
 */

/**JSON
 * Adds keyvalues into CEntityData
 *
 * @desc_md true
 * @desc_file CEntityData_AddInput
 * @type void
 * @object CEntityData
 * @name Add
 *
 * @args %string_in% strInput # Key input value
 * @args %string_in% strValue # Key value
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void Add(const string &in strInput, const string &in strValue)",
		asFUNCTION( CEntityData_AddInput ), asCALL_CDECL_OBJFIRST );

///==========================================================================================
///==========================================================================================

	const std::string szOldNS = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace( "EntityCreator" );

/**JSON
 * Creates a CEntityData@ object, which can be used to manipulate the entity spawn values
 *
 * @type object
 * @object CEntityData
 * @namespace EntityCreator
 * @name EntityData
 * @isfunc true
 *
 * @return Returns as a CEntityData@ object, which can be manipulated.
 *
 */
	engine.RegisterGlobalFunction(
		"CEntityData@ EntityData()",
		asFUNCTION( CASEntityCreator_EntityData ), asCALL_CDECL );

/**JSON
 * Create and spawn a new entity within the map
 *
 * @type object
 * @object CBaseEntity
 * @namespace EntityCreator
 * @name Create
 *
 * @args %string_in% strEntity # The entity we want to create
 * @args %Vector% vecOrigin # The position, where it should spawned and teleported to
 * @args %QAngle% qAngle # The angles, where it should look at
 * @args %class_CEntityData% EntData # [optional] Adds custom keyvalues to the entity before it spawns
 *
 * @return Returns as a CBaseEntity@ object, which can be manipulated.
 *
 */
	engine.RegisterGlobalFunction(
		"CBaseEntity@ Create( const string& in strEntity, Vector vecOrigin, QAngle qAngle )",
		asFUNCTION( CASEntityCreator_CreateEntity ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"CBaseEntity@ Create( const string& in strEntity, Vector vecOrigin, QAngle qAngle, CEntityData@ EntData )",
		asFUNCTION( CASEntityCreator_CreateEntity_Full ), asCALL_CDECL );

#if ALLOW_CUSTOM_AS_CLASSES
/**MARKDOWN
Example of usage:
```cpp
// Register an entity using class: ScriptBase_Weapon
EntityCreator::RegisterCustomEntity( k_eCustomWeapon, "weapon_glock", "CScriptWeaponGlock" );

// Register an entity using class: ScriptBase_Entity
EntityCreator::RegisterCustomEntity( k_eCustomEntity, "custom_as_ent", "CScriptCustomEntity" );
```

 * @filename RegisterCustomEntity
 */

/**JSON
 * Registers a custom entity
 *
 * @desc_md true
 * @desc_file RegisterCustomEntity
 * @type void
 * @namespace EntityCreator
 * @name RegisterCustomEntity
 *
 * @args %enum_CustomEntityTypes_t% eClassType # Specify what kind of entity we want to create
 * @args %string_in% szClassname # The classname of our custom entity
 * @args %string_in% szClassobject # The class of our custom entity
 *
 */
	engine.RegisterGlobalFunction(
		"void RegisterCustomEntity( CustomEntityTypes_t eClassType, const string& in szClassname, const string& in szClassobject )",
		asFUNCTION( CASEntityCreator_RegisterCustomEntity ), asCALL_CDECL );

/**MARKDOWN
Example of usage:
```cpp
// Register our weapon
EntityCreator::RegisterCustomWeapon( "weapon_glock", "CScriptWeaponGlock" );
```

 * @filename RegisterCustomWeapon
 */

/**JSON
 * Registers a custom weapon
 *
 * @desc_md true
 * @desc_file RegisterCustomWeapon
 * @type void
 * @namespace EntityCreator
 * @name RegisterCustomWeapon
 *
 * @infobox_enable true
 * @infobox_type deprecated
 * @infobox_desc Use The new [RegisterCustomEntity][RegisterCustomEntity] function instead.
 *
 * @args %string_in% szClassname # The classname of our custom weapon
 * @args %string_in% szClassobject # The class of our custom weapon
 *
 */
	engine.RegisterGlobalFunction(
		"void RegisterCustomWeapon( const string& in szClassname, const string& in szClassobject )",
		asFUNCTION( CASEntityCreator_RegisterCustomWeapon ), asCALL_CDECL );

/**JSON
 * Deletes the custom class from memory
 *
 * @type void
 * @namespace EntityCreator
 * @name UnregisterCustomEntity
 *
 * @args %string_in% szClassname # The classname of our custom entity
 * @args %string_in% szClassobject # The class of our custom entity
 *
 */
	engine.RegisterGlobalFunction(
		"void UnregisterCustomEntity( const string& in szClassname, const string& in szClassobject )",
		asFUNCTION( CASEntityCreator_UnregisterCustomEntity ), asCALL_CDECL );

	engine.SetDefaultNamespace( szOldNS.c_str() );

	RegisterCustomASEntities( engine );
#else
	engine.SetDefaultNamespace( szOldNS.c_str() );
#endif
}

CEntityDataObject::CEntityDataObject( CEntityData* data )
{
	for ( int i = 0; i < data->GetSize(); i++)
	{
		KeyValueData kvd;
		char buf[128];
		_snprintf(buf, sizeof(buf),  "%s",  data->GetString( i, false ) );
		kvd.szKeyName = buf;

		buf[0] = 0;
		_snprintf(buf, sizeof(buf),  "%s",  data->GetString( i, true ) );
		kvd.szValue = buf;
		m_dInput.push_back( kvd );
	}

	m_bCustomFlag = data->HasCustomFlags();
	m_nFlag = data->GetSpawnFlags();
}


bool AS_IsNumber( const char* strValue )
{
	std::string stdstr = strValue;
	return stdstr.find_first_not_of("0123456789") == std::string::npos;
}

void CEntityData::AddInput( const std::string &strInput, const std::string &strValue )
{
#if !defined( ASCOMPILER )
	if ( strInput == "spawnflags" && AS_IsNumber( strValue.c_str() ) )
	{
		m_bCustomFlags = true;
		m_nFlags = atoi( strValue.c_str() );
	}
	else
	{
		IEntityKeyData *entdata = new IEntityKeyData();
		entdata->m_strInput = strInput;
		entdata->m_strValue = strValue;
		m_hKeys.push_back( entdata );
	}
#endif
}

const char *CEntityData::GetString( int index, bool type )
{
	if ( index >= m_hKeys.size() ) return "";
	if ( index < 0 ) index = 0;
	IEntityKeyData *entdata = m_hKeys[ index ];
	if ( !entdata ) return "";
	return type ? entdata->m_strValue.c_str() : entdata->m_strInput.c_str();
}

//=========================================================================================================================
static void RegisterScriptICustomEntity( asIScriptEngine& engine )
{
	const char* const pszObjectName = "ICustomEntity";

	engine.RegisterInterface( pszObjectName );
}

static void RegisterScriptCallbackHandler( asIScriptEngine& engine )
{
	engine.RegisterFuncdef( "void ThinkFunc()" );
	engine.RegisterFuncdef( "void TouchFunc(CBaseEntity@ pOther)" );
	engine.RegisterFuncdef( "void BlockedFunc(CBaseEntity@ pOther)" );

	const char* const pszObjectName = "CCallbackHandler";

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetThink(ThinkFunc@ pFunction)",
		asMETHOD( IASCustomEntity, SetScriptThink ), asCALL_THISCALL );
}

namespace CustomEnts
{
	static void* Cast( CBaseEntity* pEntity )
	{
		if( !pEntity )
			return nullptr;

		auto pCustom = dynamic_cast<IASCustomEntity*>( pEntity );

		if( !pCustom )
			return nullptr;

		auto& instance = pCustom->GetObject();

		void* pInstance = nullptr;

		//AddRef it to prevent problems.
		as::SetObjPointer( pInstance, instance.Get(), *instance.GetTypeInfo().Get() );

		return pInstance;
	}
}

static void RegisterScriptCustomEntitiesUtils( asIScriptEngine& engine )
{
	const std::string szOldNS = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace( "CustomEnts" );

	engine.RegisterGlobalFunction(
		"ICustomEntity@ Cast(CBaseEntity@ pEntity)",
		asFUNCTION( CustomEnts::Cast ), asCALL_CDECL );

	engine.SetDefaultNamespace( szOldNS.c_str() );
}

void RegisterCustomASEntities( asIScriptEngine& engine )
{
	RegisterScriptICustomEntity( engine );
	RegisterScriptCallbackHandler( engine );
	RegisterScriptCustomEntitiesUtils( engine );
}
//=========================================================================================================================

// Custom entity registration
bool CASCustomEntities::Initialize()
{
	GenerateBaseClasses();
	return true;
}

void CASCustomEntities::Shutdown()
{
	m_BaseClasses.clear();
	m_BaseClasses.shrink_to_fit();
}

void CASCustomEntities::WorldCreated()
{
	m_bAllowRegistration = true;
}

void CASCustomEntities::WorldActivated()
{
	m_bAllowRegistration = false;
}

void CASCustomEntities::WorldEnded()
{
	m_ClassList.clear();
	m_ClassList.shrink_to_fit();
}

const BaseClassData_t* CASCustomEntities::FindBaseClassByClassName( const char* const pszClassName ) const
{
	for( const auto& baseClass : m_BaseClasses )
	{
		if( baseClass.szClassName == pszClassName )
			return &baseClass;
	}

	return nullptr;
}

const CCustomEntityClass* CASCustomEntities::FindCustomClassByMapName( const char* const pszMapName ) const
{
	for( const auto& clazz : m_ClassList )
	{
		if( clazz->GetMapName() == pszMapName )
			return clazz.get();
	}

	return nullptr;
}

bool CASCustomEntities::RegisterCustomEntity( const std::string& szClassName, const std::string& szClassObject, CustomEntityTypes_t classtype )
{
	std::string szNewClassName = szClassName;
	as::Trim( szNewClassName );

	if ( szNewClassName.empty() )
	{
		as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Empty classname given!\n" );
		return false;
	}

	std::string szNewClassObject = szClassObject;
	as::Trim( szNewClassObject );

	if ( szNewClassObject.empty() )
	{
		as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Empty class object given!\n" );
		return false;
	}

	if ( !m_bAllowRegistration )
	{
		as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Not allowed to register a custom entity at this time!\n" );
		return false;
	}

	if ( FindCustomClassByMapName( szNewClassName.c_str() ) )
	{
		as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Attempted to register a custom entity with the same entity classname \"%s\" twice!\n", szNewClassName.c_str() );
		return false;
	}

	auto pModule = GetModuleFromScriptContext( asGetActiveContext() );

	if( !pModule )
	{
		as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Couldn't get module from current script!\n" );
		return false;
	}

	auto pScriptModule = pModule->GetModule();

	if( !pScriptModule )
	{
		as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Couldn't get module from current script!\n" );
		return false;
	}

	const auto szNamespace = as::ExtractNamespaceFromName( szClassObject );
	const auto szName = as::ExtractNameFromName( szClassObject );

	const std::string szOldNS = pScriptModule->GetDefaultNamespace();
	pScriptModule->SetDefaultNamespace( szNamespace.c_str() );

	auto pTypeInfo = pScriptModule->GetTypeInfoByName( szName.c_str() );

	pScriptModule->SetDefaultNamespace( szOldNS.c_str() );

	if( !pTypeInfo )
	{
		as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Couldn't find class \"%s\" in module \"%s\"!\n", szNewClassObject.c_str(), pScriptModule->GetName() );
		return false;
	}

	if( !( pTypeInfo->GetTypeId() & asTYPEID_MASK_OBJECT ) )
	{
		as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Type \"%s\" in module \"%s\" is not a class!\n", szNewClassObject.c_str(), pScriptModule->GetName() );
		return false;
	}

	const BaseClassData_t* pBaseClassData = nullptr;

	auto pBase = pTypeInfo;

	//Walk up the inheritance tree, checking to see if the base class is one of the custom entity base classes.
	//If it doesn't match, this is not a custom entity class.
	do
	{
		pBase = pBase->GetBaseType();

		if( !pBase )
		{
			as::GameLog( as::LogType::LOG_ERR, "CCustomEntities::RegisterCustomEntity: Class \"%s\" in module \"%s\" does not have a custom entity base class!\n", szNewClassObject.c_str(), pScriptModule->GetName() );
			return false;
		}

		//Check to see if it's in a namespace.
		//This could cause crashes later if it's not checked.
		if( !( *pBase->GetNamespace() ) )
			pBaseClassData = FindBaseClassByClassName( pBase->GetName() );
	}
	while( !pBaseClassData );

	as::GameLog( as::LogType::LOG_INF, "Registering custom entity \"%s\" (class \"%s\")\n", szNewClassName.c_str(), szNewClassObject.c_str() );

	m_ClassList.emplace_back( std::make_unique<CCustomEntityClass>( std::move( szNewClassName ), pTypeInfo, *pBaseClassData, classtype ) );

	return true;
}

bool CASCustomEntities::UnregisterCustomEntity( const std::string& szClassName, const std::string& szClassObject )
{
	const auto szNamespace = as::ExtractNamespaceFromName( szClassObject );
	const auto szName = as::ExtractNameFromName( szClassObject );

	for( auto it = m_ClassList.begin(), end = m_ClassList.end(); it != end; ++it )
	{
		const auto& clazz = *it;

		if( clazz->GetMapName() == szClassName &&
			clazz->GetTypeInfo()->GetNamespace() == szNamespace && 
			clazz->GetTypeInfo()->GetName() == szName )
		{
			as::GameLog( as::LogType::LOG_INF, "Unregistering custom entity \"%s\" (class \"%s\")\n", szClassName.c_str(), szClassObject.c_str() );

			m_ClassList.erase( it );

			return true;
		}
	}

	return false;
}

CBaseEntity* CASCustomEntities::CreateCustomEntity( const std::string& szClassname, bool &bWeapon )
{
#if !defined( ASCOMPILER )
	bWeapon = false;
	auto pCustomClass = g_CustomEntities.FindCustomClassByMapName( szClassname.c_str() );
	if ( !pCustomClass )
	{
		as::GameLog( as::LogType::LOG_ERR, "Custom entity creation: Class \"%s\" does not exist\n", szClassname.c_str() );
		return nullptr;
	}

	const auto& typeInfo = pCustomClass->GetTypeInfo();

	auto pInstance = reinterpret_cast<asIScriptObject*>( typeInfo->GetEngine()->CreateScriptObject( typeInfo.Get() ) );

	if ( !pInstance )
	{
		as::GameLog( as::LogType::LOG_ERR, "Custom entity creation: Couldn't instance class \"%s\"\n", typeInfo->GetName() );
		return nullptr;
	}

	const char *szBaseEntity = nullptr;
	switch ( pCustomClass->GetClass() )
	{
		case CustomEntityTypes_t::k_eCustomWeapon: szBaseEntity = "weapon_script"; break;
		case CustomEntityTypes_t::k_eCustomEntity: szBaseEntity = "as_script_ent"; break;
	}

	if ( !szBaseEntity )
	{
		as::GameLog( as::LogType::LOG_ERR, "Custom entity creation: Failed to create class for \"%s\", base entity was not found.\n", typeInfo->GetName() );
		return nullptr;
	}

	auto pEdict = CREATE_ENTITY();
	if ( !pEdict )
	{
		as::GameLog( as::LogType::LOG_ERR, "CASCustomEntities::CreateCustomEntity: Couldn't create edict instance for \"%s\"\n", szClassname.c_str() );
		return nullptr;
	}

	auto pCPPInstance = GET_CBASEENTITY( pEdict );
	if ( !pCPPInstance )
	{
		as::GameLog( as::LogType::LOG_ERR, "Custom entity creation: Couldn't instance C++ class for custom entity \"%s\"\n", szClassname.c_str() );
		g_engfuncs.pfnRemoveEntity( pEdict );
		typeInfo->GetEngine()->ReleaseScriptObject( pInstance, typeInfo.Get() );
		return nullptr;
	}

	//ScriptBase_Weapon *pCustom = new ScriptBase_Weapon;
	auto pCustom = dynamic_cast<IASCustomEntity*>( pCPPInstance );

	if ( !pCustom )
	{
		as::GameLog( as::LogType::LOG_ERR, "Custom entity creation: C++ class for custom entity \"%s\" is not a custom entity handler\n", szClassname.c_str() );
		UTIL_RemoveNow( pCPPInstance );
		typeInfo->GetEngine()->ReleaseScriptObject( pInstance, typeInfo.Get() );
		return nullptr;
	}

	//The C++ class is now responsible for the instance's lifetime.
	//Note: must transfer ownership here since CreateScriptObject added a reference for us.
	pCustom->SetObject( CASObjPtr( pInstance, typeInfo, true ) );
	pCustom->SetClass( pCustomClass );

	if ( !CallInitMethod( *typeInfo, pInstance, pCPPInstance, ( std::string( "void SetSelf(" ) + pCustomClass->GetBaseClassData().szCPPClassName + "@)" ).c_str() ) )
	{
		UTIL_RemoveNow( pCPPInstance );
		return nullptr;
	}

	if ( !CallInitMethod( *typeInfo, pInstance, pCPPInstance, ( std::string( "void SetBaseClass(" ) + pCustomClass->GetBaseClassData().szBaseClassName + "@)" ).c_str() ) )
	{
		UTIL_RemoveNow( pCPPInstance );
		return nullptr;
	}

	//Note: pCustom is required here since the CBaseEntity pointer isn't adjusted for the class's vtable.
	if ( !CallInitMethod( *typeInfo, pInstance, pCustom, "void SetCallbackHandler(CCallbackHandler@)" ) )
	{
		UTIL_RemoveNow( pCPPInstance );
		return nullptr;
	}

	if ( !pCustom->OnScriptClassReady() )
	{
		as::GameLog( as::LogType::LOG_ERR, "Custom entity creation: Class \"%s\" failed post initialization\n", typeInfo->GetName() );
		UTIL_RemoveNow( pCPPInstance );
		return nullptr;
	}

	// Set the out value
	bWeapon = ( pCustomClass->GetClass() == CustomEntityTypes_t::k_eCustomWeapon ) ? true : false;

	return pCPPInstance;
#else
	return nullptr;
#endif
}

void CASCustomEntities::GenerateBaseClasses()
{
	{
		//CASBaseClassCreator scriptbase_ent( "ScriptBase_Entity", "CBaseEntity", "CBaseEntity", m_BaseClasses );
		//CASBaseClassCreator scriptbase_wep( "ScriptBase_Weapon", "CWeapon", "CBaseEntity", m_BaseClasses );
	}
}

bool CASCustomEntities::CallInitMethod( const asITypeInfo& typeInfo, void* pInstance, void* pCPPInstance, const char* const pszMethod )
{
	bool bSuccess = false;

	if( auto pFunction = typeInfo.GetMethodByDecl( pszMethod ) )
	{
		bSuccess = as::Call( pInstance, pFunction, pCPPInstance );
	}
	else
	{
		as::GameLog( as::LogType::LOG_ERR, "Custom entity creation: Class \"%s::%s\" does not have a required method \"%s\"\n", typeInfo.GetNamespace(), typeInfo.GetName(), pszMethod );
	}

	if( !bSuccess )
	{
		as::GameLog( as::LogType::LOG_ERR, "Custom entity creation: Class \"%s::%s\" failed to call \"%s\"\n", typeInfo.GetNamespace(), typeInfo.GetName(), pszMethod );
	}

	return bSuccess;
}

//=========================================================================================================================
void CASBaseClassCreator::GenerateCommonBaseClassContents( CASClassWriter& writer, const char* const pszEntityClass, const char* const pszBaseClass )
{
	ASSERT( pszEntityClass );
	ASSERT( pszBaseClass );

	const std::string szEntHandle = pszEntityClass + std::string( "@" );
	const std::string szBaseHandle = pszBaseClass + std::string( "@" );

	writer.StartClassDeclaration( true, "ICustomEntity" );

	writer.WriteProperty( CASClassWriter::Visibility::PRIVATE, szEntHandle.c_str(), "m_pSelf", "null" );
	writer.StartPropDeclaration( CASClassWriter::Visibility::PUBLIC, szEntHandle.c_str(), "self" );
		writer.WritePropGetter( "return @m_pSelf;", true );
	writer.EndPropDeclaration();

	writer.NewLine();

	writer.WriteProperty( CASClassWriter::Visibility::PRIVATE, szBaseHandle.c_str(), "m_pBaseClass", "null" );
	writer.StartPropDeclaration( CASClassWriter::Visibility::PUBLIC, szBaseHandle.c_str(), "BaseClass" );
		writer.WritePropGetter( "return @m_pBaseClass;", true );
	writer.EndPropDeclaration();

	writer.WriteProperty( CASClassWriter::Visibility::PRIVATE, "CCallbackHandler@", "m_pCallbackHandler", "null" );
	writer.StartPropDeclaration( CASClassWriter::Visibility::PUBLIC, "CCallbackHandler@", "CallbackHandler" );
		writer.WritePropGetter( "return @m_pCallbackHandler;", true );
	writer.EndPropDeclaration();

	writer.NewLine();

	writer.WriteConstructorHeader();
	writer.StartBracket();
	writer.EndBracket();

	writer.NewLine();

	writer.WriteMethodHeader( CASClassWriter::Visibility::PUBLIC, "void", "SetSelf", ( szEntHandle + " pSelf" ).c_str(), false, true );
	writer.StartBracket();
		writer.Write(
			"if( m_pSelf !is null )\n"
			"\treturn;\n"
			"@m_pSelf = @pSelf;\n" );
	writer.EndBracket();

	writer.NewLine();

	writer.WriteMethodHeader( CASClassWriter::Visibility::PUBLIC, "void", "SetBaseClass", ( szBaseHandle + " pBaseClass" ).c_str(), false, true );
	writer.StartBracket();
		writer.Write(
			"if( m_pBaseClass !is null )\n"
			"\treturn;\n"
			"@m_pBaseClass = @pBaseClass;\n" );
	writer.EndBracket();

	writer.NewLine();

	writer.WriteMethodHeader( CASClassWriter::Visibility::PUBLIC, "void", "SetCallbackHandler", "CCallbackHandler@ pHandler", false, true );
	writer.StartBracket();
		writer.Write(
			"if( m_pCallbackHandler !is null )\n"
			"\treturn;\n"
			"@m_pCallbackHandler = @pHandler;\n" );
	writer.EndBracket();

	writer.NewLine();

	writer.WriteMethodHeader( CASClassWriter::Visibility::PUBLIC, "void", "SetThink", "ThinkFunc@ pFunc", false, true );
	writer.StartBracket();
		writer.Write(
			"m_pCallbackHandler.SetThink( pFunc );" );
	writer.EndBracket();

	writer.NewLine();

	/*
	writer.WriteMethodHeader( CASClassWriter::Visibility::PUBLIC, "void", "SetTouch", "TouchFunc@ pFunc", false, true );
	writer.StartBracket();
		writer.Write(
			"m_pCallbackHandler.SetTouch( pFunc );" );
	writer.EndBracket();

	writer.NewLine();

	writer.WriteMethodHeader( CASClassWriter::Visibility::PUBLIC, "void", "SetBlocked", "BlockedFunc@ pFunc", false, true );
	writer.StartBracket();
		writer.Write(
			"m_pCallbackHandler.SetBlocked( pFunc );" );
	writer.EndBracket();

	writer.NewLine();
	*/

	//Implicit conversion to CBaseEntity@ to allow passing this for CBaseEntity@ parameters.
	writer.WriteMethodHeader( CASClassWriter::Visibility::PUBLIC, szEntHandle.c_str(), "opImplCast", "", true, true );
	writer.StartBracket();
		writer.Write( "return @self;" );
	writer.EndBracket();

	writer.NewLine();
}

// Base entities
#if !defined( ASCOMPILER )
LINK_ENTITY_TO_CLASS( as_script_ent, CASScript_Entity );
#endif

void CASScript_Entity::Spawn()
{
	CBaseCustomCBaseEntity<CBaseEntity>::Spawn();
#if !defined( ASCOMPILER )
	SetThink( &CASScript_Entity::OnThink );
	SetTouch( &CASScript_Entity::OnTouch );
	SetUse( &CASScript_Entity::OnUse );
	SetBlocked( &CASScript_Entity::OnBlocked );
#endif
//	SetNextThink( gpGlobals->curtime + 0.5f );
}

void CASScript_Entity::OnThink()
{
	// DEBUG
#if 0
	engine->Con_NPrintf( 10, "== TRIPMINE FUNC NAMES ==" );
	engine->Con_NPrintf( 11, "Think: %s", m_szThinkFunc.c_str() );
	engine->Con_NPrintf( 12, "Touch: %s", m_szTouchFunc.c_str() );
	engine->Con_NPrintf( 13, "Blocked: %s", m_szBlockedFunc.c_str() );
	engine->Con_NPrintf( 14, "Use: %s", m_szUseFunc.c_str() );
	SetNextThink( gpGlobals->curtime + 0.1f );
#else
	CBaseCustomCBaseEntity<CBaseEntity>::Think();
#endif
}

void CASScript_Entity::SetScriptThink( const std::string& szFunction )
{
	m_szThinkFunc = szFunction;
}
