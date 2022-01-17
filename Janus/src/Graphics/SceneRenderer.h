#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture.h"

#include "Scene/Scene.h"

namespace Janus
{

	struct SceneRendererOptions
	{
		bool ShowGrid = true;
		bool ShowBoundingBoxes = false;
	};

	struct SceneRendererCamera
	{
		Janus::Camera Camera;
		glm::mat4 ViewMatrix;
		float Near, Far;
		float FOV;
	};

	class SceneRenderer
	{
	public:
		static void Init();

		static void SetViewportSize(uint32_t width, uint32_t height);

		static void BeginScene(const Scene *scene, const SceneRendererCamera &camera);
		static void EndScene();

		static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4 &transform = glm::mat4(1.0f), Ref<MaterialInstance> overrideMaterial = nullptr);

		//static Ref<RenderPass> GetFinalRenderPass();
		static Ref<Framebuffer> GetFinalColorBuffer();

		static std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath);

		// TODO: Temp
		static uint32_t GetFinalColorBufferRendererID();

		//static void SetFocusPoint(const glm::vec2& point);

		//static SceneRendererOptions& GetOptions();

		//static void OnImGuiRender();
	private:
		static void FlushDrawList();
		static void GeometryPass();
		static void CompositePass();
		//static void BloomBlurPass();

		//static void ShadowMapPass();
	};

}