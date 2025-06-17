FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.c*")

find_package(unofficial-sqlite3 REQUIRED)

add_library(AssetDatabase SHARED ${SRC} ${HEADERS})

target_include_directories(AssetDatabase PRIVATE include)
target_include_directories(AssetDatabase PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(AssetDatabase PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/logger/include)
target_include_directories(AssetDatabase PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/io/include)

target_link_libraries(AssetDatabase PRIVATE Logger IO Shared)
target_link_libraries(AssetDatabase PRIVATE unofficial::sqlite3::sqlite3)
