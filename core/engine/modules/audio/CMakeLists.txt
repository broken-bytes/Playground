FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(Audio SHARED ${SRC} ${HEADERS})

target_include_directories(Audio PRIVATE include)
target_include_directories(Audio PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)

# Add steam audio via lib and path as it has no vcpkg support yet
# Check that the paths are supplied and raise an eror if not
if(DEFINED STEAMAUDIO_INCLUDE_DIR)
	target_include_directories(Audio PRIVATE ${STEAMAUDIO_INCLUDE_DIR})
else()
	message(FATAL_ERROR "STEAMAUDIO_INCLUDE_DIR not set")
endif()
if(DEFINED STEAMAUDIO_LIB_DIR)
	target_link_directories(Audio PRIVATE ${STEAMAUDIO_LIB_DIR})
else()
	message(FATAL_ERROR "STEAMAUDIO_LIB_DIR not set")
endif()

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

target_link_libraries(Audio PRIVATE SDL3::SDL3 phonon_fmod fmodstudio_vc fmod_vc Shared)

set_target_properties(Audio PROPERTIES OUTPUT_NAME "Playground.Audio")

message(${RUNTIME_OUTPUT_DIRECTORY}/phonon.dll)
message(${RUNTIME_OUTPUT_DIRECTORY}/fmodstudio.dll)

add_custom_command(
    TARGET Audio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${STEAMAUDIO_LIB_DIR}/phonon.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/phonon.dll
    COMMENT "Copying phonon to out dir"
)
