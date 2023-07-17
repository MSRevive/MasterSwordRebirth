#ifndef ANGELSCRIPT_CASEVENTMANAGER_H
#define ANGELSCRIPT_CASEVENTMANAGER_H

#include <cstdint>
#include <string>
#include <vector>

#include <angelscript.h>

class asIScriptEngine;
class CASModule;
class CASEvent;

/**
*	@addtogroup ASEvents
*
*	@{
*/

/**
*	Manages a list of global events.
*	Can store a maximum of UINT32_MAX events.
*/
class CASEventManager final
{
private:
	typedef std::vector<CASEvent*> Events_t;

public:
	/**
	*	Constructor.
	*	@param engine Engine.
	*	@param pszNamespace Namespace to register events in. Can be an empty string, in which case no namespace is used.
	*/
	CASEventManager( asIScriptEngine& engine, const char* const pszNamespace = "" );

	/**
	*	Destructor.
	*/
	~CASEventManager();

	/**
	*	@return The script engine.
	*/
	asIScriptEngine& GetEngine() { return m_Engine; }

	/**
	*	@return The number of events.
	*/
	uint32_t GetEventCount() const { return static_cast<uint32_t>( m_Events.size() ); }

	/**
	*	Gets an event by index.
	*	@param uiIndex Event index. Must be smaller than GetEventCount().
	*	@return If the index is valid, the event. Otherwise, null.
	*/
	CASEvent* GetEventByIndex( const uint32_t uiIndex );

	/**
	*	Finds an event by its name. The given name must specify its category if it has one.
	*	Format: \<Category\>::\<Name\>
	*	May optionally specify Events:: as the start of the category.
	*	@return If found, the event. Otherwise, null.
	*/
	CASEvent* FindEventByName( const std::string& szName );

	/**
	*	Hooks a named event. The given name must specify its category if it has one.
	*	Format: \<Category\>::\<Name\>
	*	May optionally specify Events:: as the start of the category.
	*	@param szName Name of the event.
	*	@param pValue Function pointer.
	*	@param iTypeId Function pointer type id.
	*	@return true on success, false otherwise.
	*/
	bool HookEvent( const std::string& szName, void* pValue, const int iTypeId );

	/**
	*	Unhooks a named event. The given name must specify its category if it has one.
	*	Format: \<Category\>::\<Name\>
	*	May optionally specify Events:: as the start of the category.
	*	@param szName Name of the event.
	*	@param pValue Function pointer.
	*	@param iTypeId Function pointer type id.
	*/
	void UnhookEvent( const std::string& szName, void* pValue, const int iTypeId );

	/**
	*	Adds an event.
	*	@param pEvent Event to add.
	*	@return true if the event was added, false otherwise.
	*/
	bool AddEvent( CASEvent* pEvent );

	/**
	*	Registers this class instance and all events.
	*/
	void RegisterEvents( asIScriptEngine& engine );

	/**
	*	Unhooks all functions that are part of the given module.
	*	@param pModule Module.
	*/
	void UnhookModuleFunctions( CASModule* pModule );

	/**
	*	Unhooks all functions.
	*/
	void UnhookAllFunctions();

	/**
	*	Dumps all hooked functions to stdout.
	*/
	void DumpHookedFunctions() const;

private:
	asIScriptEngine& m_Engine;

	std::string m_szNamespace;

	Events_t m_Events;

private:
	CASEventManager( const CASEventManager& ) = delete;
	CASEventManager& operator=( const CASEventManager& ) = delete;
};

/**
*	Registers the event API.
*	@param engine Script engine.
*/
void RegisterScriptEventAPI( asIScriptEngine& engine );

/** @} */

#endif //ANGELSCRIPT_CASEVENTMANAGER_H
