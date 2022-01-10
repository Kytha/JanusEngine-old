#pragma once

#include <string>

#include "Core/Buffer.h"
#include "Core/stb_image/stb_image.h"

namespace Janus
{
    class Texture : public RefCounted
    {
    public:
        Texture(const std::string &filePath);
        void Bind(int slot);
        bool Loaded() const;
        uint32_t m_Width, m_Height;
        Buffer m_ImageData;
        std::string m_FilePath;
        uint32_t m_RendererID;
        bool m_Loaded = false;
    };
}