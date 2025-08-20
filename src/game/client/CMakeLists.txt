add_library(vgui SHARED IMPORTED)
add_library(SDL2 SHARED IMPORTED)
add_library(fmod SHARED IMPORTED)
add_library(discord SHARED IMPORTED)
add_library(steam_api SHARED IMPORTED)

# for some reason it can't find these libraries, even when installed?
if (LINUX)
	set(OPENGL_gl_LIBRARY /usr/lib/i386-linux-gnu/libGL.so)
	set(OPENGL_glx_LIBRARY /usr/lib/i386-linux-gnu/libGLX.so)
endif()

find_package(OpenGL REQUIRED)

target_include_directories(vgui INTERFACE ${BASEDIR}/lib/include/vgui)
target_include_directories(SDL2 INTERFACE ${BASEDIR}/lib/include/SDL2)
target_include_directories(fmod INTERFACE ${BASEDIR}/lib/include/fmod)
target_include_directories(discord INTERFACE ${BASEDIR}/lib/include/discord)
target_include_directories(steam_api INTERFACE ${BASEDIR}/src/public/steam)

if (WIN32)
	set_target_properties(vgui PROPERTIES IMPORTED_IMPLIB "${BASEDIR}/lib/vgui/vgui.lib")
	set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB "${BASEDIR}/lib/SDL2/SDL2.lib")
	set_target_properties(fmod PROPERTIES IMPORTED_IMPLIB "${BASEDIR}/lib/fmod/fmod_vc.lib")
	set_target_properties(discord PROPERTIES IMPORTED_IMPLIB "${BASEDIR}/lib/discord/discord-rpc.lib")
	set_target_properties(steam_api PROPERTIES IMPORTED_IMPLIB "${BASEDIR}/lib/steamapi/steam_api.lib")
else()
	set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION "${BASEDIR}/lib/SDL2/libSDL2.so")
	set_target_properties(fmod PROPERTIES IMPORTED_LOCATION "${BASEDIR}/lib/fmod/libfmod.so")
	set_target_properties(discord PROPERTIES IMPORTED_LOCATION "${BASEDIR}/lib/discord/libdiscord-rpc.a")
	set_target_properties(steam_api PROPERTIES IMPORTED_LOCATION "${BASEDIR}/lib/steamapi/libsteam_api.so")
endif()

add_library(client SHARED)

set_common_properties(client)

if (MSVC)
	target_link_options(client PRIVATE /SAFESEH:NO)
else()
	# vgui.so has no SONAME and we it must be linked through a -l: flag
	# to avoid vgui.so linked by the absolute path
	# and CMake refuses to use -l: instead of -l (which makes linker to expect `lib` prefix)
	# yes, in 2024 it's still a bug!
	target_link_libraries(client PRIVATE "-L${BASEDIR}/lib/vgui -l:vgui.so")
endif()

target_link_libraries(client PRIVATE
	$<$<PLATFORM_ID:Windows>:wsock32>
	$<$<PLATFORM_ID:Windows>:vgui>
	$<$<PLATFORM_ID:Windows>:gdi32>
	SDL2
	fmod
	discord
	steam_api
	${OPENGL_LIBRARIES})

target_include_directories(client PRIVATE
	${BASEDIR}/lib/include
	${BASEDIR}/lib/include/vgui
	${BASEDIR}/src/public
	${BASEDIR}/src/public/engine
	${BASEDIR}/src/common
	${BASEDIR}/src/game/shared
	${BASEDIR}/src/game/shared/movement
	${BASEDIR}/src/game/shared/ms
	${BASEDIR}/src/game/shared/stats
	${BASEDIR}/src/game/shared/syntax
	${BASEDIR}/src/game/shared/weapons
	${BASEDIR}/src/game/server
	${BASEDIR}/src/game/server/hl
	${BASEDIR}/thirdparty
	${BASEDIR}/thirdparty/angelscript/include
	${BASEDIR}/thirdparty/spdlog/include
	${CMAKE_CURRENT_SOURCE_DIR}
	ms
	ui
	ui/ms
	${OPENGL_INCLUDE_DIR})

target_compile_definitions(client PRIVATE
	ISCLIENT
	CLIENT_WEAPONS
	DISCORD_DYNAMIC_LIB
	VERSION_SAFE_STEAM_API_INTERFACES
	USE_BREAKPAD_HANDLER)

# Check if spdlog is available
if(EXISTS "${BASEDIR}/thirdparty/spdlog/include/spdlog/spdlog.h")
	target_compile_definitions(client PRIVATE HAS_SPDLOG)
endif()

if (MSVC)
	set_target_properties(client PROPERTIES 
		VS_DEBUGGER_WORKING_DIRECTORY "${VS_DEBUG_WORK_DIR}"
		VS_DEBUGGER_COMMAND "${VS_DEBUG_WORK_DIR}/msr.exe)")
endif()

################################################################################
# Source groups
################################################################################
set(Common
	"../../common/mathlib.h"
	"../../common/cl_entity.h"
	"../../common/com_model.h"
	"../../common/const.h"
	"../../common/dlight.h"
	"../../common/entity_state.h"
	"../../common/Platform.h"
	"../../common/PlatformWin.h"
	"../../common/const.h"
)
source_group("Common" FILES ${Common})

set(Engine
	"../../public/engine/cdll_int.h"
)
source_group("Engine" FILES ${Engine})

set(SDK
	"exports.h"
	"../../public/archtypes.h"
	"../../public/cl_dll/IGameClientExports.h"
	"../../public/FileSystem.h"
	"../../public/interface.cpp"
	"../../public/interface.h"
	"../../public/keydefs.h"
	"../../public/steam/steamtypes.h"
)
source_group("SDK" FILES ${SDK})

set(HL1
	"ammo_secondary.cpp"
	"ammohistory.cpp"
	"ammohistory.h"
	"cdll_int.cpp"
	"clientfindentities.cpp"
	"clientfindentities.h"
	"clientlibrary.cpp"
	"clientlibrary.h"
	"com_weapons.cpp"
	"death.cpp"
	"demo.cpp"
	"ev_common.cpp"
	"ev_hldm.cpp"
	"events.cpp"
	"hl/hl_baseentity.cpp"
	"hl/hl_events.cpp"
	"hud.cpp"
	"hud.h"
	"hud_msg.cpp"
	"hud_redraw.cpp"
	"hud_spectator.cpp"
	"hud_spectator.h"
	"hud_update.cpp"
	"in_camera.cpp"
	"input.cpp"
	"inputw32.cpp"
	"message.cpp"
	"ms/hudbase.h"
	"parsemsg.cpp"
	"parsemsg.h"
	"saytext.cpp"
	"status_icons.cpp"
	"statusbar.cpp"
	"text_message.cpp"
	"train.cpp"
	"util.cpp"
	"view.cpp"
	"cl_util.h"
	"../shared/movement/pm_math.cpp"
	"../shared/voice_banmgr.cpp"
	"../shared/voice_status.cpp"
)
source_group("HL1" FILES ${HL1})

set(FMOD
	"fmod/musicsystem.cpp"
	"fmod/musicsystem.h"
	"fmod/soundengine.cpp"
	"fmod/soundengine.h"
)
source_group("FMOD" FILES ${FMOD})

set(MS
	"SteamClientHelper.cpp"
	"ASClientEngine.cpp"
	"entity.cpp"
	"ms/action.cpp"
	"ms/clglobal.cpp"
	"ms/clplayer.cpp"
	"ms/fatigue.cpp"
	"ms/health.cpp"
	"ms/hudid.cpp"
	"ms/hudmagic.cpp"
	"ms/hudmisc.cpp"
	"ms/hudmusic.cpp"
	"ms/hudscript.cpp"
	"ms/menu.cpp"
	"richpresence.cpp"
	"../shared/movement/pm_shared.cpp"
)
source_group("MS" FILES ${MS})

set(MS_Headers
	"inc_huditem.h"
	"ms/clenv.h"
	"ms/clglobal.h"
	"ms/health.h"
	"ms/hudid.h"
	"ms/hudmagic.h"
	"ms/hudmisc.h"
	"ms/hudmusic.h"
	"ms/hudscript.h"
	"ms/menu.h"
	"richpresence.h"
	"SteamClientHelper.h"
	"../server/monsters/msmonster.h"
	"../server/player/player.h"
	"../shared/ms/global.h"
	"../shared/ms/groupfile.h"
	"../shared/ms/msfileio.h"
	"../shared/ms/saytext.h"
	"../shared/stats/statdefs.h"
)
source_group("MS\\Headers" FILES ${MS_Headers})

set(MS_Generic_Item
	"../shared/genitemlist.cpp"
	"../shared/genitemlist.h"
	"../shared/weapons/genericitem.cpp"
	"../shared/weapons/genericitem.h"
	"../shared/weapons/giarmor.cpp"
	"../shared/weapons/giattack.cpp"
	"../shared/weapons/gidrinkable.cpp"
	"../shared/weapons/gipack.cpp"
	"../shared/weapons/weapons.cpp"
	"../shared/weapons/weapons.h"
)
source_group("MS\\Generic Item" FILES ${MS_Generic_Item})

set(Render
	"render/clrender.cpp"
	"render/clrender.h"
	"render/clrenderent.cpp"
	"render/clrendermirror.cpp"
	"render/gamestudiomodelrenderer.cpp"
	"render/gamestudiomodelrenderer.h"
	"render/opengl/clopengl.cpp"
	"render/opengl/clopengl.h"
	"render/studio_util.cpp"
	"render/studio_util.h"
	"render/studiomodelrenderer.cpp"
	"render/studiomodelrenderer.h"
	"../../../thirdparty/tartan/textureloader.cpp"
	"../../../thirdparty/tartan/tgaloader.cpp"
	"../../../thirdparty/gl/glext.h"
	"../../../thirdparty/gl/wglext.h"
)
source_group("Render" FILES ${Render})

set(Shared
	"../server/player/playershared.cpp"
	"../server/store.h"
	"../shared/filesystem_shared.cpp"
	"../shared/filesystem_shared.h"
	"../shared/ms/buildcontrol.h"
	"../shared/ms/findentities.cpp"
	"../shared/ms/findentities.h"
	"../shared/ms/global.cpp"
	"../shared/ms/globalmem.cpp"
	"../shared/ms/groupfile.cpp"
	"../shared/ms/logger.h"
	"../shared/ms/mslogger.cpp"
	"../shared/ms/mslogger.h"
	"../shared/ms/mscharacter.cpp"
	"../shared/ms/msdebug.h"
	"../shared/ms/msfileio.cpp"
	"../shared/ms/msmonstershared.cpp"
	"../shared/ms/netcodeshared.cpp"
	"../shared/ms/script.cpp"
	"../shared/ms/script.h"
	"../shared/ms/scriptcmds.cpp"
	"../shared/ms/scriptedeffects.cpp"
	"../shared/ms/scriptedeffects.h"
	"../shared/ms/scriptmgr.cpp"
	"../shared/ms/scriptmgr.h"
	"../shared/ms/sharedmenu.h"
	"../shared/ms/sharedutil.cpp"
	"../shared/ms/sharedutil.h"
	"../shared/ms/stackstring.cpp"
	"../shared/ms/stackstring.h"
	"../shared/ms/votedefs.h"
	"../shared/stats/races.cpp"
	"../shared/stats/stats.cpp"
	"../shared/strhelper.cpp"
	"../shared/strhelper.h"
	"../shared/syntax/syntax.cpp"
)
source_group("Shared" FILES ${Shared})

set(Shared_CRC
	"../shared/ms/crc/checksum_crc.cpp"
	"../shared/ms/crc/checksum_crc.h"
	"../shared/ms/crc/crchash.cpp"
	"../shared/ms/crc/crchash.h"
)
source_group("Shared\\CRC" FILES ${Shared_CRC})

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
	"../shared/ms/angelscript/ASBuiltinFunctions.cpp"
	"../shared/ms/angelscript/ASBuiltinFunctions.h"
	"../shared/ms/angelscript/ASBindings.cpp"
	"../shared/ms/angelscript/ASBindings.h"
	"../shared/ms/angelscript/ASEntityBindings.cpp"
	"../shared/ms/angelscript/ASEntityBindings.h"
	"../shared/ms/angelscript/ASCoroutines.cpp"
	"../shared/ms/angelscript/ASCoroutines.h"
	"../shared/ms/angelscript/ASDebugger.cpp"
	"../shared/ms/angelscript/ASDebugger.h"
	"../shared/ms/angelscript/ASModuleSystem.cpp"
	"../shared/ms/angelscript/ASModuleSystem.h"
	"../shared/ms/angelscript/ASObjectPool.cpp"
	"../shared/ms/angelscript/ASObjectPool.h"
	"../shared/ms/angelscript/ASScriptClasses.cpp"
	"../shared/ms/angelscript/ASScriptClasses.h"
	"../shared/ms/angelscript/ASEngineProvider.cpp"
	"../shared/ms/angelscript/ASEngineInterface.h"
	"../shared/ms/angelscript/ASEngineBindings.h"
	"../shared/ms/angelscript/ASEngineEventManager.cpp"
	"../shared/ms/angelscript/ASEngineEventManager.h"
)
source_group("Shared\\AngelScript" FILES ${Shared_AngelScript})

set(Client_AngelScript
	"angelscript/ASClientEngineImpl.cpp"
	"angelscript/ASClientEngineImpl.h"
)
source_group("Client\\AngelScript" FILES ${Client_AngelScript})

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

set(VGUI
	"ui/vgui_checkbutton2.cpp"
	"ui/vgui_controlconfigpanel.cpp"
	"ui/vgui_controlconfigpanel.h"
	"ui/vgui_customobjects.cpp"
	"ui/vgui_global.cpp"
	"ui/vgui_global.h"
	"ui/vgui_grid.cpp"
	"ui/vgui_helpers.cpp"
	"ui/vgui_int.cpp"
	"ui/vgui_int.h"
	"ui/vgui_listbox.cpp"
	"ui/vgui_loadtga.cpp"
	"ui/vgui_motdwindow.cpp"
	"ui/vgui_schememanager.cpp"
	"ui/vgui_schememanager.h"
	"ui/vgui_scorepanel.cpp"
	"ui/vgui_scorepanel.h"
	"ui/vgui_scrollbar2.cpp"
	"ui/vgui_slider2.cpp"
	"ui/vgui_spectatorpanel.cpp"
	"ui/vgui_status.h"
	"ui/vgui_teamfortressviewport.cpp"
	"ui/vgui_teamfortressviewport.h"
	"ui/vgui_voice_tweakdlg.cpp"
)
source_group("VGUI" FILES ${VGUI})

set(VGUI_MS
	"ui/ms/vgui_ihud.h"
	"ui/ms/vgui_choosecharacter.cpp"
	"ui/ms/vgui_choosecharacter.h"
	"ui/ms/vgui_container.cpp"
	"ui/ms/vgui_container.h"
	"ui/ms/vgui_containerlist.cpp"
	"ui/ms/vgui_containerlist.h"
	"ui/ms/vgui_eventconsole.h"
	"ui/ms/vgui_health.h"
	"ui/ms/vgui_healthretro.h"
	"ui/ms/vgui_hud.cpp"
	"ui/ms/vgui_hud.h"
	"ui/ms/vgui_id.h"
	"ui/ms/vgui_infowin.h"
	"ui/ms/vgui_localizedpanel.cpp"
	"ui/ms/vgui_localizedpanel.h"
	"ui/ms/vgui_menu_interact.h"
	"ui/ms/vgui_menu_main.h"
	"ui/ms/vgui_menubase.cpp"
	"ui/ms/vgui_menubase.h"
	"ui/ms/vgui_mscontrols.cpp"
	"ui/ms/vgui_mscontrols.h"
	"ui/ms/vgui_options.cpp"
	"ui/ms/vgui_options.h"
	"ui/ms/vgui_quickslot.h"
	"ui/ms/vgui_spawn.cpp"
	"ui/ms/vgui_spawn.h"
	"ui/ms/vgui_startsaytext.h"
	"ui/ms/vgui_stats.cpp"
	"ui/ms/vgui_stats.h"
	"ui/ms/vgui_storage.cpp"
	"ui/ms/vgui_storage.h"
	"ui/ms/vgui_store.cpp"
	"ui/ms/vgui_storebuy.cpp"
	"ui/ms/vgui_storebuy.h"
	"ui/ms/vgui_storemainwin.cpp"
	"ui/ms/vgui_storemainwin.h"
	"ui/ms/vgui_storesell.cpp"
	"ui/ms/vgui_storesell.h"
	"ui/ms/vgui_vote.h"
)
source_group("VGUI\\ms" FILES ${VGUI_MS})

set(ALL_FILES
	${Common}
	${Engine}
	${SDL2}
	${SDK}
	${HL1}
	${FMOD}
	${MS}
	${MS_Headers}
	${MS_Generic_Item}
	${Render}
	${Shared}
	${Shared_CRC}
	${Shared_base64}
	${Shared_AngelScript}
	${Client_AngelScript}
	${ThirdParty_AngelScript}
	${VGUI}
	${VGUI_MS}
)

target_sources(client PRIVATE ${ALL_FILES})
install(TARGETS client
	DESTINATION "msr/cl_dlls"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE)
