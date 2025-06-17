FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.c*")

find_package(unofficial-sqlite3 REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(FreeImage CONFIG REQUIRED)

add_library(AssetPipeline SHARED ${SRC} ${HEADERS})

target_include_directories(AssetPipeline PRIVATE include)
target_include_directories(AssetPipeline PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/audio/include)
target_include_directories(AssetPipeline PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/rendering/include)
target_include_directories(AssetPipeline PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(AssetPipeline PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/logger/include)
target_include_directories(AssetPipeline PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/io/include)
target_include_directories(AssetPipeline PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/assetloader/include)

target_link_libraries(AssetPipeline PRIVATE glm::glm unofficial::sqlite3::sqlite3 assimp::assimp libzip::zip freeimage::FreeImagePlus)

# Link all engine libraries
target_link_libraries(AssetPipeline PRIVATE Audio AssetLoader Logger Rendering Shared)

add_custom_command(TARGET AssetPipeline POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:AssetPipeline> $<TARGET_RUNTIME_DLLS:AssetPipeline>
  COMMAND_EXPAND_LISTS
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC} ${HEADERS})
