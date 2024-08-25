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