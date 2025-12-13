#pragma once

#include "CommandList.hxx"
#include "CommandListType.hxx"
#include "Fence.hxx"

#include <vector>

namespace playground::rendering {
	class CommandQueue {
	public:
		CommandQueue(CommandListType type) : _type(type) {}
		virtual ~CommandQueue() = default;
		virtual void Execute(const std::vector<std::shared_ptr<CommandList>>&) = 0;

		auto Type() const { return _type; }

	private:
		CommandListType _type;
	};
}
