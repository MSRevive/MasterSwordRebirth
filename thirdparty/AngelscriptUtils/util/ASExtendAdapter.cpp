#include <cassert>
#include <sstream>

#include "AngelscriptUtils/wrapper/ASCallable.h"

#include "IASExtendAdapter.h"

#include "ASExtendAdapter.h"

namespace as
{
std::string CreateExtendBaseclassDeclaration(
	const char* const pszClassName, const char* const pszBaseClassName,
	const char* const pszCPPClassName, const char* const pszCPPBaseClassName,
	const char* const pszClassContents )
{
	assert( pszClassName );
	assert( pszBaseClassName );
	assert( pszCPPClassName );

	std::stringstream stream;

	stream << "class " << pszClassName << " : " << pszBaseClassName << std::endl 
		<< '{' << std::endl;

	//Provide the self member.
	stream << "\tprivate " << pszCPPClassName << "@ m_pSelf;" << std::endl;

	stream << "\tprivate void SetSelf( " << pszCPPClassName << "@ self )" << std::endl
		<< "\t{" << std::endl
		<< "\t\t@this.m_pSelf = self;" << std::endl
		<< "\t}" << std::endl
		<< "\t" << pszCPPClassName << "@ get_self() const { return m_pSelf; }" << std::endl;

	//If the user has provided a baseclass type, provide the BaseClass member.
	if( pszCPPBaseClassName )
	{
		stream << "\tprivate " << pszCPPBaseClassName << "@ m_pBaseClass;" << std::endl;

		stream << "\tprivate void SetBaseClass( " << pszCPPBaseClassName << "@ BaseClass )" << std::endl
			<< "\t{" << std::endl
			<< "\t\t@this.m_pBaseClass = BaseClass;" << std::endl
			<< "\t}" << std::endl 
			<< "\t" << pszCPPBaseClassName << "@ get_BaseClass() const { return m_pBaseClass; }" << std::endl;
	}

	if( pszClassContents )
	{
		//Output it so it's aligned with the rest of the class contents.
		std::stringstream contents( pszClassContents );

		std::string szLine;

		while( std::getline( contents, szLine ) )
		{
			stream << '\t' << szLine << std::endl;
		}
	}

	stream << '}' << std::endl;

	return stream.str();
}

bool InitializeExtendClass( IASExtendAdapter& adapter, void* const pThis, const char* const pszCPPClassName, const char* const pszCPPBaseClassName )
{
	assert( pThis );
	assert( pszCPPClassName );

	auto obj = adapter.GetObject();

	assert( obj );

	if( !obj )
		return false;

	const auto& typeInfo = obj.GetTypeInfo();

	assert( typeInfo );

	std::stringstream stream;

	stream << "void SetSelf( " << pszCPPClassName << "@ )";

	std::string szFunction = stream.str();

	bool bSuccess = false;

	//Initialize the self member.
	if( auto pFunction = typeInfo->GetMethodByDecl( szFunction.c_str() ) )
	{
		CASOwningContext ctx( *pFunction->GetEngine() );

		CASMethod method( *pFunction, ctx, *obj );

		assert( method.IsValid() );

		if( method.Call( CallFlag::NONE, pThis ) )
		{
			bSuccess = true;
		}
	}

	//If the user has provided a baseclass type, initialize the BaseClass member.
	if( bSuccess && pszCPPBaseClassName )
	{
		bSuccess = false;

		stream.str( "" );

		stream << "void SetBaseClass( " << pszCPPBaseClassName << "@ )";

		szFunction = stream.str();

		if( auto pFunction = typeInfo->GetMethodByDecl( szFunction.c_str() ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, *obj );

			assert( method.IsValid() );

			if( method.Call( CallFlag::NONE, pThis ) )
			{
				bSuccess = true;
			}
		}
	}

	return bSuccess;
}
}