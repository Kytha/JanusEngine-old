#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "VertexArray.h"
#include "ShaderLibrary.h"
#include "Mesh.h"
#include "Scene/Scene.h"
#include "Graphics/Camera.h"

namespace Janus {

    struct SceneRendererCamera
	{
		Camera Camera;
		glm::mat4 ViewMatrix;
		float Near, Far;
		float FOV;
	};
    class Renderer
    {
    public:
        static void Init();
        static void BeginScene(Scene* scene, const SceneRendererCamera& camera);
        static void EndScene();
        static void Clear();
        static void SetClearColor(const glm::vec4& color);
        static void Submit(const Ref<VertexArray> &vertexArray);
        static void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& overrideMaterial);
        static void OnWindowResize(uint32_t width, uint32_t height);
        static ShaderLibrary &GetShaderLibrary();

    private:
        static ShaderLibrary *s_ShaderLibrary;
    };
}
