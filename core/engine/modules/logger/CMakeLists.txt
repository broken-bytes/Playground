FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(Logger SHARED ${SRC} ${HEADERS})

target_include_directories(Logger PRIVATE include)

find_package(cereal CONFIG REQUIRED)

target_link_libraries(Logger PRIVATE cereal::cereal)

set_target_properties(Logger PROPERTIES OUTPUT_NAME "Playground.Logger")
