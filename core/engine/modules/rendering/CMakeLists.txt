FILE(GLOB_RECURSE HEADERS "include/rendering/**.h*")
FILE(GLOB_RECURSE SRC "src/**.c*")

add_library(Rendering SHARED ${SRC} ${HEADERS})

target_include_directories(Rendering PRIVATE include)
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/logger/include)
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/math/include)
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/io/include)

# Temporary for testing
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/assetloader/include)

find_package(FreeImage CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(directx-dxc CONFIG REQUIRED)

target_link_libraries(Rendering PUBLIC Logger AssetLoader)
target_link_libraries(Rendering PRIVATE SDL3::SDL3 spdlog::spdlog glm::glm-header-only freeimage::FreeImage freeimage::FreeImagePlus Microsoft::DirectXShaderCompiler)

if (WIN32)
    find_package(directx-headers CONFIG REQUIRED)

    target_link_libraries(Rendering PRIVATE Microsoft::DirectX-Headers dxgi d3d12 dxguid)
endif()

find_path(ATOMIC_QUEUE_INCLUDE_DIRS "atomic_queue/atomic_queue.h")
target_include_directories(Rendering PRIVATE ${ATOMIC_QUEUE_INCLUDE_DIRS})

set_target_properties(Rendering PROPERTIES OUTPUT_NAME "Playground.Rendering")
