FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB SRC "src/*.cxx")

add_library(Math SHARED ${SRC} ${HEADERS})

target_include_directories(Math PUBLIC include ${simde_SOURCE_DIR})
target_link_libraries(Math PRIVATE glm::glm)
