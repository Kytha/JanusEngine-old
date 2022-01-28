#include "jnpch.h"
#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include "imgui_internal.h"
#include "ImGui/ImGui.h"
namespace Janus {
   	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
		: m_Context(context)
	{
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}

    void SceneHierarchyPanel::SetSelected(Entity entity)
	{
		m_SelectionContext = entity;

		if (m_SelectionChangedCallback)
			m_SelectionChangedCallback(m_SelectionContext);
	}

	void SceneHierarchyPanel::OnImGuiRender(bool window)
	{
        if (window)
		ImGui::Begin("Scene Hierarchy");
        ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };
        if (m_Context)
        {
			if (ImGui::Button("Add Entity")) 
				ImGui::OpenPopup("CreateEntityPanel");

			if (UI::BeginPopup("CreateEntityPanel")) {
				if (ImGui::MenuItem("Empty Entity")) {
					auto newEntity = m_Context->CreateEntity("Empty Entity");
					SetSelected(newEntity);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Point Light")) {
					auto newEntity = m_Context->CreateEntity("New Point Light");
					newEntity.AddComponent<PointLightComponent>();
					newEntity.GetComponent<TransformComponent>().Translation = glm::vec3{ 0 };
					SetSelected(newEntity);
					ImGui::CloseCurrentPopup();
				}
				UI::EndPopup();
			}


            for (auto entity : m_Context->m_Registry.view<IDComponent>())
			{
                DrawEntityNode({ entity, m_Context.Raw() });
            }
        }

        if (window)
		ImGui::End();
    }


    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
       	const char* name = "Unnamed Entity";
		if (entity.HasComponent<TagComponent>())
			name = entity.GetComponent<TagComponent>().Tag.c_str(); 
        ImGuiTreeNodeFlags flags = (entity == m_SelectionContext ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        flags |= ImGuiTreeNodeFlags_Leaf;
        
        const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name);
        
        if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
			if (m_SelectionChangedCallback)
				m_SelectionChangedCallback(m_SelectionContext);
		}
        bool entityDeleted = false;
		
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

        if(opened) {
            ImGui::TreePop();
        }

		
        if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (entity == m_SelectionContext)
				m_SelectionContext = {};

			if(m_EntityDeletedCallback)
				m_EntityDeletedCallback(entity);
		}
    }

}