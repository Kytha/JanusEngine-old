#include "janus.h"

class SceneEditorLayer : public Janus::Layer
{
    public:
        SceneEditorLayer()
            : Layer("Scene Editor")
        { 
            m_Scene = Janus::CreateRef<Janus::Scene>("Test Scene");
            m_Scene->SetCamera(Janus::Camera());

            Janus::Entity* camera = m_Scene->CreateEntity();
            auto mesh = Janus::CreateRef<Janus::Mesh>("./assets/marble_bust_01_4k.gltf");
            camera->SetMesh(mesh);

            Janus::Light light;
            light.Position = {1.0f, 5.0f, 0.0f};
            light.Radiance = {0.5,0.5,0.5};

            m_Scene->SetLight(light);

        }

        void OnUpdate(Janus::Timestep ts) override
        {
            m_Scene->OnUpdate(ts);
        }
    private:
        Janus::Ref<Janus::Scene> m_Scene;
};

class SceneEditor : public Janus::Application
{
    public:
        SceneEditor()
        {
            PushLayer(new SceneEditorLayer());
        }

        ~SceneEditor()
        {

        }
};

Janus::Application* Janus::CreateApplication()
{
    return new SceneEditor;
}