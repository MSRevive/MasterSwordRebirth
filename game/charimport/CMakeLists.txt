add_library(curl SHARED IMPORTED)

target_include_directories(curl INTERFACE ${BASEDIR}/thirdparty/curl)

if (WIN32)
	set_target_properties(curl PROPERTIES IMPORTED_IMPLIB "${BASEDIR}/lib/public/libcurl.lib")
endif(WIN32)

add_executable(charimport ${CMAKE_CURRENT_SOURCE_DIR}/charimport.cpp)

set_common_properties(charimport)

target_link_libraries(charimport PRIVATE curl)

target_include_directories(charimport PRIVATE
	${BASEDIR}/game/shared/ms
	${BASEDIR}/thirdparty/base64
	${CMAKE_CURRENT_SOURCE_DIR})

################################################################################
# Source groups
################################################################################
set(Source_Files
	"charimport.cpp"
	"../shared/ms/httprequesthandler.cpp"
	"../shared/ms/msfileio.cpp"
	"../shared/ms/stackstring.cpp"
	"../../public/tier0/pointeroverride.asm"
	"../../thirdparty/base64/base64.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)

target_sources(charimport PRIVATE ${ALL_FILES})