cmake_minimum_required(VERSION 3.24)
project(PlaygroundScripting LANGUAGES Swift)

set(CMAKE_OBJECT_PATH_MAX 200)

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

add_library(Playground SHARED ${ENGINE_SWIFT_SOURCES})
add_dependencies(Playground PlaygroundScriptingMacros)
target_compile_options(Playground PRIVATE -load-plugin-executable "${PlaygroundMacrosPath}")
