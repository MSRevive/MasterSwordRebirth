//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "msdllheaders.h"
#include "CASSchedule.h"

#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#include <AngelscriptUtils/CASModule.h>
#include <AngelscriptUtils/wrapper/ASCallable.h>
#else
#include "angelscript/CASCManager.h"
#endif


CASScheduleBase *g_ASSchedule;


const char *AS_itos( int flValue )
{
	static char strVal[ 250 ];
	strVal[0] = 0;

	// Convert float to int
	int iValue = flValue;
	// Convert int to string
	snprintf( strVal, sizeof( strVal ), "%i", iValue );

	// Output
	return strVal;
}


/**PAGE
 *
 * Class used to create a task and / or schedule.
 *
 * @page Schedule
 * @category Engine
 *
 * @desc_md false
 * @type void
 *
*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::Shutdown()
{
	if ( !g_ASSchedule )
		return;

	delete g_ASSchedule;
	g_ASSchedule = nullptr;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::DestroyTasks()
{
	if ( m_ASScheduleBase.size() > 0 )
		m_ASScheduleBase.clear();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::CreateSchedule( float flDelay, const char* strFunction, int iRepeat, bool bHasItem, int iItem )
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			if ( FStrEq( schedulebase->function, strFunction ) && !bHasItem )
			{
				// Set new values (if they were changed)
				schedulebase->repeat = iRepeat;
				schedulebase->delay = flDelay;
				schedulebase->m_Callback = 0;
				schedulebase->resumetimer = 0;

				schedulebase->timer.Invalidate();
				schedulebase->timer.Start( flDelay );
				return;
			}

			// Increase
			++schedulebase;
		}
	}

	ScheduleBase schedule;
	strcpy( schedule.function, strFunction );
	schedule.resumetimer = 0;
	schedule.m_Callback = 0;
	schedule.repeat = iRepeat;
	schedule.delay = flDelay;
	schedule.item = iItem;
	schedule.HasItem = bHasItem;
	schedule.timer.Invalidate();
	schedule.timer.Start( flDelay );
	m_ASScheduleBase.push_back( schedule );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::CreateSchedule( float flDelay, asIScriptFunction* pCallback, int iRepeat, bool bHasItem, int iItem )
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			if ( schedulebase->m_Callback == pCallback && !bHasItem )
			{
				// Set new values (if they were changed)
				schedulebase->repeat = iRepeat;
				schedulebase->delay = flDelay;

				if ( schedulebase->m_Callback )
					schedulebase->m_Callback->Release();

				schedulebase->m_Callback = pCallback;
				schedulebase->resumetimer = 0;

				schedulebase->timer.Invalidate();
				schedulebase->timer.Start( flDelay );
				return;
			}

			// Increase
			++schedulebase;
		}
	}

	ScheduleBase schedule;
	strcpy( schedule.function, "" );
	schedule.resumetimer = 0;
	schedule.m_Callback = pCallback;
	schedule.repeat = iRepeat;
	schedule.delay = flDelay;
	schedule.item = iItem;
	schedule.HasItem = bHasItem;
	schedule.timer.Invalidate();
	schedule.timer.Start( flDelay );
	m_ASScheduleBase.push_back( schedule );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::Think()
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			// Check if we have started, and elapsed
			if ( schedulebase->timer.HasStarted() && schedulebase->timer.IsElapsed() )
			{
				schedulebase->timer.Invalidate();

				// Call again, if we have a repeater
				if ( schedulebase->repeat >= 1 || schedulebase->repeat == -1 )
				{
					// If not in infinite repeat, start again
					if ( schedulebase->repeat >= 1 )
						schedulebase->repeat--;

					schedulebase->timer.Start( schedulebase->delay );
				}

				if ( schedulebase->m_Callback )
					FunctionCall( schedulebase->m_Callback, schedulebase->HasItem, schedulebase->item );
				else
					FunctionCall( schedulebase->function, schedulebase->HasItem, schedulebase->item );
			}

			// Increase
			++schedulebase;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::PauseSchedule( const char *strFunction )
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			if ( FStrEq( schedulebase->function, strFunction ) )
			{
				// Check if we have started, and elapsed
				if ( schedulebase->timer.HasStarted() )
				{
					schedulebase->resumetimer = schedulebase->timer.GetRemainingTime();
					schedulebase->timer.Invalidate();
				}
			}

			// Increase
			++schedulebase;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::PauseSchedule( asIScriptFunction* pCallback )
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			if ( pCallback == schedulebase->m_Callback )
			{
				// Check if we have started, and elapsed
				if ( schedulebase->timer.HasStarted() )
				{
					schedulebase->resumetimer = schedulebase->timer.GetRemainingTime();
					schedulebase->timer.Invalidate();
				}
			}

			// Increase
			++schedulebase;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::ResumeSchedule( const char *strFunction )
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			if ( FStrEq( schedulebase->function, strFunction ) )
			{
				// Check if we have started, and elapsed
				if ( schedulebase->resumetimer > 0.0f )
				{
					schedulebase->timer.Invalidate();
					schedulebase->timer.Start( schedulebase->resumetimer );
					schedulebase->resumetimer = 0;
				}
			}

			// Increase
			++schedulebase;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::ResumeSchedule( asIScriptFunction* pCallback )
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			if ( pCallback == schedulebase->m_Callback )
			{
				// Check if we have started, and elapsed
				if ( schedulebase->resumetimer > 0.0f )
				{
					schedulebase->timer.Invalidate();
					schedulebase->timer.Start( schedulebase->resumetimer );
					schedulebase->resumetimer = 0;
				}
			}

			// Increase
			++schedulebase;
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::DestroySchedule( const char *strFunction )
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			if ( FStrEq( schedulebase->function, strFunction ) )
				m_ASScheduleBase.erase( schedulebase++ );
			else
			{
				// Increase
				++schedulebase;
			}
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::DestroySchedule( asIScriptFunction* pCallback )
{
	if ( m_ASScheduleBase.size() > 0 )
	{
		std::list< ScheduleBase >::iterator schedulebase = m_ASScheduleBase.begin();
		while ( schedulebase != m_ASScheduleBase.end() )
		{
			if ( pCallback == schedulebase->m_Callback )
			{
				if ( schedulebase->m_Callback )
					schedulebase->m_Callback->Release();
				schedulebase->timer.Invalidate();
				m_ASScheduleBase.erase( schedulebase++ );
			}
			else
			{
				// Increase
				++schedulebase;
			}
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::FunctionCall( const char* strFunction, bool bHasItem, int iItem )
{
#if !defined( ASCOMPILER )
	char strFunctionVoid[ 1024 ];
	strcpy_s( strFunctionVoid, "void " );
	strcat_s( strFunctionVoid, strFunction );
	if ( bHasItem )
	{
		strcat_s( strFunctionVoid, "(" );
		strcat_s( strFunctionVoid, AS_itos( iItem ) );
		strcat_s( strFunctionVoid, ")" );
	}
	else
		strcat_s( strFunctionVoid, "()" );

	g_ASManager.CallFunctionVoid( strFunctionVoid );
#endif
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASScheduleBase::FunctionCall( asIScriptFunction* pCallback, bool bHasItem, int iItem )
{
	auto engine = g_ASManager.GetASManager().GetEngine();
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare( pCallback );

	// Set the function arguments
	if ( bHasItem )
	{
		// Debug
		//Msg( "GetFunction{gold}( {green}%s{white});\n", ctx->GetFunction()->GetName() );
		//Msg( "SetArgDWord{gold}( {blue}0{white}, {blue}%i{white});\n", iItem );
		ctx->SetArgDWord( 0, iItem );
	}

	int r = ctx->Execute();
	if ( r == asEXECUTION_FINISHED )
	{
		// The return value is only valid if the execution finished successfully
		//asDWORD ret = ctx->GetReturnDWord();
	}

	// Release the context when you're done with it
	ctx->Release();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Schedule_Task( float flDelay, const std::string& strFunction )
{
	g_ASSchedule->CreateSchedule( flDelay, strFunction.c_str(), 0, false, 0 );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Schedule_TaskItem( float flDelay, int iItem, const std::string& strFunction )
{
	g_ASSchedule->CreateSchedule( flDelay, strFunction.c_str(), 0, true, iItem );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Schedule_TaskCallBack( float flDelay, asIScriptFunction* pCallback )
{
	g_ASSchedule->CreateSchedule( flDelay, pCallback, 0, false, 0 );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Schedule_TaskCallBackItem( float flDelay, int iItem, asIScriptFunction* pCallback )
{
	g_ASSchedule->CreateSchedule( flDelay, pCallback, 0, true, iItem );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CSchedule *ConstructSchedule( const std::string& szFunction, float flTimer, int repeat )
{
	CSchedule *pSchedule = new CSchedule( szFunction, flTimer, repeat );
	pSchedule->SendToScheduler();
	return pSchedule;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CSchedule *ConstructScheduleCallback( asIScriptFunction* pCallback, float flTimer, int repeat )
{
	CSchedule *pSchedule = new CSchedule( pCallback, flTimer, repeat );
	pSchedule->SendToScheduler();
	return pSchedule;
}


void DestroySchedule( CSchedule* pMemory )
{
	pMemory->~CSchedule();
}


CSchedule::CSchedule()
{
	strcpy_s( m_strfunction, "Function" );
	m_flTimer = 0.0f;
	m_iRepeat = 0;
	m_Callback = 0;
}


CSchedule::CSchedule( const std::string& szFunction, float flTimer, int irepeat )
{
	strcat_s( m_strfunction, szFunction.c_str() );

	// Don't let it go below -1
	if ( irepeat < -1 )
		irepeat = -1;

	m_flTimer = flTimer;
	m_iRepeat = irepeat;
}


CSchedule::CSchedule( asIScriptFunction *pCallback, float flTimer, int irepeat )
{
	strcat_s( m_strfunction, "" );

	// Don't let it go below -1
	if ( irepeat < -1 )
		irepeat = -1;

	m_flTimer = flTimer;
	m_iRepeat = irepeat;
	m_Callback = pCallback;
}


CSchedule::~CSchedule()
{
	if ( m_Callback )
		g_ASSchedule->DestroySchedule( m_Callback );
	else
		g_ASSchedule->DestroySchedule( m_strfunction );
}


void CSchedule::PauseSchedule()
{
	if ( m_Callback )
		g_ASSchedule->PauseSchedule( m_Callback );
	else
		g_ASSchedule->PauseSchedule( m_strfunction );
}


void CSchedule::ResumeSchedule()
{
	if ( m_Callback )
		g_ASSchedule->ResumeSchedule( m_Callback );
	else
		g_ASSchedule->ResumeSchedule( m_strfunction );
}


void CSchedule::SendToScheduler()
{
	if ( m_Callback )
		g_ASSchedule->CreateSchedule( m_flTimer, m_Callback, m_iRepeat, false, 0 );
	else
		g_ASSchedule->CreateSchedule( m_flTimer, m_strfunction, m_iRepeat, false, 0 );
}


static void RegisterScriptScheduler_Object( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CSchedule";

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF );

	as::RegisterRefCountedBaseClass<CSchedule>( &engine, pszObjectName );

/**JSON
 * Pauses the schedule
 *
 * @type object
 * @object CSchedule
 * @name Pause
 * @isfunc true
 * @base void
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void Pause()",
		asMETHOD( CSchedule, PauseSchedule ), asCALL_THISCALL );

/**JSON
 * Resumes the schedule
 *
 * @type object
 * @object CSchedule
 * @name Resume
 * @isfunc true
 * @base void
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void Resume()",
		asMETHOD( CSchedule, ResumeSchedule ), asCALL_THISCALL );

/**JSON
 * Forces the object to fire it's schedule
 *
 * @type object
 * @object CSchedule
 * @name DoTask
 * @isfunc true
 * @base void
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void DoTask()",
		asMETHOD( CSchedule, SendToScheduler ), asCALL_THISCALL );

}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void RegisterScriptSchedule( asIScriptEngine &engine )
{
	RegisterScriptScheduler_Object( engine );

	const std::string szOldNS = engine.GetDefaultNamespace();

	// Our callback funcs for our scheduler
	engine.RegisterFuncdef( "void CallbackFunc()" );
	engine.RegisterFuncdef( "void CallbackFuncItem(int item)" );

	engine.SetDefaultNamespace( "Schedule" );

/**MARKDOWN
The function `Schedule::Task` also accepts actual function objects

Method 1:
```cpp
void ThePresident_OnMapStart()
{
	Schedule::Task( 1.5, "SomeFunction" );
}

void SomeFunction()
{
	// ...
}
```

Method 2:
```
void ThePresident_OnMapStart()
{
	Schedule::Task( 1.5, SomeFunction );
}

void SomeFunction()
{
	// ...
}

Method 3:
```
void ThePresident_OnMapStart()
{
	// Item variable
	int item = 15;
	Schedule::Task( 1.5, item, SomeFunction );
}

void SomeFunction( int item )
{
	// ...
}
```

 * @filename Schedule_Task
 */
/**JSON
 * Fires a simple task.
 *
 * @type namespace
 * @namespace Schedule
 * @name Task
 * @base void
 * @name Schedule_Task
 * @desc_md true
 *
 * @args %float_in% flDelay # When this task should be fired (Counts in seconds)
 * @args %string_in% strFunction # The name of the void function
 *
 */
	engine.RegisterGlobalFunction(
		"void Task(float flDelay, const string& in strFunction)",
		asFUNCTION( Schedule_Task ), asCALL_CDECL );
	engine.RegisterGlobalFunction(
		"void Task(float flDelay, CallbackFunc @pCallback)",
		asFUNCTION( Schedule_TaskCallBack ), asCALL_CDECL );
	engine.RegisterGlobalFunction(
		"void Task(float flDelay, int iItem, CallbackFuncItem @pCallback)",
		asFUNCTION( Schedule_TaskCallBackItem ), asCALL_CDECL );

/**MARKDOWN
The function `Schedule::Task` also accepts actual function objects

Method 1:
```cpp
void ThePresident_OnMapStart()
{
	Schedule::Create( "SomeFunction", 1.5, 2 );
}

void SomeFunction()
{
	// ...
}
```

Method 2:
```
void ThePresident_OnMapStart()
{
	Schedule::Create( SomeFunction, 1.5, 2 );
}

void SomeFunction()
{
	// ...
}
```

 * @filename Schedule_Create
 */
/**JSON
 * Creates a object of CSchedule@, which can be manipulated.
 *
 * @type namespace
 * @namespace Schedule
 * @name Create
 * @object CSchedule
 * @name Schedule_Create
 * @desc_md true
 *
 * @args %string_in% strFunction # The name of the void function
 * @args %float_in% flDelay # When this task should be fired (Counts in seconds)
 * @args %int_in% iRepeat = 0 # How many times it should repeat (-1 means it's infinite)
 *
 * @return Returns as a CSchedule@ object, which can be manipulated.
 *
 */
	engine.RegisterGlobalFunction(
		"CSchedule@ Create(const string& in strFunction, float flDelay, int iRepeat = 0)",
		asFUNCTION( ConstructSchedule ), asCALL_CDECL );
	engine.RegisterGlobalFunction(
		"CSchedule@ Create(CallbackFunc @pCallback, float flDelay, int iRepeat = 0)",
		asFUNCTION( ConstructScheduleCallback ), asCALL_CDECL );

	engine.SetDefaultNamespace( szOldNS.c_str() );
}
