#include "jnpch.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Graphics/SceneRenderer.h"
#include "Graphics/Renderer.h"
namespace Janus
{
    struct SceneRendererData
    {
        const Scene *ActiveScene = nullptr;

        struct SceneInfo
        {
            SceneRendererCamera sceneCamera;
            float SceneEnvironmentIntensity;
            Ref<MaterialInstance> SkyboxMaterial;
            Light ActiveLight;

        } sceneData;

        Ref<RenderPass> GeoPass;
        Ref<RenderPass> CompositePass;

        Ref<Shader> CompositeShader;
        struct DrawCommand
        {
            Ref<Mesh> Mesh;
            Ref<MaterialInstance> Material;
            glm::mat4 Transform;
        };
        Ref<MaterialInstance> GridMaterial;
        std::vector<DrawCommand> DrawList;
    };

    static SceneRendererData s_Data;

    void SceneRenderer::Init()
    {
        JN_PROFILE_FUNCTION();
        FramebufferSpecification geoFramebufferSpec;
        geoFramebufferSpec.Attachments = {FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::Depth};
        geoFramebufferSpec.Samples = 1;
        geoFramebufferSpec.ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        RenderPassSpecification geoRenderPassSpec;
        geoRenderPassSpec.TargetFramebuffer = Framebuffer::Create(geoFramebufferSpec);
        s_Data.GeoPass = Ref<RenderPass>::Create(geoRenderPassSpec);

        FramebufferSpecification compFramebufferSpec;
        compFramebufferSpec.Attachments = {FramebufferTextureFormat::RGBA8};
        compFramebufferSpec.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};

        RenderPassSpecification compRenderPassSpec;
        compRenderPassSpec.TargetFramebuffer = Framebuffer::Create(compFramebufferSpec);
        s_Data.CompositePass = Ref<RenderPass>::Create(compRenderPassSpec);

        auto gridShader = Renderer::GetShaderLibrary()->Get("janus_grid");
        s_Data.GridMaterial = MaterialInstance::Create(Material::Create(gridShader));
        s_Data.GridMaterial->SetFlag(MaterialFlag::TwoSided, true);
        float gridScale = 16.025f, gridSize = 0.025f;
        s_Data.GridMaterial->Set("u_Scale", gridScale);
        s_Data.GridMaterial->Set("u_Res", gridSize);
        //s_Data.CompositeShader = Ref<Shader>::Create("assets/shaders/SceneComposite.glsl");
    }

    void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
    {
        s_Data.GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
        s_Data.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
    }

    void SceneRenderer::BeginScene(const Scene *scene, const SceneRendererCamera &camera)
    {
        JN_PROFILE_FUNCTION();
        JN_ASSERT(!s_Data.ActiveScene, "SCENE_RENDERER_ERROR: Cannot begin a new scene while a scene is still active!");
        s_Data.ActiveScene = scene;
        s_Data.sceneData.sceneCamera = camera;
        s_Data.sceneData.SkyboxMaterial = scene->m_SkyboxMaterial;
        s_Data.sceneData.SceneEnvironmentIntensity = scene->m_EnvironmentIntensity;
        s_Data.sceneData.ActiveLight = scene->m_Light;
    }

    void SceneRenderer::EndScene()
    {
        JN_PROFILE_FUNCTION();
        JN_ASSERT(s_Data.ActiveScene, "SCENE_RENDERER_ERROR: Cannot end scene when there is no active scene!");
        s_Data.ActiveScene = nullptr;
        FlushDrawList();
    }

    void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4 &transform, Ref<MaterialInstance> overrideMaterial)
    {
        // TODO: Culling, sorting, etc.
        s_Data.DrawList.push_back({mesh, overrideMaterial, transform});
    }

    void SceneRenderer::GeometryPass()
    {
        JN_PROFILE_FUNCTION();
        Renderer::BeginRenderPass(s_Data.GeoPass);

        auto &sceneCamera = s_Data.sceneData.sceneCamera;
        auto viewProjection = sceneCamera.Camera.GetProjectionMatrix() * sceneCamera.ViewMatrix;
        glm::vec3 cameraPosition = glm::inverse(s_Data.sceneData.sceneCamera.ViewMatrix)[3];

		auto skyboxShader = s_Data.sceneData.SkyboxMaterial->GetShader();
		s_Data.sceneData.SkyboxMaterial->Set("u_InverseVP", glm::inverse(viewProjection));
		s_Data.sceneData.SkyboxMaterial->Set("u_SkyIntensity", s_Data.sceneData.SceneEnvironmentIntensity);
		Renderer::SubmitFullscreenQuad(s_Data.sceneData.SkyboxMaterial);

        for (auto &dc : s_Data.DrawList)
        {
            auto baseMaterial = dc.Mesh->GetMaterial();
            baseMaterial->Set("u_ViewProjectionMatrix", viewProjection);
            //baseMaterial->Set("u_ViewMatrix", sceneCamera.ViewMatrix);
            baseMaterial->Set("u_CameraPosition", cameraPosition);
            baseMaterial->Set("u_Lights", s_Data.sceneData.ActiveLight);
            auto overrideMaterial = nullptr;
            Renderer::SubmitMesh(dc.Mesh, dc.Transform, overrideMaterial);
        }
        s_Data.GridMaterial->Set("u_ViewProjection", viewProjection);
        Renderer::SubmitQuad(s_Data.GridMaterial, glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f)));

        //s_Data.GeoPass->GetSpecification().TargetFramebuffer->BindTexture();
        //s_Data.shader->Bind();
        //s_Data.texture->Bind(0);
        //Renderer::SubmitFullscreenQuad(nullptr);
        Renderer::EndRenderPass();
    }

    void SceneRenderer::CompositePass()
    {
    }

    void SceneRenderer::FlushDrawList()
    {
        GeometryPass();
        s_Data.DrawList.clear();
        s_Data.sceneData = {};
    }

    Ref<Framebuffer> SceneRenderer::GetFinalColorBuffer()
    {
        return s_Data.GeoPass->GetSpecification().TargetFramebuffer;
    }

    uint32_t SceneRenderer::GetFinalColorBufferRendererID()
    {
        return s_Data.GeoPass->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID();
    }

    static Ref<Shader> equirectangularConversionShader, envFilteringShader, envIrradianceShader;
    std::pair<Ref<TextureCube>, Ref<TextureCube>> SceneRenderer::CreateEnvironmentMap(const std::string& filepath)
	{
		const uint32_t cubemapSize = 2048;
		const uint32_t irradianceMapSize = 32;

		Ref<TextureCube> envUnfiltered = Ref<TextureCube>::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
		if (!equirectangularConversionShader)
			equirectangularConversionShader = Ref<Shader>::Create("assets/shaders/janus_EquirectangularToCubeMap.glsl");
		Ref<Texture> envEquirect = Ref<Texture>::Create(filepath);
		//JN_ASSERT(envEquirect->GetFormat() == TextureFormat::Float16, "Texture is not HDR!");

		equirectangularConversionShader->Bind();
		envEquirect->Bind();
		Renderer::Submit([envUnfiltered, cubemapSize, envEquirect]()
			{
				glBindImageTexture(0, envUnfiltered->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(cubemapSize / 32, cubemapSize / 32, 6);
				glGenerateTextureMipmap(envUnfiltered->GetRendererID());
			});

		if (!envFilteringShader)
			envFilteringShader = Ref<Shader>::Create("assets/shaders/janus_EnvironmentMipFilter.glsl");

		Ref<TextureCube> envFiltered = Ref<TextureCube>::Create(TextureFormat::Float16, cubemapSize, cubemapSize);

		Renderer::Submit([envUnfiltered, envFiltered]()
			{
				glCopyImageSubData(envUnfiltered->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
					envFiltered->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
					envFiltered->GetWidth(), envFiltered->GetHeight(), 6);
			});

		envFilteringShader->Bind();
		envUnfiltered->Bind();

		Renderer::Submit([envUnfiltered, envFiltered, cubemapSize]() {
			const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
			for (int level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2) // <= ?
			{
				const GLuint numGroups = glm::max(1, size / 32);
				glBindImageTexture(0, envFiltered->GetRendererID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glProgramUniform1f(envFilteringShader->GetRendererID(), 0, level * deltaRoughness);
				glDispatchCompute(numGroups, numGroups, 6);
			}
			});

		if (!envIrradianceShader)
			envIrradianceShader = Ref<Shader>::Create("assets/shaders/janus_EnvironmentIrradiance.glsl");

		Ref<TextureCube> irradianceMap = Ref<TextureCube>::Create(TextureFormat::Float16, irradianceMapSize, irradianceMapSize);
		envIrradianceShader->Bind();
		envFiltered->Bind();
		Renderer::Submit([irradianceMap]()
			{
				glBindImageTexture(0, irradianceMap->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);
				glGenerateTextureMipmap(irradianceMap->GetRendererID());
			});

		return { envFiltered, irradianceMap };
	}
}
