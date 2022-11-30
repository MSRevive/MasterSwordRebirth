#pragma once

#include <angelscript.h>

class CASEngine 
{
public:
	CASEngine() : m_ASEngine(asCreateScriptEngine());

	~CASEngine()
	{
		m_ASEngine->ShutDownAndRelease();
	};

	void RegisterAddons();

private:
	asIScriptEngine *m_ASEngine;
}