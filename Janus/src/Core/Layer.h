#pragma once

#include "Core/Core.h"
#include "Core/Events/Event.h"
#include "Core/Timestep.h"

namespace Janus {
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		// To do when pushed/popped from layer stack
		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate(Timestep ts) {}

		// Entry point for layer-level event handling
		virtual void OnEvent(Event& event) {}

		// Optional rendering of ImGUI done in this function
		virtual void OnImGuiRender() {}
		
		// DEBUG ONLY
		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}