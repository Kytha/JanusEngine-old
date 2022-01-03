#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "VertexArray.h"
#include "Scene/Camera.h"
#include "ShaderLibrary.h"
#include "Mesh.h"
#include "Scene/Scene.h"

namespace Janus {
    struct LineVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
    };


    class Renderer
    {
    public:
        static void Init();
        static void BeginScene(Scene* scene);
        static void EndScene();
        static void Clear();
        static void Submit(const Ref<VertexArray> &vertexArray);
        static void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& overrideMaterial);
        static ShaderLibrary &GetShaderLibrary();

    private:
        static ShaderLibrary *s_ShaderLibrary;
    };
}
