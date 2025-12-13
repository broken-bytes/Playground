FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SOURCES "src/*.c*")

Set(CMAKE_CXX_STANDARD 23)

add_library(Shared SHARED ${SOURCES} ${HEADERS})

target_include_directories(Shared PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)
target_include_directories(Shared PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/shared/include)

target_link_directories(Shared PRIVATE ${EASTL_LIB_DIR})
target_include_directories(Shared PRIVATE ${EABASE_INC_DIR} ${EASTL_INC_DIR} )

target_link_libraries(Shared PRIVATE spdlog::spdlog EASTL concurrentqueue Profiler)
