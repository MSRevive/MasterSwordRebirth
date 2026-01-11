set(ZLIB_USE_STATIC_LIBS "ON")
set(ZLIB_ROOT "${BASEDIR}/lib/zlib")
find_package(ZLIB REQUIRED)

add_library(server SHARED $<$<PLATFORM_ID:Windows>:ms.def>)
add_library(libcurl STATIC IMPORTED)

set_target_properties(server PROPERTIES OUTPUT_NAME "ms")

target_include_directories(libcurl INTERFACE "${BASEDIR}/lib/include/curl")

if (WIN32)
	set_target_properties(libcurl PROPERTIES 
		IMPORTED_LOCATION "${BASEDIR}/lib/curl/libcurl.lib"
		INTERFACE_LINK_LIBRARIES "\$<LINK_ONLY:ZLIB::ZLIB>")
else()
	set_target_properties(libcurl PROPERTIES 
		IMPORTED_LOCATION "${BASEDIR}/lib/curl/libcurl.a"
		INTERFACE_LINK_LIBRARIES "\$<LINK_ONLY:ZLIB::ZLIB>")
endif()

set_common_properties(server)

target_link_libraries(server PRIVATE
	$<$<PLATFORM_ID:Windows>:Ws2_32>
	$<$<PLATFORM_ID:Windows>:Wldap32>
	$<$<PLATFORM_ID:Windows>:Crypt32>
	$<$<PLATFORM_ID:Windows>:Normaliz>
	${ZLIB_LIBRARIES}
	libcurl)

# Include directories
target_include_directories(server PRIVATE 
	${BASEDIR}/src/public
	${BASEDIR}/src/public/engine
	${BASEDIR}/src/common
	${BASEDIR}/src/game/client #defeats the purpose of shared, but that's MSR for ya.
	${BASEDIR}/src/game/shared
	${BASEDIR}/src/game/shared/movement
	${BASEDIR}/src/game/shared/ms
	${BASEDIR}/src/game/shared/stats
	${BASEDIR}/src/game/shared/syntax
	${BASEDIR}/src/game/shared/weapons
	${BASEDIR}/thirdparty
	${BASEDIR}/thirdparty/angelscript/include
	${BASEDIR}/thirdparty/spdlog/include
	${BASEDIR}/lib/include
	${CMAKE_CURRENT_SOURCE_DIR}
	effects
	gamerules
	hl
	monsters
	player
	fn
	${ZLIB_INCLUDE_DIRS})

target_compile_definitions(server PRIVATE
	VERSION_SAFE_STEAM_API_INTERFACES
	VALVE_DLL
	CURL_STATICLIB
	HTTP_ONLY)

# Check if spdlog is available
if(EXISTS "${BASEDIR}/thirdparty/spdlog/include/spdlog/spdlog.h")
	target_compile_definitions(server PRIVATE HAS_SPDLOG)
endif()

if (UNIX)
	target_compile_definitions(server PRIVATE
		SWDS)
endif(UNIX)

if (MSVC)
	set_target_properties(server PROPERTIES 
		VS_DEBUGGER_WORKING_DIRECTORY "${VS_DEBUG_WORK_DIR}"
		VS_DEBUGGER_COMMAND "${VS_DEBUG_WORK_DIR}/hlds.exe"
		VS_DEBUGGER_COMMAND_ARGUMENTS "${VS_DEBUG_CMD_ARGS}")
endif()



################################################################################
# Source groups
################################################################################
set(Common
	"../../common/const.h"
	"../../common/cstringpool.cpp"
	"../../common/cstringpool.h"
	"../../common/Platform.h"
	"../../common/PlatformWin.h"
	"../../common/enums.h"
)
source_group("Common" FILES ${Common})

set(Engine
	"../../public/engine/progdefs.h"
)
source_group("Engine" FILES ${Engine})

set(SDK
	"../../public/FileSystem.h"
	"../../public/interface.cpp"
	"../../public/interface.h"
)
source_group("SDK" FILES ${SDK})

set(HL1
	"basemonster.h"
	"bmodels.cpp"
	"cdll_dll.h"
	"client.cpp"
	"effects.cpp"
	"enginecallback.h"
	"func_break.cpp"
	"game.cpp"
	"gamerules/gamerules.cpp"
	"gamerules/gamerules.h"
	"gamerules/multiplay_gamerules.cpp"
	"ggrenade.cpp"
	"h_export.cpp"
	"hl/animating.cpp"
	"hl/animation.cpp"
	"hl/buttons.cpp"
	"hl/cbase.cpp"
	"hl/cbase.h"
	"hl/defaultai.cpp"
	"hl/doors.cpp"
	"hl/explode.cpp"
	"hl/extdll.h"
	"hl/func_tank.cpp"
	"hl/game.h"
	"hl/globals.cpp"
	"hl/lights.cpp"
	"hl/monsters.cpp"
	"hl/monsters.h"
	"hl/monsterstate.cpp"
	"hl/nodes.cpp"
	"hl/pathcorner.cpp"
	"hl/plane.cpp"
	"hl/plats.cpp"
	"hl/schedule.cpp"
	"hl/scripted.cpp"
	"hl/skill.cpp"
	"hl/sound.cpp"
	"hl/soundent.cpp"
	"hl/soundent.h"
	"hl/spectator.cpp"
	"hl/subs.cpp"
	"hl/util.cpp"
	"hl/util.h"
	"monsters/combat.cpp"
	"serverfindentities.cpp"
	"serverfindentities.h"
	"shurispritedefs.h"
	"triggers.cpp"
	"world.cpp"
	#"entities.cpp"
	#"entities.h"
	"../shared/movement/pm_defs.h"
	"../shared/movement/pm_math.cpp"
	"../shared/movement/pm_movevars.h"
	"../shared/voice_gamemgr.cpp"
)
source_group("HL1" FILES ${HL1})

set(MS
	"effects/mseffects.cpp"
	"entity.cpp"
	"gamerules/team.cpp"
	"magic.cpp"
	"monsters/bodyparts/bodyparts.cpp"
	"monsters/corpse.cpp"
	"monsters/monsteranimation.cpp"
	"monsters/msmonsterserver.cpp"
	"monsters/npcact.cpp"
	"monsters/npcglobal.cpp"
	"monsters/npcscript.cpp"
	"msmapents.cpp"
	"player/player.cpp"
	"player/playerstats.cpp"
	"shield.cpp"
	"store.cpp"
	"sv_character.cpp"
	"svglobals.cpp"
	#"SteamServerHelper.cpp"
	"../shared/movement/pm_shared.cpp"
	"../shared/movement/pm_shared.h"
	"../shared/ms/msmonstershared.cpp"
	"../shared/ms/netcodeshared.cpp"
	"../shared/stats/races.cpp"
	"../shared/syntax/syntax.cpp"
)
source_group("MS" FILES ${MS})

set(MS_Headers
	"effects/mseffects.h"
	"gamerules/teams.h"
	"monsters/bodyparts/bodyparts.h"
	"monsters/corpse.h"
	"monsters/monsteranimation.h"
	"monsters/msmonster.h"
	"msdllheaders.h"
	"msgamerules.h"
	"msitemdefs.h"
	"player/modeldefs.h"
	"player/player.h"
	"shield.h"
	"store.h"
	"svglobals.h"
	"vehicle.cpp"
	#"SteamServerHelper.h"
	"../shared/ms/groupfile.h"
	"../shared/ms/mscharacter.h"
	"../shared/ms/mscharacterheader.h"
	"../shared/ms/titles.h"
	"../shared/ms/votedefs.h"
	"../shared/stats/races.h"
	"../shared/stats/statdefs.h"
	"../shared/stats/stats.h"
)
source_group("MS\\Headers" FILES ${MS_Headers})

set(FN
	"fn/CreateCharacterReq.cpp"
	"fn/CreateCharacterReq.h"
	"fn/DeleteCharacterReq.cpp"
	"fn/DeleteCharacterReq.h"
	"fn/FNSharedDefs.cpp"
	"fn/FNSharedDefs.h"
	"fn/LoadCharacterReq.cpp"
	"fn/LoadCharacterReq.h"
	"fn/UpdateCharacterReq.cpp"
	"fn/UpdateCharacterReq.h"
	"fn/ValidateConReq.cpp"
	"fn/ValidateConReq.h"
	"fn/ValidateMapReq.cpp"
	"fn/ValidateMapReq.h"
	"fn/ValidateScriptsReq.cpp"
	"fn/ValidateScriptsReq.h"
	"fn/RequestManager.cpp"
	"fn/RequestManager.h"
	"fn/HTTPRequest.cpp"
	"fn/HTTPRequest.h"
)
source_group("FN" FILES ${FN})

set(Shared
	"player/playershared.cpp"
	"../shared/filesystem_shared.cpp"
	"../shared/filesystem_shared.h"
	"../shared/ms/buildcontrol.h"
	"../shared/ms/findentities.cpp"
	"../shared/ms/findentities.h"
	"../shared/ms/global.cpp"
	"../shared/ms/global.h"
	"../shared/ms/globalmem.cpp"
	"../shared/ms/groupfile.cpp"
	"../shared/ms/iscript.h"
	"../shared/ms/mslogger.cpp"
	"../shared/ms/mslogger.h"
	"../shared/ms/magic.h"
	"../shared/ms/mscharacter.cpp"
	"../shared/ms/msdebug.h"
	"../shared/ms/msfileio.cpp"
	"../shared/ms/msfileio.h"
	"../shared/ms/saytext.h"
	"../shared/ms/script.cpp"
	"../shared/ms/script.h"
	"../shared/ms/scriptcmds.cpp"
	"../shared/ms/scriptedeffects.cpp"
	"../shared/ms/scriptedeffects.h"
	"../shared/ms/scriptmgr.cpp"
	"../shared/ms/scriptmgr.h"
	"../shared/ms/sharedutil.cpp"
	"../shared/ms/sharedutil.h"
	"../shared/ms/stackstring.cpp"
	"../shared/ms/stackstring.h"
	"../shared/ms/storeshared.h"
	"../shared/ms/vgui_menudefsshared.h"
	"../shared/stats/stats.cpp"
	"../shared/strhelper.cpp"
	"../shared/strhelper.h"
)
source_group("Shared" FILES ${Shared})

set(Shared_CRC
	"../shared/ms/crc/checksum_crc.cpp"
	"../shared/ms/crc/checksum_crc.h"
	"../shared/ms/crc/crchash.cpp"
	"../shared/ms/crc/crchash.h"
)
source_group("Shared\\CRC" FILES ${Shared_CRC})

set(Shared_Generic_Item
	"../shared/genitemlist.cpp"
	"../shared/genitemlist.h"
	"../shared/weapons/genericitem.cpp"
	"../shared/weapons/genericitem.h"
	"../shared/weapons/giarmor.cpp"
	"../shared/weapons/giattack.cpp"
	"../shared/weapons/gidrinkable.cpp"
	"../shared/weapons/gipack.cpp"
	"../shared/weapons/giprojectile.cpp"
	"../shared/weapons/gispawn.cpp"
	"../shared/weapons/gispell.cpp"
	"../shared/weapons/weapons.cpp"
	"../shared/weapons/weapons.h"
)
source_group("Shared\\Generic Item" FILES ${Shared_Generic_Item})

set(Shared_base64
	"../../../thirdparty/base64/base64.cpp"
	"../../../thirdparty/base64/base64.h"
)
source_group("Shared\\base64" FILES ${Shared_base64})

set(Shared_AngelScript
	"../shared/ms/angelscript/CAngelScriptManager.cpp"
	"../shared/ms/angelscript/CAngelScriptManager.h"
	"../shared/ms/angelscript/IAngelScript.cpp"
	"../shared/ms/angelscript/IAngelScript.h"
	"../shared/ms/angelscript/CAngelScript.cpp"
	"../shared/ms/angelscript/CAngelScript.h"
	"../shared/ms/angelscript/ASTest.cpp"
	"../shared/ms/angelscript/ASTest.h"
	"../shared/ms/angelscript/ASCoreTypes.cpp"
	"../shared/ms/angelscript/ASCoreTypes.h"
	"../shared/ms/angelscript/ASEntityBindings.cpp"
	"../shared/ms/angelscript/ASEntityBindings.h"
	"../shared/ms/angelscript/ASItemBindings.cpp"
	"../shared/ms/angelscript/ASItemBindings.h"
	"../shared/ms/angelscript/ASMonsterBindings.cpp"
	"../shared/ms/angelscript/ASMonsterBindings.h"
	"../shared/ms/angelscript/ASEngineProvider.cpp"
	"../shared/ms/angelscript/ASEngineInterface.h"
	"../shared/ms/angelscript/ASEngineBindings.h"
	"../shared/ms/angelscript/ASBuiltinFunctions.cpp"
	"../shared/ms/angelscript/ASBuiltinFunctions.h"
	"../shared/ms/angelscript/ASBindings.cpp"
	"../shared/ms/angelscript/ASBindings.h"
	"../shared/ms/angelscript/ASCoroutines.cpp"
	"../shared/ms/angelscript/ASCoroutines.h"
	"../shared/ms/angelscript/ASDebugger.cpp"
	"../shared/ms/angelscript/ASDebugger.h"
	"../shared/ms/angelscript/ASModuleSystem.cpp"
	"../shared/ms/angelscript/ASModuleSystem.h"
	"../shared/ms/angelscript/ASObjectPool.cpp"
	"../shared/ms/angelscript/ASObjectPool.h"
	"../shared/ms/angelscript/ASScriptContext.cpp"
	"../shared/ms/angelscript/ASScriptContext.h"
	"../shared/ms/angelscript/ASBindExtensions.h"
	"../shared/ms/angelscript/ASScriptClasses.cpp"
	"../shared/ms/angelscript/ASScriptClasses.h"
	"../shared/ms/angelscript/ASEngineEventManager.cpp"
	"../shared/ms/angelscript/ASEngineEventManager.h"
)
source_group("Shared\\AngelScript" FILES ${Shared_AngelScript})

set(Server_AngelScript
	"angelscript/ASServerEngineImpl.cpp"
	"angelscript/ASServerEngineImpl.h"
)
source_group("Server\\AngelScript" FILES ${Server_AngelScript})

set(ThirdParty_AngelScript
	"../../../thirdparty/angelscript/as_atomic.cpp"
	"../../../thirdparty/angelscript/as_builder.cpp"
	"../../../thirdparty/angelscript/as_bytecode.cpp"
	"../../../thirdparty/angelscript/as_callfunc.cpp"
	"../../../thirdparty/angelscript/as_callfunc_x86.cpp"
	"../../../thirdparty/angelscript/as_callfunc_x64_msvc.cpp"
	"../../../thirdparty/angelscript/as_compiler.cpp"
	"../../../thirdparty/angelscript/as_configgroup.cpp"
	"../../../thirdparty/angelscript/as_context.cpp"
	"../../../thirdparty/angelscript/as_datatype.cpp"
	"../../../thirdparty/angelscript/as_gc.cpp"
	"../../../thirdparty/angelscript/as_generic.cpp"
	"../../../thirdparty/angelscript/as_globalproperty.cpp"
	"../../../thirdparty/angelscript/as_memory.cpp"
	"../../../thirdparty/angelscript/as_module.cpp"
	"../../../thirdparty/angelscript/as_objecttype.cpp"
	"../../../thirdparty/angelscript/as_outputbuffer.cpp"
	"../../../thirdparty/angelscript/as_parser.cpp"
	"../../../thirdparty/angelscript/as_restore.cpp"
	"../../../thirdparty/angelscript/as_scriptcode.cpp"
	"../../../thirdparty/angelscript/as_scriptengine.cpp"
	"../../../thirdparty/angelscript/as_scriptfunction.cpp"
	"../../../thirdparty/angelscript/as_scriptnode.cpp"
	"../../../thirdparty/angelscript/as_scriptobject.cpp"
	"../../../thirdparty/angelscript/as_string.cpp"
	"../../../thirdparty/angelscript/as_string_util.cpp"
	"../../../thirdparty/angelscript/as_thread.cpp"
	"../../../thirdparty/angelscript/as_tokenizer.cpp"
	"../../../thirdparty/angelscript/as_typeinfo.cpp"
	"../../../thirdparty/angelscript/as_variablescope.cpp"
	# AngelScript add-ons
	"../shared/ms/angelscript/addons/scriptstdstring/scriptstdstring.cpp"
	"../shared/ms/angelscript/addons/scriptstdstring/scriptstdstring_utils.cpp"
	"../shared/ms/angelscript/addons/scriptarray/scriptarray.cpp"
	"../shared/ms/angelscript/addons/scriptdictionary/scriptdictionary.cpp"
	"../shared/ms/angelscript/addons/scriptbuilder/scriptbuilder.cpp"
	"../shared/ms/angelscript/addons/scriptmodule/scriptmodule.cpp"
)
source_group("ThirdParty\\AngelScript" FILES ${ThirdParty_AngelScript})

set(ALL_FILES
	${Common}
	${Engine}
	${SDK}
	${HL1}
	${MS}
	${MS_Headers}
	${FN}
	${Shared}
	${Shared_CRC}
	${Shared_Generic_Item}
	${Shared_base64}
	${Shared_AngelScript}
	${Server_AngelScript}
	${ThirdParty_AngelScript}
)

target_sources(server PRIVATE ${ALL_FILES})

install(TARGETS server
	DESTINATION "msr/dlls"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE)
