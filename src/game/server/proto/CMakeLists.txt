add_library(savedata-protoc SHARED OBJECT "${CMAKE_CURRENT_LIST_DIR}/savedata.proto")
target_link_libraries(savedata-protoc PRIVATE ${Protobuf_LIBRARIES})
target_include_directories(savedata-protoc INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)


message("🏗️ Generating protobuf.")
protobuf_generate(
               TARGET savedata-protoc
               IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}"
               PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
)
