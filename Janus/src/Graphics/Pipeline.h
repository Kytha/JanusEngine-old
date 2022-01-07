#pragma once

#include "Graphics/VertexBuffer.h"
#include "Graphics/Shader.h"

namespace Janus {

    struct PipelineSpecification
	{
		Ref<Shader> Shader;
		BufferLayout Layout;
	};

	class Pipeline : public RefCounted
	{
	public:
		Pipeline(const PipelineSpecification& spec);
		virtual ~Pipeline();

		PipelineSpecification& GetSpecification() { return m_Specification; }
		const PipelineSpecification& GetSpecification() const { return m_Specification; }

		void Invalidate();

		void Bind();
	private:
		PipelineSpecification m_Specification;
		uint32_t m_VertexArrayRendererID = 0;
	};

}