#pragma once

#include <angelscript.h>

class CASEngine 
{
public:
	asIScriptEngine *m_ASEngine;

	CASEngine() : m_ASEngine(asCreateScriptEngine());

	~CASEngine()
	{
		m_ASEngine->ShutDownAndRelease();
	};
}