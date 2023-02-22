#include "clientlibrary.h"
#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#include "net_api.h"
#include "filesystem_shared.h"
#include "steamhelper.h"
#include "richpresence.h"
#include "fmod/soundengine.h"

CSoundEngine gSoundEngine;
CHud gHUD;

bool CClientLibrary::Initialize() 
{
	logfile << Logger::LOG_INFO << "[INIT: Loading filesystem]\n";
	if (!FileSystem_Init())
		return false;

	logfile << Logger::LOG_INFO << "[INIT: Loading sound engine]\n";
	if (!gSoundEngine.InitFMOD())
		return false;

	logfile << Logger::LOG_INFO << "[INIT: Loading HUD]\n";
	gHUD.Init();

	return true;
}

void CClientLibrary::PostInitialize()
{

}

void CClientLibrary::HUDInit()
{
	
}

void CClientLibrary::VideoInit()
{
	ResetClient(); // this gets called on start of every map

	logfile << Logger::LOG_INFO << "[INIT: Video Init]\n";
	gHUD.VidInit();
}

void CClientLibrary::Shutdown()
{
	logfile << Logger::LOG_INFO << "[INIT: Shutdown]\n";
	FileSystem_Shutdown();
	gSoundEngine.ExitFMOD();
}

void CClientLibrary::ResetClient()
{
	logfile << Logger::LOG_INFO << "[INIT: Reset Client]\n";
	gHUD.ReloadClient();
}

void CClientLibrary::RunFrame() 
{
	net_status_t status;
	gEngfuncs.pNetAPI->Status(&status);

	bool isConnected = status.connected != 0;
	//const char* mapName = gEngfuncs.pfnGetLevelName();

	// An attempt to detect disconnect to stop all sounds. should make it work with map change as well.
	if (isConnected != m_IsConnected || m_ConnectionTime > status.connection_time)
	{
		//logfile << Logger::LOG_INFO << "[INIT: CLIENT DISCONNECT]\n";
		ResetClient();

		m_IsConnected = isConnected;
		m_ConnectionTime = status.connection_time;
	}

	gSoundEngine.Update();
	steamhelper->Think();
	RichPresenceUpdate();
}

extern CClientLibrary gClient;