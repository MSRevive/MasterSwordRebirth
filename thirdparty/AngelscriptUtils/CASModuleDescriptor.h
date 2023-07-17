#ifndef ANGELSCRIPT_CASMODULEDESCRIPTOR_H
#define ANGELSCRIPT_CASMODULEDESCRIPTOR_H

#include <cstdint>
#include <functional>

#include <angelscript.h>

/**
*	@addtogroup ASModule
*
*	@{
*/

namespace as
{
typedef int32_t ModulePriority_t;

namespace ModulePriority
{
/**
*	Predefined priority levels for modules.
*/
enum ModulePriority : ModulePriority_t
{
	NORMAL	= 0,
	HIGH	= INT32_MAX / 2,
	HIGHEST = INT32_MAX,
	LOW		= INT32_MIN / 2,
	LOWEST	= INT32_MIN
};
}

typedef uint32_t DescriptorID_t;

const DescriptorID_t INVALID_DESCRIPTOR_ID = 0;

const DescriptorID_t FIRST_DESCRIPTOR_ID = 1;

const DescriptorID_t LAST_DESCRIPTOR_ID = UINT32_MAX;
}

/**
*	Describes a module. This includes its name, access mask, and priority in event/function call execution.
*/
class CASModuleDescriptor final
{
public:
	/**
	*	Constructor.
	*	@param pszName Name of the module descriptor.
	*	@param accessMask Access mask.
	*	@param priority Event/function call execution priority.
	*	@param descriptorID ID assigned to this descriptor.
	*/
	CASModuleDescriptor( const char* const pszName, const asDWORD accessMask, const as::ModulePriority_t priority, const as::DescriptorID_t descriptorID );

	const char* GetName() const { return m_pszName; }

	asDWORD GetAccessMask() const { return m_AccessMask; }

	as::ModulePriority_t GetPriority() const { return m_Priority; }

	as::DescriptorID_t GetDescriptorID() const { return m_DescriptorID; }

private:
	const char* const m_pszName;

	const asDWORD m_AccessMask;

	const as::ModulePriority_t m_Priority;

	const as::DescriptorID_t m_DescriptorID;

private:
	CASModuleDescriptor( const CASModuleDescriptor& ) = delete;
	CASModuleDescriptor& operator=( const CASModuleDescriptor& ) = delete;
};

/**
*	Equality operator for descriptors.
*	@param lhs Left hand descriptor.
*	@param rhs Right hand descriptor.
*	@return true if they are equal, false otherwise.
*/
inline bool operator==( const CASModuleDescriptor& lhs, const CASModuleDescriptor& rhs )
{
	//Since each descriptor has a unique name, they can only be equal if they are the same object.
	return &lhs == &rhs;
}

/**
*	Inequality operator for descriptors.
*	@param lhs Left hand descriptor.
*	@param rhs Right hand descriptor.
*	@return true if they are not equal, false otherwise.
*/
inline bool operator!=( const CASModuleDescriptor& lhs, const CASModuleDescriptor& rhs )
{
	return !( lhs == rhs );
}

/**
*	Operator less than for descriptors.
*	@param lhs Left hand descriptor.
*	@param rhs Right hand descriptor.
*	@return true if the left hand descriptor is smaller than the right hand descriptor.
*/
inline bool operator<( const CASModuleDescriptor& lhs, const CASModuleDescriptor& rhs )
{
	//Both are invalid; lhs is smaller.
	if( lhs.GetDescriptorID() == as::INVALID_DESCRIPTOR_ID  && rhs.GetDescriptorID() == as::INVALID_DESCRIPTOR_ID )
		return true;

	//lhs is invalid, always greater.
	if( lhs.GetDescriptorID() == as::INVALID_DESCRIPTOR_ID && rhs.GetDescriptorID() != as::INVALID_DESCRIPTOR_ID )
		return false;

	//rhs is invalid, always smaller.
	if( lhs.GetDescriptorID() != as::INVALID_DESCRIPTOR_ID && rhs.GetDescriptorID() == as::INVALID_DESCRIPTOR_ID )
		return true;

	//Higher priority, smaller.
	if( lhs.GetPriority() > rhs.GetPriority() )
		return true;

	//Lower priority, greater.
	if( lhs.GetPriority() < rhs.GetPriority() )
		return false;

	//Same priority, smaller module ID is smaller.
	return lhs.GetDescriptorID() < rhs.GetDescriptorID();
}

namespace std
{
template<>
struct less<CASModuleDescriptor>
{
	typedef CASModuleDescriptor first_argument_type;
	typedef CASModuleDescriptor second_argument_type;
	typedef bool result_type;

	bool operator()( const CASModuleDescriptor& lhs, const CASModuleDescriptor& rhs ) const
	{
		return lhs < rhs;
	}
};
}

/** @} */

#endif //ANGELSCRIPT_CASMODULEDESCRIPTOR_H