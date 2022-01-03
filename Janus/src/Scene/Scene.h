
#pragma once
#include "Core/Core.h"
#include "Camera.h"
#include "Graphics/Light.h"
#include "Entity.h"
#include <string>

namespace Janus {
    class Scene
    {
    public:
        Scene(const std::string& debugName = "Scene");
        ~Scene();
        
        void Init();

        void OnUpdate(Timestep ts);

        inline void SetCamera(const Camera& camera) {m_Camera = camera;}
        inline void SetLight(const Light& light) {m_Light = light;}
        inline Camera& GetCamera() { return m_Camera; }
        inline Light& GetLight() { return m_Light; }
        void AddEntity(Entity* entity);
        Entity* CreateEntity();
    private:
        std::string m_DebugName;
        Light m_Light;
        Camera m_Camera;
        std::vector<Entity*> m_Entities;
        float m_LightMultiplier = 0.3f;
    };
}

