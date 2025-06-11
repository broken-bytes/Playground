find_package(SDL3 CONFIG REQUIRED)

if(WIN32)
add_executable(Playground WIN32 app.cxx)
endif()

target_include_directories(Playground PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/include)
target_include_directories(Playground PRIVATE ${CMAKE_SOURCE_DIR}/core/editor/assetpipeline/include)
target_include_directories(Playground PRIVATE ${CMAKE_SOURCE_DIR}/core/editor/assetdatabase/include)
target_include_directories(Playground PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/assetloader/include)
target_include_directories(Playground PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)

find_package(Stb REQUIRED)

target_link_libraries(Playground PRIVATE PlaygroundCore SDL3::SDL3 AssetDatabase AssetPipeline)

target_include_directories(Playground PRIVATE ${Stb_INCLUDE_DIR})

# Set the library output directory for all libraries
set_target_properties(Playground PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(Playground PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(Playground PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
