FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(Events SHARED ${SRC} ${HEADERS})

target_include_directories(Events PRIVATE include)
target_include_directories(Events PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)

set_target_properties(Events PROPERTIES OUTPUT_NAME "Playground.Events")