#pragma once
#include "Core/Core.h"
#include "Core/Buffer.h"

namespace Janus {
    class IndexBuffer
    {
    public:
        IndexBuffer(void *indices, uint32_t size);
        ~IndexBuffer();
        void Bind();
        void Unbind();
        uint32_t getSize() const { return m_Size; };

    private:
        uint32_t m_RendererID;
        uint32_t m_Size;
        Buffer m_LocalData;
    };
}