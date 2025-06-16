cmake_minimum_required(VERSION 3.24)
project(PlaygroundScripting LANGUAGES Swift)

set(CMAKE_OBJECT_PATH_MAX 200)
set(CMAKE_SWIFT_FLAGS_DEBUG "-g -Xlinker -DEBUG -Xlinker /INCREMENTAL:NO")

include(ExternalProject)

ExternalProject_Add(PlaygroundScriptingMacros
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Macros"
  INSTALL_COMMAND "")
ExternalProject_Get_Property(PlaygroundScriptingMacros BINARY_DIR)
if(CMAKE_HOST_WIN32)
  set(PlaygroundMacrosPath "${BINARY_DIR}/PlaygroundScriptingMacros.exe#PlaygroundScriptingMacros")
else()
  set(PlaygroundMacrosPath "${BINARY_DIR}/PlaygroundScriptingMacros#PlaygroundScriptingMacros")
endif()

file(GLOB_RECURSE ENGINE_SWIFT_SOURCES "Playground/*.swift")

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(SWIFT_FLAGS
        -DDEBUG
        -g
        -load-plugin-executable "${PlaygroundMacrosPath}"
        -debug-info-format=codeview
  )
else()
    set(SWIFT_FLAGS "")
endif()

message(INFO ${SWIFT_FLAGS})

add_library(Playground SHARED ${ENGINE_SWIFT_SOURCES})
add_dependencies(Playground PlaygroundScriptingMacros)
target_compile_options(Playground PRIVATE -load-plugin-executable "${PlaygroundMacrosPath}" ${SWIFT_FLAGS})
