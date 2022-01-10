#include "jnpch.h"

#include "Graphics/ShaderLibrary.h"

namespace Janus
{
    ShaderLibrary::ShaderLibrary()
    {
    }

    ShaderLibrary::~ShaderLibrary()
    {
    }

    Ref<Shader> ShaderLibrary::Get(const std::string &name)
    {
        if (!(m_Shaders.find(name) == m_Shaders.end()))
            return m_Shaders[name];
        else
        {
            // Shader does not exist. Halt program
            JN_ASSERT(false, "SHADER_LIBRARY_ERROR: Shader " + name + " could not be found");
            return nullptr;
        }
    }

    void ShaderLibrary::Load(const std::string &path, const std::string &name)
    {
        auto shader = Ref<Shader>(new Shader(path));
        // Check shader doesn't already exist
        if ((m_Shaders.find(name) == m_Shaders.end()))
            m_Shaders[name] = shader;
    }
}