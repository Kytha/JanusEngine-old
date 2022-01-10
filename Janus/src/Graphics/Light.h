#pragma once

#include <glm/glm.hpp>

namespace Janus {
struct Light
    {
        glm::vec3 Position;
        glm::vec3 Radiance;
        float Irradiance = 1.0f;
    };
}