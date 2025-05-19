add_executable(apidoc)

################################################################################
# Target Properties
################################################################################
set_common_properties(apidoc)

target_include_directories(apidoc PRIVATE
	${BASEDIR}/../src/common
	${BASEDIR}/../src/game/server
	${BASEDIR}/../src/game/server/hl
	${BASEDIR}/../src/game/shared
	${BASEDIR}/../src/game/shared/ms
	${BASEDIR}/../src/public
	${BASEDIR}/../src/public/engine
	${CMAKE_CURRENT_SOURCE_DIR}
)

################################################################################
# Source groups
################################################################################
set(Source_Files
	"main.cpp"
	"../../src/game/server/monsters/npcscript.cpp"
	"../../src/game/shared/weapons/genericitem.cpp"

)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)

target_sources(apidoc PRIVATE ${ALL_FILES})