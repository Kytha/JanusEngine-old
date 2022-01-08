#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "ShaderLibrary.h"
#include "Mesh.h"
#include "Scene/Scene.h"
#include "Graphics/Camera.h"
#include "RenderCommandQueue.h"
#include "Pipeline.h"
#include "RenderPass.h"

namespace Janus {

    enum class PrimitiveType
	{
		None = 0, Triangles, Lines
	};
    class Renderer
    {
    public:
        static void Init();

        static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);
		static void SetClearColor(float r, float g, float b, float a);

		static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true, bool faceCulling = true);
        
        template<typename FuncT>
        static void Submit(FuncT&& func)
        {
            auto renderCmd = [](void* ptr) {
                auto pFunc = (FuncT*)ptr;
                (*pFunc)();
            };
            auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
            new (storageBuffer) FuncT(std::forward<FuncT>(func));
        }

        static void WaitAndRender();
        static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true);
		static void EndRenderPass();
        static void SubmitQuad(Ref<MaterialInstance> material, const glm::mat4& transform = glm::mat4(1.0f));
        static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial = nullptr);
        static void SubmitFullscreenQuad(Ref<MaterialInstance> material);
        static Ref<ShaderLibrary> GetShaderLibrary();

    private:
        static RenderCommandQueue& GetRenderCommandQueue();
    };
}
