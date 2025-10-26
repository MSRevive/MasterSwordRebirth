#include "msdllheaders.h"
#include "stats/stats.h"
#include "stats/races.h"
#include "svglobals.h"
#include "global.h"
#include "weapons/genericitem.h"
#include "gamerules/gamerules.h"
#include "store.h"
#include "cstringpool.h"
#include "crc/crchash.h"
#include "filesystem_shared.h"
#include "SteamServerHelper.h"
#include "fn/FNSharedDefs.h"
#include "fn/RequestManager.h"
#include "fn/HTTPRequest.h"
#include "angelscript/CAngelScriptManager.h"
#include "angelscript/ASModuleSystem.h"
#include "groupfile.h"
#include "mslogger.h"
#include <angelscript.h>

std::ofstream modelout;
int HighestPrecache = -1;
int TotalModelPrecaches = 1;
int PreCount = 0; //Thothie OCT2007a Precache map verification
bool gFNInitialized = false;

bool CSVGlobals::LogScripts = true;
mslist<CSVGlobals::scriptlistitem_t> CSVGlobals::ScriptList[SCRIPT_TYPES];

// Global game_master entity handle
CBaseEntity* g_pGameMasterEntity = nullptr;

//Thothie MAR2012_27 - duplicate precache trackers
int gModelPrecacheCount = 0;
int gSoundPrecacheCount = 0;
mslist<modelprecachelist_t> gModelPrecacheList;
mslist<modelprecachelist_t> gSoundPrecacheList;

CStringPool g_StringPool;
CRequestManager g_FNRequestManager;

//Master Sword CVARs
/*
cvar_t	debug			= {"ms_debug","0", FCVAR_SERVER };
cvar_t	betakey			= {"ms_key","", FCVAR_PROTECTED|FCVAR_EXTDLL };*/
cvar_t ms_dev_mode = {"ms_dev_mode", "0", FCVAR_SERVER}; //MiB JUL2010_13 - Dev mode..
cvar_t ms_dynamicnpc = {"ms_dynamicnpc", "", 0};
cvar_t msallowkickvote = {"ms_allowkickvote", "1", FCVAR_SERVER};
cvar_t msallowtimevote = {"ms_allowtimevote", "1", FCVAR_SERVER};
cvar_t ms_reset_time = {"ms_reset_time", "10", FCVAR_SERVER};
cvar_t ms_reset_map = {"ms_reset_map", "edana", FCVAR_SERVER};
cvar_t ms_version = {"ms_version", __DATE__, FCVAR_EXTDLL};
cvar_t ms_pklevel = {"ms_pklevel", "0", FCVAR_SERVER};
//cvar_t	ms_trans_req	= {"ms_trans_req","0", FCVAR_SERVER }; //Thothie JUN2007 - max players required to activate a transition (0 = all on server) - nvm, changed method - nvm, changed method
cvar_t ms_fxlimit = {"ms_fxlimit", "0", FCVAR_SERVER};
//cvar_t	ms_currentfx	= {"ms_currentfx","0", 0 }; //Thothie - want to make FX control total ms.dll, but can't figure how
cvar_t ms_serverchar = {"ms_serverchar", "1", FCVAR_SERVER};
cvar_t ms_joinreset = {"ms_joinreset", "1", FCVAR_SERVER};
cvar_t ms_hp_limit = {"ms_hp_limit", "0", FCVAR_SERVER};
cvar_t msvote_farm_all_day = {"msvote_farm_all_day", "0", FCVAR_SERVER};		 //Thothie FEB2008a - Allow voting for the map the players are on
cvar_t msvote_map_type = {"msvote_map_type", "all", FCVAR_SERVER};				 //Thothie FEB2008a - Map vote type (current: all, and root)
cvar_t msvote_map_enable = {"msvote_map_enable", "1", FCVAR_SERVER};			 //Thothie FEB2008a - Map vote enable
cvar_t msvote_kick_enable = {"msvote_kick_enable", "1", FCVAR_SERVER};			 //Thothie FEB2008a - Vote kick at chat/console enable
cvar_t msvote_ban_enable = {"msvote_ban_enable", "1", FCVAR_SERVER};			 //Thothie FEB2008a - Vote ban at chat/console enable
cvar_t msvote_ban_time = {"msvote_ban_time", "60", FCVAR_SERVER};				 //Thothie FEB2008a - Time to ban a person voted banned
cvar_t msvote_pvp_enable = {"msvote_pvp_enable", "60", FCVAR_SERVER};			 //Thothie FEB2008a - Allow voting for PVP
cvar_t msvote_lock_enable = {"msvote_lock_enable", "1", FCVAR_SERVER};			 //Thothie JAN2011_04 - Allow voting to password lock server
cvar_t ms_chatlog = {"ms_chatlog", "1", FCVAR_SERVER};							 //Thothie FEB2008a - write chatlog
cvar_t ms_admin_contact = {"ms_admin_contact", "[none provided]", FCVAR_SERVER}; //Thothie FEB2008a - server host contact info
cvar_t ms_ban_to_cfg = {"ms_ban_to_cfg", "1", FCVAR_SERVER};					 //Thothie FEB2008a - write scriptside perm bans to server/listenserver.cfg
cvar_t ms_central_enabled = {"ms_central_enabled", "0", FCVAR_SERVER};
cvar_t ms_fake_hp = {"ms_fake_hp", "0", FCVAR_SERVER};			 //Thothie AUG2011_17 - moving Fakehp to cvar for use with triggers
cvar_t ms_fake_players = {"ms_fake_players", "0", FCVAR_SERVER}; //Thothie DEC2013_07 - for returning false # of players for some functions during testing
cvar_t ms_central_addr = {"ms_central_addr", "0", FCVAR_PROTECTED};
cvar_t ms_debug_mem = {"ms_debug_mem", "0", 0};
//cvar_t ms_crashcfg = {"ms_crashcfg", "crashed", FCVAR_SERVER};

//AngelScript CVARs
cvar_t as_enabled = {const_cast<char*>("as_enabled"), "1", FCVAR_SERVER};
cvar_t as_memory_limit = {const_cast<char*>("as_memory_limit"), "1073741824", FCVAR_SERVER}; // 1GB
cvar_t as_memory_debug = {const_cast<char*>("as_memory_debug"), "0", FCVAR_SERVER};
cvar_t as_gc_interval = {const_cast<char*>("as_gc_interval"), "60", FCVAR_SERVER};
cvar_t as_stack_size = {const_cast<char*>("as_stack_size"), "4096", FCVAR_SERVER}; // 4KB
cvar_t as_debug_mode = {const_cast<char*>("as_debug_mode"), "0", FCVAR_SERVER};
cvar_t as_auto_discovery = {const_cast<char*>("as_auto_discovery"), "1", FCVAR_SERVER}; // Enable module auto-discovery
cvar_t as_module_debug = {const_cast<char*>("as_module_debug"), "0", FCVAR_SERVER}; // Debug module discovery

#ifdef DEV_BUILD
cvar_t ms_devlog = {"ms_devlog", "1", 0};
cvar_t ms_allowdev = {"ms_allowdev", "1", 0};
#endif

//------------

//SOCKET g_PingSock;
void UnBanAll();

bool MSGlobalInit() //Called upon DLL Initialization
{
	//Master Sword CVARs
	CVAR_REGISTER(&ms_version);
	CVAR_REGISTER(&ms_pklevel);
	//CVAR_REGISTER (&ms_trans_req); //Thothie JUN2007 - max players required to activate a transition (0 = all on server)
	CVAR_REGISTER(&ms_fxlimit);
	//CVAR_REGISTER (&debug);
	//CVAR_REGISTER (&betakey);
	CVAR_REGISTER(&ms_dev_mode); //MiB "Mapper Scripts.rtf"
	CVAR_REGISTER(&ms_dynamicnpc);
	CVAR_REGISTER(&msallowkickvote);
	CVAR_REGISTER(&msallowtimevote);
	CVAR_REGISTER(&ms_serverchar);
	CVAR_REGISTER(&ms_joinreset);
	CVAR_REGISTER(&ms_reset_time);
	CVAR_REGISTER(&ms_reset_map);
	CVAR_REGISTER(&ms_hp_limit);
	CVAR_REGISTER(&msvote_farm_all_day);
	CVAR_REGISTER(&msvote_map_type);
	CVAR_REGISTER(&msvote_map_enable);
	CVAR_REGISTER(&msvote_kick_enable);
	CVAR_REGISTER(&msvote_ban_enable);
	CVAR_REGISTER(&msvote_ban_time);
	CVAR_REGISTER(&msvote_pvp_enable);
	CVAR_REGISTER(&msvote_lock_enable);
	CVAR_REGISTER(&ms_chatlog);
	CVAR_REGISTER(&ms_admin_contact);
	CVAR_REGISTER(&ms_ban_to_cfg);
	CVAR_REGISTER(&ms_central_enabled);
	CVAR_REGISTER(&ms_central_addr);
	CVAR_REGISTER(&ms_debug_mem);
	CVAR_REGISTER(&ms_fake_hp);		 //AUG2011_17 Thothie - moving fakehp functions to cvar
	CVAR_REGISTER(&ms_fake_players); //DEC2013_07 Thothie - fake players cvar

	//AngelScript CVARs
	CVAR_REGISTER(&as_enabled);
	CVAR_REGISTER(&as_memory_limit);
	CVAR_REGISTER(&as_memory_debug);
	CVAR_REGISTER(&as_gc_interval);
	CVAR_REGISTER(&as_stack_size);
	CVAR_REGISTER(&as_debug_mode);
	CVAR_REGISTER(&as_auto_discovery);
	CVAR_REGISTER(&as_module_debug);

#ifdef DEV_BUILD
	CVAR_REGISTER(&ms_devlog);
	CVAR_REGISTER(&ms_allowdev);
#endif

	// Initialize AngelScript if enabled
	if (as_enabled.value > 0)
	{
		if (!CAngelScriptManager::Instance()->Initialize())
		{
			g_engfuncs.pfnServerPrint("\nAngelScript initialization FAILED!");
			// Don't fail the entire initialization, just disable AngelScript
			CVAR_SET_FLOAT("as_enabled", 0);
		}
		else
		{
			// Configure memory limit
			CAngelScriptManager::Instance()->SetMemoryLimit((size_t)as_memory_limit.value);
			// AngelScript initialization is now logged through MSLogger
			
			// Load AngelScript modules from scripts.pak
			if (CAngelScriptManager::Instance()->IsInitialized())
			{
				g_engfuncs.pfnServerPrint("Loading AngelScript modules...\n");
				
				// The module discovery system below will handle loading all modules
				g_engfuncs.pfnServerPrint("AngelScript core initialized - proceeding to module discovery...\n");
			}
		}
	}

	SERVER_COMMAND("exec msstartup.cfg\n");

	return true;
}

void WriteCrashCfg()
{
	char fileName[MAX_PATH], content[128];
	const char* cfgName = CVAR_GET_STRING("sv_crashcfg");

	_snprintf(fileName, MAX_PATH, "%s.cfg", (cfgName && cfgName[0]) ? cfgName : "crashed");
	_snprintf(content, sizeof(content), "map %s", MSGlobals::MapName.c_str());

	FileSystem_WriteTextToFile(fileName, content);
}

//Called from CWorld::Spawn() each map change
void MSWorldSpawn()
{
	MS_INFO("=== MSWorldSpawn: Starting map initialization ===");
	
	//Setup global variables that can't be changed during a game
	MSGlobals::PKAllowed = ms_pklevel.value > 0 ? true : false;
	//Thothie attemptitng to remove FN upload sploit (Thanx to MiB)
	MSGlobals::CentralEnabled = CVAR_GET_FLOAT("ms_central_enabled") > 0.0f ? true : false;
	
	// CRITICAL: Reload AngelScript modules after level change
	// All modules were cleared in ServerDeactivate via PrepareForLevelChange()
	// We need to reload them now for the new map
	if (as_enabled.value > 0)
	{
		CAngelScriptManager* pASManager = CAngelScriptManager::Instance();
		if (pASManager && pASManager->IsInitialized())
		{
			MS_INFO("MSWorldSpawn: Reloading AngelScript modules for new map...");
			
			// Open the scripts.pak file for reading AngelScript modules
			CGameGroupFile groupFile;
			if (!groupFile.Open("scripts.pak"))
			{
				MS_ERROR("MSWorldSpawn: Failed to open scripts.pak for module reloading");
			}
			else
			{
				ASModuleSystem* pModuleSystem = ASModuleSystem::Instance();
				if (pModuleSystem)
				{
					// Check if auto-discovery is enabled
					if (as_auto_discovery.value > 0)
					{
						MS_INFO("MSWorldSpawn: Using automatic module discovery...");
						
						// Discover modules with 'module ModuleName {' syntax
						if (pModuleSystem->DiscoverModulesInPak(&groupFile))
						{
							// Load all discovered modules
							if (pModuleSystem->LoadDiscoveredModules(&groupFile))
							{
								MS_INFO("MSWorldSpawn: AngelScript modules reloaded successfully!");
							}
							else
							{
								MS_ERROR("MSWorldSpawn: Some AngelScript modules failed to reload");
							}
						}
						else
						{
							MS_ERROR("MSWorldSpawn: No modules discovered during reload");
						}
					}
					else
					{
						MS_INFO("MSWorldSpawn: Module auto-discovery disabled, skipping reload");
					}
				}
				else
				{
					MS_ERROR("MSWorldSpawn: ASModuleSystem not available for reload");
				}
			}
		}
	}
	
	MS_INFO("=== MSWorldSpawn: Map initialization complete ===");
	MSGlobals::DevModeEnabled = ms_dev_mode.value > 0 && !MSGlobals::CentralEnabled ? true : false;
	//return MSGlobals::CentralEnabled && !MSGlobals::IsLanGame && MSGlobals::ServerSideChar;
	//MSGlobals::FXLimit = CVAR_GET_FLOAT("ms_fxlimit");
	MSGlobals::PKAllowedinTown = ms_pklevel.value > 1 ? true : false;
	MSGlobals::IsLanGame = CVAR_GET_FLOAT("sv_lan") ? true : false;
	MSGlobals::CanCreateCharOnMap = false;
	MSGlobals::ServerSideChar = ms_serverchar.value ? true : false;
	MSGlobals::MapName = STRING(gpGlobals->mapname);
	
	//Force the client to use the same client lib as the server. - Solokiller
	//This ensures that clients don't replace their client and send exploit commands.
#if !defined(_DEBUG)
	ENGINE_FORCE_UNMODIFIED(force_exactfile, NULL, NULL, "cl_dlls/client.dll");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, NULL, NULL, "cl_dlls/client.so");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, NULL, NULL, "cl_dlls/client.dylib");
	ENGINE_FORCE_UNMODIFIED(force_exactfile, NULL, NULL, "scripts.pak");
#endif

	HTTPRequest::SetBaseURL(CVAR_GET_STRING("ms_central_addr"));

	//TODO: move to MSGlobalInit so it's not called every map change.
	//g_SteamServerHelper.Init();

	// if (MSGlobals::CentralEnabled)
	// {
	// 	// Initialize FN Request Manager
	// 	g_FNRequestManager.Init();

	// 	bool fail = true;

	// 	for (int retry = 0; retry < 5; retry++)
	// 	{
	// 		if (FNShared::IsValidConnection())
	// 		{
	// 			fail = false;
	// 			g_engfuncs.pfnServerPrint("FuzzNet connected!\n");
	// 			logfile << Logger::LOG_INFO << "FuzzNet connected\n";
	// 			break;
	// 		}
	// 		else if (retry != 5)
	// 		{
	// 			g_engfuncs.pfnServerPrint("FuzzNet connection failed! Retrying...\n");
	// 		}
	// 	}

	// 	if (fail == true)
	// 	{
	// 		g_engfuncs.pfnServerPrint("FuzzNet connection failed. Turning off FN.\n");
	// 		logfile << Logger::LOG_INFO << "FuzzNet connection failed.\n";
	// 		MSGlobals::CentralEnabled = false;
	// 	}
	// }

	if (FNShared::IsEnabled())
	{	
		MS_INFO("[FuzzNet] Initalize FN Request Manager");
		g_FNRequestManager.Init();
		
		// here we try to connect to FN and retry 5 times if it fails.
		bool fail = true;
		for (int retry = 0; retry < 5; retry++)
		{
			if (FNShared::ValidateFN())
			{
				fail = false;
				g_engfuncs.pfnServerPrint("FuzzNet connected!\n");
				MS_INFO("[FuzzNet] FuzzNet Connected");
				break;
			}
			else if (retry != 5)
			{
				g_engfuncs.pfnServerPrint("FuzzNet connection failed! Retrying...\n");
			}
		}

		if (fail == true)
		{
			g_engfuncs.pfnServerPrint("FuzzNet connection failed. Turning off FN.\n");
			MS_INFO("[FuzzNet] FuzzNet Connection Failed");
			MSGlobals::CentralEnabled = false;
		}
	}

	if (!FNShared::ValidateSC())
	{
		MSGlobals::CentralEnabled = false;
	}

	if (!FNShared::ValidateMap())
	{
		SERVER_COMMAND("map edana");
	}

	WriteCrashCfg();

	// Re-initialize AngelScript system if it was destroyed by previous map end
	if (as_enabled.value > 0)
	{
		if (!CAngelScriptManager::Instance()->IsInitialized())
		{
			g_engfuncs.pfnServerPrint("Re-initializing AngelScript Manager after map change...\n");
			MS_INFO("Re-initializing AngelScript Manager after map change...");
			
			if (!CAngelScriptManager::Instance()->Initialize())
			{
				g_engfuncs.pfnServerPrint("ERROR: Failed to re-initialize AngelScript Manager!\n");
				MS_ERROR("Failed to re-initialize AngelScript Manager!");
			}
			else
			{
				g_engfuncs.pfnServerPrint("AngelScript Manager re-initialized successfully\n");
				MS_INFO("AngelScript Manager re-initialized successfully");
			}
		}
	}

	// Initialize AngelScript Module System
	if (as_enabled.value > 0 && CAngelScriptManager::Instance()->IsInitialized())
	{
		g_engfuncs.pfnServerPrint("Initializing AngelScript Module System...\n");
		MS_INFO("Initializing AngelScript Module System...");
		
		// Open the scripts.pak file for reading AngelScript modules
		CGameGroupFile groupFile;
		if (!groupFile.Open("scripts.pak"))
		{
			g_engfuncs.pfnServerPrint("ERROR: Failed to open scripts.pak for modules\n");
			MS_INFO("Failed to open scripts.pak for modules");
		}
		else
		{
			ASModuleSystem* pModuleSystem = ASModuleSystem::Instance();
			if (pModuleSystem)
			{
				// Check if auto-discovery is enabled
				if (as_auto_discovery.value > 0)
				{
					g_engfuncs.pfnServerPrint("Using automatic module discovery...\n");
					MS_INFO("Using automatic module discovery...");
					
					// Discover modules with 'module ModuleName {' syntax
					if (pModuleSystem->DiscoverModulesInPak(&groupFile))
					{
						// Load all discovered modules
						if (pModuleSystem->LoadDiscoveredModules(&groupFile))
						{
							g_engfuncs.pfnServerPrint("AngelScript modules loaded successfully!\n");
							MS_INFO("AngelScript modules loaded successfully!");
						}
						else
						{
							g_engfuncs.pfnServerPrint("WARNING: Some AngelScript modules failed to load\n");
							MS_INFO("Some AngelScript modules failed to load");
						}
					}
					else
					{
						g_engfuncs.pfnServerPrint("No modules discovered in scripts.pak\n");
						MS_INFO("No modules discovered in scripts.pak");
					}
				}
				else
				{
					g_engfuncs.pfnServerPrint("Module auto-discovery disabled. Using legacy loading...\n");
					MS_INFO("Module auto-discovery disabled. Using legacy loading...");
					
					// Fallback to legacy hardcoded loading for backward compatibility
					// Note: GameMaster.as now uses module syntax and will be auto-discovered
					// Note: GameMasterInit.as has been merged into GameMaster.as
					const char* legacyModules[] = {
						"angelscript/AdvancedTriggerSystem.as",
						"angelscript/EntitySpawner.as",
						"angelscript/HPSequenceTrigger.as", 
						"angelscript/EntityCommunicationInit.as",
						"angelscript/GameMasterData.as",
						"angelscript/GameMasterEvents.as",
						"angelscript/GameMasterUtils.as"
					};
					
					bool bSuccess = true;
					for (int i = 0; i < 7; i++)
					{
						unsigned long fileSize;
						if (!groupFile.ReadEntry(legacyModules[i], NULL, fileSize))
						{
							char errorMsg[256];
							snprintf(errorMsg, sizeof(errorMsg), "Legacy module not found: %s\n", legacyModules[i]);
							g_engfuncs.pfnServerPrint(errorMsg);
							MS_ERROR(errorMsg);
							bSuccess = false;
							break;
						}
						
						char* scriptContent = new char[fileSize + 1];
						if (!groupFile.ReadEntry(legacyModules[i], (byte*)scriptContent, fileSize))
						{
							delete[] scriptContent;
							bSuccess = false;
							break;
						}
						scriptContent[fileSize] = '\0';
						
						const char* moduleName = strrchr(legacyModules[i], '/');
						if (moduleName) moduleName++;
						else moduleName = legacyModules[i];
						
						std::string modName = moduleName;
						size_t dotPos = modName.find(".as");
						if (dotPos != std::string::npos)
							modName = modName.substr(0, dotPos);
						
						ASModuleLoadOptions options;
						options.allowOverwrite = true;
						options.resolveDependencies = true;
						
						if (!pModuleSystem->LoadModuleFromMemory(modName, scriptContent, &groupFile, options))
						{
							bSuccess = false;
						}
						
						delete[] scriptContent;
					}
					
					// Legacy systems loaded successfully
					if (bSuccess)
					{
						g_engfuncs.pfnServerPrint("Legacy AngelScript support systems loaded successfully\n");
						MS_INFO("Legacy AngelScript support systems loaded successfully");
						g_engfuncs.pfnServerPrint("Note: GameMaster module will be auto-discovered and initialized separately\n");
						MS_INFO("Note: GameMaster module will be auto-discovered and initialized separately");
					}
				}
			}
			else
			{
				g_engfuncs.pfnServerPrint("ERROR: ASModuleSystem not available\n");
				MS_INFO("ASModuleSystem not available");
			}
		}
	}
}

//Called every frame
void MSGameThink()
{
	// CRITICAL: Don't run any script-related code during level transitions
	// g_serveractive is 0 during ServerDeactivate/ServerActivate
	
	//g_SteamServerHelper->Think();
	g_FNRequestManager.Think();

	// AngelScript maintenance - only run when server is fully active
	// This prevents script execution during level changes when entity references are invalid
	if (g_serveractive && as_enabled.value > 0 && CAngelScriptManager::Instance()->IsInitialized())
	{
		CAngelScriptManager::Instance()->Think();
	}

	// if(!gFNInitialized && FNShared::IsEnabled())
	// {
	// 	MSConnectFN();
	// 	gFNInitialized = true;
	// }
}

//Called when the map changes or server is shutdown from ServerDeactivate
//Note that ClientDisconnect is called after this, and the player is deallocated again!
#define WORLD_MAX 6000
void MSGameEnd()
{
	if(MSGlobals::GameScript)
	{
		//Moved here from MSGlobals::EndMap because commands can access entities that are freed below - Solokiller 3/10/2017
		MSGlobals::GameScript->CallScriptEvent( "game_end" );
	}
	
	//Save all characters now - Solokiller 5/10/2017
	for(int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
		//TODO: make sure player is actually connected and in valid state (i.e. not missing inventory) - Solokiller
		if(pPlayer)
		{
			pPlayer->SaveChar();
			if(!MSGlobals::ServerSideChar) pPlayer->m_TimeCharLastSent = 0;
		}
	}
	
	//Thothie MAR2012_27 - clear duplicate precaches for next map
	gSoundPrecacheList.clearitems();
	gModelPrecacheList.clearitems();
	gModelPrecacheCount = 0;
	gSoundPrecacheCount = 0;
	
	//Deallocate any 'extra' memory the mod allocated for any entity
	edict_t *pEdict = g_engfuncs.pfnPEntityOfEntIndex(0);
	if (pEdict)
		for (int i = 0; i < gpGlobals->maxEntities; i++, pEdict++)
		{
			if (pEdict->free)
				continue;

			CBaseEntity *pEntity = MSInstance(pEdict);
			if (!pEntity)
				continue;

			msstring dbgstr_classname = STRING(pEntity->pev->classname);

			pEntity->Deactivate();
			REMOVE_ENTITY(pEntity->edict());
		}

	//Delete global items
	CGenericItemMgr::DeleteItems();

	//Delete global stores
	CStore::RemoveAllStores();

	//Delete global script commands -- UNDONE: Keep these through level changes
	//CScript::Globals.Deactivate();

	CRaceManager::DeleteAllRaces();

	//Delete gamerules
	if (g_pGameRules)
	{
		delete g_pGameRules;
		g_pGameRules = nullptr;
	}

	//We handle all remaining requests and shutdown.
	//g_FNRequestManager.SendAndWait();

	//Thothie - I've not added anything here but there's a game error that generates here
	//MSGameEnd --> Call MSGlobals::EndMap
	MSGlobals::EndMap();

	//Model precache dumpfile
	if (modelout.is_open())
		modelout.close();
	HighestPrecache = -1;
	TotalModelPrecaches = 1;
	CSVGlobals::LogScripts = true;

	//g_SteamServerHelper->Shutdown();

	gFNInitialized = false;

	// Shutdown AngelScript GameMaster
	if (as_enabled.value > 0 && CAngelScriptManager::Instance()->IsInitialized())
	{
		// General AngelScript cleanup
		CAngelScriptManager::Instance()->Destroy();
	}
	
	//Clear the string pool now, after any references to its strings have been released.
	//Note: any attempts to access allocated strings between now and the next map start will fail and probably cause crashes.
	ClearStringPool();
}

void SendHUDMsgAll(const char* Title, const char* Text)
{
	for (int p = 0; p < gpGlobals->maxClients; p++)
		if (MSInstance(INDEXENT(p + 1)))
			((CBasePlayer *)MSInstance(INDEXENT(p + 1)))->SendHUDMsg(Title, Text);
}

//Server versions of these functions
const char *EngineFunc::GetGameDir()
{
	static char cGameDir[MAX_PATH];
	cGameDir[0] = 0;
	GET_GAME_DIR(cGameDir);
	return cGameDir;
}

void WRITE_FLOAT(float Float)
{
	byte *pData = (byte *)&Float;
	for (int i = 0; i < sizeof(float); i++)
		WRITE_BYTE(pData[i]);
}

int PRECACHE_SOUND(const char *pszSound)
{
	//Thothie tracking model precaches, avoiding duplicates
	bool bNoLog = false;
	for (int i = 0; i < gSoundPrecacheList.size(); i++)
	{
		msstring msPrecacheSoundName = pszSound;
		if (strcmp(msPrecacheSoundName.c_str(), gSoundPrecacheList[i].PrecacheName.c_str()) == 0)
		{
			//logfile << "(Precache Duplicate Avoided)" << "\n"; //temporary
			//return 0;
			bNoLog = true;
			break;
		}
	}
	if (!bNoLog)
	{
		gSoundPrecacheList.add_blank();
		gSoundPrecacheList[gSoundPrecacheCount].PrecacheName = pszSound;
		gSoundPrecacheCount++;
		MS_INFO("Precache_Sound(%i): %s", gSoundPrecacheCount, pszSound);
	}
	return (*g_engfuncs.pfnPrecacheSound)((char *)pszSound);
}

int PRECACHE_MODEL(const char *pszModelname)
{
	//Thothie tracking model precaches, avoiding duplicates
	bool bNoLog = false;
	for (int i = 0; i < gModelPrecacheList.size(); i++)
	{
		msstring msPrecacheModelName = pszModelname;
		if (strcmp(msPrecacheModelName.c_str(), gModelPrecacheList[i].PrecacheName.c_str()) == 0)
		{
			//logfile << "(Precache Duplicate Avoided)" << "\n"; //temporary
			//return 0;
			bNoLog = true;
			break;
		}
	}
	if (!bNoLog)
	{
		gModelPrecacheList.add_blank();
		gModelPrecacheList[gModelPrecacheCount].PrecacheName = pszModelname;
		gModelPrecacheCount++;
		MS_INFO("Precache_Model(%i): %s", gModelPrecacheCount, pszModelname);
	}

#ifdef DEV_BUILD
	if (!ms_devlog.value)
		return (*g_engfuncs.pfnPrecacheModel)((char *)pszModelname);

	if (!modelout.is_open())
		modelout.open(msstring(EngineFunc::GetGameDir()) + "/log_models.txt");
#endif

	int LastModel = (*g_engfuncs.pfnPrecacheModel)((char *)pszModelname);

#ifdef DEV_BUILD
	if (LastModel > HighestPrecache)
	{
		if (TotalModelPrecaches == 1)
		{
			modelout << "Brush entities: " << LastModel << endl;
			modelout << "Num\tIndex in Engine" << endl;
		}
		char NumStr[512];
		_snprintf(NumStr, sizeof(NumStr), "%.3i\t#%.3i - %s", TotalModelPrecaches, LastModel, pszModelname);
		modelout << NumStr << endl;
		HighestPrecache = LastModel;
		TotalModelPrecaches++;
	}
#endif

	return LastModel;
}

int ALLOC_STRING(const char *szValue) //Master Sword - Keep track of all string allocations
{
	return MAKE_STRING(g_StringPool.FindOrAllocate(szValue));
}

void ClearStringPool()
{
	g_StringPool.Clear();
}

void CSVGlobals::LogScript(const char* ScriptName, CBaseEntity *pOwner, int includelevel, bool PrecacheOnly, bool Sucess)
{
#ifdef DEV_BUILD
	if (!LogScripts ||
		!ms_devlog.value)
		return;

	int idx = 0;
	if (PrecacheOnly)
		idx = 3;
	else if (!pOwner)
		idx = 0;
	else if (pOwner->IsMSMonster())
		idx = 1;
	else
		idx = 2;

	scriptlistitem_t Item;
	Item.FileName = ScriptName;
	Item.Included = (includelevel > 1) ? true : false;

	ScriptList[idx].add(Item);
#endif
}

void CSVGlobals::WriteScriptLog()
{
	// TODO: use valve's filesystem instead.
#ifdef DEV_BUILD
	if (!ms_devlog.value)
		return;

	LogScripts = false; //Stop logging scripts

	ofstream scriptout;

	scriptout.open(msstring(EngineFunc::GetGameDir()) + "/log_scripts.txt");
	scriptout << "Scripts loaded for " << STRING(gpGlobals->mapname) << endl;

	int Total = 0;
	for (int i = 0; i < SCRIPT_TYPES; i++)
		Total += ScriptList[i].size();

	scriptout << "Total: " << Total << endl;

	for (int i = 0; i < SCRIPT_TYPES; i++)
	{
		scriptout << endl;

		const char* Name = "Global:";
		if (i == 1)
			Name = "Monsters:";
		else if (i == 2)
			Name = "Items:";
		else if (i == 3)
			Name = "Precache only:";

		scriptout << Name << endl;
		scriptout << "------------" << endl;
		scriptout << "Total: " << ScriptList[i].size() << endl;

		for (int s = 0; s < ScriptList[i].size(); s++)
		{
			CSVGlobals::scriptlistitem_t &ScriptListItem = ScriptList[i][s];
			if (ScriptListItem.Included)
				if (ms_devlog.value > 1)
					scriptout << "   ";
				else
					continue; // Admin chose not to log #included scripts

			scriptout << ScriptListItem.FileName.c_str() << endl;
		}

		ScriptList[i].clear(); //save ourselves some memory
	}
#endif
}
