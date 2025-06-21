add_subdirectory(modules)

FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(PlaygroundCore SHARED ${SRC} ${HEADERS})
target_include_directories(PlaygroundCore PRIVATE include)

target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)

#find_package(flecs CONFIG REQUIRED)

target_link_libraries(PlaygroundCore PRIVATE AssetLoader Audio Events flecs Input IO Logger Rendering System Profiler Math EASTL concurrentqueue)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/assetloader/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/audio/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/input/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/io/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/logger/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/math/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/profiler/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/rendering/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/system/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/events/include)
