add_executable(apidoc)

################################################################################
# Target Properties
################################################################################
set_common_properties(apidoc)

target_include_directories(apidoc PRIVATE
	${BASEDIR}/../lib/include
	${BASEDIR}/../lib/include/vgui
	${BASEDIR}/../src/common
	${BASEDIR}/../src/game/client
	${BASEDIR}/../src/game/client/hl
	${BASEDIR}/../src/game/client/ms
	${BASEDIR}/../src/game/client/ui
	${BASEDIR}/../src/game/server
	${BASEDIR}/../src/game/server/gamerules
	${BASEDIR}/../src/game/server/hl
	${BASEDIR}/../src/game/server/monsters
	${BASEDIR}/../src/game/server/player
	${BASEDIR}/../src/game/shared
	${BASEDIR}/../src/game/shared/movement
	${BASEDIR}/../src/game/shared/ms
	${BASEDIR}/../src/game/shared/ms/crc
	${BASEDIR}/../src/game/shared/stats
	${BASEDIR}/../src/game/shared/syntax
	${BASEDIR}/../src/game/shared/weapons
	${BASEDIR}/../src/public
	${BASEDIR}/../src/public/engine
	${BASEDIR}/../thirdparty
	${BASEDIR}/../thirdparty/angelscript/include
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(apidoc PRIVATE
			VALVE_DLL)

################################################################################
# Source groups
################################################################################
set(Source_Files
	"main.cpp"
	"../../src/game/server/monsters/npcscript.cpp"
	"../../src/game/shared/weapons/genericitem.cpp"

	# sigh, need to include all this so that all the declared functions and methods can resolve
	"../../src/game/server/effects.cpp"
	"../../src/game/server/entity.cpp"
	"../../src/game/server/magic.cpp"
	"../../src/game/server/store.cpp"
	"../../src/game/server/svglobals.cpp"
	"../../src/game/server/fn/FNSharedDefs.cpp"
	"../../src/game/server/hl/animating.cpp"
	"../../src/game/server/hl/cbase.cpp"
	"../../src/game/server/hl/sound.cpp"
	"../../src/game/server/hl/subs.cpp"
	"../../src/game/server/hl/util.cpp"
	"../../src/game/server/monsters/combat.cpp"
	"../../src/game/server/monsters/msmonsterserver.cpp"
	"../../src/game/server/player/player.cpp"
	"../../src/game/server/player/playershared.cpp"
	"../../src/game/shared/ms/global.cpp"
	"../../src/game/shared/ms/groupfile.cpp"
	"../../src/game/shared/ms/msmonstershared.cpp"
	"../../src/game/shared/ms/script.cpp"
	"../../src/game/shared/ms/scriptcmds.cpp"
	"../../src/game/shared/ms/scriptedeffects.cpp"
	"../../src/game/shared/ms/sharedutil.cpp"
	"../../src/game/shared/ms/stackstring.cpp"
	"../../src/game/shared/stats/stats.cpp"
	"../../src/game/shared/weapons/giarmor.cpp"
	"../../src/game/shared/weapons/giattack.cpp"
	"../../src/game/shared/weapons/gidrinkable.cpp"
	"../../src/game/shared/weapons/gipack.cpp"
	"../../src/game/shared/weapons/weapons.cpp"

)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)

target_sources(apidoc PRIVATE ${ALL_FILES})