
#include "angelscript/CHL2ASServerManager.h"
#include "svglobals_angelscript.h"

void MSC::ASManager::Initialize()
{
	g_ASManager.Initialize();
}

void MSC::ASManager::Shutdown()
{
	g_ASManager.Shutdown();
}

void MSC::ASManager::SetReady( bool state )
{
	g_ASManager.SetReady( state );
}

void MSC::ASManager::Think()
{
	g_ASManager.Think();
}

void MSC::ASManager::WorldCreated( const char* const szScript )
{
	g_ASManager.WorldCreated( szScript );
}

void MSC::ASManager::WorldEnded()
{
	g_ASManager.WorldEnded();
}
