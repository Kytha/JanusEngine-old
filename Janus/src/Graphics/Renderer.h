#pragma once

#include <glm/glm.hpp>

#include "Graphics/Shader.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/ShaderLibrary.h"
#include "Graphics/Mesh.h"
#include "Graphics/Camera.h"
#include "Graphics/RenderCommandQueue.h"
#include "Graphics/Pipeline.h"
#include "Graphics/RenderPass.h"

#include "Scene/Scene.h"

namespace Janus
{

    enum class PrimitiveType
    {
        None = 0,
        Triangles,
        Lines
    };
    class Renderer
    {
    public:
        static void Init();

        static void Clear();
        static void Clear(float r, float g, float b, float a = 1.0f);
        static void SetClearColor(float r, float g, float b, float a);

        static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true, bool faceCulling = true);

        template <typename FuncT>
        static void Submit(FuncT &&func)
        {
            auto renderCmd = [](void *ptr)
            {
                auto pFunc = (FuncT *)ptr;
                (*pFunc)();
                //static_assert(std::is_trivially_destructible_v<FuncT>, "FuncT must be trivially destructible");
                pFunc->~FuncT();
            };
            auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
            new (storageBuffer) FuncT(std::forward<FuncT>(func));
        }

        static void WaitAndRender();
        static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true);
        static void EndRenderPass();
        static void SubmitQuad(Ref<Material> material, const glm::mat4 &transform = glm::mat4(1.0f));
        static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4 &transform, Ref<Material> overrideMaterial = nullptr);
        static void SubmitFullscreenQuad(Ref<Material> material);
        static Ref<TextureCube> GetBlackCubeTexture();
        static Ref<ShaderLibrary> GetShaderLibrary();

    private:
        static RenderCommandQueue &GetRenderCommandQueue();
    };
}
