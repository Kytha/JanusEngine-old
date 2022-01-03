#pragma once
#include <string>

namespace Janus {
    class Texture
    {
    public:
        Texture(const std::string &filePath);
        void Bind(int slot);
        bool Loaded() const;
        uint32_t m_Width, m_Height;
        std::string m_FilePath;
        uint32_t m_RendererID;
        bool m_Loaded = false;
    };
}