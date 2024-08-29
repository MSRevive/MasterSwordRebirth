add_executable(scriptpack ${CMAKE_CURRENT_SOURCE_DIR}/scriptpack.cpp)

set_common_properties(scriptpack)

target_include_directories(scriptpack PRIVATE
	${BASEDIR}/game/server
	${BASEDIR}/game/shared
	${BASEDIR}/game/shared/ms
	${BASEDIR}/thirdparty
	${CMAKE_CURRENT_SOURCE_DIR}
	tclap)

################################################################################
# Source groups
################################################################################
set(Source_Files
	"cbase.h"
	"compat.h"
	"dirent.h"
	"packer.cpp"
	"packer.h"
	"parser.h"
	"scriptpack.cpp"
	"../shared/ms/crc/checksum_crc.cpp"
	"../shared/ms/crc/checksum_crc.h"
	"../shared/ms/crc/crchash.cpp"
	"../shared/ms/crc/crchash.h"
	"../shared/ms/groupfile.cpp"
	"../shared/ms/groupfile.h"
	"../shared/ms/msfileio.cpp"
	"../shared/ms/stackstring.cpp"
	"../../public/tier0/pointeroverride.asm"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)

target_sources(scriptpack PRIVATE ${ALL_FILES})