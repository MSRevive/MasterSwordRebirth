//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_ASSCHEDULER_H
#define GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_ASSCHEDULER_H

#if defined( SOURCE )
#include <stdstring.h>
#else
#include <string.h>
#endif
#include <AngelscriptUtils/util/CASBaseClass.h>

class asIScriptEngine;
class asIScriptFunction;

// GoldSource has no CountdownTimer class, so let's make sure it exists.
class CountdownTimer
{
public:
	CountdownTimer( void )
	{
		m_timestamp = -1.0f;
		m_duration = 0.0f;
	}

	void Reset( void )
	{
		m_timestamp = Now() + m_duration;
	}		

	void Start( float duration )
	{
		m_timestamp = Now() + duration;
		m_duration = duration;
	}

	void Invalidate( void )
	{
		m_timestamp = -1.0f;
	}		

	bool HasStarted( void ) const
	{
		return (m_timestamp > 0.0f);
	}

	bool IsElapsed( void ) const
	{
		return (Now() > m_timestamp);
	}

	float GetElapsedTime( void ) const
	{
		return Now() - m_timestamp + m_duration;
	}

	float GetRemainingTime( void ) const
	{
		return (m_timestamp - Now());
	}

	/// return original countdown time
	float GetCountdownDuration( void ) const
	{
		return (m_timestamp > 0.0f) ? m_duration : 0.0f;
	}

private:
	float m_duration;
	float m_timestamp;
	virtual float Now( void ) const { return 0; }
};

class CASScheduleBase
{
public:
	CASScheduleBase() = default;
	~CASScheduleBase() = default;

	// Shutdown
	void Shutdown();

	// Destroy all tasks
	void DestroyTasks();

	// Create the schedule
	void CreateSchedule( float flDelay, const char* strFunction, int iRepeat, bool bHasItem, int iItem );
	void CreateSchedule( float flDelay, asIScriptFunction* pCallback, int iRepeat, bool bHasItem, int iItem );

	// Calls the function
	void FunctionCall( const char* strFunction, bool bHasItem, int iItem );
	void FunctionCall( asIScriptFunction* pCallback, bool bHasItem, int iItem );

	// Called from CHL2ASManager.cpp, calls whenever it thinks
	void Think();

	// If our specific schedule is set to be paused
	void PauseSchedule( const char *strFunction );
	void PauseSchedule( asIScriptFunction* pCallback );

	// Resume our schedule
	void ResumeSchedule( const char *strFunction );
	void ResumeSchedule( asIScriptFunction* pCallback );

	// Destroy our schedule
	void DestroySchedule( const char *strFunction );
	void DestroySchedule( asIScriptFunction* pCallback );

	// Setup the tasks
	typedef struct ASSchedule_t
	{
		asIScriptFunction		*m_Callback;
		char					function[250];
		int						repeat;
		float					delay;
		bool					HasItem;
		int						item;

		// If paused
		float					resumetimer;

		CountdownTimer			timer;
	} ScheduleBase;

	// list of our sources
	std::list<ScheduleBase>	m_ASScheduleBase;
};

extern CASScheduleBase *g_ASSchedule;

class CSchedule final : public CASAtomicRefCountedBaseClass
{
public:
	CSchedule();
	CSchedule( const std::string& szFunction, float flTimer, int irepeat );
	CSchedule( asIScriptFunction *pCallback, float flTimer, int irepeat );
	~CSchedule();
	
	void Release() const
	{
		if ( InternalRelease() )
			delete this;
	}

	void SendToScheduler();
	void PauseSchedule();
	void ResumeSchedule();

private:
	char					m_strfunction[ 250 ];
	float					m_flTimer;
	int						m_iRepeat;
	asIScriptFunction		*m_Callback;
};

/**
*	Registers math types, constants and functions for Angelscript.
*	@param engine Script engine.
*/
void RegisterScriptSchedule( asIScriptEngine &engine );

#endif //GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_ASSCHEDULER_H
