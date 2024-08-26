set(PROJECT_NAME maphash)

################################################################################
# Source groups
################################################################################
set(Source_Files
	"../game/maphash/maphash.cpp"
	"../game/shared/ms/crc/checksum_crc.cpp"
	"../game/shared/ms/crc/checksum_crc.h"
	"../game/shared/ms/crc/crchash.cpp"
	"../game/shared/ms/crc/crchash.h"
	"../game/shared/ms/msfileio.cpp"
	"../game/shared/ms/stackstring.cpp"
	"../public/tier0/pointeroverride.asm"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)