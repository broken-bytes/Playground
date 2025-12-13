FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(Audio SHARED ${SRC} ${HEADERS})

target_include_directories(Audio PRIVATE include)
target_include_directories(Audio PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/shared/include)
target_include_directories(Audio PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/math/include)
target_include_directories(Audio PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/io/include)
target_include_directories(Audio PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)

if(DEFINED FMOD_CORE_INC_DIR)
    target_include_directories(Audio PRIVATE ${FMOD_CORE_INC_DIR})
else()
    message(FATAL_ERROR "FMOD_CORE_INC_DIR not set")
endif()

if(DEFINED FMOD_CORE_LIB_DIR)
    target_link_directories(Audio PRIVATE ${FMOD_CORE_LIB_DIR})
else()
    message(FATAL_ERROR "FMOD_CORE_LIB_DIR not set")
endif()

if(DEFINED FMOD_STUDIO_INC_DIR)
    target_include_directories(Audio PRIVATE ${FMOD_STUDIO_INC_DIR})
else()
    message(FATAL_ERROR "FMOD_STUDIO_INC_DIR not set")
endif()

if(DEFINED FMOD_STUDIO_LIB_DIR)
    target_link_directories(Audio PRIVATE ${FMOD_STUDIO_LIB_DIR})
else()
    message(FATAL_ERROR "FMOD_STUDIO_LIB_DIR not set")
endif()

target_link_libraries(Audio PRIVATE SDL3::SDL3 fmodstudio_vc fmod_vc Shared Profiler Math IO)

set_target_properties(Audio PROPERTIES OUTPUT_NAME "Playground.Audio")

message(${RUNTIME_OUTPUT_DIRECTORY}/fmodstudio.dll)
