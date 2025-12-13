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
        // Print the address of the subscription function
        logging::logger::Info("Subscription address: " + std::to_string(reinterpret_cast<uintptr_t>(subscription.target<void(Event*)>())), "events");
		subscriptions[type].push_back(subscription);
    }

    auto Emit(Event* event) -> void {
        for (auto subscription : subscriptions[event->Type]) {
            logging::logger::Debug("Emitting subscriber: " + std::to_string(*reinterpret_cast<uintptr_t*>(&subscription)), "events");
            logging::logger::Info("Emitting event of type: " + std::to_string(static_cast<uint32_t>(event->Type)), "events");
			subscription(event);
        }
    }
}
