#ifndef ANGELSCRIPT_UTIL_ASLOGGING_H
#define ANGELSCRIPT_UTIL_ASLOGGING_H

#include <memory>

#include <spdlog/logger.h>

#ifdef WIN32
#undef VOID
#undef GetObject
#endif

/**
*	@file
*	Provides the log global
*/

namespace as
{
/**
*	@brief The logger used by AngelscriptUtils. Expected to never be null
*	
*	@details The user is responsible for setting this and freeing it on program shutdown
*	If not provided, CASManager will create a default logger that outputs to stdout
*/
extern std::shared_ptr<spdlog::logger> log;
}

#endif //ANGELSCRIPT_UTIL_ASLOGGING_H
