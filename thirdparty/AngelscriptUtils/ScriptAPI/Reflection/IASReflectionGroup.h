#ifndef ANGELSCRIPT_SCRIPTAPI_REFLECTION_IASREFLECTIONGROUP_H
#define ANGELSCRIPT_SCRIPTAPI_REFLECTION_IASREFLECTIONGROUP_H

#include <string>

#include <angelscript.h>

class IASReflectionGroup
{
public:
	virtual ~IASReflectionGroup() = 0;

	virtual asIScriptFunction* FindGlobalFunction( const std::string& szName, bool bSearchByDecl = false ) = 0;

	virtual asUINT GetGlobalFunctionCount() const = 0;

	virtual asIScriptFunction* GetGlobalFunctionByIndex( asUINT uiIndex ) = 0;

	virtual asITypeInfo* FindTypeInfo( const std::string& szName, bool bSearchByDecl = false ) = 0;

	virtual asUINT GetObjectTypeCount() const = 0;

	virtual asITypeInfo* GetObjectTypeByIndex( asUINT uiIndex ) = 0;
};

inline IASReflectionGroup::~IASReflectionGroup()
{
}

#endif //ANGELSCRIPT_SCRIPTAPI_REFLECTION_IASREFLECTIONGROUP_H