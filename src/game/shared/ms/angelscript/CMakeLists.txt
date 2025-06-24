# CMakeLists.txt for AngelScript test program
cmake_minimum_required(VERSION 3.10)
project(AngelScriptTest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../thirdparty/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../thirdparty/angelscript
)

# AngelScript source files
file(GLOB ANGELSCRIPT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../thirdparty/angelscript/*.cpp"
)

# Test program sources
set(TEST_SOURCES
    test_angelscript_init.cpp
    CAngelScriptManager.cpp
    IAngelScript.cpp
    CAngelScript.cpp
    ASCoreTypes.cpp
    ASBuiltinFunctions.cpp
    ASBindings.cpp
    ../global.cpp
    ../globalmem.cpp
    ../stackstring.cpp
    ../sharedutil.cpp
    ../msfileio.cpp
    ../groupfile.cpp
    ../scriptmgr.cpp
    ../script.cpp
    ../findentities.cpp
    ../../../../common/interface.cpp
)

# Create test executable
add_executable(test_angelscript ${TEST_SOURCES} ${ANGELSCRIPT_SOURCES})

# Platform-specific settings
if(WIN32)
    target_compile_definitions(test_angelscript PRIVATE _WIN32 GAME_DLL)
else()
    target_compile_definitions(test_angelscript PRIVATE _LINUX POSIX GAME_DLL)
    target_link_libraries(test_angelscript pthread)
endif()

# Output directory
set_target_properties(test_angelscript PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)