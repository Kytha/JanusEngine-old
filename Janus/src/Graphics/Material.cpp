#include "Material.h"
#include "Light.h"

namespace Janus {
    Material::Material(Ref<Shader> shader) : m_Shader(shader)
    {
        metalness = 0.5f;
    }

    void Material::Set(const std::string &name, Ref<Texture> texture)
    {
        // Find where the texture is located and upload location and size info to shader
        m_Textures.push_back(texture);
        int32_t location = m_Shader->GetUniformLocation(name);
        glUniform1i(location, 0);
        m_Shader->Bind();
        m_Shader->UploadUniformInt(location, static_cast<uint32_t>(m_Textures.size() - 1));
    }

    void Material::Set(const std::string &name, float value) {
        m_Shader->Bind();
        m_Shader->UploadUniformFloat(name, value);
    }

    void Material::Set(const std::string &name, glm::vec3 values) {
        m_Shader->Bind();
        m_Shader->UploadUniformFloat3(name, values);
    }

    void Material::Set(Light &light)
    {
        // Uploads light data to shader
        m_Shader->Bind();
        m_Shader->UploadUniformFloat3("lights.Position", light.Position);
        m_Shader->UploadUniformFloat3("lights.Radiance", light.Radiance);
        m_Shader->UploadUniformFloat("lights.Irradiance", light.Irradiance);
    }

    void Material::SetViewProjection(glm::mat4 viewProjectionMatrix)
    {
        m_ViewProjectionMatrix = &viewProjectionMatrix;
    }

    void Material::SetView(const glm::vec3 &view)
    {
        m_View = view;
    }

    Ref<Shader> Material::GetShader() {
        return m_Shader;
    }

    void Material::Bind()
    {
        m_Shader->Bind();
        m_Shader->UploadUniformMat4("u_ViewProjection", *m_ViewProjectionMatrix);
        m_Shader->UploadUniformFloat3("u_View", m_View);
        m_Shader->UploadUniformFloat("u_Metalness", metalness);
        m_Shader->UploadUniformFloat("u_MetalnessTexToggle", 1.0);
        for (int i = 0; i < m_Textures.size(); i++)
        {
            m_Textures[i]->Bind(i);
        }
    }
}