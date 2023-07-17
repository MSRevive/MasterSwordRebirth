#ifndef ANGELSCRIPT_UTIL_CASOBJPTR_H
#define ANGELSCRIPT_UTIL_CASOBJPTR_H

#include <cassert>
#include <utility>

#include <angelscript.h>

#include "CASRefPtr.h"

/**
*	@addtogroup ASUtil
*
*	@{
*/

/**
*	Smart pointer for script objects.
*/
class CASObjPtr final
{
public:
	/**
	*	Default constructor.
	*/
	CASObjPtr() = default;

	/**
	*	Initializing constructor.
	*	@param pThis This pointer. Can be null.
	*	@param typeInfo Type info. Must be valid if pThis is not null.
	*	@param bTransferOwnership Whether to transfer ownership or not.
	*	@see as::SetObjPointer
	*/
	CASObjPtr( void* pThis, CASRefPtr<asITypeInfo> typeInfo, const bool bTransferOwnership = false );

	/**
	*	Copy constructor.
	*/
	CASObjPtr( const CASObjPtr& other );

	/**
	*	Assignment operator.
	*/
	CASObjPtr& operator=( const CASObjPtr& other );

	/**
	*	Move constructor.
	*/
	CASObjPtr( CASObjPtr&& other );

	/**
	*	Move assignment operator.
	*/
	CASObjPtr& operator=( CASObjPtr&& other );

	/**
	*	Destructor.
	*/
	~CASObjPtr();

	/**
	*	@return Whether this points to an object or not.
	*/
	operator bool() const;

	/**
	*	@return Pointer to the object. The reference count is not incremented.
	*/
	const void* Get() const;

	/**
	*	@copydoc Get() const
	*/
	void* Get();

	/**
	*	@return Type info.
	*/
	const CASRefPtr<asITypeInfo>& GetTypeInfo() const;

	/**
	*	@copydoc GetTypeInfo() const
	*/
	CASRefPtr<asITypeInfo> GetTypeInfo();

	/**
	*	Releases the reference held by this pointer. The reference count is unchanged.
	*/
	void* ReleaseOwnership();

	/**
	*	Releases the reference held by this pointer. The reference count is decremented.
	*/
	void Reset();

	/**
	*	Sets the object and type info.
	*	@param pThis This pointer. Can be null.
	*	@param typeInfo Type info. Must be valid if pThis is not null.
	*	@param bTransferOwnership Whether to transfer ownership or not.
	*	@see as::SetObjPointer
	*/
	void Set( void* pThis, CASRefPtr<asITypeInfo> typeInfo, const bool bTransferOwnership = false );

	/**
	*	Operator dereference.
	*/
	const void* operator*() const
	{
		return m_pThis;
	}

	/**
	*	@copydoc operator*() const
	*/
	void* operator*()
	{
		return m_pThis;
	}

	/**
	*	Operator const void*.
	*/
	operator const void*() const
	{
		return m_pThis;
	}

	/**
	*	Operator void*.
	*/
	operator void*()
	{
		return m_pThis;
	}

private:
	void* m_pThis = nullptr;
	CASRefPtr<asITypeInfo> m_TypeInfo;
};

inline CASObjPtr::CASObjPtr( void* pThis, CASRefPtr<asITypeInfo> typeInfo, const bool bTransferOwnership )
{
	Set( pThis, typeInfo, bTransferOwnership );
}

inline CASObjPtr::CASObjPtr( const CASObjPtr& other )
	: m_pThis( nullptr )
	, m_TypeInfo( other.m_TypeInfo )
{
	//The this pointer can't be valid without typeinfo being valid as well.
	assert( !other.m_pThis || m_TypeInfo );

	if( other.m_pThis )
	{
		as::SetObjPointer( m_pThis, other.m_pThis, *m_TypeInfo );
	}
}

inline CASObjPtr& CASObjPtr::operator=( const CASObjPtr& other )
{
	if( this != &other )
	{
		Reset();

		if( other.m_pThis )
		{
			m_TypeInfo = other.m_TypeInfo;

			as::SetObjPointer( m_pThis, other.m_pThis, *m_TypeInfo );
		}
	}

	return *this;
}

inline CASObjPtr::CASObjPtr( CASObjPtr&& other )
	: m_pThis( other.m_pThis )
	, m_TypeInfo( std::move( other.m_TypeInfo ) )
{
	other.m_pThis = nullptr;
}

inline CASObjPtr& CASObjPtr::operator=( CASObjPtr&& other )
{
	if( this != &other )
	{
		Reset();
		std::swap( m_pThis, other.m_pThis );
		std::swap( m_TypeInfo, other.m_TypeInfo );
	}

	return *this;
}

inline CASObjPtr::~CASObjPtr()
{
	Reset();
}

inline CASObjPtr::operator bool() const
{
	return m_pThis != nullptr;
}

inline const void* CASObjPtr::Get() const
{
	return m_pThis;
}

inline void* CASObjPtr::Get()
{
	return m_pThis;
}

inline const CASRefPtr<asITypeInfo>& CASObjPtr::GetTypeInfo() const
{
	return m_TypeInfo;
}

inline CASRefPtr<asITypeInfo> CASObjPtr::GetTypeInfo()
{
	return m_TypeInfo;
}

inline void* CASObjPtr::ReleaseOwnership()
{
	auto pThis = m_pThis;

	m_pThis = nullptr;

	return pThis;
}

inline void CASObjPtr::Reset()
{
	//The this pointer can't be valid without typeinfo being valid as well.
	assert( !m_pThis || m_TypeInfo );

	if( m_pThis )
	{
		as::SetObjPointer( m_pThis, nullptr, *m_TypeInfo );

		m_TypeInfo = nullptr;
	}
}

inline void CASObjPtr::Set( void* pThis, CASRefPtr<asITypeInfo> typeInfo, const bool bTransferOwnership )
{
	Reset();

	//TODO: should the type info pointer be stored even if this is null?

	if( pThis )
	{
		if( !typeInfo )
		{
			assert( false );
		}
		else
		{
			m_TypeInfo = std::move( typeInfo );
			as::SetObjPointer( m_pThis, pThis, *m_TypeInfo, bTransferOwnership );
		}
	}
}

/** @} */

#endif //ANGELSCRIPT_UTIL_CASOBJPTR_H