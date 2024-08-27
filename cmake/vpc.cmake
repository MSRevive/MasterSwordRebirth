# Taken from https://github.com/Spirrwell/source-sdk-2013-cmake, to help convert our project from VPC to CMake
# vpc.cmake - Allows us to directly translate options into cmake from VPC.

# Compile options that are populated and set for each target depending on their type
set(ADDITIONAL_COMPILE_OPTIONS_EXE)
set(ADDITIONAL_COMPILE_OPTIONS_DLL)
set(ADDITIONAL_COMPILE_OPTIONS_LIB)

# Libraries that are linked to for each target depending on their type
set(ADDITIONAL_LINK_LIBRARIES_EXE)
set(ADDITIONAL_LINK_LIBRARIES_DLL)

# Linker options that are populated and set for each target depending on their type
set(ADDITIONAL_LINK_OPTIONS_EXE)
set(ADDITIONAL_LINK_OPTIONS_DLL)
set(ADDITIONAL_LINK_OPTIONS_LIB)

# Sources that are added to each target depending on their type
set(ADDITIONAL_SOURCES_EXE)
set(ADDITIONAL_SOURCES_DLL)
set(ADDITIONAL_SOURCES_LIB)

# Compile definitions that are added to each target depending on their type
set(ADDITIONAL_COMPILE_DEFINITIONS_EXE)
set(ADDITIONAL_COMPILE_DEFINITIONS_DLL)
set(ADDITIONAL_COMPILE_DEFINITIONS_LIB)

foreach(target ${ALL_TARGETS})
	get_target_property(target_type ${target} TYPE)
    
	if (${target_type} STREQUAL "EXECUTABLE")
		add_compile_options_filtered(${target} ADDITIONAL_COMPILE_OPTIONS_EXE)
		add_libraries_filtered(${target} ADDITIONAL_LINK_LIBRARIES_EXE)
		add_sources_filtered(${target} ADDITIONAL_SOURCES_EXE)
		target_link_options(${target} PRIVATE ${ADDITIONAL_LINK_OPTIONS_EXE})
		target_compile_definitions(${target} PRIVATE MEMOVERRIDE_MODULE=$<TARGET_NAME_IF_EXISTS:${target}>)
		add_compile_definitions_filtered(${target} ADDITIONAL_COMPILE_DEFINITIONS_EXE)

		# Only applies to Linux and OSX
		target_strip_symbols(${target})
	elseif((${target_type} STREQUAL "SHARED_LIBRARY") OR (${target_type} STREQUAL "MODULE_LIBRARY"))
		add_compile_options_filtered(${target} ADDITIONAL_COMPILE_OPTIONS_DLL)
		add_libraries_filtered(${target} ADDITIONAL_LINK_LIBRARIES_DLL)
		add_sources_filtered(${target} ADDITIONAL_SOURCES_DLL)
		target_link_options(${target} PRIVATE ${ADDITIONAL_LINK_OPTIONS_DLL})
		target_compile_definitions(${target} PRIVATE MEMOVERRIDE_MODULE=$<TARGET_NAME_IF_EXISTS:${target}> DLLNAME=$<TARGET_NAME_IF_EXISTS:${target}>)
		add_compile_definitions_filtered(${target} ADDITIONAL_COMPILE_DEFINITIONS_DLL)

		# Only applies to Linux and OSX
		target_strip_symbols(${target})
	elseif(${target_type} STREQUAL "STATIC_LIBRARY")
		add_compile_options_filtered(${target} ADDITIONAL_COMPILE_OPTIONS_LIB)
		add_sources_filtered(${target} ADDITIONAL_SOURCES_LIB)
		target_link_options(${target} PRIVATE ${ADDITIONAL_LINK_OPTIONS_LIB})
		target_compile_definitions(${target} PRIVATE LIBNAME=$<TARGET_NAME_IF_EXISTS:${target}>)
		add_compile_definitions_filtered(${target} ADDITIONAL_COMPILE_DEFINITIONS_LIB)
	endif()
endforeach()