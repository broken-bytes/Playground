#pragma once

#include <math/Vector3.hxx>

struct TranslationComponent {
    bool isDirty = true;
    playground::math::Vector3 position{ 0.0f, 0.0f, 0.0f };
};

