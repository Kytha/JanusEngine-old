#include "jnpch.h"
#include "IndexBuffer.h"
#include <glad/glad.h>
#include "Renderer.h"

namespace Janus {
    IndexBuffer::IndexBuffer(void *indices, uint32_t size)
        : m_RendererID(0), m_Size(size)
    {
        m_LocalData = Buffer::Copy(indices, size);
        Ref<IndexBuffer> instance = this;
		    Renderer::Submit([instance]() mutable {
            glCreateBuffers(1, &instance->m_RendererID);
            glNamedBufferData(instance->m_RendererID, instance->m_Size, instance->m_LocalData.Data, GL_STATIC_DRAW);
        });
    }

    IndexBuffer::~IndexBuffer()
    {
		GLuint rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteBuffers(1, &rendererID);
		});
    }

    void IndexBuffer::Bind()
    {
		Ref<IndexBuffer> instance = this;
		Renderer::Submit([instance]() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->m_RendererID);
		});
    }

    void IndexBuffer::Unbind()
    {
        Ref<IndexBuffer> instance = this;
		Renderer::Submit([instance]() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		});
    }
}