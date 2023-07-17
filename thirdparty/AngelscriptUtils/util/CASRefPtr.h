#ifndef ANGELSCRIPT_UTIL_CASREFPTR_H
#define ANGELSCRIPT_UTIL_CASREFPTR_H

#include <angelscript.h>

#include "ASUtil.h"

/**
*	@addtogroup ASUtil
*
*	@{
*/

/**
*	Smart pointer for Angelscript objects.
*	@tparam T Object type.
*	@tparam ADAPTER Adapter that implements static AddRef and Release functions for the given object type.
*/
template<typename T, typename ADAPTER = as::CASReferenceAdapter<T>>
class CASRefPtr final
{
public:
	typedef T Type_t;
	typedef ADAPTER Adapter_t;

public:
	/**
	*	Sets the given pointer to the given object. Uses this class's template parameters.
	*	@see as::SetRefPointer
	*/
	static T* Set( T*& pPtr, T* const pSourcePtr, const bool bTransferOwnership = false );

	/**
	*	Default constructor.
	*/
	CASRefPtr();

	/**
	*	Initializing constructor.
	*	@param pPointer Object to set. Can be null.
	*	@param bTransferOwnership Whether to transfer ownership or not.
	*	@see as::SetRefPointer
	*/
	CASRefPtr( T* pPointer, const bool bTransferOwnership = false );

	/**
	*	Assignment operator.
	*/
	CASRefPtr& operator=( T* const pPtr );

	/**
	*	Copy constructor.
	*/
	CASRefPtr( const CASRefPtr& other );

	/**
	*	Assignment operator.
	*/
	CASRefPtr& operator=( const CASRefPtr& other );

	/**
	*	Move constructor.
	*/
	CASRefPtr( CASRefPtr&& other );

	/**
	*	Move assignment operator.
	*/
	CASRefPtr& operator=( CASRefPtr&& other );

	/**
	*	Destructor.
	*/
	~CASRefPtr();

	/**
	*	@return Whether this points to an object or not.
	*/
	operator bool() const;

	/**
	*	@return Pointer to the object. The reference count is not incremented.
	*/
	const T* Get() const;

	/**
	*	@copydoc Get() const
	*/
	T* Get();

	/**
	*	Releases the reference held by this pointer. The reference count is unchanged.
	*/
	T* ReleaseOwnership();

	/**
	*	Releases the reference held by this pointer. The reference count is decremented.
	*/
	void Reset();

	/**
	*	Sets the pointer to the given object. If this pointer is currently holding a reference to another object, that object is released.
	*	@param pSourcePtr Object to point to. Can be null.
	*	@param bTransferOwnership Whether to transfer ownership or not.
	*	@return Pointer to the object.
	*	@see as::SetRefPointer
	*/
	T* Set( T* const pSourcePtr, const bool bTransferOwnership = false );

	/**
	*	Operator dereference.
	*/
	const T& operator*() const
	{
		return *m_pPointer;
	}

	/**
	*	@copydoc operator*() const
	*/
	T& operator*()
	{
		return *m_pPointer;
	}

	/**
	*	Operator class member.
	*/
	const T* operator->() const
	{
		return m_pPointer;
	}

	/**
	*	@copydoc operator->() const
	*/
	T* operator->()
	{
		return m_pPointer;
	}

	/**
	*	Operator const T*.
	*/
	operator const T*() const
	{
		return m_pPointer;
	}

	/**
	*	Operator T*.
	*/
	operator T*()
	{
		return m_pPointer;
	}

private:
	T* m_pPointer = nullptr;
};

template<typename T, typename ADAPTER>
T* CASRefPtr<T, ADAPTER>::Set( T*& pPtr, T* const pSourcePtr, const bool bTransferOwnership )
{
	return as::SetRefPointer<T, ADAPTER>( pPtr, pSourcePtr, bTransferOwnership );
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>::CASRefPtr()
{
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>::CASRefPtr( T* pPointer, const bool bTransferOwnership )
{
	Set( m_pPointer, pPointer, bTransferOwnership );
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>& CASRefPtr<T, ADAPTER>::operator=( T* const pPtr )
{
	if( m_pPointer != pPtr )
	{
		Set( m_pPointer, pPtr, false );
	}

	return *this;
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>::CASRefPtr( const CASRefPtr<T, ADAPTER>& other )
{
	Set( m_pPointer, other.m_pPointer, false );
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>& CASRefPtr<T, ADAPTER>::operator=( const CASRefPtr<T, ADAPTER>& other )
{
	if( this != &other )
	{
		Set( m_pPointer, other.m_pPointer, false );
	}

	return *this;
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>::CASRefPtr( CASRefPtr<T, ADAPTER>&& other )
	: m_pPointer( other.m_pPointer )
{
	other.m_pPointer = nullptr;
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>& CASRefPtr<T, ADAPTER>::operator=( CASRefPtr<T, ADAPTER>&& other )
{
	if( this != &other )
	{
		Set( m_pPointer, other.m_pPointer, true );
		other.m_pPointer = nullptr;
	}

	return *this;
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>::~CASRefPtr()
{
	Reset();
}

template<typename T, typename ADAPTER>
CASRefPtr<T, ADAPTER>::operator bool() const
{
	return m_pPointer != nullptr;
}

template<typename T, typename ADAPTER>
const T* CASRefPtr<T, ADAPTER>::Get() const
{
	return m_pPointer;
}

template<typename T, typename ADAPTER>
T* CASRefPtr<T, ADAPTER>::Get()
{
	return m_pPointer;
}

template<typename T, typename ADAPTER>
T* CASRefPtr<T, ADAPTER>::ReleaseOwnership()
{
	if( !m_pPointer )
		return nullptr;

	auto pPointer = m_pPointer;

	m_pPointer = nullptr;

	return pPointer;
}

template<typename T, typename ADAPTER>
void CASRefPtr<T, ADAPTER>::Reset()
{
	Set( m_pPointer, nullptr, false );
}

template<typename T, typename ADAPTER>
T* CASRefPtr<T, ADAPTER>::Set( T* const pSourcePtr, const bool bTransferOwnership )
{
	return Set( m_pPointer, pSourcePtr, bTransferOwnership );
}

/** @} */

#endif //ANGELSCRIPT_UTIL_CASREFPTR_H