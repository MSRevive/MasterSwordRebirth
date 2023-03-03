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
	//Client CVARs
	CVAR_CREATE("ms_status_icons", "1", FCVAR_CLIENTDLL); // Drigien MAY2008 - Shows/Hides The HUD Status Icons
	//JAN2010_11 - not optional - used to blind player sometimes
	//CVAR_CREATE( "ms_showhudimgs", "1", FCVAR_CLIENTDLL );		// Drigien MAY2008 - Shows/Hides The HUD Images
	CVAR_CREATE("hud_classautokill", "1", FCVAR_ARCHIVE);				// controls whether or not to suicide immediately on TF class switch
	CVAR_CREATE("hud_takesshots", "0", FCVAR_ARCHIVE);					// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE("ms_hidehud", "0", FCVAR_ARCHIVE);						// Hides the HUD and viewmodel completely
	CVAR_CREATE("ms_lildude", "1", FCVAR_ARCHIVE);						// Thothie MAR2007a - Hides the 3d Guy if set 0
	CVAR_CREATE("ms_clgender", "0", FCVAR_CLIENTDLL);					// Thothie FEB2011_22 - Adding a cvar to store client gender
	CVAR_CREATE("ms_xpdisplay", "0", FCVAR_ARCHIVE);					// Thothie AUG2007a - XP Display Options
	CVAR_CREATE("ms_developer", "0", FCVAR_CLIENTDLL);					// Thothie MAR2007b - Hides client side developer messages when set to 0
	CVAR_CREATE("ms_evthud_decaytime", "5", FCVAR_ARCHIVE);				// Time each line in the Event Console lasts before it shrinks
	CVAR_CREATE("ms_evthud_history", "10", FCVAR_ARCHIVE);				// Max number of text lines to keep in the Event Console history
	CVAR_CREATE("ms_evthud_size", "5", FCVAR_ARCHIVE);					// Max number of text lines shown at once
	CVAR_CREATE("ms_evthud_bgtrans", "0", FCVAR_CLIENTDLL);				// Transparency of the background
	CVAR_CREATE("ms_txthud_decaytime", "9", FCVAR_ARCHIVE);				// Time each line in the Event Console lasts before it shrinks
	CVAR_CREATE("ms_txthud_history", "50", FCVAR_ARCHIVE);				// Max number of text lines to keep in the Event Console history
	CVAR_CREATE("ms_txthud_size", "8", FCVAR_ARCHIVE);					// Max number of text lines shown at once
	CVAR_CREATE("ms_txthud_bgtrans", "0", FCVAR_ARCHIVE);				// Transparency of the background
	CVAR_CREATE("ms_txthud_width", "640", FCVAR_ARCHIVE);				// Width of console
	CVAR_CREATE("ms_help", "1", FCVAR_ARCHIVE); // Whether help tips are shown
	CVAR_CREATE("ms_reflect", "1", FCVAR_ARCHIVE);						// Allow reflective surfaces
	CVAR_CREATE("ms_reflect_dbg", "0", FCVAR_ARCHIVE);					// Debug reflective surfaces
	CVAR_CREATE("ms_bloom_darken", "-1", FCVAR_ARCHIVE);				// MiB DEC2010 - Darken bloom
	CVAR_CREATE("ms_bloom_level", "0", FCVAR_ARCHIVE);					// Thothie DEC2010_30 - Fix ms_bloom_level stickiness
	//CVAR_CREATE("ms_reconnect_delay", "5", FCVAR_ARCHIVE);				// Thothie AUG2017 - Make reconnect delay adjustable client side
	CVAR_CREATE("ms_quickslot_timeout", "2.5", FCVAR_ARCHIVE);		// Timeout for the quickslots
	CVAR_CREATE("ms_autocharge", "1", FCVAR_ARCHIVE);					// MiB MAR2012_05 - Let's you auto-charge your attack
	CVAR_CREATE("ms_doubletapdodge", "0", FCVAR_ARCHIVE);				// MiB MAR2012_05 -Enable/Disable double tapping to dodge
	CVAR_CREATE("ms_invtype", "1", FCVAR_ARCHIVE);						// MiB FEB2012_12 - Inventory types (added post-doc by Thothie)
	CVAR_CREATE("ms_showotherglow", "1", FCVAR_ARCHIVE);
	CVAR_CREATE("ms_chargebar_sound", "magic/chargebar_alt1.wav", FCVAR_ARCHIVE);
	CVAR_CREATE("ms_glowcolor", "(255,255,255)", FCVAR_ARCHIVE); // This is called in player think and was breaking the entire think cycle.
	CVAR_CREATE("ms_chargebar_volume", "15", FCVAR_ARCHIVE);
	CVAR_CREATE("ms_doubletap_delay", "0.45", FCVAR_ARCHIVE); // The higher the amount, the longer the player has to hit left/right/back for a second time
	CVAR_CREATE("ms_sprint_verbose", "2", FCVAR_ARCHIVE); // 0 for no messages , 1 for only warnings , 2 for everything
	CVAR_CREATE("ms_sprint_toggle", "1", FCVAR_ARCHIVE);
	CVAR_CREATE("ms_sprint_doubletap", "1", FCVAR_ARCHIVE);
	CVAR_CREATE("msui_id_offsetx", "100", FCVAR_ARCHIVE);
	CVAR_CREATE("msui_id_offsety", "100", FCVAR_ARCHIVE);
	CVAR_CREATE("msui_id_background", "0", FCVAR_ARCHIVE);

	RichPresenceInitialize();
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
	gHUD.Shutdown();
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

	// An attempt to detect disconnect to stop all sounds. should make it work with map change as well.
	if (isConnected != m_IsConnected || m_ConnectionTime > status.connection_time)
	{
		ResetClient();

		m_IsConnected = isConnected;
		m_ConnectionTime = status.connection_time;
	}

	gSoundEngine.Update();
	steamhelper->Think();
	RichPresenceUpdate();
}

extern CClientLibrary gClient;