#include "jnpch.h"
#include "Core/Core.h"
#include "Core/Input.h"
#include "Scene.h"
#include "Graphics/SceneRenderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Janus {
    Scene::Scene(const std::string& debugName)
        : m_DebugName(debugName)
    {
        Init();
    }

    Scene::~Scene()
    {

    }

    void Scene::Init()
    {

    }

    void Scene::OnUpdate(Timestep ts, EditorCamera& editorCamera)
    {
        glEnable(GL_DEPTH_TEST);
        // Render entities
        SceneRenderer::BeginScene(this, { editorCamera, editorCamera.GetViewMatrix(), 0.1f, 1000.0f, 45.0f });
        for (auto entity : m_Entities)
        {
            auto mesh = entity->GetMesh();
            if (mesh) {
                Ref<MaterialInstance> overrideMaterial = nullptr;
                 SceneRenderer::SubmitMesh(mesh, entity->GetTransform(), overrideMaterial);
            }
        }
        SceneRenderer::EndScene();
    }

    void Scene::AddEntity(Entity* entity)
    {
        m_Entities.push_back(entity);
    }

    Entity* Scene::CreateEntity()
    {
        Entity* entity = new Entity();
        AddEntity(entity);
        return entity;
    }
}

