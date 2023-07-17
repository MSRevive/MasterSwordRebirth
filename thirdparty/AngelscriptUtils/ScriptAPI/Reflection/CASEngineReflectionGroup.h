#ifndef ANGELSCRIPT_SCRIPTAPI_REFLECTION_CASENGINEREFLECTIONGROUP_H
#define ANGELSCRIPT_SCRIPTAPI_REFLECTION_CASENGINEREFLECTIONGROUP_H

#include "IASReflectionGroup.h"

class CASEngineReflectionGroup : public IASReflectionGroup
{
public:
	CASEngineReflectionGroup() = default;
	~CASEngineReflectionGroup() = default;

	asIScriptFunction* FindGlobalFunction( const std::string& szName, bool bSearchByDecl = false ) override;

	asUINT GetGlobalFunctionCount() const override;

	asIScriptFunction* GetGlobalFunctionByIndex( asUINT uiIndex ) override;

	asITypeInfo* FindTypeInfo( const std::string& szName, bool bSearchByDecl = false ) override;

	asUINT GetObjectTypeCount() const override;

	asITypeInfo* GetObjectTypeByIndex( asUINT uiIndex ) override;
	
	void SetEngine( asIScriptEngine& engine )
	{
		m_pEngine = &engine;
	}

private:
	asIScriptEngine* m_pEngine = nullptr;

private:
	CASEngineReflectionGroup( const CASEngineReflectionGroup& ) = delete;
	CASEngineReflectionGroup& operator=( const CASEngineReflectionGroup& ) = delete;
};

#endif //ANGELSCRIPT_SCRIPTAPI_REFLECTION_CASENGINEREFLECTIONGROUP_H