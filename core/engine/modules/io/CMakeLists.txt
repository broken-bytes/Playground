FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB SRC "src/*.cxx")

find_package(libzip CONFIG REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
find_package(stduuid REQUIRED)
find_package(glm CONFIG REQUIRED)

add_library(IO SHARED ${SRC} ${HEADERS})

target_include_directories(IO PRIVATE include)

target_link_libraries(IO PUBLIC SDL3::SDL3 unofficial-sodium::sodium libzip::zip stduuid glm::glm)

set_target_properties(IO PROPERTIES OUTPUT_NAME "Playground.IO")
