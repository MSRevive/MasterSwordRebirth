#ifndef ANGELSCRIPT_UTIL_IASEXTENDADAPTER_H
#define ANGELSCRIPT_UTIL_IASEXTENDADAPTER_H

#include "CASObjPtr.h"

/**
*	@addtogroup ASExtend
*
*	@{
*/

/**
*	Interface used to adapt C++ classes for extension in Angelscript.
*/
class IASExtendAdapter
{
public:
	virtual ~IASExtendAdapter() = 0;

	/**
	*	@return The object pointer.
	*/
	virtual const CASObjPtr& GetObject() const = 0;

	/**
	*	@copydoc GetObject() const
	*/
	virtual CASObjPtr GetObject() = 0;
};

inline IASExtendAdapter::~IASExtendAdapter()
{
}

/** @} */

#endif //ANGELSCRIPT_UTIL_IASEXTENDADAPTER_H