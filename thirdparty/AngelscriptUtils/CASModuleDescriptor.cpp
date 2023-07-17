#include <cassert>

#include "CASModuleDescriptor.h"

CASModuleDescriptor::CASModuleDescriptor( const char* const pszName, const asDWORD accessMask, const as::ModulePriority_t priority, const as::DescriptorID_t descriptorID )
	: m_pszName( pszName )
	, m_AccessMask( accessMask )
	, m_Priority( priority )
	, m_DescriptorID( descriptorID )
{
	assert( pszName );
	assert( pszName && *pszName );
	//A module with no access to anything is rather useless.
	assert( accessMask != 0 );
	assert( descriptorID != as::INVALID_DESCRIPTOR_ID );
}