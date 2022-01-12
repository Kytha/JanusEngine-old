
#include "jnpch.h"

#include <glad/glad.h>

#include "Graphics/VertexBuffer.h"
#include "Graphics/Renderer.h"
namespace Janus
{
    GLenum VertexBuffer::Usage(VertexBufferUsage usage)
    {
        switch (usage)
        {
        case VertexBufferUsage::Static:
            return GL_STATIC_DRAW;
        case VertexBufferUsage::Dynamic:
            return GL_DYNAMIC_DRAW;
        }
        return 0;
    }

    VertexBuffer::VertexBuffer(uint32_t size, VertexBufferUsage usage) : m_Size(size), m_Usage(usage)
    {
        Ref<VertexBuffer> instance = this;
        Renderer::Submit([instance]() mutable
                         {
                             glCreateBuffers(1, &instance->m_RendererID);
                             glNamedBufferData(instance->m_RendererID, instance->m_Size, nullptr, Usage(instance->m_Usage));
                         });
    }

    VertexBuffer::VertexBuffer(void *vertices, uint32_t size) : m_Size(size), m_Usage(VertexBufferUsage::Static)
    {
        m_LocalData = Buffer::Copy(vertices, size);
        Ref<VertexBuffer> instance = this;
        Renderer::Submit([instance]() mutable
                         {
                             glCreateBuffers(1, &instance->m_RendererID);
                             glNamedBufferData(instance->m_RendererID, instance->m_Size, instance->m_LocalData.Data, Usage(instance->m_Usage));
                         });
    }

    VertexBuffer::~VertexBuffer()
    {
        GLuint rendererID = m_RendererID;
        Renderer::Submit([rendererID]()
                         { glDeleteBuffers(1, &rendererID); });
    }

    void VertexBuffer::Bind()
    {
        Ref<const VertexBuffer> instance = this;
        Renderer::Submit([instance]()
                         { JN_PROFILE_FUNCTION(); glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID); });
    }

    void VertexBuffer::Unbind()
    {
        Ref<const VertexBuffer> instance = this;
        Renderer::Submit([instance]()
                         { glBindBuffer(GL_ARRAY_BUFFER, 0); });
    }

    const BufferLayout &VertexBuffer::GetLayout()
    {
        return m_Layout;
    }

    void VertexBuffer::SetLayout(const BufferLayout &layout)
    {
        m_Layout = layout;
    }

    void VertexBuffer::SetData(void *data, uint32_t size, uint32_t offset)
    {
        m_LocalData = Buffer::Copy(data, size);
        m_Size = size;
        Ref<VertexBuffer> instance = this;
        Renderer::Submit([instance, offset]()
                         { glNamedBufferSubData(instance->m_RendererID, offset, instance->m_LocalData.Size, instance->m_LocalData.Data); });
    }
}