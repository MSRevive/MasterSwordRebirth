#ifndef ANGELSCRIPT_CASMODULEMANAGER_H
#define ANGELSCRIPT_CASMODULEMANAGER_H

#include <memory>
#include <utility>
#include <unordered_map>
#include <vector>

#include <angelscript.h>

#include "util/StringUtils.h"

#include "CASModuleDescriptor.h"

class CASEventManager;
class CASModule;
class IASModuleBuilder;
class IASModuleUserData;

/**
*	@addtogroup ASModule
*
*	@{
*/

/**
*	Manages a list of module descriptors and modules.
*/
class CASModuleManager final
{
private:
	typedef std::unordered_map<const char*, std::unique_ptr<CASModuleDescriptor>, as::Hash_C_String<const char*>, as::EqualTo_C_String<const char*>> Descriptors_t;
	typedef std::vector<CASModule*> Modules_t;

public:
	/**
	*	Constructor.
	*	@param engine Script engine.
	*	@param eventManager Optional. The event manager that manages the global events that modules use.
	*/
	CASModuleManager( asIScriptEngine& engine, const std::shared_ptr<CASEventManager>& eventManager = nullptr );

	/**
	*	Destructor.
	*/
	~CASModuleManager();

	/**
	*	@return The script engine.
	*/
	asIScriptEngine& GetEngine() { return m_Engine; }

	/**
	*	@return The event manager, if this manager has one.
	*/
	CASEventManager* GetEventManager() { return m_EventManager.get(); }

	/**
	*	Finds a descriptor by name.
	*	@param pszName Name of the descriptor. Case sensitive.
	*	@return Descriptor, or null if it couldn't be found.
	*/
	const CASModuleDescriptor* FindDescriptorByName( const char* const pszName ) const;

	/**
	*	Adds a new descriptor.
	*	@param pszName Name of the descriptor. Must be unique.
	*	@param accessMask Access mask.
	*	@param priority Priority.
	*	@return Pair containing the descriptor and whether the descriptor was added in this call. If the descriptor already existed, first contains that descriptor and second is set to false.
	*	@see CASModuleDescriptor
	*/
	std::pair<const CASModuleDescriptor*, bool> AddDescriptor( const char* const pszName, const asDWORD accessMask, const as::ModulePriority_t priority = as::ModulePriority::NORMAL );

	/**
	*	Builds a module using the given descriptor.
	*	@param descriptor Descriptor to use.
	*	@param pszModuleName Name of the module. Must be unique.
	*	@param builder Builder to use.
	*	@param pUserData Optional. User data to associate with the module. Will be released if the module failed to build.
	*	@return On successful build, the module. Otherwise, null.
	*/
	CASModule* BuildModule( const CASModuleDescriptor& descriptor, const char* const pszModuleName, IASModuleBuilder& builder, IASModuleUserData* pUserData = nullptr );

	/**
	*	Builds a module using the given descriptor.
	*	@param pszName Name of the descriptor to use.
	*	@param pszModuleName Name of the module. Must be unique.
	*	@param builder Builder to use.
	*	@param pUserData Optional. User data to associate with the module. Will be released if the module failed to build.
	*	@return On successful build, the module. Otherwise, null.
	*/
	CASModule* BuildModule( const char* const pszName, const char* const pszModuleName, IASModuleBuilder& builder, IASModuleUserData* pUserData = nullptr );

private:
	/**
	*	Builds a module using the given descriptor.
	*	@param descriptor Descriptor to use.
	*	@param pszModuleName Name of the module. Must be unique.
	*	@param builder Builder to use.
	*	@param pUserData Optional. User data to associate with the module. Will be released if the module failed to build.
	*	@return On successful build, the module. Otherwise, null.
	*/
	CASModule* BuildModuleInternal( const CASModuleDescriptor& descriptor, const char* const pszModuleName, IASModuleBuilder& builder, IASModuleUserData* pUserData = nullptr );

public:
	/**
	*	@return The number of modules that are currently loaded.
	*/
	size_t GetModuleCount() const;

	/**
	*	Finds a module by name.
	*	@param pszModuleName Name of the module. Case sensitive.
	*	@return Module, or null if the module couldn't be found.
	*/
	const CASModule* FindModuleByName( const char* const pszModuleName ) const;

	/**
	*	@copydoc FindModuleByName( const char* const pszModuleName ) const
	*/
	CASModule* FindModuleByName( const char* const pszModuleName );

	/**
	*	Finds a module by index.
	*	@param uiIndex Index of the module.
	*	@return Module, or null if the module couldn't be found.
	*/
	const CASModule* FindModuleByIndex( const size_t uiIndex ) const;

	/**
	*	@copydoc FindModuleByIndex( const size_t uiIndex ) const
	*/
	CASModule* FindModuleByIndex( const size_t uiIndex );

private:
	/**
	*	Adds a module.
	*	@param pModule Module to add.
	*	@return true if the module was added, false otherwise.
	*/
	bool AddModule( CASModule* pModule );

public:
	/**
	*	Removes a module. The module may not be destroyed immediately if there are active references held to it.
	*	@param pModule Module to remove.
	*/
	void RemoveModule( CASModule* pModule );

	/**
	*	@see RemoveModule( CASModule* pModule )
	*	@param pszModuleName Name of the module. Case sensitive.
	*/
	void RemoveModule( const char* const pszModuleName );

	/**
	*	Removes all modules and descriptors.
	*/
	void Clear();

private:
	asIScriptEngine& m_Engine;

	std::shared_ptr<CASEventManager> m_EventManager;

	Descriptors_t m_Descriptors;

	as::DescriptorID_t m_NextDescriptorID = as::FIRST_DESCRIPTOR_ID;

	Modules_t m_Modules;

private:
	CASModuleManager( const CASModuleManager& ) = delete;
	CASModuleManager& operator=( const CASModuleManager& ) = delete;
};

/** @} */

#endif //ANGELSCRIPT_CASMODULEMANAGER_H