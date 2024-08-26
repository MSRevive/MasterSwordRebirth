set(PROJECT_NAME charimport)

################################################################################
# Source groups
################################################################################
set(Source_Files
	"../game/charimport/charimport.cpp"
	"../game/shared/ms/httprequesthandler.cpp"
	"../game/shared/ms/msfileio.cpp"
	"../game/shared/ms/stackstring.cpp"
	"../public/tier0/pointeroverride.asm"
	"../thirdparty/base64/base64.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)