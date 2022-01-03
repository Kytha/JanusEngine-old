#include "jnpch.h"
#include "Framebuffer.h"
#include "Renderer.h"
#include "OpenGLFramebuffer.h"

namespace Janus {
	
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
        return CreateRef<OpenGLFramebuffer>(spec);
	}

}

