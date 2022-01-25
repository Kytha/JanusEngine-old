#pragma once

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Graphics/Mesh.h"

namespace Janus {

    using MaterialList = std::vector<Ref<MaterialInstance>>;
	class InspectorPanel
	{
	public:
		InspectorPanel() = default;
		InspectorPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);
		void SetSelected(Entity entity);

		void OnImGuiRender(bool window = true);
	private:
		void DrawTransformationComponent(TransformComponent& transformComponent);
        void DrawMeshComponent(const MeshComponent& meshComponent);
		void DrawSkylightComponent(SkyLightComponent& skylightComponent);
		void DrawMaterials(const MaterialList& materials);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}