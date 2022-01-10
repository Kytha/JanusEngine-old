#pragma once

#include "Core/Core.h"

#include "Graphics/Shader.h"

namespace Janus
{
    class ShaderLibrary : public RefCounted
    {
    public:
        ShaderLibrary();
        ~ShaderLibrary();

        void Load(const std::string &path, const std::string &name);
        Ref<Shader> Get(const std::string &name);

    private:
        std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    };
}