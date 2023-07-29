#
#   AngelscriptUtils library
#

set( TARGET_NAME AngelscriptUtils )

set( ANGELSCRIPTUTILS_VERSION_MAJOR 0 )
set( ANGELSCRIPTUTILS_VERSION_MINOR 1 )
set( ANGELSCRIPTUTILS_VERSION_PATCH 0 )
set( PROJECT_VERSION ${ANGELSCRIPTUTILS_VERSION_MAJOR}.${ANGELSCRIPTUTILS_VERSION_MINOR}.${ANGELSCRIPTUTILS_VERSION_PATCH} )

find_package( Threads REQUIRED )
find_package( spdlog CONFIG REQUIRED )
find_package( Angelscript CONFIG 2.32.0 REQUIRED )

set( ASUTILS_CASMODULE_USER_DATA_ID "10001" CACHE STRING "Value for the CASModule user data ID" )
set( ASUTILS_CONTEXT_RESULTHANDLER_USERDATA_ID "20001" CACHE STRING "Value for the context result handler user data ID" )

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/ASUtilsConfig.h.in
	${CMAKE_CURRENT_SOURCE_DIR}/ASUtilsConfig.h
	@ONLY
)

add_sources(
	ASUtilsConfig.h
	CASLoggingContextResultHandler.cpp
	CASLoggingContextResultHandler.h
	CASManager.cpp
	CASManager.h
	CASModuleDescriptor.cpp
	CASModuleDescriptor.h
	CASModule.cpp
	CASModule.h
	CASModuleManager.cpp
	CASModuleManager.h
	IASContextResultHandler.h
	IASInitializer.h
	IASModuleBuilder.h
	add_on/scriptbuilder/scriptbuilder.cpp
	add_on/scriptbuilder/scriptbuilder.h
)

add_includes(
	ASUtilsConfig.h
	CASLoggingContextResultHandler.h
	CASManager.h
	CASModuleDescriptor.h
	CASModule.h
	CASModuleManager.h
	IASContextResultHandler.h
	IASInitializer.h
	IASModuleBuilder.h
	std_make_unique.h
	add_on/scriptbuilder/scriptbuilder.h
)

add_subdirectory( event )
add_subdirectory( ScriptAPI )
add_subdirectory( util )
add_subdirectory( wrapper )

preprocess_sources()

add_library( ${TARGET_NAME} STATIC ${PREP_SRCS} )

check_winxp_support( ${TARGET_NAME} )

configure_msvc_runtime( ${TARGET_NAME} ${MSVC_RUNTIME_CONFIG} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${SRC_DIR}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE
	${SHARED_DEFS}
)

target_link_libraries( ${TARGET_NAME}
	Angelscript::angelscript
	spdlog::spdlog
)

set_target_properties( ${TARGET_NAME} PROPERTIES
	COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}"
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

set_pdb_names( ${TARGET_NAME} )

#Create filters
create_source_groups( "${SRC_DIR}" )

install_pdbs( ${TARGET_NAME} lib )
install_includes( "${SRC_DIR}" COMPONENT Devel )

#See https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#creating-packages for a detailed explanation about this part
install( TARGETS ${TARGET_NAME} EXPORT AngelscriptUtilsTargets
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
)

include( CMakePackageConfigHelpers )
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/AngelscriptUtils/AngelscriptUtilsConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)

export( EXPORT AngelscriptUtilsTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/AngelscriptUtils/AngelscriptUtilsTargets.cmake"
	NAMESPACE AngelscriptUtils::
)
configure_file( cmake/AngelscriptUtilsConfig.cmake
	"${CMAKE_CURRENT_BINARY_DIR}/AngelscriptUtils/AngelscriptUtilsConfig.cmake"
	COPYONLY
)

set( ConfigPackageLocation lib/cmake/AngelscriptUtils)
install( EXPORT AngelscriptUtilsTargets
	FILE AngelscriptUtilsTargets.cmake
	NAMESPACE AngelscriptUtils::
	DESTINATION ${ConfigPackageLocation}
)
install(
	FILES
		cmake/AngelscriptUtilsConfig.cmake
		"${CMAKE_CURRENT_BINARY_DIR}/AngelscriptUtils/AngelscriptUtilsConfigVersion.cmake"
	DESTINATION ${ConfigPackageLocation}
	COMPONENT Devel
)

clear_sources()
