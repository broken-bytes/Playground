FILE(GLOB_RECURSE HEADERS "include/**.h*")
FILE(GLOB_RECURSE SRC "src/**.c*")

add_library(UI SHARED ${SRC} ${HEADERS})

target_include_directories(System PRIVATE include)
target_include_directories(System PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/logger/include)

set_target_properties(UI PROPERTIES OUTPUT_NAME "Playground.UI")
