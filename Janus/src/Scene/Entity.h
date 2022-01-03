#pragma once

#include <glm/glm.hpp>
#include "Core/Core.h"
#include "Graphics/Mesh.h"

namespace Janus {
class Entity
{
    public:
        Entity();
        ~Entity();

        void SetMesh(const Ref<Mesh>& mesh) { m_Mesh = mesh; }
        Ref<Mesh> GetMesh() { return m_Mesh; }

        void SetMaterial(const Ref<Material>& material) { m_Material = material; }
        Ref<Material> GetMaterial() { return m_Material; }

        const glm::mat4& GetTransform() const { return m_Transform; }
        glm::mat4& Transform() { return m_Transform; }
    private:
        glm::mat4 m_Transform;
        Ref<Mesh> m_Mesh;
        Ref<Material> m_Material;
    };
}

