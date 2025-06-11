FILE(GLOB_RECURSE HEADERS "include/*.h*")

Set(CMAKE_CXX_STANDARD 23)

add_custom_target(Shared SOURCES ${HEADERS} ${SRC})
