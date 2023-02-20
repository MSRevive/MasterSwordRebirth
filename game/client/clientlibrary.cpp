#include "clientlibrary.h"
#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#include "net_api.h"
#include "filesystem_shared.h"
#include "steamhelper.h"
#include "richpresence.h"

bool CClientLibrary::Initialize() 
{
	if(!FileSystem_Init())
		return false;

	return true;
}

void CClientLibrary::PostInitialize()
{

}

void CClientLibrary::HUDInit()
{

}

void CClientLibrary::Shutdown()
{
	FileSystem_Shutdown();
}

void CClientLibrary::ResetClient()
{

}

void CClientLibrary::RunFrame() 
{
	net_status_t status;
	gEngfuncs.pNetAPI->Status(&status);

	const bool isConnected = status.connected != 0;

	auto mapName = gEngfuncs.pfnGetLevelName();
	if (!mapName)
		mapName = "";

	if (m_IsConnected != isConnected || m_ConnectionTime > status.connection_time || m_ServerAddress != status.remote_address || stricmp(mapName, m_MapName) != 0)
	{
		ResetClient();

		m_IsConnected = isConnected;
		m_ConnectionTime = status.connection_time;
		m_ServerAddress = status.remote_address;
		m_MapName = mapName;
	}

	steamhelper->Think();
	RichPresenceUpdate();
}

extern CClientLibrary gClient;