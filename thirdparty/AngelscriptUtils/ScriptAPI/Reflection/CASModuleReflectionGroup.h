#ifndef ANGELSCRIPT_SCRIPTAPI_REFLECTION_CASMODULEREFLECTIONGROUP_H
#define ANGELSCRIPT_SCRIPTAPI_REFLECTION_CASMODULEREFLECTIONGROUP_H

#include "IASReflectionGroup.h"

class CASModuleReflectionGroup : public IASReflectionGroup
{
public:
	CASModuleReflectionGroup( ) = default;
	~CASModuleReflectionGroup() = default;

	asIScriptFunction* FindGlobalFunction( const std::string& szName, bool bSearchByDecl = false ) override;

	asUINT GetGlobalFunctionCount() const override;

	asIScriptFunction* GetGlobalFunctionByIndex( asUINT uiIndex ) override;

	asITypeInfo* FindTypeInfo( const std::string& szName, bool bSearchByDecl = false ) override;

	asUINT GetObjectTypeCount() const override;

	asITypeInfo* GetObjectTypeByIndex( asUINT uiIndex ) override;

private:
	CASModuleReflectionGroup( const CASModuleReflectionGroup& ) = delete;
	CASModuleReflectionGroup& operator=( const CASModuleReflectionGroup& ) = delete;
};

#endif //ANGELSCRIPT_SCRIPTAPI_REFLECTION_CASMODULEREFLECTIONGROUP_H