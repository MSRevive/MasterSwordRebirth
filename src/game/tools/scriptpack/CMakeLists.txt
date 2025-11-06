add_executable(scriptpack)

################################################################################
# Target Properties
################################################################################
set_common_properties(scriptpack)

target_compile_definitions(scriptpack PRIVATE
	SCRIPTPACK_TOOL
	CLIENT_DLL
	_MSR_UTILS)

target_include_directories(scriptpack PRIVATE
	${BASEDIR}/src/common
	${BASEDIR}/src/game/server
	${BASEDIR}/src/game/server/hl
	${BASEDIR}/src/game/shared
	${BASEDIR}/src/game/shared/ms
	${BASEDIR}/src/game/shared/ms/angelscript/addons
	${BASEDIR}/src/public
	${BASEDIR}/src/public/engine
	${BASEDIR}/thirdparty
	${BASEDIR}/thirdparty/angelscript/include
	${CMAKE_CURRENT_SOURCE_DIR}
	tclap)

################################################################################
# Source groups
################################################################################
set(SRC_Files
	"cbase.h"
	"dirent.h"
	"packer.cpp"
	"packer.h"
	"parser.cpp"
	"parser.h"
	"scriptpack.cpp"
	"${BASEDIR}/src/game/shared/ms/crc/checksum_crc.cpp"
	"${BASEDIR}/src/game/shared/ms/crc/checksum_crc.h"
	"${BASEDIR}/src/game/shared/ms/crc/crchash.cpp"
	"${BASEDIR}/src/game/shared/ms/crc/crchash.h"
	"${BASEDIR}/src/game/shared/ms/groupfile.cpp"
	"${BASEDIR}/src/game/shared/ms/groupfile.h"
	"${BASEDIR}/src/game/shared/ms/msfileio.cpp"
	"${BASEDIR}/src/game/shared/ms/stackstring.cpp"
	"${BASEDIR}/src/game/shared/ms/angelscript/addons/scriptmodule/scriptmodule.cpp"
	"${BASEDIR}/src/game/shared/ms/angelscript/addons/scriptmodule/scriptmodule.h"
)
source_group("Source Files" FILES ${SRC_Files})

set(ALL_FILES
	${SRC_Files}
)

target_sources(scriptpack PRIVATE ${ALL_FILES})