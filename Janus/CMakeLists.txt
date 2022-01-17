
########################### SOURCE CODE ########################################

SET ( JANUS_SOURCE
    src/Core/Log.cpp
    src/Core/Layer.cpp
    src/Core/LayerStack.cpp
    src/Core/Application.cpp
    src/Core/UUID.cpp
    src/Graphics/Shader.cpp
    src/Graphics/ShaderUniform.cpp
    src/Graphics/Texture.cpp
    src/Graphics/Mesh.cpp
    src/Graphics/VertexBuffer.cpp
    src/Graphics/IndexBuffer.cpp
    src/Graphics/FrameBuffer.cpp
    src/Graphics/OpenGLFrameBuffer.cpp
    src/Graphics/ShaderLibrary.cpp
    src/Graphics/Material.cpp
    src/Graphics/Renderer.cpp
    src/Graphics/Pipeline.cpp
    src/Graphics/RenderPass.cpp
    src/Graphics/RenderCommandQueue.cpp
    src/Graphics/SceneRenderer.cpp
    src/Graphics/Camera.cpp
    src/Scene/Entity.cpp
    src/Scene/EditorCamera.cpp
    src/Graphics/Environment.cpp
    src/Scene/Scene.cpp
    src/Scene/SceneHierarchyPanel.cpp
    src/Core/stb_image/stb_imageBuild.cpp
    src/Platform/Windows/WindowsWindow.cpp
    src/Platform/Windows/WindowsInput.cpp
    src/Utilities/StringUtils.cpp
    src/Scene/InspectorPanel.cpp
    src/ImGui/Colours.cpp
    src/ImGui/ImGuiBuild.cpp
    src/ImGui/ImGuiLayer.cpp
  )

SET ( JANUS_HEADERS
    src/Core/Core.h
    src/Core/Log.h
    src/Core/Events/Event.h
    src/Core/Events/KeyEvent.h
    src/Core/Events/MouseEvent.h
    src/Core/Events/ApplicationEvent.h
    src/Core/KeyCodes.h
    src/Core/MouseButtonCodes.h
    src/Core/Input.h
    src/Core/Timestep.h
    src/Core/Layer.h
    src/Core/LayerStack.h
    src/Core/Window.h
    src/Core/Application.h
    src/Core/UUID.h
    src/Debug/Instrumentor.h
    src/Graphics/Shader.h
    src/Graphics/ShaderUniform.h
    src/Graphics/Texture.h
    src/Graphics/Mesh.h
    src/Graphics/VertexBuffer.h
    src/Graphics/IndexBuffer.h
    src/Graphics/FrameBuffer.h
    src/Graphics/OpenGLFrameBuffer.h
    src/Graphics/ShaderLibrary.h
    src/Graphics/Material.h
    src/Graphics/Light.h
    src/Graphics/Renderer.h
    src/Graphics/Pipeline.h
    src/Graphics/RenderPass.h
    src/Graphics/SceneRenderer.h
    src/Graphics/RenderCommandQueue.h
    src/Graphics/Environment.h
    src/Graphics/Camera.h
    src/Scene/Scene.h
    src/Scene/Entity.h
    src/Scene/EditorCamera.h
    src/Scene/Components.h
    src/Scene/SceneHierarchyPanel.h
    src/Scene/InspectorPanel.h
    src/Core/stb_image/stb_image.h
    src/Platform/Windows/WindowsWindow.h
    src/Platform/Windows/WindowsInput.h
    src/Utilities/StringUtils.h
    src/ImGui/ImGui.h
    src/ImGui/ImGuiUtilities.h
    src/ImGui/Colours.h
    src/ImGui/ImGuiLayer.h
  )

############################# DEPENDENCIES ###########################################

ADD_LIBRARY ( janus STATIC
  ${JANUS_SOURCE}
  ${JANUS_HEADERS}
  )

ADD_SUBDIRECTORY(vendors/glfw)
ADD_SUBDIRECTORY(vendors/Glad)
ADD_SUBDIRECTORY(vendors/glm)
ADD_SUBDIRECTORY(vendors/spdlog)

# Workaround to googletest using std::tr1 which is deprecated in c++17
ADD_DEFINITIONS(-DGTEST_LANG_CXX11=1)

SET(ASSIMP_BUILD_TESTS OFF CACHE BOOL "disable assimp tests" FORCE)
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Build as static library" FORCE)
SET(ASSIMP_BUILD_ZLIB ON CACHE BOOL "Build Zlib" FORCE)
SET(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Disable builidng assimp tools" FORCE)
SET(ASSIMP_INSTALL OFF CACHE BOOL "Use assimp as submodule" FORCE)

ADD_SUBDIRECTORY(vendors/assimp)

ADD_SUBDIRECTORY(vendors/entt)
ADD_SUBDIRECTORY(vendors/imgui)

TARGET_INCLUDE_DIRECTORIES( janus PUBLIC src)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/glfw/include)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/glad/include)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/glm/glm)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/spdlog/include)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/assimp/include)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/imgui)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/imgui/backends)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/entt/single_include)

TARGET_LINK_LIBRARIES( janus PUBLIC glfw PUBLIC Glad PUBLIC spdlog PUBLIC glm PUBLIC assimp PUBLIC EnTT PUBLIC imgui)

TARGET_PRECOMPILE_HEADERS( janus
    PUBLIC "src/jnpch.h")

INSTALL ( TARGETS
  janus
  DESTINATION
  lib
  )
