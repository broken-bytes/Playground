FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(AssetLoader SHARED ${SRC} ${HEADERS})

target_include_directories(AssetLoader PRIVATE include)
target_include_directories(AssetLoader PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(AssetLoader PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/rendering/include)

target_link_libraries(AssetLoader PRIVATE Rendering)

set_target_properties(AssetLoader PROPERTIES OUTPUT_NAME "Playground.AssetLoader")