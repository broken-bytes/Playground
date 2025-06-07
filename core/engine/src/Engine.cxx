#include "playground/Engine.hxx"
#include "playground/SceneManager.hxx"
#include <chrono>
#include <thread>
#include <audio/Audio.hxx>
#include <input/Input.hxx>
#include <rendering/Rendering.hxx>
#include <system/System.hxx>
#include <events/Events.hxx>
#include <events/Event.hxx>
#include <events/SystemEvent.hxx>
#include <assetloader/AssetLoader.hxx>
#include <io/IO.hxx>

void Shutdown() {
    playground::rendering::Shutdown();
}

[[noreturn]] void PlaygroundCoreMain(const PlaygroundConfig& config) {
    playground::audio::Init();
    playground::input::Init();
    playground::scenemanager::Init();
    auto window = playground::system::Init(config.Window);

    Subscribe(playground::events::EventType::System, [](playground::events::Event* event) {
        if (reinterpret_cast<playground::events::SystemEvent*>(event)->SystemType == playground::events::SystemEventType::Quit) {
            playground::rendering::Shutdown();
            exit(0);
        }
        });

    
    auto buffer = playground::io::LoadFileFromArchive(
        "C:\\Users\\marce\\Desktop\\TestArchive.pak",
        "Checker"
    );
    auto texture = playground::assetloader::LoadTexture(buffer);
    playground::rendering::UploadTexture(texture);

    auto meshBuffer = playground::io::LoadFileFromArchive(
        "C:\\Users\\marce\\Desktop\\TestArchive.pak",
        "Cube"
    );
    auto mesh = playground::assetloader::LoadMeshes(meshBuffer);
    playground::rendering::UploadMesh(mesh.front());

    config.Delegate("Playground_CreateGameObject\0", playground::scenemanager::CreateGameObject);
    config.Delegate("Playground_GetGameObjectTransform\0", playground::gameobjects::GetGameObjectTransform);
    config.Delegate("Playground_DestroyGameObject\0", playground::scenemanager::DestroyGameObject);
    config.Delegate("Playground_Shutdown\0", Shutdown);
    config.Delegate("Rendering_Init\0", playground::rendering::Init);
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
}
