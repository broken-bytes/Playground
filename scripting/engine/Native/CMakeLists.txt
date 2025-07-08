cmake_minimum_required(VERSION 3.24)

file(GLOB_RECURSE ENGINE_NATIVE_SOURCES "src/*.cxx")
file(GLOB_RECURSE ENGINE_NATIVE_HEADERS "include/*.h")

add_library(PlaygroundNative STATIC ${ENGINE_NATIVE_SOURCES} ${ENGINE_NATIVE_HEADERS})

target_include_directories(PlaygroundNative PUBLIC
    include
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# Then apply this to your native lib
set_target_properties(PlaygroundNative PROPERTIES
    MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
)
