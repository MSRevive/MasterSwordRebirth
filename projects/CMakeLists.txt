set(PROJECT_NAME Client)

################################################################################
# Source groups
################################################################################
set(FMOD
    "../game/client/fmod/musicsystem.cpp"
    "../game/client/fmod/musicsystem.h"
    "../game/client/fmod/soundengine.cpp"
    "../game/client/fmod/soundengine.h"
)
source_group("FMOD" FILES ${FMOD})

set(HL1
    "../common/mathlib.h"
    "../game/client/ammo_secondary.cpp"
    "../game/client/ammohistory.cpp"
    "../game/client/ammohistory.h"
    "../game/client/cdll_int.cpp"
    "../game/client/clientfindentities.cpp"
    "../game/client/clientfindentities.h"
    "../game/client/clientlibrary.cpp"
    "../game/client/clientlibrary.h"
    "../game/client/com_weapons.cpp"
    "../game/client/death.cpp"
    "../game/client/demo.cpp"
    "../game/client/ev_common.cpp"
    "../game/client/ev_hldm.cpp"
    "../game/client/events.cpp"
    "../game/client/flashlight.cpp"
    "../game/client/geiger.cpp"
    "../game/client/hl/hl_baseentity.cpp"
    "../game/client/hl/hl_events.cpp"
    "../game/client/hud.cpp"
    "../game/client/hud.h"
    "../game/client/hud_msg.cpp"
    "../game/client/hud_redraw.cpp"
    "../game/client/hud_spectator.cpp"
    "../game/client/hud_update.cpp"
    "../game/client/in_camera.cpp"
    "../game/client/input.cpp"
    "../game/client/inputw32.cpp"
    "../game/client/message.cpp"
    "../game/client/ms/hudbase.h"
    "../game/client/parsemsg.cpp"
    "../game/client/parsemsg.h"
    "../game/client/saytext.cpp"
    "../game/client/soundsystem.cpp"
    "../game/client/status_icons.cpp"
    "../game/client/statusbar.cpp"
    "../game/client/text_message.cpp"
    "../game/client/train.cpp"
    "../game/client/util.cpp"
    "../game/client/view.cpp"
    "../game/shared/cbase.h"
    "../game/shared/movement/pm_math.c"
    "../game/shared/voice_banmgr.cpp"
    "../game/shared/voice_status.cpp"
)
source_group("HL1" FILES ${HL1})

set(HL1__Headers
    "../common/cl_entity.h"
    "../common/com_model.h"
    "../common/const.h"
    "../common/dlight.h"
    "../common/entity_state.h"
    "../game/client/cl_util.h"
    "../game/client/hud_spectator.h"
    "../public/engine/cdll_int.h"
)
source_group("HL1\\Headers" FILES ${HL1__Headers})

set(MS
    "../game/client/entity.cpp"
    "../game/client/ms/action.cpp"
    "../game/client/ms/clglobal.cpp"
    "../game/client/ms/clplayer.cpp"
    "../game/client/ms/fatigue.cpp"
    "../game/client/ms/health.cpp"
    "../game/client/ms/hudid.cpp"
    "../game/client/ms/hudmagic.cpp"
    "../game/client/ms/hudmisc.cpp"
    "../game/client/ms/hudmusic.cpp"
    "../game/client/ms/hudscript.cpp"
    "../game/client/ms/menu.cpp"
    "../game/client/richpresence.cpp"
    "../game/client/richpresence.h"
    "../game/shared/movement/pm_shared.cpp"
    "../thirdparty/tartan/textureloader.cpp"
    "../thirdparty/tartan/tgaloader.cpp"
)
source_group("MS" FILES ${MS})

set(MS__Generic_Item
    "../game/shared/genitemlist.cpp"
    "../game/shared/genitemlist.h"
    "../game/shared/weapons/genericitem.cpp"
    "../game/shared/weapons/genericitem.h"
    "../game/shared/weapons/giarmor.cpp"
    "../game/shared/weapons/giattack.cpp"
    "../game/shared/weapons/gidrinkable.cpp"
    "../game/shared/weapons/gipack.cpp"
    "../game/shared/weapons/weapons.cpp"
    "../game/shared/weapons/weapons.h"
)
source_group("MS\\Generic Item" FILES ${MS__Generic_Item})

set(MS__Headers
    "../game/client/inc_huditem.h"
    "../game/client/ms/clenv.h"
    "../game/client/ms/clglobal.h"
    "../game/client/ms/health.h"
    "../game/client/ms/hudid.h"
    "../game/client/ms/hudmagic.h"
    "../game/client/ms/hudmisc.h"
    "../game/client/ms/hudmusic.h"
    "../game/client/ms/hudscript.h"
    "../game/client/ms/menu.h"
    "../game/server/monsters/msmonster.h"
    "../game/server/player/player.h"
    "../game/shared/ms/global.h"
    "../game/shared/ms/groupfile.h"
    "../game/shared/ms/msfileio.h"
    "../game/shared/ms/saytext.h"
    "../game/shared/stats/statdefs.h"
)
source_group("MS\\Headers" FILES ${MS__Headers})

set(Misc
    "../public/tier0/pointeroverride.asm"
)
source_group("Misc" FILES ${Misc})

set(Render
    "../game/client/render/clrender.cpp"
    "../game/client/render/clrender.h"
    "../game/client/render/clrenderent.cpp"
    "../game/client/render/clrendermirror.cpp"
    "../game/client/render/gamestudiomodelrenderer.cpp"
    "../game/client/render/gamestudiomodelrenderer.h"
    "../game/client/render/opengl/clopengl.cpp"
    "../game/client/render/opengl/clopengl.h"
    "../game/client/render/studio_util.cpp"
    "../game/client/render/studio_util.h"
    "../game/client/render/studiomodelrenderer.cpp"
    "../game/client/render/studiomodelrenderer.h"
)
source_group("Render" FILES ${Render})

set(SDK2013
    "../game/client/exports.h"
    "../public/archtypes.h"
    "../public/cl_dll/IGameClientExports.h"
    "../public/FileSystem.h"
    "../public/interface.cpp"
    "../public/interface.h"
    "../public/keydefs.h"
    "../public/particleman.h"
    "../public/pman_particlemem.h"
    "../public/pman_triangleffect.h"
    "../public/steam/steamtypes.h"
    "../thirdparty/SDL2/begin_code.h"
    "../thirdparty/SDL2/close_code.h"
    "../thirdparty/SDL2/SDL.h"
    "../thirdparty/SDL2/SDL_assert.h"
    "../thirdparty/SDL2/SDL_atomic.h"
    "../thirdparty/SDL2/SDL_audio.h"
    "../thirdparty/SDL2/SDL_bits.h"
    "../thirdparty/SDL2/SDL_blendmode.h"
    "../thirdparty/SDL2/SDL_clipboard.h"
    "../thirdparty/SDL2/SDL_config.h"
    "../thirdparty/SDL2/SDL_config_android.h"
    "../thirdparty/SDL2/SDL_config_iphoneos.h"
    "../thirdparty/SDL2/SDL_config_macosx.h"
    "../thirdparty/SDL2/SDL_config_minimal.h"
    "../thirdparty/SDL2/SDL_config_nintendods.h"
    "../thirdparty/SDL2/SDL_config_pandora.h"
    "../thirdparty/SDL2/SDL_config_windows.h"
    "../thirdparty/SDL2/SDL_config_wiz.h"
    "../thirdparty/SDL2/SDL_copying.h"
    "../thirdparty/SDL2/SDL_cpuinfo.h"
    "../thirdparty/SDL2/SDL_endian.h"
    "../thirdparty/SDL2/SDL_error.h"
    "../thirdparty/SDL2/SDL_events.h"
    "../thirdparty/SDL2/SDL_gamecontroller.h"
    "../thirdparty/SDL2/SDL_gesture.h"
    "../thirdparty/SDL2/SDL_haptic.h"
    "../thirdparty/SDL2/SDL_hints.h"
    "../thirdparty/SDL2/SDL_input.h"
    "../thirdparty/SDL2/SDL_joystick.h"
    "../thirdparty/SDL2/SDL_keyboard.h"
    "../thirdparty/SDL2/SDL_keycode.h"
    "../thirdparty/SDL2/SDL_loadso.h"
    "../thirdparty/SDL2/SDL_log.h"
    "../thirdparty/SDL2/SDL_main.h"
    "../thirdparty/SDL2/SDL_messagebox.h"
    "../thirdparty/SDL2/SDL_mouse.h"
    "../thirdparty/SDL2/SDL_mutex.h"
    "../thirdparty/SDL2/SDL_name.h"
    "../thirdparty/SDL2/SDL_opengl.h"
    "../thirdparty/SDL2/SDL_opengles.h"
    "../thirdparty/SDL2/SDL_opengles2.h"
    "../thirdparty/SDL2/SDL_pixels.h"
    "../thirdparty/SDL2/SDL_platform.h"
    "../thirdparty/SDL2/SDL_power.h"
    "../thirdparty/SDL2/SDL_quit.h"
    "../thirdparty/SDL2/SDL_rect.h"
    "../thirdparty/SDL2/SDL_render.h"
    "../thirdparty/SDL2/SDL_revision.h"
    "../thirdparty/SDL2/SDL_rwops.h"
    "../thirdparty/SDL2/SDL_scancode.h"
    "../thirdparty/SDL2/SDL_shape.h"
    "../thirdparty/SDL2/SDL_stdinc.h"
    "../thirdparty/SDL2/SDL_surface.h"
    "../thirdparty/SDL2/SDL_system.h"
    "../thirdparty/SDL2/SDL_syswm.h"
    "../thirdparty/SDL2/SDL_test.h"
    "../thirdparty/SDL2/SDL_test_assert.h"
    "../thirdparty/SDL2/SDL_test_common.h"
    "../thirdparty/SDL2/SDL_test_compare.h"
    "../thirdparty/SDL2/SDL_test_crc32.h"
    "../thirdparty/SDL2/SDL_test_font.h"
    "../thirdparty/SDL2/SDL_test_fuzzer.h"
    "../thirdparty/SDL2/SDL_test_harness.h"
    "../thirdparty/SDL2/SDL_test_images.h"
    "../thirdparty/SDL2/SDL_test_log.h"
    "../thirdparty/SDL2/SDL_test_md5.h"
    "../thirdparty/SDL2/SDL_test_random.h"
    "../thirdparty/SDL2/SDL_thread.h"
    "../thirdparty/SDL2/SDL_timer.h"
    "../thirdparty/SDL2/SDL_touch.h"
    "../thirdparty/SDL2/SDL_types.h"
    "../thirdparty/SDL2/SDL_version.h"
    "../thirdparty/SDL2/SDL_video.h"
)
source_group("SDK2013" FILES ${SDK2013})

set(Shared
    "../game/client/steamhelper.cpp"
    "../game/client/steamhelper.h"
    "../game/server/player/playershared.cpp"
    "../game/server/store.h"
    "../game/shared/ms/buildcontrol.h"
    "../game/shared/ms/dbginputs.cpp"
    "../game/shared/ms/filesystem_shared.cpp"
    "../game/shared/ms/filesystem_shared.h"
    "../game/shared/ms/findentities.cpp"
    "../game/shared/ms/findentities.h"
    "../game/shared/ms/global.cpp"
    "../game/shared/ms/globalmem.cpp"
    "../game/shared/ms/groupfile.cpp"
    "../game/shared/ms/logger.h"
    "../game/shared/ms/msbasic.h"
    "../game/shared/ms/mscharacter.cpp"
    "../game/shared/ms/msdebug.h"
    "../game/shared/ms/msfileio.cpp"
    "../game/shared/ms/msmonstershared.cpp"
    "../game/shared/ms/netcodeshared.cpp"
    "../game/shared/ms/script.cpp"
    "../game/shared/ms/script.h"
    "../game/shared/ms/scriptcmds.cpp"
    "../game/shared/ms/scriptedeffects.cpp"
    "../game/shared/ms/scriptedeffects.h"
    "../game/shared/ms/scriptmgr.cpp"
    "../game/shared/ms/scriptmgr.h"
    "../game/shared/ms/sharedmenu.h"
    "../game/shared/ms/sharedutil.cpp"
    "../game/shared/ms/sharedutil.h"
    "../game/shared/ms/stackstring.cpp"
    "../game/shared/ms/stackstring.h"
    "../game/shared/ms/votedefs.h"
    "../game/shared/stats/races.cpp"
    "../game/shared/stats/stats.cpp"
    "../game/shared/strhelper.cpp"
    "../game/shared/strhelper.h"
    "../game/shared/syntax/syntax.cpp"
)
source_group("Shared" FILES ${Shared})

set(Shared__CRC
    "../game/shared/ms/crc/checksum_crc.cpp"
    "../game/shared/ms/crc/checksum_crc.h"
    "../game/shared/ms/crc/crchash.cpp"
    "../game/shared/ms/crc/crchash.h"
)
source_group("Shared\\CRC" FILES ${Shared__CRC})

set(Shared__base64
    "../thirdparty/base64/base64.cpp"
    "../thirdparty/base64/base64.h"
)
source_group("Shared\\base64" FILES ${Shared__base64})

set(VGUI
    "../game/client/ui/vgui_checkbutton2.cpp"
    "../game/client/ui/vgui_consolepanel.cpp"
    "../game/client/ui/vgui_consolepanel.h"
    "../game/client/ui/vgui_controlconfigpanel.cpp"
    "../game/client/ui/vgui_controlconfigpanel.h"
    "../game/client/ui/vgui_customobjects.cpp"
    "../game/client/ui/vgui_global.cpp"
    "../game/client/ui/vgui_global.h"
    "../game/client/ui/vgui_grid.cpp"
    "../game/client/ui/vgui_helpers.cpp"
    "../game/client/ui/vgui_int.cpp"
    "../game/client/ui/vgui_int.h"
    "../game/client/ui/vgui_listbox.cpp"
    "../game/client/ui/vgui_loadtga.cpp"
    "../game/client/ui/vgui_motdwindow.cpp"
    "../game/client/ui/vgui_schememanager.cpp"
    "../game/client/ui/vgui_schememanager.h"
    "../game/client/ui/vgui_scorepanel.cpp"
    "../game/client/ui/vgui_scorepanel.h"
    "../game/client/ui/vgui_scrollbar2.cpp"
    "../game/client/ui/vgui_slider2.cpp"
    "../game/client/ui/vgui_spectatorpanel.cpp"
    "../game/client/ui/vgui_status.h"
    "../game/client/ui/vgui_teamfortressviewport.cpp"
    "../game/client/ui/vgui_teamfortressviewport.h"
    "../game/client/ui/vgui_voice_tweakdlg.cpp"
    "../utils/vgui/include/VGUI_Panel.h"
)
source_group("VGUI" FILES ${VGUI})

set(VGUI__ms
    "../game/client/ui/ms/vgui_choosecharacter.cpp"
    "../game/client/ui/ms/vgui_choosecharacter.h"
    "../game/client/ui/ms/vgui_container.cpp"
    "../game/client/ui/ms/vgui_container.h"
    "../game/client/ui/ms/vgui_containerlist.cpp"
    "../game/client/ui/ms/vgui_containerlist.h"
    "../game/client/ui/ms/vgui_eventconsole.h"
    "../game/client/ui/ms/vgui_health.h"
    "../game/client/ui/ms/vgui_health2.h"
    "../game/client/ui/ms/vgui_hud.cpp"
    "../game/client/ui/ms/vgui_hud.h"
    "../game/client/ui/ms/vgui_id.h"
    "../game/client/ui/ms/vgui_infowin.h"
    "../game/client/ui/ms/vgui_localizedpanel.cpp"
    "../game/client/ui/ms/vgui_localizedpanel.h"
    "../game/client/ui/ms/vgui_menu_interact.h"
    "../game/client/ui/ms/vgui_menu_main.h"
    "../game/client/ui/ms/vgui_menubase.cpp"
    "../game/client/ui/ms/vgui_menubase.h"
    "../game/client/ui/ms/vgui_mscontrols.cpp"
    "../game/client/ui/ms/vgui_mscontrols.h"
    "../game/client/ui/ms/vgui_options.cpp"
    "../game/client/ui/ms/vgui_options.h"
    "../game/client/ui/ms/vgui_quickslot.h"
    "../game/client/ui/ms/vgui_spawn.cpp"
    "../game/client/ui/ms/vgui_spawn.h"
    "../game/client/ui/ms/vgui_startsaytext.h"
    "../game/client/ui/ms/vgui_stats.cpp"
    "../game/client/ui/ms/vgui_stats.h"
    "../game/client/ui/ms/vgui_storage.cpp"
    "../game/client/ui/ms/vgui_storage.h"
    "../game/client/ui/ms/vgui_store.cpp"
    "../game/client/ui/ms/vgui_storebuy.cpp"
    "../game/client/ui/ms/vgui_storebuy.h"
    "../game/client/ui/ms/vgui_storemainwin.cpp"
    "../game/client/ui/ms/vgui_storemainwin.h"
    "../game/client/ui/ms/vgui_storesell.cpp"
    "../game/client/ui/ms/vgui_storesell.h"
    "../game/client/ui/ms/vgui_vote.h"
)
source_group("VGUI\\ms" FILES ${VGUI__ms})

set(ALL_FILES
    ${FMOD}
    ${HL1}
    ${HL1__Headers}
    ${MS}
    ${MS__Generic_Item}
    ${MS__Headers}
    ${Misc}
    ${Render}
    ${SDK2013}
    ${Shared}
    ${Shared__CRC}
    ${Shared__base64}
    ${VGUI}
    ${VGUI__ms}
)

################################################################################
# Target
################################################################################

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Compile definitions
################################################################################

################################################################################
# Compile and link options
################################################################################


























set(PROJECT_NAME Server)

################################################################################
# Source groups
################################################################################
set(HL1
    "../common/const.h"
    "../common/cstringpool.cpp"
    "../common/cstringpool.h"
    "../game/server/basemonster.h"
    "../game/server/bmodels.cpp"
    "../game/server/cdll_dll.h"
    "../game/server/client.cpp"
    "../game/server/effects.cpp"
    "../game/server/enginecallback.h"
    "../game/server/func_break.cpp"
    "../game/server/game.cpp"
    "../game/server/gamerules/gamerules.cpp"
    "../game/server/gamerules/gamerules.h"
    "../game/server/gamerules/multiplay_gamerules.cpp"
    "../game/server/ggrenade.cpp"
    "../game/server/h_export.cpp"
    "../game/server/hl/animating.cpp"
    "../game/server/hl/animation.cpp"
    "../game/server/hl/buttons.cpp"
    "../game/server/hl/cbase.cpp"
    "../game/server/hl/defaultai.cpp"
    "../game/server/hl/doors.cpp"
    "../game/server/hl/explode.cpp"
    "../game/server/hl/extdll.h"
    "../game/server/hl/func_tank.cpp"
    "../game/server/hl/game.h"
    "../game/server/hl/globals.cpp"
    "../game/server/hl/lights.cpp"
    "../game/server/hl/monsters.cpp"
    "../game/server/hl/monsters.h"
    "../game/server/hl/monsterstate.cpp"
    "../game/server/hl/nodes.cpp"
    "../game/server/hl/pathcorner.cpp"
    "../game/server/hl/plane.cpp"
    "../game/server/hl/plats.cpp"
    "../game/server/hl/schedule.cpp"
    "../game/server/hl/scripted.cpp"
    "../game/server/hl/skill.cpp"
    "../game/server/hl/sound.cpp"
    "../game/server/hl/soundent.cpp"
    "../game/server/hl/soundent.h"
    "../game/server/hl/spectator.cpp"
    "../game/server/hl/subs.cpp"
    "../game/server/hl/util.cpp"
    "../game/server/hl/util.h"
    "../game/server/monsters/combat.cpp"
    "../game/server/serverfindentities.cpp"
    "../game/server/serverfindentities.h"
    "../game/server/shurispritedefs.h"
    "../game/server/triggers.cpp"
    "../game/server/world.cpp"
    "../game/shared/cbase.h"
    "../game/shared/movement/pm_defs.h"
    "../game/shared/movement/pm_math.c"
    "../game/shared/movement/pm_movevars.h"
    "../game/shared/voice_gamemgr.cpp"
    "../public/engine/progdefs.h"
)
source_group("HL1" FILES ${HL1})

set(MS
    "../game/server/effects/mseffects.cpp"
    "../game/server/entity.cpp"
    "../game/server/gamerules/team.cpp"
    "../game/server/magic.cpp"
    "../game/server/monsters/bodyparts/bodyparts.cpp"
    "../game/server/monsters/bodyparts/bodyparts_human.cpp"
    "../game/server/monsters/corpse.cpp"
    "../game/server/monsters/monsteranimation.cpp"
    "../game/server/monsters/msmonsterserver.cpp"
    "../game/server/monsters/npcact.cpp"
    "../game/server/monsters/npcglobal.cpp"
    "../game/server/monsters/npcscript.cpp"
    "../game/server/msmapents.cpp"
    "../game/server/player/player.cpp"
    "../game/server/player/playerstats.cpp"
    "../game/server/shield.cpp"
    "../game/server/store.cpp"
    "../game/server/sv_character.cpp"
    "../game/server/svglobals.cpp"
    "../game/shared/movement/pm_shared.cpp"
    "../game/shared/movement/pm_shared.h"
    "../game/shared/ms/msmonstershared.cpp"
    "../game/shared/ms/netcodeshared.cpp"
    "../game/shared/stats/races.cpp"
    "../game/shared/syntax/syntax.cpp"
)
source_group("MS" FILES ${MS})

set(MS__Headers
    "../game/server/effects/mseffects.h"
    "../game/server/gamerules/teams.h"
    "../game/server/monsters/bodyparts/bodyparts.h"
    "../game/server/monsters/bodyparts/bodyparts_human.h"
    "../game/server/monsters/corpse.h"
    "../game/server/monsters/monsteranimation.h"
    "../game/server/monsters/msmonster.h"
    "../game/server/msdllheaders.h"
    "../game/server/msgamerules.h"
    "../game/server/msitemdefs.h"
    "../game/server/player/modeldefs.h"
    "../game/server/player/player.h"
    "../game/server/shield.h"
    "../game/server/store.h"
    "../game/server/svglobals.h"
    "../game/shared/ms/groupfile.h"
    "../game/shared/ms/mscharacter.h"
    "../game/shared/ms/mscharacterheader.h"
    "../game/shared/ms/titles.h"
    "../game/shared/ms/votedefs.h"
    "../game/shared/stats/races.h"
    "../game/shared/stats/statdefs.h"
    "../game/shared/stats/stats.h"
)
source_group("MS\\Headers" FILES ${MS__Headers})

set(Misc
    "../public/tier0/pointeroverride.asm"
)
source_group("Misc" FILES ${Misc})

set(SDK2013
    "../public/FileSystem.h"
    "../public/interface.cpp"
    "../public/interface.h"
)
source_group("SDK2013" FILES ${SDK2013})

set(Shared
    "../game/server/player/playershared.cpp"
    "../game/shared/ms/buildcontrol.h"
    "../game/shared/ms/dbginputs.cpp"
    "../game/shared/ms/filesystem_shared.cpp"
    "../game/shared/ms/filesystem_shared.h"
    "../game/shared/ms/findentities.cpp"
    "../game/shared/ms/findentities.h"
    "../game/shared/ms/fndatahandler.cpp"
    "../game/shared/ms/fndatahandler.h"
    "../game/shared/ms/global.cpp"
    "../game/shared/ms/global.h"
    "../game/shared/ms/globalmem.cpp"
    "../game/shared/ms/groupfile.cpp"
    "../game/shared/ms/httprequesthandler.cpp"
    "../game/shared/ms/httprequesthandler.h"
    "../game/shared/ms/iscript.h"
    "../game/shared/ms/logger.h"
    "../game/shared/ms/magic.h"
    "../game/shared/ms/msbasic.h"
    "../game/shared/ms/mscharacter.cpp"
    "../game/shared/ms/msdebug.h"
    "../game/shared/ms/msfileio.cpp"
    "../game/shared/ms/msfileio.h"
    "../game/shared/ms/saytext.h"
    "../game/shared/ms/script.cpp"
    "../game/shared/ms/script.h"
    "../game/shared/ms/scriptcmds.cpp"
    "../game/shared/ms/scriptedeffects.cpp"
    "../game/shared/ms/scriptedeffects.h"
    "../game/shared/ms/scriptmgr.cpp"
    "../game/shared/ms/scriptmgr.h"
    "../game/shared/ms/sharedutil.cpp"
    "../game/shared/ms/sharedutil.h"
    "../game/shared/ms/stackstring.cpp"
    "../game/shared/ms/stackstring.h"
    "../game/shared/ms/storeshared.h"
    "../game/shared/ms/versioncontrol.h"
    "../game/shared/ms/vgui_menudefsshared.h"
    "../game/shared/stats/stats.cpp"
    "../game/shared/strhelper.cpp"
    "../game/shared/strhelper.h"
)
source_group("Shared" FILES ${Shared})

set(Shared__CRC
    "../game/shared/ms/crc/checksum_crc.cpp"
    "../game/shared/ms/crc/checksum_crc.h"
    "../game/shared/ms/crc/crchash.cpp"
    "../game/shared/ms/crc/crchash.h"
)
source_group("Shared\\CRC" FILES ${Shared__CRC})

set(Shared__Generic_Item
    "../game/shared/genitemlist.cpp"
    "../game/shared/genitemlist.h"
    "../game/shared/weapons/genericitem.cpp"
    "../game/shared/weapons/genericitem.h"
    "../game/shared/weapons/giarmor.cpp"
    "../game/shared/weapons/giattack.cpp"
    "../game/shared/weapons/gidrinkable.cpp"
    "../game/shared/weapons/gipack.cpp"
    "../game/shared/weapons/giprojectile.cpp"
    "../game/shared/weapons/gispawn.cpp"
    "../game/shared/weapons/gispell.cpp"
    "../game/shared/weapons/weapons.cpp"
    "../game/shared/weapons/weapons.h"
)
source_group("Shared\\Generic Item" FILES ${Shared__Generic_Item})

set(Shared__base64
    "../thirdparty/base64/base64.cpp"
    "../thirdparty/base64/base64.h"
)
source_group("Shared\\base64" FILES ${Shared__base64})

set(ALL_FILES
    ${HL1}
    ${MS}
    ${MS__Headers}
    ${Misc}
    ${SDK2013}
    ${Shared}
    ${Shared__CRC}
    ${Shared__Generic_Item}
    ${Shared__base64}
)

################################################################################
# Target
################################################################################

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Compile definitions
################################################################################

################################################################################
# Compile and link options
################################################################################


























set(PROJECT_NAME scriptpack)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "../game/scriptpack/cbase.h"
    "../game/scriptpack/compat.h"
    "../game/scriptpack/dirent.h"
    "../game/scriptpack/packer.cpp"
    "../game/scriptpack/packer.h"
    "../game/scriptpack/parser.h"
    "../game/scriptpack/scriptpack.cpp"
    "../game/shared/ms/crc/checksum_crc.cpp"
    "../game/shared/ms/crc/checksum_crc.h"
    "../game/shared/ms/crc/crchash.cpp"
    "../game/shared/ms/crc/crchash.h"
    "../game/shared/ms/groupfile.cpp"
    "../game/shared/ms/groupfile.h"
    "../game/shared/ms/msfileio.cpp"
    "../game/shared/ms/stackstring.cpp"
    "../public/tier0/pointeroverride.asm"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

################################################################################
# Target
################################################################################

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Compile definitions
################################################################################

################################################################################
# Compile and link options
################################################################################


























set(PROJECT_NAME maphash)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "../game/maphash/maphash.cpp"
    "../game/shared/ms/crc/checksum_crc.cpp"
    "../game/shared/ms/crc/checksum_crc.h"
    "../game/shared/ms/crc/crchash.cpp"
    "../game/shared/ms/crc/crchash.h"
    "../game/shared/ms/msfileio.cpp"
    "../game/shared/ms/stackstring.cpp"
    "../public/tier0/pointeroverride.asm"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
    ${VPC_Scripts}
)

################################################################################
# Target
################################################################################

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Compile definitions
################################################################################

################################################################################
# Compile and link options
################################################################################


























set(PROJECT_NAME charimport)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "../game/charimport/charimport.cpp"
    "../game/shared/ms/httprequesthandler.cpp"
    "../game/shared/ms/msfileio.cpp"
    "../game/shared/ms/stackstring.cpp"
    "../public/tier0/pointeroverride.asm"
    "../thirdparty/base64/base64.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
    ${VPC_Scripts}
)

################################################################################
# Target
################################################################################

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Compile definitions
################################################################################

################################################################################
# Compile and link options
################################################################################
