#pragma once

#include "Core/Core.h"
#include "Core/Timestep.h"
#include "Core/Window.h"
#include "Core/Events/Event.h"
#include "Core/LayerStack.h"
#include "Core/Events/ApplicationEvent.h"

#include "ImGui/ImGuiLayer.h"
namespace Janus
{

	// Application is a signleton object which maintains the window and game loop
	class Application
	{

	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event &e);

		void PushLayer(Layer *layer);

		void PushOverlay(Layer *layer);

		void Close();
		// Exposes the window object for input polling
		inline Window &GetWindow() { return *m_Window; }
		ImGuiLayer *GetImGuiLayer() { return m_ImGuiLayer; }
		void RenderImGui();
		inline static Application &Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent &e);
		bool OnWindowResize(WindowResizeEvent &e);
		// Pointer to the applications window object. An application can only have one window
		std::unique_ptr<Window> m_Window;
		ImGuiLayer *m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		// Pointer to the apps application object
		static Application *s_Instance;
	};

	// To be defined in client
	// Client defines a Janus Application object to be returned by this function
	Application *CreateApplication();
}
