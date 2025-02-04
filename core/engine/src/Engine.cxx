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

    double deltaTime = 0.0f;

	while (true) {
		auto now = std::chrono::high_resolution_clock::now();

		playground::input::Update();
		playground::audio::Update();
		playground::rendering::PreFrame();
		playground::rendering::Update(deltaTime);
		playground::rendering::PostFrame();

		deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - now).count();
	}
}
