FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(AssetLoader SHARED ${SRC} ${HEADERS})

find_package(cereal CONFIG REQUIRED)

target_include_directories(AssetLoader PRIVATE include)
target_include_directories(AssetLoader PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(AssetLoader PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/io/include)

target_link_libraries(AssetLoader PRIVATE cereal::cereal IO Shared)

set_target_properties(AssetLoader PROPERTIES OUTPUT_NAME "Playground.AssetLoader")
