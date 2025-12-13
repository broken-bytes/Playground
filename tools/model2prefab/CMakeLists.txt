FILE(GLOB_RECURSE HEADERS "include/rendering/**.h*")
FILE(GLOB_RECURSE SRC "src/**.c*")

add_executable(model2prefab ${SRC} ${HEADERS})

find_package(cereal CONFIG REQUIRED)

target_include_directories(model2prefab PRIVATE include)
target_include_directories(model2prefab PRIVATE ${CMAKE_SOURCE_DIR}/core/editor/assetpipeline/include)
target_include_directories(model2prefab PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/assetloader/include)
target_include_directories(model2prefab PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/math/include)

target_link_libraries(model2prefab PRIVATE AssetPipeline AssetLoader cereal::cereal Math)

# Set the library output directory for all libraries
set_target_properties(model2prefab PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(model2prefab PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/tools/bin)
set_target_properties(model2prefab PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
