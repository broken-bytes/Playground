FILE(GLOB_RECURSE HEADERS "include/**.h*")
FILE(GLOB_RECURSE SRC "src/**.c*")

add_library(System SHARED ${SRC} ${HEADERS})

target_include_directories(System PRIVATE include)
target_include_directories(System PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)

target_link_libraries(System PRIVATE SDL3::SDL3 Shared)

set_target_properties(System PROPERTIES OUTPUT_NAME "Playground.System")
