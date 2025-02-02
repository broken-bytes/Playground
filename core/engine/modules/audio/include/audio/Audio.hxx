#pragma once

#include <memory>
#include <string>
#include <vector>

namespace playground::audio {
    struct AudioDevice;
	struct AudioClip;
    
    auto Init() -> void;
    auto Update() -> void;
	auto LoadClip(std::vector<float> buffer) -> std::shared_ptr<AudioClip>;
}
