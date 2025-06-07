cmake_minimum_required(VERSION 3.24)
project(PlaygroundScriptingMacros LANGUAGES Swift)

set(CMAKE_Swift_FLAGS "-enable-experimental-feature Macros")

include(FetchContent)

FetchContent_Declare(SwiftSyntax
  GIT_REPOSITORY https://github.com/apple/swift-syntax.git
  GIT_TAG f99ae8aa18f0cf0d53481901f88a0991dc3bd4a2)
FetchContent_MakeAvailable(SwiftSyntax)

add_executable(PlaygroundScriptingMacros main.swift ComponentMacro.swift)
target_compile_options(PlaygroundScriptingMacros PRIVATE -parse-as-library)
target_link_libraries(PlaygroundScriptingMacros
  SwiftSyntax
  SwiftSyntaxMacros
  SwiftCompilerPlugin
)
