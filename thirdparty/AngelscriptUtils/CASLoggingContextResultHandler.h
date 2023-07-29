#ifndef CASLOGGINGCONTEXTRESULTHANDLER_H
#define CASLOGGINGCONTEXTRESULTHANDLER_H

#include <cstdint>

#include "util/CASBaseClass.h"

#include "IASContextResultHandler.h"

/**
*	Context result handler that logs all errors to the default logger.
*	Suspended contexts are treated as normal behavior by default, unless the Flag::SUSPEND_IS_ERROR flag is set.
*/
class CASLoggingContextResultHandler : public IASContextResultHandler, public CASAtomicRefCountedBaseClass
{
public:
	using Flags_t = uint32_t;

	struct Flag
	{
		enum EFlag : Flags_t
		{
			NONE = 0,

			/**
			*	Suspended contexts should be treated as errors.
			*/
			SUSPEND_IS_ERROR = 1 << 0,
		};
	};

public:
	/**
	*	@param flags Handler flags.
	*/
	CASLoggingContextResultHandler( const Flags_t flags = Flag::NONE );

	void AddRef() const override
	{
		CASAtomicRefCountedBaseClass::AddRef();
	}

	void Release() const override
	{
		if( InternalRelease() )
			delete this;
	}

	void ProcessPrepareResult( asIScriptFunction& function, asIScriptContext& context, int iResult ) override;

	void ProcessExecuteResult( asIScriptFunction& function, asIScriptContext& context, int iResult ) override;

	void ProcessUnprepareResult( asIScriptContext& context, int iResult ) override;

	Flags_t GetFlags() const { return m_Flags; }

	void SetFlags( const Flags_t flags )
	{
		m_Flags = flags;
	}

private:
	void LogCurrentFunction( asIScriptContext& context, const char* const pszAction );

private:
	Flags_t m_Flags;

private:
	CASLoggingContextResultHandler( const CASLoggingContextResultHandler& ) = delete;
	CASLoggingContextResultHandler& operator=( const CASLoggingContextResultHandler& ) = delete;
};

#endif //CASLOGGINGCONTEXTRESULTHANDLER_H
