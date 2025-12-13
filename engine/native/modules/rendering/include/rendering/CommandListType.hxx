#pragma once

namespace playground::rendering {
	enum class CommandListType {
		Graphics,
		Compute,
		Copy,
        Transfer
	};
}