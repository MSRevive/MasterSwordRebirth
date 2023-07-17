#ifndef ANGELSCRIPT_WRAPPER_ASCALLABLECONST_H
#define ANGELSCRIPT_WRAPPER_ASCALLABLECONST_H

#include <cstdint>

/**
*	@addtogroup ASCallable
*
*	@{
*/

typedef uint32_t CallFlags_t;

namespace CallFlag
{
/**
*	Flags to affect function calls.
*/
enum CallFlag : CallFlags_t
{
	/**
	*	No flags.
	*/
	NONE = 0,
};
}

/** @} */

#endif //ANGELSCRIPT_WRAPPER_ASCALLABLECONST_H
