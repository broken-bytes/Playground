FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB SRC "src/*.cxx")

add_library(Physics SHARED ${SRC} ${HEADERS})

target_include_directories(Physics PRIVATE include)
target_include_directories(Physics PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/events/include)
target_include_directories(Physics PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)

target_link_directories(Physics PRIVATE ${EASTL_LIB_DIR})
target_include_directories(Physics PRIVATE ${EASTL_INC_DIR} ${EABASE_INC_DIR})
target_link_libraries(Physics PRIVATE Events EASTL glm::glm Shared concurrentqueue)

target_include_directories(Physics PRIVATE ${PHYSX_INC_DIR})
target_link_directories(Physics PRIVATE ${PHYSX_LIB_DIR})
target_link_libraries(Physics PRIVATE Events Shared PhysX_64 PhysXCommon_64 PhysXFoundation_64 PVDRuntime_64 PhysXPvdSDK_static_64)

set_target_properties(Physics PROPERTIES OUTPUT_NAME "Playground.Physics")
