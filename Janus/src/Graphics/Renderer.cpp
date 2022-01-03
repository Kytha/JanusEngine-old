#include "jnpch.h"
#include "Renderer.h"
#include "Graphics/Camera.h"
#include "Core/Application.h"
#include "Mesh.h"

namespace Janus {
    ShaderLibrary *Renderer::s_ShaderLibrary = new ShaderLibrary();

    struct SceneData
    {
        SceneRendererCamera SceneCamera;
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

    void Renderer::BeginScene(Scene* scene, const SceneRendererCamera& camera)
    {
        glEnable(GL_DEPTH_TEST);
        s_SceneData.SceneCamera = camera;
        //Camera& camera = scene->GetCamera();
        //s_SceneData.ViewProjectionMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix();
        //s_SceneData.CameraPos = camera.GetPosition();
        s_SceneData.Light = scene->GetLight();
    }

    void Renderer::EndScene()
    {

    }

    void Renderer::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

    void Renderer::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::Submit(const Ref<VertexArray> &vertexArray)
    {

    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}

    void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<Material>& overrideMaterial) {
        mesh->m_VertexArray->Bind();
        auto&& materials = mesh->GetMaterials();
        
        auto& sceneCamera = s_SceneData.SceneCamera;
        auto viewProjection = sceneCamera.Camera.GetProjectionMatrix() * sceneCamera.ViewMatrix;
		glm::vec3 cameraPosition = glm::inverse(s_SceneData.SceneCamera.ViewMatrix)[3];

        for (Submesh& submesh : mesh->m_Submeshes)
        {
            // Material
            auto material = overrideMaterial ? overrideMaterial : materials[submesh.MaterialIndex];
            material->SetViewProjection(viewProjection);
            material->SetView(cameraPosition);
            auto shader = material->GetShader();
            material->Set(s_SceneData.Light);
            material->Bind();
            shader->UploadUniformMat4("u_Transform", transform * submesh.Transform);
            glDrawElementsBaseVertex(GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.BaseIndex), submesh.BaseVertex);
        }
    }
}
