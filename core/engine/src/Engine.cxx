#include "playground/Engine.hxx"
#include "playground/SceneManager.hxx"
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
    auto window = playground::system::Init(config.Window);
    renderThread = std::thread([window, config] {
        playground::rendering::Init(window, config.Width, config.Height, config.IsOffscreen);
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

    config.Delegate("Playground_CreateGameObject\0", playground::scenemanager::CreateGameObject);
    config.Delegate("Playground_GetGameObjectTransform\0", playground::gameobjects::GetGameObjectTransform);
    config.Delegate("Playground_DestroyGameObject\0", playground::scenemanager::DestroyGameObject);
    config.Delegate("Playground_Shutdown\0", Shutdown);
    config.Delegate("Rendering_PreFrame\0", playground::rendering::PreFrame);
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
    config.Delegate("Logger_Info", playground::logging::logger::Info);
    config.Delegate("Logger_Warn", playground::logging::logger::Info);
    config.Delegate("Logger_Error", playground::logging::logger::Info);

    tracy::SetThreadName("Game");

    static const char* CPU_FRAME = "CPU:Update";

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
            config.updateCallback();
        }
        FrameMarkEnd(CPU_FRAME);
    }

#if ENABLE_PROFILER
    tracy::ShutdownProfiler();
#endif
}
