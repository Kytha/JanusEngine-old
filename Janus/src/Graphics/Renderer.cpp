#include "jnpch.h"
#include "Renderer.h"
#include "Scene/Camera.h"
#include "Core/Application.h"
#include "Mesh.h"

namespace Janus {
    ShaderLibrary *Renderer::s_ShaderLibrary = new ShaderLibrary();

    struct SceneData
    {
        glm::mat4 ViewProjectionMatrix;
        glm::vec3 CameraPos;
        Light Light;
    };

    static SceneData s_SceneData;

    void Renderer::Init()
    {
        s_ShaderLibrary->Load("./assets/shaders/janus_pbr.glsl", "janus_pbr");
    }

    ShaderLibrary &Renderer::GetShaderLibrary()
    {
        return *s_ShaderLibrary;
    }

    void Renderer::BeginScene(Scene* scene)
    {
        glEnable(GL_DEPTH_TEST);
        Camera& camera = scene->GetCamera();
        s_SceneData.ViewProjectionMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix();
        s_SceneData.CameraPos = camera.GetPosition();
        s_SceneData.Light = scene->GetLight();
    }

    void Renderer::EndScene()
    {

    }

    void Renderer::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::Submit(const Ref<VertexArray> &vertexArray)
    {

    }

    void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& overrideMaterial) {
        mesh->m_VertexArray->Bind();
        auto&& materials = mesh->GetMaterials();
        
        for (Submesh& submesh : mesh->m_Submeshes)
        {
            // Material
            auto material = overrideMaterial ? overrideMaterial : materials[submesh.MaterialIndex];
            material->SetViewProjection(s_SceneData.ViewProjectionMatrix);
            material->SetView(s_SceneData.CameraPos);
            auto shader = material->GetShader();
            material->Set(s_SceneData.Light);
            material->Bind();
            shader->UploadUniformMat4("u_Transform", transform * submesh.Transform);
            glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
        }
    }
}
