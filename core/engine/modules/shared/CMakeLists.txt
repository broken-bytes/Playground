FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SOURCES "src/*.c*")

Set(CMAKE_CXX_STANDARD 23)

add_library(Shared SHARED ${SOURCES} ${HEADERS})

target_include_directories(Shared PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)

target_link_libraries(Shared PRIVATE EASTL)
