#include "playground/Engine.hxx"
#include "playground/AssetManager.hxx"
#include "playground/ECS.hxx"
#include "playground/DrawCallbatcher.hxx"
#include "playground/InputManager.hxx"
#include "playground/PhysicsManager.hxx"
#include "playground/renderdoc_app.h"
#include <chrono>
#include <string>
#include <thread>
#include <shared/Hardware.hxx>
#include <shared/JobSystem.hxx>
#include <shared/Logger.hxx>
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
#include <shared/Logger.hxx>
#include <profiler/Profiler.hxx>
#include <math/Math.hxx>
#include <SDL3/SDL.h>
#include <tracy/Tracy.hpp>
#include <thread>
#include <future>

typedef void(*ScriptingEventCallback)(playground::events::Event* event);

bool isRunning = true;
auto now = std::chrono::high_resolution_clock::now();
std::thread renderThread;

double timeSinceStart = 0.0;
double deltaTime = 0.0;
double combinedDeltaTime = 0.0;
int deltaStep = 0;

uint8_t Startup(const PlaygroundConfig& config);
uint8_t SetupSubsystems(const PlaygroundConfig& config);
void SetupPointerLookupTable(const PlaygroundConfig& config);
#if EDITOR
void SetupEditorPointerLookupTable(const PlaygroundConfig& config);
#endif
void StartRenderThread(const PlaygroundConfig& config, void* window);
void LoadCoreAssets();
void SubscribeToEventsFromScripting(playground::events::EventType type, ScriptingEventCallback callback);
void Update();
double GetTimeSinceStart();
double GetDeltaTime();
void Shutdown();

/// 0 - OK
/// 1 - Error (Unknown error)
/// 2 - Error (No AVX or AVX2 support)
uint8_t PlaygroundCoreMain(const PlaygroundConfig& config) {
#if ENABLE_PROFILER
    tracy::StartupProfiler();
#endif

    playground::logging::logger::Init();
    playground::logging::logger::SetLogLevel(playground::logging::logger::LogLevel::Verbose);
    playground::logging::logger::SetupSubsystem("core");

    playground::logging::logger::Info("Starting Playground Core Engine...", "core");
    auto code = Startup(config);

    if (code == 2) {
        auto cpuName = playground::hardware::GetCPUBrandString();
        playground::logging::logger::Error(cpuName + " is not supported by this game", "core");
        std::string message =
            "Your system does not meet the minimum requirements to run this game.\nAVX instructions are required.\n\n"
            + cpuName;
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Unsupported CPU",
            message.c_str(),
            nullptr
        );
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    if (code != 0) {
        playground::logging::logger::Error("Failed to start Playground Core Engine", "core");
        return code;
    }

    playground::logging::logger::Info("Playground Core Engine started.", "core");
    playground::logging::logger::Info("Initialising Scripting Layer...", "core");

    // Register mandatory assets

    LoadCoreAssets();

    auto cores = playground::hardware::GetCoresByEfficiency(playground::hardware::CPUEfficiencyClass::Performance);

    playground::hardware::PinCurrentThreadToCore(cores[0].id);

    playground::logging::logger::SetupSubsystem("scripting");

    playground::logging::logger::Info("Calling scripting startup", "core");
    config.startupCallback();
    playground::logging::logger::Info("Scripting startup completed", "core");

    tracy::SetThreadName("Game Thread");

    playground::logging::logger::Info("Starting main loop", "core");
    while (isRunning) {
        Update();
    }

    Shutdown();

    return 0;
}

double GetTimeSinceStart() {
    return timeSinceStart;
}

double GetDeltaTime() {
    return deltaTime;
}

void Shutdown() {
    isRunning = false;
    playground::input::Shutdown();
    playground::rendering::Shutdown();
    playground::audio::Shutdown();
    playground::physicsmanager::Shutdown();
    playground::ecs::Shutdown();
    playground::jobsystem::Shutdown();
    renderThread.join();

#if ENABLE_PROFILER
    tracy::ShutdownProfiler();
#endif
}

uint8_t SetupSubsystems(const PlaygroundConfig& config) {
    SetupPointerLookupTable(config);
#if EDITOR
    SetupEditorPointerLookupTable(config);
#endif

    void* window = nullptr;
    if (config.WindowHandle == nullptr) {
        playground::logging::logger::Info("No window handle provided. Starting in standalone mode", "core");
        window = playground::system::Init(config.Width, config.Height, config.Fullscreen, config.Name);
    }
    else {
        playground::logging::logger::Info("Window handle provided. Starting in embedded mode", "core");
        window = config.WindowHandle;
    }

    playground::hardware::Init();
    if (playground::hardware::SupportsAVX2()) {
        playground::logging::logger::Info("CPU supports AVX2.", "core");
    }
    else if (playground::hardware::SupportsAVX()) {
        playground::logging::logger::Warn("CPU supports AVX.", "core");
    }
    else {
        playground::logging::logger::Error("CPU does not support AVX or AVX2. This application requires at least AVX support.", "core");

        return 2;
    }

    playground::jobsystem::Init();
    playground::events::Init();

    playground::assetloader::Init(config.Path);
    playground::audio::Init(
        playground::io::OpenFileFromArchive,
        playground::io::ReadFileFromArchive,
        playground::io::SeekFileInArchive,
        playground::io::CloseFile
    );
    playground::inputmanager::Init(window);
    playground::physicsmanager::Init();
    StartRenderThread(config, window);

#ifdef ENABLE_INSPECTOR
    playground::ecs::Init(true);
#else
    playground::ecs::Init(false);
#endif

    return 0;
}

void SetupPointerLookupTable(const PlaygroundConfig& config) {
    playground::logging::logger::Info("Setting up pointer lookup table", "core");
    config.Delegate("Logger_Info", reinterpret_cast<void*>(playground::logging::logger::Info_C));
    config.Delegate("Logger_Warn", reinterpret_cast<void*>(playground::logging::logger::Info_C));
    config.Delegate("Logger_Error", reinterpret_cast<void*>(playground::logging::logger::Info_C));

    config.Delegate("AssetManager_LoadModel\0", reinterpret_cast<void*>(playground::assetmanager::LoadModel));
    config.Delegate("AssetManager_LoadMaterial\0", reinterpret_cast<void*>(playground::assetmanager::LoadMaterial));
    config.Delegate("AssetManager_LoadPhysicsMaterial\0", reinterpret_cast<void*>(playground::assetmanager::LoadPhysicsMaterial));

    config.Delegate("Batcher_Batch\0", reinterpret_cast<void*>(playground::drawcallbatcher::Batch));
    config.Delegate("Batcher_SetSun\0", reinterpret_cast<void*>(playground::drawcallbatcher::SetSun));
    config.Delegate("Batcher_AddCamera\0", reinterpret_cast<void*>(playground::drawcallbatcher::AddCamera));

    config.Delegate("ECS_CreateEntity\0", reinterpret_cast<void*>(playground::ecs::CreateEntity));
    config.Delegate("ECS_DestroyEntity\0", reinterpret_cast<void*>(playground::ecs::DestroyEntity));
    config.Delegate("ECS_SetParent\0", reinterpret_cast<void*>(playground::ecs::SetParent));
    config.Delegate("ECS_GetParent\0", reinterpret_cast<void*>(playground::ecs::GetParent));
    config.Delegate("ECS_GetEntityByName\0", reinterpret_cast<void*>(playground::ecs::GetEntityByName));
    config.Delegate("ECS_RegisterComponent\0", reinterpret_cast<void*>(playground::ecs::RegisterComponent));
    config.Delegate("ECS_AddComponent\0", reinterpret_cast<void*>(playground::ecs::AddComponent));
    config.Delegate("ECS_SetComponent\0", reinterpret_cast<void*>(playground::ecs::SetComponent));
    config.Delegate("ECS_GetComponent\0", reinterpret_cast<void*>(playground::ecs::GetComponent));
    config.Delegate("ECS_HasComponent\0", reinterpret_cast<void*>(playground::ecs::HasComponent));
    config.Delegate("ECS_DestroyComponent\0", reinterpret_cast<void*>(playground::ecs::DestroyComponent));
    config.Delegate("ECS_CreateSystem\0", reinterpret_cast<void*>(playground::ecs::CreateUpdateSystem));
    config.Delegate("ECS_GetComponentBuffer\0", reinterpret_cast<void*>(playground::ecs::GetComponentBuffer));
    config.Delegate("ECS_GetIteratorSystem\0", reinterpret_cast<void*>(playground::ecs::GetIteratorSystem));
    config.Delegate("ECS_GetIteratorSize\0", reinterpret_cast<void*>(playground::ecs::GetIteratorSize));
    config.Delegate("ECS_GetIteratorOffset\0", reinterpret_cast<void*>(playground::ecs::GetIteratorOffset));
    config.Delegate("ECS_GetEntitiesFromIterator\0", reinterpret_cast<void*>(playground::ecs::GetEntitiesFromIterator));
    config.Delegate("ECS_CreateHook\0", reinterpret_cast<void*>(playground::ecs::CreateHook));
    config.Delegate("ECS_DeleteAllEntitiesByTag\0", reinterpret_cast<void*>(playground::ecs::DeleteAllEntitiesByTag));
    config.Delegate("ECS_CreateTag\0", reinterpret_cast<void*>(playground::ecs::CreateTag));
    config.Delegate("ECS_AddTag\0", reinterpret_cast<void*>(playground::ecs::AddTag));

    config.Delegate("Input_GetAxis\0", reinterpret_cast<void*>(playground::inputmanager::GetAxis));
    config.Delegate("Input_IsButtonPressed\0", reinterpret_cast<void*>(playground::inputmanager::IsButtonPressed));
    config.Delegate("Input_IsButtonDown\0", reinterpret_cast<void*>(playground::inputmanager::IsButtonDown));
    config.Delegate("Input_IsButtonUp\0", reinterpret_cast<void*>(playground::inputmanager::IsButtonUp));

    config.Delegate("Physics_CreateRigidBody\0", reinterpret_cast<void*>(playground::physicsmanager::CreateRigidBody));
    config.Delegate("Physics_CreateStaticBody\0", reinterpret_cast<void*>(playground::physicsmanager::CreateStaticBody));
    config.Delegate("Physics_CreateBoxCollider\0", reinterpret_cast<void*>(playground::physicsmanager::CreateBoxCollider));
    config.Delegate("Physics_AttachCollider\0", reinterpret_cast<void*>(playground::physicsmanager::AttachCollider));
    config.Delegate("Physics_DestroyBody\0", reinterpret_cast<void*>(playground::physicsmanager::RemoveBody));
    config.Delegate("Physics_DestroyCollider\0", reinterpret_cast<void*>(playground::physicsmanager::RemoveCollider));
    config.Delegate("Physics_GetBodyPosition\0", reinterpret_cast<void*>(playground::physicsmanager::GetBodyPosition));
    config.Delegate("Physics_GetBodyRotation\0", reinterpret_cast<void*>(playground::physicsmanager::GetBodyRotation));

    config.Delegate("Time_GetTimeSinceStart\0", reinterpret_cast<void*>(GetTimeSinceStart));
    config.Delegate("Time_GetDeltaTime\0", reinterpret_cast<void*>(GetDeltaTime));

    config.Delegate("Events_Subscribe", reinterpret_cast<void*>(SubscribeToEventsFromScripting));
}

#if EDITOR
void SetupEditorPointerLookupTable(const PlaygroundConfig& config) {
    playground::logging::logger::Info("Setting up editor pointer lookup table", "core");

    config.EditorDelegate("Input_SetCapturesInput\0", playground::inputmanager::SetCapturesInput);

    config.EditorDelegate("Events_Subscribe", SubscribeToEventsFromScripting);

    config.EditorDelegate("ECS_CreateEntityHook\0", playground::ecs::SetEntityCreateHook);
    config.EditorDelegate("ECS_DestroyEntityHook\0", playground::ecs::SetEntityDestroyHook);
    config.EditorDelegate("ECS_SetEntityParentHook\0", playground::ecs::SetEntitySetParentHook);
}
#endif

void StartRenderThread(const PlaygroundConfig& config, void* window) {
    std::promise<void> rendererReadyPromise;
    std::future<void> rendererReadyFuture = rendererReadyPromise.get_future();

    renderThread = std::thread([window, config, &rendererReadyPromise] {
        auto cores = playground::hardware::GetCoresByEfficiency(playground::hardware::CPUEfficiencyClass::Performance);

        playground::hardware::PinCurrentThreadToCore(cores[1].id);
        playground::rendering::Init(window, config.Width, config.Height, false, rendererReadyPromise);
        });

    Subscribe(playground::events::EventType::System, [](playground::events::Event* event) {
        if (reinterpret_cast<playground::events::SystemEvent*>(event)->SystemType == playground::events::SystemEventType::Quit) {
            isRunning = false;
        }
        });

    rendererReadyFuture.wait();
}

void LoadCoreAssets() {
    // Shadow shader
    auto shadowShader = playground::assetmanager::LoadShader("shadows.shader");
    playground::rendering::RegisterShadowShader(shadowShader->vertexShader);

    playground::assetmanager::LoadAudio("Master.audio");
    playground::assetmanager::LoadAudio("Master.strings.audio");
    playground::assetmanager::LoadAudio("Ambient.audio");
    playground::assetmanager::LoadAudio("Dialogue.audio");
    playground::assetmanager::LoadAudio("SFX.audio");
    playground::assetmanager::LoadAudio("Music.audio");
    playground::audio::SetVolume(1);
}

uint8_t Startup(const PlaygroundConfig& config) {
    RENDERDOC_API_1_1_2* rdoc_api = nullptr;

    // At init, on windows
    if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
    {
        auto RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void**>(&rdoc_api));
        assert(ret == 1);
    }

    auto code = SetupSubsystems(config);

    return code;
}

void Update() {
    static const char* CPU_FRAME = "CPU:Update";

    FrameMark;
    FrameMarkStart(CPU_FRAME);

    {
        ZoneScopedNC("Engine: Input Tick", tracy::Color::AliceBlue);
        playground::inputmanager::Update();
    }
    {
        ZoneScopedNC("Engine: ECS Tick", tracy::Color::VioletRed1);
        playground::ecs::Update(deltaTime);
    }
    {
        ZoneScopedNC("Engine: Physics Tick", tracy::Color::Salmon);
        playground::physicsmanager::Update(deltaTime);
    }
    {
        ZoneScopedNC("Engine: Audio Tick", tracy::Color::DarkSeaGreen1);
        playground::audio::Update();
    }
    {
        ZoneScopedNC("Engine: Batcher Tick", tracy::Color::DarkSalmon);
        playground::drawcallbatcher::Submit();
    }
    auto next = std::chrono::high_resolution_clock::now();
    const auto int_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(next - now);
    deltaTime = (double)int_ns.count() / 1000000000.0;
    combinedDeltaTime += deltaTime;
    deltaStep++;

    if (deltaStep >= 60) {
        combinedDeltaTime /= 60;
        deltaStep = 0;
        SetWindowTextA(
            GetActiveWindow(),
            ("Playground Core Engine - FPS: " + std::to_string(1 / combinedDeltaTime) + " - CPU Time: " + std::to_string(combinedDeltaTime) + "s - GPU Time: " + std::to_string(playground::rendering::GetGPUFrameTime()) + "s").c_str());
    }

    timeSinceStart += deltaTime;
    now = next;
    FrameMarkEnd(CPU_FRAME);
}

void SubscribeToEventsFromScripting(playground::events::EventType type, ScriptingEventCallback callback) {
    Subscribe(type, [callback](playground::events::Event* event) {
        callback(event);
    });
}

