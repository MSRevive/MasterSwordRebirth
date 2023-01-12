#pragma once

#include <angelscript.h>
#include "groupfile.h"

class CASEngine 
{
public:
	CGameGroupFile m_GroupFile;
	
	CASEngine() : m_ASEngine(asCreateScriptEngine());

	~CASEngine()
	{
		m_ASEngine->ShutDownAndRelease();
	};

	void RegisterAddons();

private:
	asIScriptEngine *m_ASEngine;
}