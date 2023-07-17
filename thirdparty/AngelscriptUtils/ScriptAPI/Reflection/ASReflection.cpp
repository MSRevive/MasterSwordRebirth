#include "AngelscriptUtils/util/ASUtil.h"
#include "AngelscriptUtils/util/CASBaseClass.h"

#include "IASReflectionGroup.h"
#include "CASEngineReflectionGroup.h"
#include "CASModuleReflectionGroup.h"

#include "ASReflection.h"

namespace as
{
namespace Reflect
{
static CASEngineReflectionGroup Engine;
static CASModuleReflectionGroup Module;
}
}

static void Function_AddRef( const asIScriptFunction* const pThis )
{
	pThis->AddRef();
}

static void Function_Release( const asIScriptFunction* const pThis )
{
	pThis->Release();
}

static asITypeInfo* Function_GetObjectType( const asIScriptFunction* const pThis )
{
	if( auto pType = pThis->GetObjectType() )
	{
		pThis->AddRef();
		return pType;
	}

	return nullptr;
}

static std::string Function_GetObjectName( const asIScriptFunction* const pThis )
{
	if( auto pszName = pThis->GetObjectName() )
		return pszName;

	return "";
}

static std::string Function_GetNamespace( const asIScriptFunction* const pThis )
{
	return pThis->GetNamespace();
}

static std::string Function_GetName( const asIScriptFunction* const pThis )
{
	return pThis->GetName();
}

static std::string Function_GetDeclaration( const asIScriptFunction* const pThis, 
											const bool bIncludeObjectName = true, const bool bIncludeNamespace = false, const bool bIncludeParamNames = false )
{
	return pThis->GetDeclaration( bIncludeObjectName, bIncludeNamespace, bIncludeParamNames );
}

static void RegisterScriptReflectionCallable( asIScriptEngine& engine, const char* const pszObjectName )
{
	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_ADDREF, "void AddRef()",
		asFUNCTION( Function_AddRef ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_RELEASE, "void Release()",
		asFUNCTION( Function_Release ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "TypeInfo@ GetObjectType() const",
		asFUNCTION( Function_GetObjectType ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetObjectName() const",
		asFUNCTION( Function_GetObjectName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNamespace() const",
		asFUNCTION( Function_GetNamespace ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetName() const",
		asFUNCTION( Function_GetName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName,  "string GetDeclaration(const bool bIncludeObjectName = true, const bool bIncludeNamespace = false, const bool bIncludeParamNames = false) const",
		asFUNCTION( Function_GetDeclaration ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsReadOnly() const",
		asMETHOD( asIScriptFunction, IsReadOnly ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsPrivate() const",
		asMETHOD( asIScriptFunction, IsPrivate ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsProtected() const",
		asMETHOD( asIScriptFunction, IsProtected ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsFinal() const",
		asMETHOD( asIScriptFunction, IsFinal ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsOverride() const",
		asMETHOD( asIScriptFunction, IsOverride ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "bool IsShared() const",
		asMETHOD( asIScriptFunction, IsShared ), asCALL_THISCALL );
}

static void RegisterScriptReflectionFunction( asIScriptEngine& engine )
{
	RegisterScriptReflectionCallable( engine, "Function" );
}

static void RegisterScriptReflectionMethod( asIScriptEngine& engine )
{
	RegisterScriptReflectionCallable( engine, "Method" );
}

static void TypeInfo_AddRef( const asITypeInfo* const pThis )
{
	pThis->AddRef();
}

static void TypeInfo_Release( const asITypeInfo* const pThis )
{
	pThis->Release();
}

static std::string TypeInfo_GetNamespace( const asITypeInfo* const pThis )
{
	return pThis->GetNamespace();
}

static std::string TypeInfo_GetName( const asITypeInfo* const pThis )
{
	return pThis->GetName();
}

static asIScriptFunction* TypeInfo_GetMethodByIndex( const asITypeInfo* const pThis, const asUINT uiIndex, const bool bGetVirtual )
{
	if( auto pFunc = pThis->GetMethodByIndex( uiIndex, bGetVirtual ) )
	{
		pFunc->AddRef();
		return pFunc;
	}

	return nullptr;
}

static asIScriptFunction* TypeInfo_GetMethodByDecl( const asITypeInfo* const pThis, const std::string& szDecl, const bool bGetVirtual )
{
	if( auto pFunc = pThis->GetMethodByDecl( szDecl.c_str(), bGetVirtual ) )
	{
		pFunc->AddRef();
		return pFunc;
	}

	return nullptr;
}

static asIScriptFunction* TypeInfo_GetMethodByName( const asITypeInfo* const pThis, const std::string& szName, const bool bGetVirtual )
{
	if( auto pFunc = pThis->GetMethodByName( szName.c_str(), bGetVirtual ) )
	{
		pFunc->AddRef();
		return pFunc;
	}

	return nullptr;
}

static void RegisterScriptReflectionTypeInfo( asIScriptEngine& engine )
{
	const char* const pszObjectName = "TypeInfo";

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_ADDREF, "void AddRef()",
		asFUNCTION( TypeInfo_AddRef ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_RELEASE, "void Release()",
		asFUNCTION( TypeInfo_Release ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetNamespace() const",
		asFUNCTION( TypeInfo_GetNamespace ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "string GetName() const",
		asFUNCTION( TypeInfo_GetName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "uint GetMethodCount() const",
		asMETHOD( asITypeInfo, GetMethodCount ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Method@ GetMethodByIndex(const uint uiIndex, const bool bGetVirtual = true) const",
		asFUNCTION( TypeInfo_GetMethodByIndex ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Method@ GetMethodByDecl(const string& in szDecl, const bool bGetVirtual = true) const",
		asFUNCTION( TypeInfo_GetMethodByDecl ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Method@ GetMethodByName(const string& in szName, const bool bGetVirtual = true) const",
		asFUNCTION( TypeInfo_GetMethodByName ), asCALL_CDECL_OBJFIRST );
}

static void RegisterScriptReflectionGroup( asIScriptEngine& engine )
{
	const char* const pszObjectName = "IGroup";

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	engine.RegisterObjectMethod(
		pszObjectName, "Function@ FindGlobalFunction(const string& in szName, bool fSearchByDecl = false)",
		asMETHOD( IASReflectionGroup, FindGlobalFunction ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "uint GetGlobalFunctionCount() const",
		asMETHOD( IASReflectionGroup, GetGlobalFunctionCount ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Function@ GetGlobalFunctionByIndex(uint uiIndex)",
		asMETHOD( IASReflectionGroup, GetGlobalFunctionByIndex ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "TypeInfo@ FindTypeInfo(const string& in szName, bool fSearchByDecl = false)",
		asMETHOD( IASReflectionGroup, FindTypeInfo ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "uint GetObjectTypeCount() const",
		asMETHOD( IASReflectionGroup, GetObjectTypeCount ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "TypeInfo@ GetObjectTypeByIndex(uint uiIndex) const",
		asMETHOD( IASReflectionGroup, GetObjectTypeByIndex ), asCALL_THISCALL );
}

void RegisterScriptReflection( asIScriptEngine& engine )
{
	const std::string szOldNS = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace( "Reflect" );

	//Forward declarations.
	engine.RegisterObjectType( "TypeInfo", 0, asOBJ_REF );

	RegisterScriptReflectionFunction( engine );
	RegisterScriptReflectionMethod( engine );
	RegisterScriptReflectionTypeInfo( engine );
	RegisterScriptReflectionGroup( engine );

	as::Reflect::Engine.SetEngine( engine );

	engine.RegisterGlobalProperty( "IGroup Engine", &as::Reflect::Engine );
	engine.RegisterGlobalProperty( "IGroup Module", &as::Reflect::Module );

	engine.SetDefaultNamespace( szOldNS.c_str() );
}