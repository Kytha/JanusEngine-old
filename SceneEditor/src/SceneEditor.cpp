#include "janus.h"
#include "imgui.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

class SceneEditorLayer : public Janus::Layer
{
    public:
        SceneEditorLayer()
            : Layer("Scene Editor"), m_EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 1000.0f))
        { 
            m_Scene = Janus::CreateRef<Janus::Scene>("Test Scene");
            Janus::Entity* camera = m_Scene->CreateEntity();
            auto mesh = Janus::CreateRef<Janus::Mesh>("./assets/marble_bust_01_4k.gltf");
            camera->SetMesh(mesh);

            Janus::Light light;
            light.Position = {1.0f, 5.0f, 0.0f};
            light.Radiance = {0.5,0.5,0.5};

            m_Scene->SetLight(light);

        }

        void OnAttach()
        {
            Janus::FramebufferSpecification fbSpec;
            fbSpec.Attachments = { Janus::FramebufferTextureFormat::RGBA8, Janus::FramebufferTextureFormat::RED_INTEGER, Janus::FramebufferTextureFormat::Depth };
		    fbSpec.Width = 1280;
		    fbSpec.Height = 720;
		    m_Framebuffer = Janus::Framebuffer::Create(fbSpec);

        }
        void OnUpdate(Janus::Timestep ts) override
        {
            if (Janus::FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			    m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			    (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		    {
			    m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			    m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		    }
            m_Framebuffer->Bind();
            Janus::Renderer::SetClearColor({ 0.4f, 0.4f, 0.4f, 1 });
            Janus::Renderer::Clear();
            m_Framebuffer->ClearAttachment(1, -1);
            m_EditorCamera.OnUpdate(ts);
            m_Scene->OnUpdate(ts, m_EditorCamera);
            m_Framebuffer->Unbind();
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
            
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

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
                
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::Begin("Viewport");
                    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		            auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		            auto viewportOffset = ImGui::GetWindowPos();
		            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

                    m_ViewportFocused = ImGui::IsWindowFocused();
		            m_ViewportHovered = ImGui::IsWindowHovered();
		            Janus::Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

                    auto viewportPanelSize = ImGui::GetContentRegionAvail();
                    m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
                    uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
                    ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                ImGui::End();
                
                ImGui::PopStyleVar();

            ImGui::End();
            
        }

    private:
        Janus::Ref<Janus::Scene> m_Scene;
        Janus::EditorCamera m_EditorCamera;
        Janus::Ref<Janus::Framebuffer> m_Framebuffer;
        bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
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