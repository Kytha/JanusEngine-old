#pragma once

#include "Core/Core.h"
#include "FrameBuffer.h"

namespace Janus {

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
	};

	class RenderPass : public RefCounted
	{
	public:
		RenderPass(const RenderPassSpecification& spec);

		RenderPassSpecification& GetSpecification() { return m_Specification; }
		const RenderPassSpecification& GetSpecification() const { return m_Specification; }
    private:
		RenderPassSpecification m_Specification;
	};

}