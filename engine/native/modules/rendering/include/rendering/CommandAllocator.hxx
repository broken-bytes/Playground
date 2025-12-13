#pragma once

#include <memory>
#include "rendering/CommandList.hxx"

namespace playground::rendering {
	class CommandAllocator {
	public:
		virtual ~CommandAllocator() = default;
		virtual auto Reset() -> void = 0;
		virtual auto ResetCommandList(std::shared_ptr<rendering::CommandList> list) -> std::shared_ptr<CommandList> = 0;
	};
}
