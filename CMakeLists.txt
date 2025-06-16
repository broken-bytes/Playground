cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(PROJECT_NAME "Playground" LANGUAGES C CXX)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(${PROJECT_NAME})

enable_testing()


include(FetchContent)

FetchContent_Declare(flecs
  GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
  GIT_TAG bc0a901e4608a12f5162f32421589b74d4a35f14)
FetchContent_MakeAvailable(flecs)

FetchContent_Declare(SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG f5d1402c287963bf71f7a4cf14f16bbedffae143)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(SIMDe
  GIT_REPOSITORY https://github.com/simd-everywhere/simde.git
  GIT_TAG 51743e7920b6e867678cb50e9c62effe28f70b33)
FetchContent_MakeAvailable(SIMDe)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

if(ENABLE_PROFILER)
    add_compile_definitions(ENABLE_TRACY)
    add_compile_definitions(ENABLE_PROFILER)
endif()

if(ENABLE_INSPECTOR)
    add_compile_definitions(ENABLE_INSPECTOR FLECS_DEBUG)
endif()


# Set the output directory for the build
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/out)

# Set output directories for all configurations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)

# Set output directories for each configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR}/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR}/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIR}/lib)

add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(tools)

# Set the library output directory for all libraries
set_target_properties(AssetDatabase PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(AssetDatabase PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(AssetPipeline PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(AssetPipeline PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(Audio PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Audio PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(Events PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Events PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(Input PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Input PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(IO PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(IO PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(Logger PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Logger PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(Math PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Math PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(Profiler PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Profiler PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(Rendering PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Rendering PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

set_target_properties(System PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(System PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)

# Set the runtime output directory for all libraries
set_target_properties(AssetDatabase PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(AssetPipeline PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(Audio PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(Events PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(Input PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(IO PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(Logger PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(Math PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(Profiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(Rendering PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(System PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(PlaygroundCore PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
