
#include "VertexBuffer.h"
#include <glad/glad.h>

namespace Janus {
    GLenum VertexBuffer::Usage(VertexBufferUsage usage)
    {
        switch (usage)
        {
            case VertexBufferUsage::Static:    return GL_STATIC_DRAW;
            case VertexBufferUsage::Dynamic:   return GL_DYNAMIC_DRAW;
        }
        return 0;
    }

    VertexBuffer::VertexBuffer(uint32_t size, VertexBufferUsage usage) : m_Size(size), m_Usage(usage)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, m_Size, nullptr, Usage(m_Usage));
    }

    VertexBuffer::VertexBuffer(void *vertices, uint32_t size)
    {
        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    VertexBuffer::~VertexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void VertexBuffer::Bind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void VertexBuffer::Unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    const BufferLayout &VertexBuffer::GetLayout()
    {
        return m_Layout;
    }

    void VertexBuffer::SetLayout(const BufferLayout &layout)
    {
        m_Layout = layout;
    }


    void VertexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
    {
        m_LocalData = Buffer::Copy(data, size);
        m_Size = size;
        glNamedBufferSubData(m_RendererID, offset, m_Size, m_LocalData.Data);
    }
}