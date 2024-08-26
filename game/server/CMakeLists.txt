set(PROJECT_NAME Server)

add_library(steam_api SHARED IMPORTED)
add_library(curl SHARED IMPORTED)

target_include_directories(curl INTERFACE ${BASEDIR}/thirdparty/curl)

if (WIN32)
	set_target_properties(steam_api PROPERTIES IMPORTED_IMPLIB "${BASEDIR}/lib/public/steam_api.lib")
	set_target_properties(curl PROPERTIES IMPORTED_IMPLIB "${BASEDIR}/lib/public/libcurl.lib")
else()
	set_target_properties(steam_api PROPERTIES IMPORTED_LOCATION "${BASEDIR}/lib/public/linux/libsteam_api.so")
	set_target_properties(curl PROPERTIES IMPORTED_LOCATION "${BASEDIR}/lib/public/linux/libcurl.a")
endif()

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

target_precompile_headers(server PRIVATE entities/cbase.h)