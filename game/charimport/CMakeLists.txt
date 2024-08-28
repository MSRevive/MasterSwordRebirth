add_executable(charimport WIN32_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/charimport.cpp)

set_common_properties(charimport)

target_include_directories(charimport PRIVATE
	${BASEDIR}
	game/shared/ms
	thirdparty/base64
	${CMAKE_CURRENT_SOURCE_DIR})

################################################################################
# Source groups
################################################################################
set(Source_Files
	"/game/charimport/charimport.cpp"
	"/game/shared/ms/httprequesthandler.cpp"
	"/game/shared/ms/msfileio.cpp"
	"/game/shared/ms/stackstring.cpp"
	"/public/tier0/pointeroverride.asm"
	"/thirdparty/base64/base64.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)

target_sources(charimport PRIVATE ${ALL_FILES})