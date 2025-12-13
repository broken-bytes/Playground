#pragma once

namespace playground::rendering {
    enum class RenderPass {
        Preparation,
        Skybox,
        Shadow,
        PostShadow,
        Opaque,
        PostOpaque,
        PostProcessing,
        UI,
        Completion
    };
}
