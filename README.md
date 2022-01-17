# Janus Engine

A cross-platform game engine built in C++ for creating 2D & 3D games. This is an educational project.

## Building

Janus Engine uses CMake for it's build system, so you have to make sure that you have a working cmake-installation on your system. You can download it at https://cmake.org/ or for linux install it via

```bash
sudo apt-get install cmake
```

Make sure you have a working git-installation. Open a command prompt and recursively clone the repo via:

```bash
git clone --recursive https://github.com/Kytha/Janus.git
```

Alternatively, if the repository was not cloned recursively use the following command

```bash
git submodule update --init
```

Once your local repo is setup, you'll want to execute the following commands in the root directory to configure CMake

```bash
mkdir build
cd build
cmake ..
make
```

All Janus Engine dependencies are included in the repo, so the first compilation may take some time.

## Dependancies

- [GLFW](https://github.com/glfw/glfw) - Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop
- [GLAD](https://glad.dav1d.de/) - Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
- [glm](https://github.com/g-truc/glm) - Header only C++ mathematics library for graphics software based on the GLSL
- [imgui](https://github.com/ocornut/imgui) - Immediate-mode, bloat-free graphical user interface library for C++
- [spdlog](https://github.com/gabime/spdlog) - Very fast, header-only/compiled, C++ logging library
- [entt](https://github.com/skypjack/entt) - A header-only, tiny and easy to use entity component system library written in modern C++
- [assimp](https://github.com/assimp/assimp) - A library to import and export various 3d-model-formats

Shout out to the authors and communities of these open-source libraries, which make projects like Janus Engine possible.

## Platform Support

The Janus engine is intended to be a cross-platform engine to design cross-platform games. Applications built with the Janus Engine have only been officially tested on Windows, but in its current state should support the following platforms;

- Windows ✔️
- Mac ✔️
- Linux ✔️
- Android ❌
- IOS ❌
