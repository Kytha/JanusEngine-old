#include "Texture.h"
#include "Core/stb_image/stb_image.h"
#include <glad/glad.h>
#include "Renderer.h"
namespace Janus {
    Texture::Texture(const std::string &filePath)
    {
        int width, height, nrChannels;
        m_ImageData.Data = (byte*)stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
        GLenum format = GL_RGB;

        if(!m_ImageData.Data)
            return;
        
        m_Loaded = true;
        // Determine type of image formatting
        switch (nrChannels)
        {
        case STBI_grey:
            format = GL_RED;
            break;
        case STBI_grey_alpha:
            format = GL_RG;
            break;
        case STBI_rgb:
            format = GL_RGB;
            break;
        case STBI_rgb_alpha:
            format = GL_RGBA;
            break;
        }

        m_Height = height;
        m_Width = width;
        m_FilePath = filePath;
        Ref<Texture> instance = this;
		Renderer::Submit([instance, format]() mutable
		{
            glGenTextures(1, &instance->m_RendererID);
            glBindTexture(GL_TEXTURE_2D, instance->m_RendererID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, format, instance->m_Width, instance->m_Height, 0, format, GL_UNSIGNED_BYTE, instance->m_ImageData.Data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);

            stbi_image_free(instance->m_ImageData.Data);
        });
    }

    bool Texture::Loaded() const{
        return m_Loaded;
    }

    void Texture::Bind(int slot)
    {
        Ref<Texture> instance = this;
        Renderer::Submit([instance, slot]() mutable
        {
            glBindTextureUnit(slot, instance->m_RendererID);
        }); 
       
    }
}