
#pragma once
#include "Core/Core.h"
#include "Scene/EditorCamera.h"
#include "Graphics/Texture.h"
#include "Graphics/Material.h"
#include "Graphics/Light.h"
#include <string>
#include "entt/entt.hpp"
#include "Core/UUID.h"
#include "Graphics/Environment.h"
namespace Janus {

    class Entity;
    using EntityMap = std::unordered_map<UUID, Entity>;
    class Scene : public RefCounted
    {
    public:
        Scene(const std::string& debugName = "Scene");
        ~Scene();
        
        void Init();

        void OnUpdate(Timestep ts, EditorCamera& camera);

        inline void SetLight(const Light& light) {m_Light = light;}
        inline Light& GetLight() { return m_Light; }

        UUID GetUUID() const { return m_SceneID; }
		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithID(UUID uuid, const std::string& name = "", bool runtimeMap = false);
		void DestroyEntity(Entity entity);

        Entity FindEntityByTag(const std::string& tag);
		Entity FindEntityByUUID(UUID id);

        const EntityMap& GetEntityMap() const { return m_EntityIDMap; }
        void SetEnvironmentMap(Ref<Environment> environment) {m_Environment = environment;}
        float& GetSkyboxLOD() {return m_SkyboxLod;}
        void SetSkybox(const Ref<TextureCube>& skybox);

        Light m_Light;
    public:
        static Ref<Scene> CreateEmpty();
        Ref<Material> m_SkyboxMaterial;
    private:
        UUID m_SceneID;
        std::string m_DebugName;
        float m_LightMultiplier = 0.3f;
        entt::entity m_SceneEntity;
		entt::registry m_Registry;
        EntityMap m_EntityIDMap;
        Ref<TextureCube> m_SkyboxTexture;
        Ref<Environment> m_Environment;
        float m_SkyboxLod = 0.1f;
        float m_EnvironmentIntensity = 1.0f;

        friend class Entity;
        friend class SceneRenderer;
        friend class SceneHierarchyPanel;
    };
}

