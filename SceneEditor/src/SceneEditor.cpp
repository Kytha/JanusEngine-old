#include "janus.h"
#include "imgui.h"
#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLFW/glfw3.h"
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

            m_SceneHierarchyPanel = Janus::CreateScope<Janus::SceneHierarchyPanel>(m_Scene);
            m_InspectorPanel = Janus::CreateScope<Janus::InspectorPanel>(m_Scene);
            

            m_SceneHierarchyPanel->SetSelectionChangedCallback(std::bind(&SceneSceneEditorLayer::SelectEntity, this, std::placeholders::_1));
            m_SceneHierarchyPanel->SetEntityDeletedCallback(std::bind(&SceneSceneEditorLayer::OnEntityDeleted, this, std::placeholders::_1));
            auto mesh = Janus::Ref<Janus::Mesh>::Create("./assets/tg1lch1fa_LOD0.fbx");
            Janus::Entity entity = m_Scene->CreateEntity("bust");
            entity.AddComponent<Janus::MeshComponent>(mesh);

            Janus::Entity skybox = m_Scene->CreateEntity("skybox");
            skybox.AddComponent<Janus::SkyLightComponent>(Janus::Environment::Load("assets/env/pink_sunrise_4k.hdr"));            

            Janus::Light light;
            light.Position = {1.0f, 5.0f, 0.0f};
            light.Radiance = {0.5,0.5,0.5};
            
            m_Scene->SetLight(light);

        }

        void OnAttach()
        {
             m_CheckerboardTex = Janus::Ref<Janus::Texture2D>::Create("assets/textures/Checkerboard.tga");
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
            
            if (m_AllowViewportCameraEvents) {
				m_EditorCamera.OnUpdate(ts);
            }
            m_Scene->OnUpdate(ts, m_EditorCamera);
        }

        void OnEvent(Janus::Event& e) 
        {
            m_EditorCamera.OnEvent(e);
        }

        void SelectEntity(Janus::Entity entity)
	    {
            if (!entity)
                return;
            m_SelectionContext.clear();
            m_SelectionContext.push_back(entity);
            m_InspectorPanel->SetSelected(entity);
	    }

        void OnEntityDeleted(Janus::Entity entity)
	    {
		    m_SelectionContext.clear();
            m_InspectorPanel->SetSelected({});
	    }


        void OnImGuiRender() 
        {
            
            static bool opt_fullscreen_persistant = true;
            bool opt_fullscreen = opt_fullscreen_persistant;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
            
            ImGuiIO& io = ImGui::GetIO();
            ImGuiStyle& style = ImGui::GetStyle();
            io.ConfigWindowsResizeFromEdges = io.BackendFlags & ImGuiBackendFlags_HasMouseCursors;
            ImGuiWindowFlags window_flags = /*ImGuiWindowFlags_MenuBar | */ImGuiWindowFlags_NoDocking;
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
            auto* window = static_cast<GLFWwindow*>(Janus::Application::Get().GetWindow().GetNativeWindow());
            bool isMaximized = (bool)glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.0f, 6.0f) : ImVec2(1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
            ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
           	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
            ImGui::PopStyleColor(); // MenuBarBg
		    ImGui::PopStyleVar(2);

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

                float minWinSizeX = style.WindowMinSize.x;
                style.WindowMinSize.x = 370.0f;
                if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
                {
                    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                }

                style.WindowMinSize.x = minWinSizeX;
                
                ImGui::Begin("Environment");

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

            ImGui::Begin("Materials");
            if (m_SelectionContext.size()) {
                Janus::Entity selectedEntity = m_SelectionContext.front();
                if (selectedEntity.HasComponent<Janus::MeshComponent>()) 
                {
                    Janus::Ref<Janus::Mesh> mesh = selectedEntity.GetComponent<Janus::MeshComponent>().Mesh;
                    if(mesh){
                        auto& materials = mesh->GetMaterials();
                        static uint32_t selectedMaterialIndex = 0;
                        for (uint32_t i = 0; i < materials.size(); i++)
                        {
                            auto& material = materials[i];
                            std::string materialName = material->GetName();
                            if (materialName.empty())
                                materialName = "Unnamed Material #" + std::to_string(i);
                            ImGuiTreeNodeFlags node_flags = (selectedMaterialIndex == i ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
                            bool opened = ImGui::TreeNodeEx((void*)(&material), node_flags, materialName.c_str());
                            if (ImGui::IsItemClicked())
                            {
                                selectedMaterialIndex = i;
                            }
                            if (opened)
                                ImGui::TreePop();
                        }
                        ImGui::Separator();  

                        if (selectedMaterialIndex < materials.size())
                        {
                            Janus::Ref<Janus::Material> material = materials[selectedMaterialIndex];
                            ImGui::Text("Shader: %s", material->GetShader()->GetName().c_str());
                            {
                                if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); 
                                    float albedoTexToggle = material->Get<float>("u_AlbedoTexToggle");
                                    bool useAlbedoMap = albedoTexToggle > 0; 
                                    glm::vec3 albedoColor = material->Get<glm::vec3>("u_AlbedoColor");
                                    //auto& albedoColor = material->GetVector3("u_MaterialUniforms.AlbedoColor");
                                    Janus::Ref<Janus::Texture2D> albedoMap = material->TryGetResource<Janus::Texture2D>("u_AlbedoTexture");
                                    if(albedoMap)
                                        ImGui::Image((void*)albedoMap->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                                    else {
                                        ImGui::Image((void*)m_CheckerboardTex->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                                    }
                                    ImGui::PopStyleVar();

                                    if (ImGui::IsItemClicked())
                                    {
                                        std::string filename = Janus::Application::Get().OpenFile("");
                                        if (filename != "")
                                        {
                                            albedoMap = Janus::Ref<Janus::Texture2D>::Create(filename);
                                            material->Set("u_AlbedoTexture", albedoMap);
                                            material->Set("u_AlbedoTexToggle", 1.0f);
                                        }
                                    }
                                    
                                    ImGui::SameLine();
                                    ImGui::BeginGroup();
                                    ImGui::Checkbox("Use##AlbedoMap", &useAlbedoMap);
                                    if(useAlbedoMap) {
                                        material->Set("u_AlbedoTexToggle", 1.0f);
                                    } else {
                                        material->Set("u_AlbedoTexToggle", 0.0f);
                                    };
                                    
                                    ImGui::EndGroup();
                                    ImGui::SameLine();
                                    if(ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(albedoColor), ImGuiColorEditFlags_NoInputs)) {
                                        material->Set("u_AlbedoColor", albedoColor);
                                    }
                                }

                                if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); 
                                    Janus::Ref<Janus::Texture2D> normalMap = material->TryGetResource<Janus::Texture2D>("u_NormalTexture");
                                    if(normalMap)
                                        ImGui::Image((void*)normalMap->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                                    else {
                                        ImGui::Image((void*)m_CheckerboardTex->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                                    }
                                    ImGui::PopStyleVar();

                                    if (ImGui::IsItemClicked())
                                    {
                                        std::string filename = Janus::Application::Get().OpenFile("");
                                        if (filename != "")
                                        {
                                            normalMap = Janus::Ref<Janus::Texture2D>::Create(filename);
                                            material->Set("u_NormalTexture", normalMap);
                                            material->Set("u_NormalTexToggle", 1.0f);
                                        }
                                    }
                                }

                                if(ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); 

                                    float metalnessTexToggle = material->Get<float>("u_MetalnessTexToggle");
                                    bool useMetalnessMap = metalnessTexToggle > 0; 

                                    Janus::Ref<Janus::Texture2D> metalnessMap = material->TryGetResource<Janus::Texture2D>("u_MetalnessTexture");
                                    if(metalnessMap)
                                        ImGui::Image((void*)metalnessMap->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                                    else {
                                        ImGui::Image((void*)m_CheckerboardTex->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                                    }
                                    float metalness = material->Get<float>("u_Metalness");
                                    ImGui::PopStyleVar();
                                    ImGui::SameLine();

                                    ImGui::Checkbox("Use##MetalnessMap", &useMetalnessMap);

                                    if(useMetalnessMap) {
                                        material->Set("u_MetalnessTexToggle", 1.0);
                                    } else {
                                        material->Set("u_MetalnessTexToggle", 0.0);
                                    }
                                    ImGui::SameLine();
                                    if(ImGui::SliderFloat("Value##MetalnessInput", &metalness, 0.0f, 1.0f)) {
                                        material->Set("u_Metalness", metalness);
                                    }
                                }
                                if(ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); 
                                    float roughnessTexToggle = material->Get<float>("u_RoughnessTexToggle");
                                    bool useRoughnessMap = roughnessTexToggle > 0; 

                                    Janus::Ref<Janus::Texture2D> roughnessMap = material->TryGetResource<Janus::Texture2D>("u_RoughnessTexture");
                                    if(roughnessMap)
                                        ImGui::Image((void*)roughnessMap->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                                    else {
                                        ImGui::Image((void*)m_CheckerboardTex->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                                    }
                                    float roughness = material->Get<float>("u_Roughness");
                                    ImGui::PopStyleVar();

                                    if (ImGui::IsItemClicked())
                                    {
                                        std::string filename = Janus::Application::Get().OpenFile("");
                                        if (filename != "")
                                        {
                                            roughnessMap = Janus::Ref<Janus::Texture2D>::Create(filename);
                                            material->Set("u_RoughnessTexture", roughnessMap);
                                            material->Set("u_RoughnessTexToggle", 1.0f);
                                        }
                                    }
                                    ImGui::SameLine();
                                    ImGui::Checkbox("Use##RoughnessMap", &useRoughnessMap);
                                    if(useRoughnessMap) {
                                        material->Set("u_RoughnessTexToggle", 1.0f);
                                    } else {
                                        material->Set("u_RoughnessTexToggle", 0.0f);
                                    }
                                    ImGui::SameLine();
                                    if(ImGui::SliderFloat("Value##RoughnessInput", &roughness, 0.0f, 1.0f)) {
                                        material->Set("u_Roughness", roughness);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ImGui::End();
            m_SceneHierarchyPanel->OnImGuiRender();
            m_InspectorPanel->OnImGuiRender();
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
        Janus::Scope<Janus::SceneHierarchyPanel> m_SceneHierarchyPanel;
        Janus::Scope<Janus::InspectorPanel> m_InspectorPanel;
        std::vector<Janus::Entity> m_SelectionContext;
        Janus::EditorCamera m_EditorCamera;
        bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
        bool m_AllowViewportCameraEvents;
        Janus::Ref<Janus::Texture2D> m_CheckerboardTex;
};

class SceneEditor : public Janus::Application
{
    public:
        SceneEditor() : Application("Janus Editor - Example Scene")
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