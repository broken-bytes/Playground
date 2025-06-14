#include "playground/Engine.hxx"
#include "playground/AssetManager.hxx"
#include "playground/SceneManager.hxx"
#include "playground/ECS.hxx"
#include <chrono>
#include <string>
#include <thread>
#include <audio/Audio.hxx>
#include <input/Input.hxx>
#include <rendering/Rendering.hxx>
#include <rendering/Mesh.hxx>
#include <system/System.hxx>
#include <events/Events.hxx>
#include <events/Event.hxx>
#include <events/SystemEvent.hxx>
#include <assetloader/AssetLoader.hxx>
#include <io/IO.hxx>
#include <logger/ConsoleLogger.hxx>
#include <logger/Logger.hxx>
#include <profiler/Profiler.hxx>
#include <tracy/Tracy.hpp>
#include <thread>
#include <future>

typedef void(*ScriptingEventCallback)(playground::events::Event* event);

bool isRunning = true;

std::thread renderThread;

void Shutdown() {
    isRunning = false;

    playground::input::Shutdown();
    playground::rendering::Shutdown();
    renderThread.join();
}

void SubscribeToEventsFromScripting(playground::events::EventType type, ScriptingEventCallback callback) {
    Subscribe(type, [callback](playground::events::Event* event) {
        callback(event);
    });
}

void PlaygroundCoreMain(const PlaygroundConfig& config) {
    playground::audio::Init();
    playground::input::Init();
    playground::scenemanager::Init();
#if ENABLE_INSPECTOR
    playground::ecs::Init(true);
#else
    playground::ecs::Init(false);
#endif

    auto window = playground::system::Init(config.Window);

    std::promise<void> rendererReadyPromise;
    std::future<void> rendererReadyFuture = rendererReadyPromise.get_future();

    renderThread = std::thread([window, config, &rendererReadyPromise] {
        playground::rendering::Init(window, config.Width, config.Height, config.IsOffscreen, rendererReadyPromise);
        });

    Subscribe(playground::events::EventType::System, [](playground::events::Event* event) {
        if (reinterpret_cast<playground::events::SystemEvent*>(event)->SystemType == playground::events::SystemEventType::Quit) {
            Shutdown();
        }
        });

    playground::logging::logger::AddLogger(std::make_shared<playground::logging::ConsoleLogger>());
    playground::logging::logger::SetLogLevel(LogLevel::Info);

#if ENABLE_PROFILER
    tracy::StartupProfiler();
#endif

    config.Delegate("Logger_Info", playground::logging::logger::Info);
    config.Delegate("Logger_Warn", playground::logging::logger::Info);
    config.Delegate("Logger_Error", playground::logging::logger::Info);

    config.Delegate("AssetManager_LoadModel\0", playground::assetmanager::LoadModel);
    config.Delegate("AssetManager_LoadMaterial\0", playground::assetmanager::LoadMaterial);

    config.Delegate("ECS_CreateEntity\0", playground::ecs::CreateEntity);
    config.Delegate("ECS_DestroyEntity\0", playground::ecs::DestroyEntity);
    config.Delegate("ECS_SetParent\0", playground::ecs::SetParent);
    config.Delegate("ECS_GetParent\0", playground::ecs::GetParent);
    config.Delegate("ECS_GetEntityByName\0", playground::ecs::GetEntityByName);
    config.Delegate("ECS_RegisterComponent\0", playground::ecs::RegisterComponent);
    config.Delegate("ECS_AddComponent\0", playground::ecs::AddComponent);
    config.Delegate("ECS_SetComponent\0", playground::ecs::SetComponent);
    config.Delegate("ECS_GetComponent\0", playground::ecs::GetComponent);
    config.Delegate("ECS_HasComponent\0", playground::ecs::HasComponent);
    config.Delegate("ECS_DestroyComponent\0", playground::ecs::DestroyComponent);
    config.Delegate("ECS_CreateSystem\0", playground::ecs::CreateSystem);
    config.Delegate("ECS_GetComponentBuffer\0", playground::ecs::GetComponentBuffer);
    config.Delegate("ECS_GetIteratorSize\0", playground::ecs::GetIteratorSize);
    config.Delegate("ECS_GetEntitiesFromIterator\0", playground::ecs::GetEntitiesFromIterator);
    config.Delegate("ECS_CreateHook\0", playground::ecs::CreateHook);
    config.Delegate("ECS_DeleteAllEntitiesByTag\0", playground::ecs::DeleteAllEntitiesByTag);
    config.Delegate("ECS_CreateTag\0", playground::ecs::CreateTag);
    config.Delegate("ECS_AddTag\0", playground::ecs::AddTag);

    config.Delegate("Rendering_Update\0", playground::rendering::Update);
    config.Delegate("Rendering_PostFrame\0", playground::rendering::PostFrame);
    config.Delegate("Rendering_ReadBackBuffer\0", playground::rendering::ReadbackBuffer);
    config.Delegate("Rendering_CreateCamera\0", playground::rendering::CreateCamera);
    config.Delegate("Rendering_SetCameraFOV\0", playground::rendering::SetCameraFOV);
    config.Delegate("Rendering_SetCameraAspectRatio\0", playground::rendering::SetCameraAspectRatio);
    config.Delegate("Rendering_SetCameraNear\0", playground::rendering::SetCameraNear);
    config.Delegate("Rendering_SetCameraFar\0", playground::rendering::SetCameraFar);
    config.Delegate("Rendering_SetCameraPosition\0", playground::rendering::SetCameraPosition);
    config.Delegate("Rendering_SetCameraRotation\0", playground::rendering::SetCameraRotation);
    config.Delegate("Rendering_SetCameraTarget\0", playground::rendering::SetCameraRenderTarget);
    config.Delegate("Rendering_DestroyCamera\0", playground::rendering::DestroyCamera);

    config.Delegate("Input_Update\0", playground::input::Update);

    config.Delegate("Events_Subscribe", SubscribeToEventsFromScripting);

    tracy::SetThreadName("Game");

    playground::logging::logger::Info("Playground Core Engine started.");
    playground::logging::logger::Info("Initializing Scripting Layer...");

    rendererReadyFuture.wait();

    config.startupCallback();

    static const char* CPU_FRAME = "CPU:Update";

    double deltaTime = 0;

    auto now = std::chrono::high_resolution_clock::now();
    while (isRunning) {
        FrameMark;
        FrameMarkStart(CPU_FRAME);
        {
            ZoneScopedN("Input");
            ZoneColor(tracy::Color::AliceBlue);
            playground::input::Update();
        }
        {
            ZoneScopedN("Scripts");
            ZoneColor(tracy::Color::LightSeaGreen);
        }
        {
            ZoneScopedN("Scene");
            ZoneColor(tracy::Color::LightSalmon);
            playground::ecs::Update(deltaTime);
            //playground::scenemanager::Update();
        }
        auto next = std::chrono::high_resolution_clock::now();
        const auto int_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(next - now);
        deltaTime = (double)int_ns.count() / 1000000000.0;
        now = next;
        FrameMarkEnd(CPU_FRAME);
    }

#if ENABLE_PROFILER
    tracy::ShutdownProfiler();
#endif
}
