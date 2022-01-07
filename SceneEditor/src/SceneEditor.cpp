#include "janus.h"
#include "imgui.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

class SceneSceneEditorLayer : public Janus::Layer
{
    public:
    	enum class PropertyFlag
		{
			None = 0, ColorProperty = 1, DragProperty = 2, SliderProperty = 4
		};
    public:
        SceneSceneEditorLayer()
            : Layer("Scene Editor"), m_EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 1000.0f))
        { 
            m_Scene = Janus::Ref<Janus::Scene>::Create("Test Scene");
            Janus::Entity* bust = m_Scene->CreateEntity();
            auto mesh = Janus::Ref<Janus::Mesh>::Create("./assets/marble_bust_01_4k.gltf");
            bust->SetMesh(mesh);

            Janus::Light light;
            light.Position = {1.0f, 5.0f, 0.0f};
            light.Radiance = {0.5,0.5,0.5};
            
            m_Scene->SetLight(light);

        }

        void OnAttach()
        {

        }

        void OnUpdate(Janus::Timestep ts) override
        {
            if (Janus::FramebufferSpecification spec = (Janus::SceneRenderer::GetFinalColorBuffer())->GetSpecification();
			    m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && 
			    (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		    {
                Janus::SceneRenderer::SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
                m_EditorCamera.SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
                m_EditorCamera.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), m_ViewportSize.x, m_ViewportSize.y, 0.1f, 1000.0f));
		    }
            
            if (m_ViewportFocused && m_AllowViewportCameraEvents) {
				m_EditorCamera.OnUpdate(ts);
            }
            m_Scene->OnUpdate(ts, m_EditorCamera);
        }

        void OnEvent(Janus::Event& e) 
        {
            m_EditorCamera.OnEvent(e);
        }

        void OnImGuiRender() 
        {
            
            static bool dockspaceOpen = true;
            static bool opt_fullscreen_persistant = true;
            bool opt_fullscreen = opt_fullscreen_persistant;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
		    {
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		    }
            
            //if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			//window_flags |= ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
           	ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
                ImGui::PopStyleVar(); 
                if (opt_fullscreen)
                    ImGui::PopStyleVar(2);

                
                ImGuiIO& io = ImGui::GetIO();
                ImGuiStyle& style = ImGui::GetStyle();
                float minWinSizeX = style.WindowMinSize.x;
                style.WindowMinSize.x = 370.0f;
                if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
                {
                    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                }

                style.WindowMinSize.x = minWinSizeX;
                
                ImGui::Begin("Environment");
                
                float skyboxLod;
                ImGui::SliderFloat("Skybox LOD", &skyboxLod, 0.0f, 11.0f);

                ImGui::Columns(2);
                ImGui::AlignTextToFramePadding();

                auto& light = m_Scene->GetLight();
                Property("Light Radiance", light.Radiance, PropertyFlag::ColorProperty);
                Property("Light Position", light.Position, -30.0f, 30.0f, PropertyFlag::SliderProperty);
                Property("Light Multiplier", light.Irradiance, 0.0f, 5.0f, PropertyFlag::SliderProperty);

                ImGui::Columns(1);
                ImGui::End();


                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::Begin("Viewport");
                
                    m_ViewportFocused = ImGui::IsWindowFocused();
		            m_ViewportHovered = ImGui::IsWindowHovered();

		            auto viewportOffset = ImGui::GetCursorPos();
                    auto viewportSize = ImGui::GetContentRegionAvail();

		            Janus::Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);
                    
                    ImGui::Image((void*)Janus::SceneRenderer::GetFinalColorBufferRendererID(), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                    
                    m_ViewportSize = { viewportSize.x, viewportSize.y };
                    auto windowSize = ImGui::GetWindowSize();
                    ImVec2 minBound = ImGui::GetWindowPos();
                    minBound.x += viewportOffset.x;
                    minBound.y += viewportOffset.y;

                    ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
                    m_ViewportBounds[0] = { minBound.x, minBound.y };
                    m_ViewportBounds[1] = { maxBound.x, maxBound.y };
                    m_AllowViewportCameraEvents = ImGui::IsMouseHoveringRect(minBound, maxBound);
                ImGui::End();
                
                ImGui::PopStyleVar();
                
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("File"))
                    {
                    // Disabling fullscreen would allow the window to be moved to the front of other windows, 
                    // which we can't undo at the moment without finer window depth/z control.
                    //ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
                    if (ImGui::MenuItem("New", "Ctrl+N")) {}
                    if (ImGui::MenuItem("Open...", "Ctrl+O")) {}
                    if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {}
                    if (ImGui::MenuItem("Exit")) Janus::Application::Get().Close();
                    ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

            ImGui::End();
            
        }

        bool Property(const std::string& name, bool& value)
        {
            ImGui::Text(name.c_str());
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);

            std::string id = "##" + name;
            bool result = ImGui::Checkbox(id.c_str(), &value);

            ImGui::PopItemWidth();
            ImGui::NextColumn();
            
            return result;
        }

        bool Property(const std::string& name, float& value, float min, float max, PropertyFlag flags)
        {
            ImGui::Text(name.c_str());
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);

            std::string id = "##" + name;
            bool changed = false;
            if (flags == PropertyFlag::SliderProperty)
                changed = ImGui::SliderFloat(id.c_str(), &value, min, max);
            else
                changed = ImGui::DragFloat(id.c_str(), &value, 1.0f, min, max);

            ImGui::PopItemWidth();
            ImGui::NextColumn();
            
            return changed;
        }

        bool Property(const std::string& name, glm::vec2& value, PropertyFlag flags)
        {
            return Property(name, value, -1.0f, 1.0f, flags);
        }

        bool Property(const std::string& name, glm::vec2& value, float min, float max, PropertyFlag flags)
        {
            ImGui::Text(name.c_str());
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);

            std::string id = "##" + name;
            bool changed = false;
            if (flags == PropertyFlag::SliderProperty)
                changed = ImGui::SliderFloat2(id.c_str(), glm::value_ptr(value), min, max);
            else
                changed = ImGui::DragFloat2(id.c_str(), glm::value_ptr(value), 1.0f, min, max);

            ImGui::PopItemWidth();
            ImGui::NextColumn();

            return changed;
        }

        bool Property(const std::string& name, glm::vec3& value, PropertyFlag flags)
        {
            return Property(name, value, -1.0f, 1.0f, flags);
        }

        bool Property(const std::string& name, glm::vec3& value, float min, float max, PropertyFlag flags)
        {
            ImGui::Text(name.c_str());
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);

            std::string id = "##" + name;
            bool changed = false;
            if ((int)flags & (int)PropertyFlag::ColorProperty)
                changed = ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
            else if (flags == PropertyFlag::SliderProperty)
                changed = ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max);
            else
                changed = ImGui::DragFloat3(id.c_str(), glm::value_ptr(value), 1.0f, min, max);

            ImGui::PopItemWidth();
            ImGui::NextColumn();

            return changed;
        }

        bool Property(const std::string& name, glm::vec4& value, PropertyFlag flags)
        {
            return Property(name, value, -1.0f, 1.0f, flags);
        }

        bool Property(const std::string& name, glm::vec4& value, float min, float max, PropertyFlag flags)
        {
            ImGui::Text(name.c_str());
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);

            std::string id = "##" + name;
            bool changed = false;
            if ((int)flags & (int)PropertyFlag::ColorProperty)
                changed = ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
            else if (flags == PropertyFlag::SliderProperty)
                changed = ImGui::SliderFloat4(id.c_str(), glm::value_ptr(value), min, max);
            else
                changed = ImGui::DragFloat4(id.c_str(), glm::value_ptr(value), 1.0f, min, max);

            ImGui::PopItemWidth();
            ImGui::NextColumn();

            return changed;
        }

    private:
        Janus::Ref<Janus::Scene> m_Scene;
        Janus::EditorCamera m_EditorCamera;
        bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
        bool m_AllowViewportCameraEvents;
};

class SceneEditor : public Janus::Application
{
    public:
        SceneEditor()
        {
            PushLayer(new SceneSceneEditorLayer());
        }

        ~SceneEditor()
        {

        }
};

Janus::Application* Janus::CreateApplication()
{
    return new SceneEditor;
}