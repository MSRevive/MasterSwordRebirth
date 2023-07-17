#ifndef ANGELSCRIPT_CASEVENT_H
#define ANGELSCRIPT_CASEVENT_H

#include <angelscript.h>

#include "CASBaseEvent.h"

/**
*	@addtogroup ASEvents
*
*	@{
*/

/**
*	Stop modes for events. Allows you to specify whether events should continue executing after a function has handled it.
*/
enum class EventStopMode
{
	/**
	*	Call all functions no matter what.
	*/
	CALL_ALL,

	/**
	*	If any function in a module has handled the event, stop after executing the last function in that module.
	*/
	MODULE_HANDLED,

	/**
	*	Stop as soon as a function has handled it.
	*/
	ON_HANDLED
};

/**
*	Represents an event that script functions can hook into.
*/
class CASEvent final : public CASBaseEvent
{
public:
	/**
	*	Constructor.
	*	@param pszName Name of this event.
	*	@param pszArguments The arguments passed to hooked functions. This is a comma delimited list of argument types, e.g. "const string& in, bool& out".
	*	@param pszCategory Which category this hook is in. This is a double colon delimited list, e.g. "Game::Player".
	*	@param accessMask Access mask. Which module types this hook is available to.
	*	@param stopMode Stop mode.
	*	@see EventStopMode
	*/
	CASEvent( const char* const pszName, const char* pszArguments, const char* const pszCategory, const asDWORD accessMask, const EventStopMode stopMode );

	/**
	*	@return Hook name.
	*/
	const char* GetName() const { return m_pszName; }

	/**
	*	@return Hook arguments.
	*/
	const char* GetArguments() const { return m_pszArguments; }

	/**
	*	@return Hook category.
	*/
	const char* GetCategory() const { return m_pszCategory; }

	/**
	*	@return Stop mode.
	*/
	EventStopMode GetStopMode() const { return m_StopMode; }

	/**
	*	Dumps all hooked functions to stdout.
	*/
	void DumpHookedFunctions() const;

private:
	const char* const m_pszName;
	const char* const m_pszArguments;
	const char* const m_pszCategory;

	const EventStopMode m_StopMode;

private:
	CASEvent( const CASEvent& ) = delete;
	CASEvent& operator=( const CASEvent& ) = delete;
};
/**
*	Registers the CASEvent class.
*	@param engine Script engine.
*/
void RegisterScriptCEvent( asIScriptEngine& engine );

/** @} */

#endif //ANGELSCRIPT_CASEVENT_H
