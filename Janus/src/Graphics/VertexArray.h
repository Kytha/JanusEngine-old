#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Janus {
    class VertexArray : public RefCounted
    {
    public:
        VertexArray();
        ~VertexArray(){};

        void Bind();
        void Unbind();

        void AddVertexBuffer(Ref<VertexBuffer> vertexBuffer);
        void SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer);

        const std::vector<Ref<VertexBuffer>> &GetVertexBuffers();
        const Ref<IndexBuffer> &GetIndexBuffer();

    private:
        uint32_t m_RendererID;
        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer> m_IndexBuffer;
    };
}