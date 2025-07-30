#include "events/Events.hxx"
#include "events/Event.hxx"
#include "events/EventType.hxx"
#include <shared/Logger.hxx>
#include <algorithm>
#include <map>
#include <vector>

namespace playground::events {
	std::map<EventType, std::vector<std::function<void(Event*)>>> subscriptions = {};

    auto Init() -> void {
        logging::logger::SetupSubsystem("events");
        subscriptions.insert({ EventType::System, {} });
        subscriptions.insert({ EventType::Input, {} });
        subscriptions.insert({ EventType::Health, {} });
        subscriptions.insert({ EventType::Network, {} });
    }

    auto Subscribe(EventType type, std::function<void(Event*)> subscription) -> void {
        logging::logger::Info("Subscribing to event type: " + std::to_string(static_cast<uint32_t>(type)), "events");
		subscriptions[type].push_back(subscription);
    }

    auto Emit(Event* event) -> void {
        for (auto subscription : subscriptions[event->Type]) {
			subscription(event);
        }
    }
}
