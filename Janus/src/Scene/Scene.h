
#pragma once
#include "Core/Core.h"
#include "Scene/EditorCamera.h"
#include "Graphics/Light.h"
#include "Entity.h"
#include <string>

namespace Janus {
    class Scene : public RefCounted
    {
    public:
        Scene(const std::string& debugName = "Scene");
        ~Scene();
        
        void Init();

        void OnUpdate(Timestep ts, EditorCamera& camera);

        inline void SetLight(const Light& light) {m_Light = light;}
        inline Light& GetLight() { return m_Light; }
        void AddEntity(Entity* entity);
        Entity* CreateEntity();
        Light m_Light;
    private:
        std::string m_DebugName;
        std::vector<Entity*> m_Entities;
        float m_LightMultiplier = 0.3f;
    };
}

