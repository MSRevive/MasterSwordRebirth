#include "util/ASUtil.h"

#include "CASLoggingContextResultHandler.h"

CASLoggingContextResultHandler::CASLoggingContextResultHandler( const Flags_t flags )
{
	SetFlags( flags );
}

void CASLoggingContextResultHandler::ProcessPrepareResult( asIScriptFunction&, asIScriptContext&, int iResult )
{
	if( iResult < 0 )
	{
		switch( iResult )
		{
		case asCONTEXT_ACTIVE:
			{
				as::log->error( "Tried to prepare context that is currently executing or suspended" );
				break;
			}

			//This can't happen without throwing an exception when passing the function into as::Call since it dereferences the pointer.
		case asNO_FUNCTION:
			{
				as::log->error( "Tried to prepare null function" );
				break;
			}

		case asINVALID_ARG:
			{
				as::log->error( "Tried to prepare context with function from different engine" );
				break;
			}

		case asOUT_OF_MEMORY:
			{
				as::log->error( "Ran out of memory while preparing context for execution" );
				break;
			}

		default:
			{
				as::log->error( "Unknown error \"{}\" occurred while preparing context for execution", iResult );
				break;
			}
		}
	}
}

void CASLoggingContextResultHandler::ProcessExecuteResult( asIScriptFunction& function, asIScriptContext& context, int iResult )
{
	if( iResult != asEXECUTION_FINISHED )
	{
		const auto szFunctionName = as::FormatFunctionName( function );

		switch( iResult )
		{
		case asEXECUTION_SUSPENDED:
			{
				//Suspended execution is an error if the user wants it to be.
				if( m_Flags & Flag::SUSPEND_IS_ERROR )
				{
					as::log->error( "Script execution unexpectedly suspended while executing function \"{}\"", szFunctionName );

					LogCurrentFunction( context, "Suspended" );
				}

				break;
			}

		case asCONTEXT_NOT_PREPARED:
			{
				as::log->error( "Context not prepared to execute function \"{}\"", szFunctionName );
				break;
			}

		case asEXECUTION_ABORTED:
			{
				as::log->error( "Script execution aborted while executing function \"{}\"", szFunctionName );

				LogCurrentFunction( context, "Aborted" );

				break;
			}

		case asEXECUTION_EXCEPTION:
			{
				const auto szExceptionFunction = as::FormatFunctionName( *context.GetExceptionFunction() );

				int iColumn = 0;
				const char* pszSection = nullptr;

				const int iLineNumber = context.GetExceptionLineNumber( &iColumn, &pszSection );

				as::log->warn(
					"Exception occurred while executing function \"{}\"\nFunction \"{}\" at line {}, column {} in section \"{}\":\n{}",
					szFunctionName, szExceptionFunction, iLineNumber, iColumn, pszSection, context.GetExceptionString()
				);

				break;
			}

			//Unexpected error states.
		case asEXECUTION_FINISHED:
		case asEXECUTION_PREPARED:
		case asEXECUTION_UNINITIALIZED:
		case asEXECUTION_ACTIVE:
		case asEXECUTION_ERROR:
			{
				as::log->error( "Unexpected context state \"{}\" encountered while executing function \"{}\"", iResult, szFunctionName );
				break;
			}

			//Unknown error.
		default:
		case asERROR:
			{
				as::log->error( "Unknown error \"{}\" occurred while executing function \"{}\"\n", iResult, szFunctionName );
				break;
			}
		}
	}
}

void CASLoggingContextResultHandler::ProcessUnprepareResult( asIScriptContext&, int iResult )
{
	if( iResult < 0 )
	{
		switch( iResult )
		{
			//This can happen if a longjmp occurs while executing a script. The context state is corrupted after such a call.
		case asCONTEXT_ACTIVE:
			{
				as::log->error( "Tried to unprepare context that is still active" );
				break;
			}

		default:
			{
				as::log->error( "Unknown error \"{}\" occurred while unpreparing", iResult );
				break;
			}
		}
	}
}

void CASLoggingContextResultHandler::LogCurrentFunction( asIScriptContext& context, const char* const pszAction )
{
	assert( pszAction );
	assert( *pszAction );

	as::CASCallerInfo info;

	as::GetCallerInfo( info, &context );

	auto pCurrentFunc = context.GetFunction( context.GetCallstackSize() - 1 );

	if( pCurrentFunc )
	{
		const auto szFunctionName = as::FormatFunctionName( *pCurrentFunc );

		as::log->error( "{} while in function \"{}\" at line {}, column {} in section \"{}\"", pszAction, szFunctionName, info.iLine, info.iColumn, info.pszSection );
	}
	else
	{
		as::log->error( "{} in unknown function", pszAction );
	}
}
