#include "jnpch.h"
#include "Core/Core.h"
#include "Core/Input.h"
#include "Scene.h"
#include "Graphics/Renderer.h"
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

    void Scene::OnUpdate(Timestep ts)
    {
        glEnable(GL_DEPTH_TEST);
        m_Camera.Update(ts);
        auto viewProjection = m_Camera.GetProjectionMatrix() * m_Camera.GetViewMatrix();
        // Render entities
        Renderer::BeginScene(this);
        Renderer::Clear();
        for (auto entity : m_Entities)
        {
            auto mesh = entity->GetMesh();
            if (mesh) {
                Ref<Material> overrideMaterial = nullptr;
                 Renderer::SubmitMesh(mesh, entity->GetTransform(), overrideMaterial);
            }
        }
        Renderer::EndScene();
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

