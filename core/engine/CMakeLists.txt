add_subdirectory(modules)

FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(PlaygroundCore SHARED ${SRC} ${HEADERS})
target_include_directories(PlaygroundCore PRIVATE include)

target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)

target_link_directories(PlaygroundCore PRIVATE ${EASTL_LIB_DIR})
target_include_directories(PlaygroundCore PRIVATE ${EASTL_INC_DIR} ${EABASE_INC_DIR})

target_link_libraries(PlaygroundCore PRIVATE AssetLoader Audio Events flecs::flecs_static Input IO Logger Physics Rendering Shared System Profiler Math EASTL concurrentqueue)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/assetloader/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/audio/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/input/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/io/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/logger/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/math/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/physics/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/profiler/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/rendering/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/system/include)
target_include_directories(PlaygroundCore PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/events/include)
