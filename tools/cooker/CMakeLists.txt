FILE(GLOB_RECURSE HEADERS "include/rendering/**.h*")
FILE(GLOB_RECURSE SRC "src/**.c*")

add_executable(Cooker ${SRC} ${HEADERS})

find_package(cereal CONFIG REQUIRED)

target_include_directories(Cooker PRIVATE include)
target_include_directories(Cooker PRIVATE ${CMAKE_SOURCE_DIR}/core/editor/assetpipeline/include)
target_include_directories(Cooker PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/assetloader/include)
target_include_directories(Cooker PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/math/include)

target_include_directories(Cooker PRIVATE ${CMAKE_SOURCE_DIR}/vendor/dxc/inc)
target_link_directories(Cooker PRIVATE ${CMAKE_SOURCE_DIR}/vendor/dxc/lib)

target_link_libraries(Cooker PRIVATE AssetPipeline AssetLoader cereal::cereal Math)

# Set the library output directory for all libraries
set_target_properties(Cooker PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Cooker PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/tools/bin)
set_target_properties(Cooker PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
