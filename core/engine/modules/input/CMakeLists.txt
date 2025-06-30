FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB SRC "src/*.cxx")

add_library(Input SHARED ${SRC} ${HEADERS})

target_include_directories(Input PRIVATE include)
target_include_directories(Input PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/events/include)
target_include_directories(Input PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
if (WIN32)
    message(INFO ${GDK_PATH})
    #target_include_directories(Input PRIVATE ${GDK_PATH}/GameKit/Include)
    #target_link_directories(Input PRIVATE ${GDK_PATH}/GameKit/Lib/amd64)
    target_link_libraries(Input PRIVATE GameInput)

endif()

target_link_directories(Input PRIVATE ${EASTL_LIB_DIR})
target_include_directories(Input PRIVATE ${EASTL_INC_DIR} ${EABASE_INC_DIR})

target_link_libraries(Input PRIVATE Events SDL3::SDL3 EASTL Shared concurrentqueue)

set_target_properties(Input PROPERTIES OUTPUT_NAME "Playground.Input")
