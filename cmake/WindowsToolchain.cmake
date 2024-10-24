# VCPKG
set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_TARGET_TRIPLET "x86-windows" CACHE INTERNAL "Triplet to use for Windows development. Do not modify.")
include(${CMAKE_CURRENT_LIST_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake)