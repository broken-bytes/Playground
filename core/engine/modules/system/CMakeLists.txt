FILE(GLOB_RECURSE HEADERS "include/**.h*")
FILE(GLOB_RECURSE SRC "src/**.c*")

add_library(System SHARED ${SRC} ${HEADERS})

target_include_directories(System PRIVATE include)
target_include_directories(System PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/logger/include)

target_link_libraries(System PRIVATE Logger)
target_link_libraries(System PRIVATE SDL3::SDL3)

set_target_properties(System PROPERTIES OUTPUT_NAME "Playground.System")
