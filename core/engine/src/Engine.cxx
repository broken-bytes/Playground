#include "playground/Engine.hxx"
#include "playground/AssetManager.hxx"
#include "playground/ECS.hxx"
#include "playground/DrawCallbatcher.hxx"
#include "playground/InputManager.hxx"
#include <chrono>
#include <string>
#include <thread>
#include <shared/Hardware.hxx>
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
#include <physics/Physics.hxx>
#include <profiler/Profiler.hxx>
#include <math/Math.hxx>
#include <SDL3/SDL.h>
#include <tracy/Tracy.hpp>
#include <thread>
#include <future>

typedef void(*ScriptingEventCallback)(playground::events::Event* event);

bool isRunning = true;

std::thread gameThread;
std::thread renderThread;

double timeSinceStart = 0.0;
double deltaTime = 0.0;

double GetTimeSinceStart() {
    return timeSinceStart;
}

double GetDeltaTime() {
    return deltaTime;
}

void Shutdown() {
    isRunning = false;
}

void SubscribeToEventsFromScripting(playground::events::EventType type, ScriptingEventCallback callback) {
    Subscribe(type, [callback](playground::events::Event* event) {
        callback(event);
    });
}

/// 0 - OK
/// 1 - Error (Unknown error)
/// 2 - Error (No AVX or AVX2 support)
uint8_t PlaygroundCoreMain(const PlaygroundConfig& config) {
    playground::hardware::Init();

    if (playground::hardware::SupportsAVX2()) {
        playground::logging::logger::Info("CPU supports AVX2.");
    } else if (playground::hardware::SupportsAVX()){
        playground::logging::logger::Warn("CPU supports AVX.");
    }
    else {
        playground::logging::logger::Error("CPU does not support AVX or AVX2. This application requires at least AVX support.");
        return 2;
    }

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
    config.Delegate("AssetManager_LoadPhysicsMaterial\0", playground::assetmanager::LoadPhysicsMaterial);

    config.Delegate("Batcher_Batch\0", playground::drawcallbatcher::Batch);
    config.Delegate("Batcher_SetSun\0", playground::drawcallbatcher::SetSun);
    config.Delegate("Batcher_AddCamera\0", playground::drawcallbatcher::AddCamera);

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
    config.Delegate("ECS_GetIteratorOffset\0", playground::ecs::GetIteratorOffset);
    config.Delegate("ECS_GetEntitiesFromIterator\0", playground::ecs::GetEntitiesFromIterator);
    config.Delegate("ECS_CreateHook\0", playground::ecs::CreateHook);
    config.Delegate("ECS_DeleteAllEntitiesByTag\0", playground::ecs::DeleteAllEntitiesByTag);
    config.Delegate("ECS_CreateTag\0", playground::ecs::CreateTag);
    config.Delegate("ECS_AddTag\0", playground::ecs::AddTag);

    config.Delegate("Input_GetAxis\0", playground::inputmanager::GetAxis);
    config.Delegate("Input_IsButtonPressed\0", playground::inputmanager::IsButtonPressed);
    config.Delegate("Input_IsButtonDown\0", playground::inputmanager::IsButtonDown);
    config.Delegate("Input_IsButtonUp\0", playground::inputmanager::IsButtonUp);

    config.Delegate("Physics_AddRigidbody\0", playground::physics::AddBody);
    config.Delegate("Physics_AddBoxCollider\0", playground::physics::AddBoxCollider);

    config.Delegate("Time_GetTimeSinceStart\0", GetTimeSinceStart);
    config.Delegate("Time_GetDeltaTime\0", GetDeltaTime);

    config.Delegate("Input_Update\0", playground::input::Update);

    config.Delegate("Events_Subscribe", SubscribeToEventsFromScripting);

    config.Delegate("Math_Mat4FromPRS", playground::math::utils::Mat4FromPRS);
    config.Delegate("Math_Mat4FromPRSBulk", playground::math::utils::Mat4FromPRSBulk);

    playground::logging::logger::Info("Playground Core Engine started.");
    playground::logging::logger::Info("Initializing Scripting Layer...");

    rendererReadyFuture.wait();

    playground::input::Init(config.Window);
    playground::inputmanager::Init();
    playground::physics::Init();

    gameThread = std::thread([config]() {
        playground::audio::Init();
#if ENABLE_INSPECTOR
        playground::ecs::Init(128, true);
#else
        playground::ecs::Init(128, false);
#endif
        config.startupCallback();

        static const char* CPU_FRAME = "CPU:Update";

        tracy::SetThreadName("Game Thread");

        auto now = std::chrono::high_resolution_clock::now();
        while (isRunning) {
            FrameMark;
            FrameMarkStart(CPU_FRAME);

            playground::inputmanager::Update();

            {
                ZoneScopedN("Scripts");
                ZoneColor(tracy::Color::LightSeaGreen);
            }
            {
                ZoneScopedN("ECS Tick");
                ZoneColor(tracy::Color::LightSalmon);
                playground::ecs::Update(deltaTime);
            }
            {
                ZoneScopedN("Batcher Submit");
                ZoneColor(tracy::Color::Salmon);
                playground::drawcallbatcher::Submit();
            }
            auto next = std::chrono::high_resolution_clock::now();
            const auto int_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(next - now);
            deltaTime = (double)int_ns.count() / 1000000000.0;
            timeSinceStart += deltaTime;
            now = next;
            FrameMarkEnd(CPU_FRAME);
        }

        playground::ecs::Shutdown();
    });


    while (isRunning) {
        ZoneScopedN("Input");
        ZoneColor(tracy::Color::AliceBlue);
        playground::input::Update();
    }

    playground::input::Shutdown();
    playground::rendering::Shutdown();
    renderThread.join();
    gameThread.join();

#if ENABLE_PROFILER
    tracy::ShutdownProfiler();
#endif

    return 0;
}
