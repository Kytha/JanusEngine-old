#include "jnpch.h"
#include "Renderer.h"
#include "Graphics/Camera.h"
#include "Core/Application.h"
#include "Mesh.h"
#include "SceneRenderer.h"
#include "RenderPass.h"

namespace Janus {

	struct RendererData
	{
		Ref<RenderPass> m_ActiveRenderPass;
		RenderCommandQueue m_CommandQueue;
		Ref<ShaderLibrary> m_ShaderLibrary;

		Ref<VertexBuffer> m_FullscreenQuadVertexBuffer;
		Ref<IndexBuffer> m_FullscreenQuadIndexBuffer;
		Ref<Pipeline> m_FullscreenQuadPipeline;
	};

    static RendererData s_Data;

    void Renderer::Init()
    {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		unsigned int vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glFrontFace(GL_CCW);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_STENCIL_TEST);




        s_Data.m_ShaderLibrary = Ref<ShaderLibrary>::Create();
        Renderer::GetShaderLibrary()->Load("./assets/shaders/janus_pbr.glsl", "janus_pbr");
		Renderer::GetShaderLibrary()->Load("./assets/shaders/janus_grid.glsl", "janus_grid");
		//Renderer::GetShaderLibrary()->Load("./assets/shaders/janus_quad.glsl", "janus_quad");
        SceneRenderer::Init();
		float x = -1;
		float y = -1;
		float width = 2, height = 2;
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];

		data[0].Position = glm::vec3(x, y, 0.1f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.1f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.1f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.1f);
		data[3].TexCoord = glm::vec2(0, 1);

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		s_Data.m_FullscreenQuadPipeline = Ref<Pipeline>::Create(pipelineSpecification);

		s_Data.m_FullscreenQuadVertexBuffer = Ref<VertexBuffer>::Create(data, 4 * sizeof(QuadVertex));
		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		s_Data.m_FullscreenQuadIndexBuffer = Ref<IndexBuffer>::Create(indices, 6 * sizeof(uint32_t));

    }

	Ref<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_Data.m_ShaderLibrary;
	}

    void Renderer::Clear(float r, float g, float b, float a)
	{
		Renderer::Submit([=]() {
			glClearColor(r, g, b, a);
		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		});
	}

    void Renderer::SetClearColor(float r, float g, float b, float a)
	{
	}

    void Renderer::DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest, bool faceCulling)
	{
		Renderer::Submit([=]() {
			if (!depthTest)
			glDisable(GL_DEPTH_TEST);

		    GLenum glPrimitiveType = 0;
		    switch (type)
		    {
			    case PrimitiveType::Triangles:
				    glPrimitiveType = GL_TRIANGLES;
				    break;
			    case PrimitiveType::Lines:
				    glPrimitiveType = GL_LINES;
				    break;
		    }

		    if (faceCulling)
			    glEnable(GL_CULL_FACE);
		    else
			    glDisable(GL_CULL_FACE);

		    glDrawElements(glPrimitiveType, count, GL_UNSIGNED_INT, nullptr);

		    if (!depthTest)
			    glEnable(GL_DEPTH_TEST);
		});
	}

    void Renderer::WaitAndRender()
	{
		s_Data.m_CommandQueue.Execute();
	}

    void Renderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
	{
		JN_ASSERT(renderPass, "Render pass cannot be null!");
		s_Data.m_ActiveRenderPass = renderPass;
		
		renderPass->GetSpecification().TargetFramebuffer->Bind();
		if (clear)
		{
			const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
			Renderer::Submit([=]() {
                glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			});
		}
	}

    void Renderer::EndRenderPass()
	{
		JN_ASSERT(s_Data.m_ActiveRenderPass, "No active render pass! Have you called Renderer::EndRenderPass twice?");
		s_Data.m_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
		s_Data.m_ActiveRenderPass = nullptr;
	}

	void Renderer::SubmitQuad(Ref<MaterialInstance> material, const glm::mat4& transform)
	{
		bool depthTest = true;
		bool cullFace = true;
		if (material)
		{
			material->Bind();
			depthTest = material->GetFlag(MaterialFlag::DepthTest);
			cullFace = !material->GetFlag(MaterialFlag::TwoSided);

			auto shader = material->GetShader();
			shader->SetMat4("u_Transform", transform);
		}

		s_Data.m_FullscreenQuadVertexBuffer->Bind();
		s_Data.m_FullscreenQuadPipeline->Bind();
		s_Data.m_FullscreenQuadIndexBuffer->Bind();
		Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest, cullFace);
	}


    void Renderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
    {
        mesh->m_VertexBuffer->Bind();
        mesh->m_Pipeline->Bind();
		mesh->m_IndexBuffer->Bind();

        auto& materials = mesh->GetMaterials();
        for(Submesh& submesh : mesh->m_Submeshes)
        {
            auto material = overrideMaterial ? overrideMaterial : materials[submesh.MaterialIndex];
            auto shader = material->GetShader();
            material->Bind();

            shader->SetMat4("u_Transform", transform * submesh.Transform);
            Renderer::Submit([submesh, material]() {
                if(material->GetFlag(MaterialFlag::DepthTest))
                    glEnable(GL_DEPTH_TEST);
                else    
                    glDisable(GL_DEPTH_TEST);
                if(!material->GetFlag(MaterialFlag::TwoSided))
                    glEnable(GL_CULL_FACE);
                else
                    glDisable(GL_CULL_FACE);
                glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex );
            });
        }
    }

	void Renderer::SubmitFullscreenQuad(Ref<MaterialInstance> material)
	{
		bool depthTest = true;
		bool cullFace = true;
		if (material)
		{
			material->Bind();
			depthTest = material->GetFlag(MaterialFlag::DepthTest);
			cullFace = !material->GetFlag(MaterialFlag::TwoSided);
		}

		s_Data.m_FullscreenQuadVertexBuffer->Bind();
		s_Data.m_FullscreenQuadPipeline->Bind();
		s_Data.m_FullscreenQuadIndexBuffer->Bind();

		Renderer::DrawIndexed(6, PrimitiveType::Triangles, depthTest, cullFace);
	}

    RenderCommandQueue& Renderer::GetRenderCommandQueue()
	{
		return s_Data.m_CommandQueue;
	}
}
