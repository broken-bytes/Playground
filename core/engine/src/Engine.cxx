#include "playground/Engine.hxx"
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

[[noreturn]] void PlaygroundMain(const PlaygroundConfig& config) {
    playground::audio::Init();
    playground::input::Init();
    auto window = playground::system::Init(config.Window);
    playground::rendering::Init(window, config.Width, config.Height);

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


    config.Delegate("Rendering_PreFrame\0", playground::rendering::PreFrame);
    config.Delegate("Rendering_Update\0", playground::rendering::Update);
    config.Delegate("Rendering_PostFrame\0", playground::rendering::PostFrame);
}
