#include "jnpch.h"
#include "Core/Core.h"
#include "Core/Input.h"
#include "Scene.h"
#include "Graphics/SceneRenderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Scene/Entity.h"
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
        JN_PROFILE_FUNCTION();
        glEnable(GL_DEPTH_TEST);
        // Render entities
        /*
        auto view = m_Registry.view<TransformComponent>();
        for (auto entity : view)
        {
            auto& transformComponent = view.get(entity);
            Entity e = Entity(entity, this);
            glm::mat4 transform = e.Transform().GetTransform();
            glm::vec3 translation;
			glm::vec3 rotation;
			glm::vec3 scale;
            Math::DecomposeTransform(transform, translation, rotation, scale);
            glm::quat rotationQuat = glm::quat(rotation);
            transformComponent.Up = glm::normalize(glm::rotate(rotationQuat, glm::vec3(0.0f, 1.0f, 0.0f)));
            transformComponent.Right = glm::normalize(glm::rotate(rotationQuat, glm::vec3(1.0f, 0.0f, 0.0f)));
            transformComponent.Forward = glm::normalize(glm::rotate(rotationQuat, glm::vec3(0.0f, 0.0f, -1.0f)));
        }
        */

        SceneRenderer::BeginScene(this, { editorCamera, editorCamera.GetViewMatrix(), 0.1f, 1000.0f, 45.0f });
        auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
        for (auto entity : group)
        {
            auto [transformComponent, meshComponent] = group.get<TransformComponent, MeshComponent>(entity);
            if (meshComponent.Mesh) 
            {
                Entity e = Entity(entity, this);
                Ref<MaterialInstance> overrideMaterial = nullptr;
                SceneRenderer::SubmitMesh(meshComponent.Mesh, transformComponent.GetTransform(), overrideMaterial);
            }
        }
        SceneRenderer::EndScene();
    }

	Entity Scene::CreateEntity(const std::string& name)
	{
		JN_PROFILE_FUNCTION();

		auto entity = Entity{ m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<IDComponent>();
		idComponent.ID = {};

		entity.AddComponent<TransformComponent>();
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);

		m_EntityIDMap[idComponent.ID] = entity;
		return entity;
	}

    Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name, bool runtimeMap)
	{
		JN_PROFILE_FUNCTION();

		auto entity = Entity{ m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<IDComponent>();
		idComponent.ID = uuid;

		entity.AddComponent<TransformComponent>();
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);

		JN_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end(), "");
		m_EntityIDMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		JN_PROFILE_FUNCTION();
		m_Registry.destroy(entity.m_EntityHandle);
	}

    Entity Scene::FindEntityByTag(const std::string& tag)
	{
		// TODO: If this becomes used often, consider indexing by tag
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const auto& canditate = view.get<TagComponent>(entity).Tag;
			if (canditate == tag)
				return Entity(entity, this);
		}

		return Entity{};
	}

    Entity Scene::FindEntityByUUID(UUID id)
	{
		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			auto& idComponent = m_Registry.get<IDComponent>(entity);
			if (idComponent.ID == id)
				return Entity(entity, this);
		}

		return Entity{};
	}

    Ref<Scene> Scene::CreateEmpty()
	{
		return new Scene("Empty");
	}
}

