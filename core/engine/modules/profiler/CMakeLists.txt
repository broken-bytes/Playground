FILE(GLOB_RECURSE HEADERS "include/*.h*")
FILE(GLOB_RECURSE SRC "src/*.cxx")

add_library(Profiler SHARED ${SRC} ${HEADERS}  ${CMAKE_SOURCE_DIR}/vendor/tracy/public/TracyClient.cpp)

target_include_directories(Profiler PRIVATE include)
target_include_directories(Profiler PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(Profiler PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)

set_target_properties(Profiler PROPERTIES OUTPUT_NAME "Playground.Profiler")

if (WIN32)
    find_package(directx-headers CONFIG REQUIRED)
    if(ENABLE_PROFILER)
        target_compile_definitions(Profiler PUBLIC -DTRACY_ENABLE)
    endif()
    target_compile_definitions(Profiler PUBLIC -DTRACY_MANUAL_LIFETIME -DTRACY_DELAYED_INIT -DTRACY_GPU_CONTEXT -DTRACY_GPU_D3D12)
    target_link_libraries(Profiler PRIVATE Microsoft::DirectX-Headers dxgi d3d12 dxguid)
endif()
