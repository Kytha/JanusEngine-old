#include "jnpch.h"
#include "InspectorPanel.h"
#include <imgui.h>
#include "imgui_internal.h"
#include "Core/Application.h"
#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Janus {
   	InspectorPanel::InspectorPanel(const Ref<Scene>& context)
		: m_Context(context)
	{
        m_CheckerboardTex = Janus::Ref<Janus::Texture>::Create("assets/textures/Checkerboard.tga");
	}
	void InspectorPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}

    void InspectorPanel::SetSelected(Entity entity)
	{
		m_SelectionContext = entity;

		//if (m_SelectionChangedCallback)
			//m_SelectionChangedCallback(m_SelectionContext);
	}

	void InspectorPanel::OnImGuiRender(bool window)
	{
        if (window)
		    ImGui::Begin("Inspector");
        
        
        if (m_SelectionContext)
        {
            if (m_SelectionContext.HasComponent<Janus::TransformComponent>())
            {
                DrawTransformationComponent(m_SelectionContext.GetComponent<Janus::TransformComponent>());
            }
            if (m_SelectionContext.HasComponent<Janus::MeshComponent>())
            {
                Janus::Ref<Janus::Mesh> mesh = m_SelectionContext.GetComponent<MeshComponent>().Mesh;
                if(mesh) {
                    DrawMaterials(mesh->GetMaterials());
                }
            }
        }
        
       
        if (window)
			ImGui::End();
    }

    void InspectorPanel::DrawTransformationComponent(TransformComponent& transformComponent) 
    {

        if (ImGui::CollapsingHeader("Transform", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
        {
            std::string id = "##Translation";
            ImGui::Text("Translation");
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat3(id.c_str(), glm::value_ptr(transformComponent.Translation), 0.25f);

            ImGui::PopItemWidth();
            ImGui::NextColumn();
            
            ImGui::Text("Rotation");
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat3("##rotation", glm::value_ptr(transformComponent.Rotation), 0.25f);


            ImGui::PopItemWidth();
            ImGui::NextColumn();

            ImGui::Text("Scale");
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            glm::vec3 scale = transformComponent.Scale;
            ImGui::DragFloat3("##scale", glm::value_ptr(transformComponent.Scale), 0.25f);
            ImGui::PopItemWidth();
        }
    }

    void InspectorPanel::DrawMaterials(const MaterialList& materials)
    {
        if (ImGui::CollapsingHeader("Materials", nullptr, ImGuiTreeNodeFlags_DefaultOpen)){
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
            Janus::Ref<Janus::MaterialInstance> material = materials[selectedMaterialIndex];
            ImGui::Text("Shader: %s", material->GetShader()->GetName().c_str());
            {
                if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); 
                    bool useAlbedoMap = true;
                    glm::vec3 albedoColor = {1.0,0.0,0.0};
                    //auto& albedoColor = material->GetVector3("u_MaterialUniforms.AlbedoColor");
                    Janus::Ref<Janus::Texture> albedoMap = material->TryGetResource("u_AlbedoTexture");
                    if(albedoMap)
                        ImGui::Image((void*)albedoMap->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                    ImGui::PopStyleVar();

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Checkbox("Use##AlbedoMap", &useAlbedoMap);
                    ImGui::EndGroup();
                    ImGui::SameLine();
                    ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(albedoColor), ImGuiColorEditFlags_NoInputs);
                }

                if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); 
                    bool useAlbedoMap = true;
                    Janus::Ref<Janus::Texture> normalMap = material->TryGetResource("u_NormalTexture");
                    if(normalMap)
                        ImGui::Image((void*)normalMap->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                    ImGui::PopStyleVar();
                }

                if(ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); 
                    bool useMetalnessMap = false;
                    Janus::Ref<Janus::Texture> metalnessMap = material->TryGetResource("u_MetalnessTexture");
                    if(metalnessMap)
                        ImGui::Image((void*)metalnessMap->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                    else {
                        ImGui::Image((void*)m_CheckerboardTex->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                    }
                    float metalness = 0.5;
                    ImGui::PopStyleVar();
                    ImGui::SameLine();
                    ImGui::Checkbox("Use##MetalnessMap", &useMetalnessMap);
                    ImGui::SameLine();
                    ImGui::SliderFloat("Value##MetalnessInput", &metalness, 0.0f, 1.0f);
                }
                if(ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10)); 
                    bool useRoughnessMap = false;
                    Janus::Ref<Janus::Texture> roughnessMap = material->TryGetResource("u_RoughnessTexture");
                    if(roughnessMap)
                        ImGui::Image((void*)roughnessMap->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                    else {
                        ImGui::Image((void*)m_CheckerboardTex->m_RendererID, ImVec2{ 64,64 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                    }
                    float roughness = 0.5;
                    ImGui::PopStyleVar();

                    if (ImGui::IsItemClicked())
                    {
                        std::string filename = Application::Get().OpenFile("");
                        if (filename != "")
                        {
                            roughnessMap = Janus::Ref<Janus::Texture>::Create(filename);
                            material->Set("u_RoughnessTexture", roughnessMap);
                            material->Set("u_RoughnessTexToggle", 1.0f);
                        }
                    }
                    ImGui::SameLine();
                    ImGui::Checkbox("Use##RoughnessMap", &useRoughnessMap);
                    ImGui::SameLine();
                    ImGui::SliderFloat("Value##RoughnessInput", &roughness, 0.0f, 1.0f);
                }
            }
        }

        } 
    }
}