#include "jnpch.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Core/Application.h"

#include "ImGui/ImGuiLayer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Janus
{

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	void ImGuiLayer::OnAttach()
	{

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	// Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		float fontSize = 18.0f; // *2.0f;
		io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", fontSize);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", fontSize);

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle &style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColorsV2();

		Application &app = Application::Get();
		GLFWwindow *window = static_cast<GLFWwindow *>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event &e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO &io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO &io = ImGui::GetIO();
		Application &app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow *backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto &colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

		// Headers
		colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
		colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
		colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
	}

	void ImGuiLayer::SetDarkThemeColorsV2()
	{
		auto& style = ImGui::GetStyle();
		auto& colors = ImGui::GetStyle().Colors;

		//========================================================
		/// Colours

		// Headers
		colors[ImGuiCol_Header]				= ImGui::ColorConvertU32ToFloat4(Colours::Theme::groupHeader);
		colors[ImGuiCol_HeaderHovered]		= ImGui::ColorConvertU32ToFloat4(Colours::Theme::groupHeader);
		colors[ImGuiCol_HeaderActive]		= ImGui::ColorConvertU32ToFloat4(Colours::Theme::groupHeader);

		// Buttons
		colors[ImGuiCol_Button]				= ImColor(56, 56, 56, 200);
		colors[ImGuiCol_ButtonHovered]		= ImColor(70, 70, 70, 255);
		colors[ImGuiCol_ButtonActive]		= ImColor(56, 56, 56, 150);

		// Frame BG
		colors[ImGuiCol_FrameBg]			= ImGui::ColorConvertU32ToFloat4(Colours::Theme::propertyField);
		colors[ImGuiCol_FrameBgHovered]		= ImGui::ColorConvertU32ToFloat4(Colours::Theme::propertyField);
		colors[ImGuiCol_FrameBgActive]		= ImGui::ColorConvertU32ToFloat4(Colours::Theme::propertyField);

		// Tabs
		colors[ImGuiCol_Tab]				= ImGui::ColorConvertU32ToFloat4(Colours::Theme::titlebar);
		colors[ImGuiCol_TabHovered]			= ImColor(139, 92, 246, 30);
		colors[ImGuiCol_TabActive]			= ImColor(139, 92, 246, 60);
		colors[ImGuiCol_TabUnfocused]		= ImGui::ColorConvertU32ToFloat4(Colours::Theme::titlebar);
		colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];

		// Title
		colors[ImGuiCol_TitleBg]			= ImGui::ColorConvertU32ToFloat4(Colours::Theme::titlebar);
		colors[ImGuiCol_TitleBgActive]		= ImGui::ColorConvertU32ToFloat4(Colours::Theme::titlebar);
		colors[ImGuiCol_TitleBgCollapsed]	= ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Resize Grip
		colors[ImGuiCol_ResizeGrip]			= ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered]	= ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive]	= ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg]		= ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab]		= ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colors[ImGuiCol_CheckMark]			= ImGui::ColorConvertU32ToFloat4(Colours::Theme::accent);

		// Slider
		colors[ImGuiCol_SliderGrab]			= ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive]	= ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

		// Text
		colors[ImGuiCol_Text]				= ImGui::ColorConvertU32ToFloat4(Colours::Theme::text);


		// Separator
		colors[ImGuiCol_Separator]			= ImGui::ColorConvertU32ToFloat4(Colours::Theme::backgroundDark);
		colors[ImGuiCol_SeparatorActive]	= ImGui::ColorConvertU32ToFloat4(Colours::Theme::highlight);
		colors[ImGuiCol_SeparatorHovered]	= ImColor(39, 185, 242, 150);

		// Window Background
		colors[ImGuiCol_WindowBg]			= ImGui::ColorConvertU32ToFloat4(Colours::Theme::titlebar);
		colors[ImGuiCol_ChildBg]			= ImGui::ColorConvertU32ToFloat4(Colours::Theme::background);
		colors[ImGuiCol_PopupBg]			= ImGui::ColorConvertU32ToFloat4(Colours::Theme::backgroundPopup);
		colors[ImGuiCol_Border]				= ImGui::ColorConvertU32ToFloat4(Colours::Theme::backgroundDark);

		// Tables
		colors[ImGuiCol_TableHeaderBg]		= ImGui::ColorConvertU32ToFloat4(Colours::Theme::groupHeader);
		colors[ImGuiCol_TableBorderLight]	= ImGui::ColorConvertU32ToFloat4(Colours::Theme::backgroundDark);

		// Menubar
		colors[ImGuiCol_MenuBarBg]			= ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };

		//========================================================
		/// Style
		style.FrameRounding = 2.5f;
		style.FrameBorderSize = 1.0f;
		style.IndentSpacing = 11.0f;
	}
}
