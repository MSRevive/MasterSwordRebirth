add_executable(maphash WIN32_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/maphash.cpp)

set_common_properties(maphash)

target_include_directories(maphash PRIVATE
	${BASEDIR}
	game/server
	game/shared
	game/shared/ms
	game/shared/ms/crc
	thirdparty
	${CMAKE_CURRENT_SOURCE_DIR})

################################################################################
# Source groups
################################################################################
set(Source_Files
	"/game/maphash/maphash.cpp"
	"/game/shared/ms/crc/checksum_crc.cpp"
	"/game/shared/ms/crc/checksum_crc.h"
	"/game/shared/ms/crc/crchash.cpp"
	"/game/shared/ms/crc/crchash.h"
	"/game/shared/ms/msfileio.cpp"
	"/game/shared/ms/stackstring.cpp"
	"/public/tier0/pointeroverride.asm"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)

source_group(TREE ${BASEDIR} FILES ${ALL_FILES})