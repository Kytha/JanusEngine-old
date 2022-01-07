#include "jnpch.h"
#include "OpenGLFramebuffer.h"
#include "Graphics/Renderer.h"
#include <glad/glad.h>

namespace Janus {

	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), 1, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static GLenum DataType(GLenum format)
		{
			switch (format)
			{
				case GL_RGBA8: return GL_UNSIGNED_BYTE;
				case GL_RG16F:
				case GL_RG32F:
				case GL_RGBA16F:
				case GL_RGBA32F: return GL_FLOAT;
				case GL_DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
			}

			JN_ASSERT(false, "Unknown format!");
			return 0;
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum format, uint32_t width, uint32_t height, int index)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				// Only RGBA access for now
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, DataType(format), nullptr);

				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case FramebufferTextureFormat::DEPTH24STENCIL8:
				case FramebufferTextureFormat::DEPTH32F:
					return true;
			}
			return false;
		}

	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec), m_Width(spec.Width), m_Height(spec.Height)
	{
		for (auto format : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.TextureFormat))
				m_ColorAttachmentFormats.emplace_back(format.TextureFormat);
			else
				m_DepthAttachmentFormat = format.TextureFormat;
		}

		Resize(spec.Width, spec.Height, true);
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		Ref<OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance]() {
			glDeleteFramebuffers(1, &instance->m_RendererID);
		});
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
	{
		if (!forceRecreate && (m_Width == width && m_Height == height))
			return;

		m_Width = width;
		m_Height = height;

		Ref<OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance]() mutable
		{
			if (instance->m_RendererID)
			{
				glDeleteFramebuffers(1, &instance->m_RendererID);
				glDeleteTextures(instance->m_ColorAttachments.size(), instance->m_ColorAttachments.data());
				glDeleteTextures(1, &instance->m_DepthAttachment);

				instance->m_ColorAttachments.clear();
				instance->m_DepthAttachment = 0;
			}

			glGenFramebuffers(1, &instance->m_RendererID);
			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);

			bool multisample = instance->m_Specification.Samples > 1;

			if (instance->m_ColorAttachmentFormats.size())
			{
				instance->m_ColorAttachments.resize(instance->m_ColorAttachmentFormats.size());
				Utils::CreateTextures(multisample, instance->m_ColorAttachments.data(), instance->m_ColorAttachments.size());

				// Create color attachments
				for (int i = 0; i < instance->m_ColorAttachments.size(); i++)
				{
					Utils::BindTexture(multisample, instance->m_ColorAttachments[i]);
					switch (instance->m_ColorAttachmentFormats[i])
					{
					case FramebufferTextureFormat::RGBA8:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_RGBA8, instance->m_Width, instance->m_Height, i);
						break;
					case FramebufferTextureFormat::RGBA16F:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_RGBA16F, instance->m_Width, instance->m_Height, i);
						break;
					case FramebufferTextureFormat::RGBA32F:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_RGBA32F, instance->m_Width, instance->m_Height, i);
						break;
					case FramebufferTextureFormat::RG32F:
						Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, GL_RG32F, instance->m_Width, instance->m_Height, i);
						break;
					}

				}
			}

			if (instance->m_DepthAttachmentFormat != FramebufferTextureFormat::None)
			{
				Utils::CreateTextures(multisample, &instance->m_DepthAttachment, 1);
				Utils::BindTexture(multisample, instance->m_DepthAttachment);
				switch (instance->m_DepthAttachmentFormat)
				{
				case FramebufferTextureFormat::DEPTH24STENCIL8:
					Utils::AttachDepthTexture(instance->m_DepthAttachment, instance->m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, instance->m_Width, instance->m_Height);
					break;
				case FramebufferTextureFormat::DEPTH32F:
					Utils::AttachDepthTexture(instance->m_DepthAttachment, instance->m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, instance->m_Width, instance->m_Height);
					break;
				}

			}

			if (instance->m_ColorAttachments.size() > 1)
			{
				JN_ASSERT(instance->m_ColorAttachments.size() <= 4, "");
				GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
				glDrawBuffers(instance->m_ColorAttachments.size(), buffers);
			}
			else if (instance->m_ColorAttachments.size() == 0)
			{
				// Only depth-pass
				glDrawBuffer(GL_NONE);
			}

			JN_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});
	}

	void OpenGLFramebuffer::Bind() const
	{
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance]() {
			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);
			glViewport(0, 0, instance->m_Width, instance->m_Height);
		});
	}

	void OpenGLFramebuffer::Unbind() const
	{
		Renderer::Submit([]() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});
	}

	void OpenGLFramebuffer::BindTexture(uint32_t attachmentIndex, uint32_t slot) const
	{
		Ref<const OpenGLFramebuffer> instance = this;
		Renderer::Submit([instance, attachmentIndex, slot]() {
			glBindTextureUnit(slot, instance->m_ColorAttachments[attachmentIndex]);
		});
	}
}