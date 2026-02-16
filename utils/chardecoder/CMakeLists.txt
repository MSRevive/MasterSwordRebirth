add_executable(chardecoder)

################################################################################
# Target Properties
################################################################################
set_common_properties(chardecoder)

target_include_directories(chardecoder PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR})

################################################################################
# Source groups
################################################################################
set(Source_Files
	"chardecoder.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
	${Source_Files}
)

target_sources(chardecoder PRIVATE ${ALL_FILES})