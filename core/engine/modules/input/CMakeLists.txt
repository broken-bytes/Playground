FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB SRC "src/*.cxx")

add_library(Input SHARED ${SRC} ${HEADERS})

target_include_directories(Input PRIVATE include)
target_include_directories(Input PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/events/include)

find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(Input PRIVATE Events SDL3::SDL3)

set_target_properties(Input PROPERTIES OUTPUT_NAME "Playground.Input")
