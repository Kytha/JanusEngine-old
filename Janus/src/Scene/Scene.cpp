#include "jnpch.h"
#include "Core/Core.h"
#include "Core/Input.h"
#include "Scene.h"
#include "Graphics/SceneRenderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Scene/Entity.h"
#include "Graphics/Renderer.h"
namespace Janus
{
	Scene::Scene(const std::string &debugName)
		: m_DebugName(debugName)
	{
		Init();
	}

	Scene::~Scene()
	{
	}

	void Scene::Init()
	{
		auto skyboxShader = Renderer::GetShaderLibrary()->Get("janus_skybox");
		m_SkyboxMaterial = Material::Create(skyboxShader, "skybox_material");
		m_SkyboxMaterial->SetFlag(MaterialFlag::DepthTest, false);
	}

	void Scene::OnUpdate(Timestep ts, EditorCamera &editorCamera)
	{
		JN_PROFILE_FUNCTION();
		glEnable(GL_DEPTH_TEST);

		auto lights = m_Registry.group<SkyLightComponent>(entt::get<TransformComponent>);
		for (auto entity : lights)
		{
			auto [transformComponent, skyLightComponent] = lights.get<TransformComponent, SkyLightComponent>(entity);
			m_Environment = skyLightComponent.SceneEnvironment;
			m_EnvironmentIntensity = skyLightComponent.Intensity;
			m_SkyboxLod = skyLightComponent.LOD;
		}
		if (lights.empty() || !m_Environment)
		{
			m_Environment = Ref<Environment>::Create(Renderer::GetBlackCubeTexture(), Renderer::GetBlackCubeTexture());
		}

		SetSkybox(m_Environment->RadianceMap);
		m_SkyboxMaterial->Set("u_TextureLod", m_SkyboxLod);

		{
			auto pointLights = m_Registry.group<PointLightComponent>(entt::get<TransformComponent>);
			m_LightEnvironment.PointLights.resize(pointLights.size());
			uint32_t pointLightIndex = 0;
			for (auto entity : pointLights)
			{
				auto [transformComponent, lightComponent] = pointLights.get<TransformComponent, PointLightComponent>(entity);
				//Also copy the light size?
				m_LightEnvironment.PointLights[pointLightIndex++] = {
					transformComponent.Translation,
					lightComponent.Radiance,
					lightComponent.Intensity,
					lightComponent.Radius,
					lightComponent.Falloff,
				};
			}
		}

		SceneRenderer::BeginScene(this, {editorCamera, editorCamera.GetViewMatrix(), 0.1f, 1000.0f, 45.0f});
		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto [transformComponent, meshComponent] = group.get<TransformComponent, MeshComponent>(entity);
			if (meshComponent.Mesh)
			{
				Entity e = Entity(entity, this);
				Ref<Material> overrideMaterial = nullptr;
				SceneRenderer::SubmitMesh(meshComponent.Mesh, transformComponent.GetTransform(), overrideMaterial);
			}
		}
		SceneRenderer::EndScene();
	}

	Entity Scene::CreateEntity(const std::string &name)
	{
		JN_PROFILE_FUNCTION();

		auto entity = Entity{m_Registry.create(), this};
		auto &idComponent = entity.AddComponent<IDComponent>();
		idComponent.ID = {};

		entity.AddComponent<TransformComponent>();
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);

		m_EntityIDMap[idComponent.ID] = entity;
		return entity;
	}

	Entity Scene::CreateEntityWithID(UUID uuid, const std::string &name, bool runtimeMap)
	{
		JN_PROFILE_FUNCTION();

		auto entity = Entity{m_Registry.create(), this};
		auto &idComponent = entity.AddComponent<IDComponent>();
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

	Entity Scene::FindEntityByTag(const std::string &tag)
	{
		// TODO: If this becomes used often, consider indexing by tag
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const auto &canditate = view.get<TagComponent>(entity).Tag;
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
			auto &idComponent = m_Registry.get<IDComponent>(entity);
			if (idComponent.ID == id)
				return Entity(entity, this);
		}

		return Entity{};
	}

	Ref<Scene> Scene::CreateEmpty()
	{
		return new Scene("Empty");
	}

	void Scene::SetSkybox(const Ref<TextureCube> &skybox)
	{
		m_SkyboxTexture = skybox;
		m_SkyboxMaterial->Set("u_Texture", skybox);
	}
}
