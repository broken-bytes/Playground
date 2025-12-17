if(WIN32)
add_executable(PlaygroundApp app.cxx executable_path.cxx)
endif()

target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/app/include/app)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/vendor/coreclr/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/core/editor/assetpipeline/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/core/editor/assetdatabase/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/assetloader/include)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)
target_include_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/engine/native/modules/shared/include)

find_package(Stb REQUIRED)

target_link_directories(PlaygroundApp PRIVATE ${PLAYGROUND_LIB_DIR})
if(WIN32)
    target_link_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/vendor/coreclr/lib/win64)
else()
    target_link_directories(PlaygroundApp PRIVATE ${CMAKE_SOURCE_DIR}/vendor/coreclr/lib/linux)
endif()
target_link_libraries(PlaygroundApp PRIVATE PlaygroundCore Profiler SDL3::SDL3 Shared nethost)

target_include_directories(PlaygroundApp PRIVATE ${Stb_INCLUDE_DIR})

target_compile_definitions(PlaygroundApp PRIVATE -DTRACY_IMPORTS)

# Set the library output directory for all libraries
set_target_properties(PlaygroundApp PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
set_target_properties(PlaygroundApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
set_target_properties(PlaygroundApp PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)
