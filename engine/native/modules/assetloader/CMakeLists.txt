FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(AssetLoader SHARED ${SRC} ${HEADERS})

find_package(cereal CONFIG REQUIRED)

target_include_directories(AssetLoader PRIVATE include)
target_include_directories(AssetLoader PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/shared/include)
target_include_directories(AssetLoader PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/io/include)
target_include_directories(AssetLoader PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/math/include)

target_link_libraries(AssetLoader PRIVATE cereal::cereal IO Math Shared)

set_target_properties(AssetLoader PROPERTIES OUTPUT_NAME "Playground.AssetLoader")
