add_executable(maphash ${CMAKE_CURRENT_SOURCE_DIR}/maphash.cpp)

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
	"maphash.cpp"
	"../shared/ms/crc/checksum_crc.cpp"
	"../shared/ms/crc/checksum_crc.h"
	"../shared/ms/crc/crchash.cpp"
	"../shared/ms/crc/crchash.h"
	"../shared/ms/msfileio.cpp"
	"../shared/ms/stackstring.cpp"
	"../../public/tier0/pointeroverride.asm"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)

target_sources(maphash PRIVATE ${ALL_FILES})