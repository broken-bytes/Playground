FILE(GLOB_RECURSE HEADERS "include/rendering/**.h*")
FILE(GLOB_RECURSE SRC "src/**.c*")

add_library(Rendering SHARED ${SRC} ${HEADERS})

target_include_directories(Rendering PRIVATE include)
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/shared/include)
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/math/include)
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/io/include)
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/profiler/include)

target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/vendor/tracy/public)

# Temporary for testing
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/assetloader/include)

find_package(FreeImage CONFIG REQUIRED)
find_package(Stb REQUIRED)

target_include_directories(Rendering PRIVATE ${Stb_INCLUDE_DIR})

target_link_directories(Rendering PRIVATE ${EASTL_LIB_DIR})
target_include_directories(Rendering PRIVATE ${EASTL_INC_DIR} ${EABASE_INC_DIR})
target_include_directories(Rendering PRIVATE ${CMAKE_SOURCE_DIR}/core/engine/modules/math/include)

target_link_libraries(Rendering PRIVATE AssetLoader Profiler Math Shared EASTL)
target_link_libraries(Rendering PRIVATE SDL3::SDL3 freeimage::FreeImage freeimage::FreeImagePlus concurrentqueue)

if (WIN32)
    find_package(directx-headers CONFIG REQUIRED)

    target_link_libraries(Rendering PRIVATE Microsoft::DirectX-Headers dxgi d3d12 dxguid)
endif()

set_target_properties(Rendering PROPERTIES OUTPUT_NAME "Playground.Rendering")
