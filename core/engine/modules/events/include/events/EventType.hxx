#pragma once

#include <cstdint>

namespace playground::events {
	enum class EventType: int32_t {
		Health,
		System,
		Input,
		Network,
	};
}
