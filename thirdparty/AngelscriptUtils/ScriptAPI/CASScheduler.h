#ifndef ANGELSCRIPT_SCRIPTAPI_CASSCHEDULER_H
#define ANGELSCRIPT_SCRIPTAPI_CASSCHEDULER_H

#include <cassert>
#include <string>

#include <angelscript.h>

#include "AngelscriptUtils/util/CASBaseClass.h"

class CASModule;
class CASArguments;

/**
*	Schedules functions for execution at a set time.
*/
class CASScheduler final
{
public:
	static const int REPEAT_INF_TIMES = -1;
	const int REPEAT_INFINITE_TIMES = REPEAT_INF_TIMES; //For scripts

public:

	/**
	*	Represents a scheduled function. Scripts can access this and modify the settings.
	*/
	class CScheduledFunction final : public CASRefCountedBaseClass
	{
	public:
		/**
		*	Constructor.
		*	@param pFunction Function.
		*	@param flNextCallTime Time when the function should be called.
		*	@param flRepeatTime Time between calls.
		*	@param iRepeatCount Number of times to call the function, or infinite if REPEAT_INF_TIMES is given.
		*	@param pThis This pointer. Can be null.
		*	@param iTypeId This pointer type id.
		*	@param pArguments Function arguments.
		*	@param pNext Next function in the list.
		*/
		CScheduledFunction( asIScriptFunction* const pFunction,
			const float flNextCallTime, const float flRepeatTime, const int iRepeatCount, 
			void* const pThis, const int iTypeId, CASArguments* pArguments, CScheduledFunction* const pNext = nullptr )
			: CASRefCountedBaseClass()
			, m_pFunction( pFunction )
			, m_flNextCallTime( flNextCallTime )
			, m_flRepeatTime( flRepeatTime )
			, m_iRepeatCount( iRepeatCount )
			, m_pThis( pThis )
			, m_iTypeId( iTypeId )
			, m_pArguments( pArguments )
			, m_pNext( pNext )
		{
			pFunction->AddRef();
		}

		void Release() const
		{
			if( InternalRelease() )
				delete this;
		}

		/**
		*	@return The next function in the list.
		*/
		CScheduledFunction* GetNext() const { return m_pNext; }

		/**
		*	Sets the next function in the list.
		*	@param pNext Next function.
		*/
		void SetNext( CScheduledFunction* const pNext ) 
		{ 
			m_pNext = pNext;
		}

		/**
		*	@return The function to call.
		*/
		asIScriptFunction* GetFunction() const { return m_pFunction; }

		/**
		*	@return Next call time.
		*/
		float GetNextCallTime() const { return m_flNextCallTime; }

		/**
		*	Sets the next call time.
		*	@param flNextCallTime Next call time.
		*/
		void SetNextCallTime( const float flNextCallTime ) { m_flNextCallTime = flNextCallTime; }

		/**
		*	@return The time between calls.
		*/
		float GetRepeatTime() const { return m_flRepeatTime; }

		/**
		*	Sets the time between calls.
		*	@param flRepeatTime Time between calls.
		*/
		void SetRepeatTime( const float flRepeatTime )
		{
			if( flRepeatTime < 0 )
				return;

			m_flRepeatTime = flRepeatTime;
		}

		/**
		*	@return The number of times to call the function.
		*/
		int GetRepeatCount() const { return m_iRepeatCount; }

		/**
		*	@return Whether this function should be called an infinite number of times.
		*/
		bool IsInfiniteRepeat() const { return m_iRepeatCount == REPEAT_INF_TIMES; }

		/**
		*	Sets the number of times to call the function, or an infinite number of times if REPEAT_INF_TIMES is given.
		*	@param iRepeatCount Number of times to call the function.
		*/
		void SetRepeatCount( const int iRepeatCount )
		{
			//Allow 0, will cause immediate removal after call completes, or next call
			if( iRepeatCount < REPEAT_INF_TIMES )
				return;

			m_iRepeatCount = iRepeatCount;
		}

		/**
		*	Makes the function be called an infinite number of times.
		*/
		void MakeInfiniteRepeat()
		{
			m_iRepeatCount = REPEAT_INF_TIMES;
		}

		/**
		*	Internal method used to notify this object that it's been called.
		*/
		void Called()
		{
			assert( m_iRepeatCount > 0 || ( IsInfiniteRepeat() && m_iRepeatCount == REPEAT_INF_TIMES ) );

			if( !IsInfiniteRepeat() )
				--m_iRepeatCount;
		}

		/**
		*	@return Whether this object should be removed.
		*/
		bool ShouldRemove() const
		{
			return !IsInfiniteRepeat() && ( GetRepeatCount() <= 0 );
		}

		/**
		*	@return Whether this object has been removed.
		*/
		bool HasBeenRemoved() const { return m_bRemoved; }

		/**
		*	Removes all references to script data and code.
		*	@param engine Script engine.
		*/
		void Remove( asIScriptEngine& engine );

		/**
		*	@return The this pointer.
		*/
		void* GetThis() const { return m_pThis; }

		/**
		*	@return The this pointer type id.
		*/
		int GetTypeId() const { return m_iTypeId; }

		/**
		*	@return The list of arguments.
		*/
		CASArguments* GetArguments() const { return m_pArguments; }

	private:
		/**
		*	Destructor. Should never be called directly.
		*/
		~CScheduledFunction();

	private:
		asIScriptFunction*	m_pFunction;
		float				m_flNextCallTime;
		float				m_flRepeatTime;
		int					m_iRepeatCount;

		void*				m_pThis;
		const int			m_iTypeId;

		CASArguments*		m_pArguments;

		CScheduledFunction*	m_pNext;

		bool				m_bRemoved = false;

	private:
		CScheduledFunction( const CScheduledFunction& ) = delete;
		CScheduledFunction& operator=( const CScheduledFunction& ) = delete;
	};

public:
	/**
	*	Constructor.
	*	@param owningModule The module that owns this scheduler.
	*/
	CASScheduler( CASModule& owningModule );

	/**
	*	Destructor.
	*/
	~CASScheduler();

	/**
	*	@return Whether the scheduler is currently thinking (calling functions).
	*/
	bool IsThinking() const { return m_bThinking; }

	static void SetTimeoutHandler( asIScriptGeneric* pArguments );

	static void SetIntervalHandler( asIScriptGeneric* pArguments );

	static void SetInterval_NoArgs( asIScriptGeneric* pArguments );

	static void SetTimeoutObj( asIScriptGeneric* pArguments );

	static void SetIntervalObj( asIScriptGeneric* pArguments );

	static void SetIntervalObj_NoArgs( asIScriptGeneric* pArguments );

	/**
	*	Sets an interval (call function every N seconds).
	*	@param szFunctionName Name of the function to call.
	*	@param flRepeatTime Time between calls.
	*	@param iRepeatCount Number of times to call the function.
	*	@param uiStartIndex First argument to get for the function call.
	*	@param arguments Generic call instance to get function call arguments from.
	*/
	void SetInterval( const std::string& szFunctionName, float flRepeatTime, int iRepeatCount, asUINT uiStartIndex, asIScriptGeneric& arguments );

	/**
	*	Sets an interval (call object method or function every N seconds).
	*	@param pThis This pointer. Can be null, in which case it looks for global functions.
	*	@param iTypeId This pointer type id.
	*	@param szFunctionName Name of the function to call.
	*	@param flRepeatTime Time between calls.
	*	@param iRepeatCount Number of times to call the function.
	*	@param uiStartIndex First argument to get for the function call.
	*	@param arguments Generic call instance to get function call arguments from.
	*/
	void SetInterval( void* pThis, int iTypeId, const std::string& szFunctionName, float flRepeatTime, int iRepeatCount, asUINT uiStartIndex, asIScriptGeneric& arguments );

	/**
	*	Removes a scheduled function.
	*	@param pFunction Function to remove.
	*/
	void RemoveTimer( CScheduledFunction* pFunction );

	/**
	*	@return The function that is currently being executed, if any.
	*/
	CScheduledFunction* GetCurrentFunction() const;

	/**
	*	Calls functions whose next call time falls between the last think time and flCurrentTime.
	*	@param flCurrentTime Current time.
	*/
	void Think( const float flCurrentTime );

	/**
	*	Removes all scheduled functions.
	*/
	void ClearTimerList();

	/**
	*	Adjusts the next call time for all functions to be called at prevTime - flTime.
	*	@param flTime Delta time between the previous current time and the next current time.
	*/
	void AdjustTime( float flTime );

private:
	/**
	*	Removes a single function from the list.
	*	@param engine Script engine.
	*	@param pLast Last function. Can be null.
	*	@param pCurrent Function to remove.
	*/
	void RemoveFunction( asIScriptEngine& engine, CScheduledFunction* pLast, CScheduledFunction* pCurrent );

private:
	CASModule& m_OwningModule;
	float m_flLastTime = 0.0f;

	CScheduledFunction* m_pFunctionListHead = nullptr;

	/*
	*	Used to store functions scheduled in another scheduled function that was being executed. Is merged with the main list at the end of Think.
	*/
	CScheduledFunction* m_pThinkListHead = nullptr;

	/*
	*	The current function being executed, if any.
	*/
	CScheduledFunction* m_pCurrentFunction = nullptr;

	bool m_bThinking = false;

	/*
	*	Used to determine if the current function should be removed.
	*/
	bool m_bShouldRemove = false;

private:

	CASScheduler( const CASScheduler& ) = delete;
	CASScheduler& operator=( const CASScheduler& ) = delete;
};

void RegisterScriptScheduler( asIScriptEngine* pEngine );

#endif //ANGELSCRIPT_SCRIPTAPI_CASSCHEDULER_H
