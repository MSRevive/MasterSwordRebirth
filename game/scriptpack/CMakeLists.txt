set(PROJECT_NAME scriptpack)

add_executable(scriptpack WIN32_EXECUTABLE)

set_common_properties(scriptpack)

target_include_directories(scriptpack PRIVATE
	${BASEDIR}
	game/server
	game/shared
	game/shared/ms
	thirdparty
	${CMAKE_CURRENT_SOURCE_DIR}
	tclap)

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