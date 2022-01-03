#pragma once
#include "Core/Core.h"
#include "Texture.h"
#include "Shader.h"
#include "Light.h"

namespace Janus {
    class Material
    {
    public:
        Material(Ref<Shader> shader);
        void Set(const std::string &name, Ref<Texture> texture);
        void Set(const std::string &name, float value);
        void Set(const std::string &name, glm::vec3 values);
        void Set(Light &light);
        void Bind();
        void SetViewProjection(glm::mat4 viewProjectionMatrix);
        void SetView(const glm::vec3 &view);
        Ref<Shader> GetShader();
        float* GetMetalness() {return &metalness;}
        std::vector<Ref<Texture>> m_Textures;
        float metalness = 0.5;
    private:
        Ref<Shader> m_Shader;
        glm::mat4 *m_ViewProjectionMatrix;
        glm::vec3 m_View;
    };
}