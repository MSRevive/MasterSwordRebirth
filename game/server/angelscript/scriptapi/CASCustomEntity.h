#ifndef GAME_SERVER_ANGELSCRIPT_ENTITIES_CASCUSTOMENTITIES_H
#define GAME_SERVER_ANGELSCRIPT_ENTITIES_CASCUSTOMENTITIES_H

#include <angelscript.h>

// Include our interface, which contains the rest of the data
#include <angelscript/ScriptAPI/IASCustomEntity.h>

class CScriptBase_Entity;
class CScriptBase_Weapon;

class CBaseEntity;

class CASClassWriter;

/**
*	Manages the list of custom entity base classes and registered classes for the current map.
*/
class CASCustomEntities final
{
public:
	using BaseClassList_t = std::vector<BaseClassData_t>;
	using ClassList_t = std::vector<std::unique_ptr<CCustomEntityClass>>;

public:
	CASCustomEntities() = default;
	~CASCustomEntities() = default;

	const BaseClassList_t& GetBaseClassList() const { return m_BaseClasses; }

	/**
	*	Initializes the custom entities system.
	*/
	bool Initialize();

	/**
	*	Shuts down the custom entities system.
	*/
	void Shutdown();

	void WorldCreated();

	void WorldActivated();

	void WorldEnded();

	/**
	*	Finds a base class by class name.
	*/
	const BaseClassData_t* FindBaseClassByClassName( const char* const pszClassName ) const;

	/**
	*	Finds a custom class by map name.
	*/
	const CCustomEntityClass* FindCustomClassByMapName( const char* const pszMapName ) const;

	/**
	*	Registers a custom entity. Registration is only allowed between MapInit and MapActivate.
	*	@param szMapName Map name of the entity. Maps will use this name to create instances.
	*	@param szClassName Angelscript class name to instantiate for this entity. Must inherit from a custom entity base class.
	*	@return true if registration succeeded, false otherwise.
	*/
	bool RegisterCustomEntity( const std::string& szClassName, const std::string& szClassObject, CustomEntityTypes_t classtype );

	/**
	*	Unregisters a custom entity class.
	*	@param szClassName Map name of the entity.
	*	@param szClassObject Angelscript class name.
	*	@return true if the class was removed, false otherwise.
	*/
	bool UnregisterCustomEntity( const std::string& szClassName, const std::string& szClassObject );

	/**
	*	Creates a custom entity. Used by scripts.
	*/
	CBaseEntity* CreateCustomEntity( const std::string& szClassname, bool &bWeapon );

private:
	void GenerateBaseClasses();

	static bool CallInitMethod( const asITypeInfo& typeInfo, void* pInstance, void* pCPPInstance, const char* const pszMethod );

private:
	BaseClassList_t m_BaseClasses;
	ClassList_t m_ClassList;

	bool m_bAllowRegistration = false;

private:
	CASCustomEntities( const CASCustomEntities& ) = delete;
	CASCustomEntities& operator=( const CASCustomEntities& ) = delete;
};

extern CASCustomEntities g_CustomEntities;

#endif //GAME_SERVER_ANGELSCRIPT_ENTITIES_CASCUSTOMENTITIES_H