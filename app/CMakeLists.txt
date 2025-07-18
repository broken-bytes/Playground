if(WIN32)
add_executable(PlaygroundApp WIN32 app.cxx)
endif()

target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/core/editor/assetpipeline/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/core/editor/assetdatabase/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/assetloader/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)

find_package(Stb REQUIRED)

target_link_directories(PlaygroundApp PRIVATE ${PLAYGROUND_LIB_DIR})
target_link_libraries(PlaygroundApp PRIVATE Playground PlaygroundCore SDL3::SDL3 Shared)

target_include_directories(PlaygroundApp PRIVATE ${Stb_INCLUDE_DIR})

# Set the library output directory for all libraries
set_target_properties(PlaygroundApp PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(PlaygroundApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(PlaygroundApp PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
