add_executable(maphash ${CMAKE_CURRENT_SOURCE_DIR}/maphash.cpp)

set_common_properties(maphash)

target_include_directories(maphash PRIVATE
	${BASEDIR}/game/server
	${BASEDIR}/game/shared
	${BASEDIR}/game/shared/ms
	${BASEDIR}/game/shared/ms/crc
	${BASEDIR}/thirdparty
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