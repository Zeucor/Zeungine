# Zeungine

<img src="images/zeungine-logo.png" alt="Zeungine Logo" width="400" height="400">

A library that simplifies 3D Game abstraction written in C++. Supports on Windows 10/11, Linux (X11) and MacOS

Uses CMake for it's build system and comes with some included tests

![Build Status](https://github.com/Zeucor/Zeungine/actions/workflows/every-tag.yml/badge.svg)

***zg*** uses many libraries to help piece together the engine

 - [OpenGL](https://www.opengl.org/)
 - [Vulkan](https://www.vulkan.org/)
 - [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools)
 - [glslang](https://github.com/KhronosGroup/glslang)
 - [Boost Libraries](https://www.boost.org/)
 - [OpenSSL](https://www.openssl.org/)
 - [FFmpeg](https://www.ffmpeg.org/)
 - [zlib](https://github.com/madler/zlib)
 - [bzip2](https://github.com/libarchive/bzip2)
 - [lzma](https://tukaani.org/xz/)
 - [zstd](https://github.com/facebook/zstd)
 - [exprtk](https://github.com/ArashPartow/exprtk)
 - [brotli](https://github.com/google/brotli)
 - [freetype](https://freetype.org/)
 - [glm](https://github.com/icaven/glm)
 - [harfbuzz](https://harfbuzz.github.io/)
 - [png](http://www.libpng.org/pub/png/libpng.html)

### Cloning

```bash
git clone git@github.com:Zeungine/Zeungine.git
```

### Releases

Releases are available on GitHub, see [here](https://github.com/Zeucor/Zeungine/releases), for development you'll need either static or shared dependencies and zeungine library, as well as headers. I recommend static builds for shipping production binaries as all dependencies will be bundled into your final game executable. 

### Builing from Source

If ultimately you want a installed copy of Zeungine and dependencies then you'll be best off using...

`win-package.bat` or `unx-package.sh`

You can analyze these files for their configure, compile, install and package commands

###### All platforms

The following programs must be available in your PATH 
 - `cmake`
 - `make`
 - `bash` (on windows you can add `C:\Program Files\Git\bin` to your PATH)
 - `nasm`

###### Windows

 - `Visual Studio 2022` or newer for MSVC
 - [msys2](https://www.msys2.org/)
 - `C:\\msys64\\msys2_shell.cmd -defterm -no-start -mingw64 -here -use-full-path -c "pacman -S --noconfirm make diffutils"`
 - `choco install dos2unix ninja`
 - run all commands from Command Prompt (cmd.exe) as Administrator (for install/package)

###### Linux

```bash
apt install nasm libvulkan-dev libx11-dev uuid-dev libglx-dev libgl1-mesa-dev libxfixes-dev libxrandr-dev libxkbcommon-dev libxcb-keysyms1-dev libdrm-dev ninja-build libwayland-client0 libwayland-server0 libwayland-dev libxfixes-dev libxcb-xfixes0-dev libxrender-dev libxrandr-dev libxcb-util-dev libdrm-dev libgtkmm-3.0-dev
```

apt packages for zeungine coming soon

###### MacOS

 - *XCode* for `clang` compiler
 - `brew install cmake`

### Testing

```bash
ctest --test-dir build --rerun-failed -VV -C Debug
```

### Usage

###### zedit

Create and load projects using `zedit` (scene, hotreloader) (should be in path)

###### some sample codes

Cube Demo
```cpp
#include <zg/Window.hpp>
#include <zg/Scene.hpp>
#include <zg/vp/VML.hpp>
#include <zg/entities/Cube.hpp>
using namespace zg;
struct ExampleScene : Scene
{
    vp::VML vml; // view mouse look
    ExampleScene(Window &window):
        Scene(window,
              {0, 10, 10}, // camera position
              glm::normalize(glm::vec3(0, -1, -1)), //camera direction
              81.f // fov
        ),
        vml(*this)
    {
        addEntity(std::make_shared<entities::Cube>(
            window, // reference to window
            *this, // reference to scene
            glm::vec3(0, 0, 0), // position
            glm::vec3(0, 0, 0), // rotation
            glm::vec3(1, 1, 1), // scale
            glm::vec3(2, 1, 4), // cube size
            shaders::RuntimeConstants() // additional shader constants
        ));
    };
};
int main()
{
    Window window("Cube Window", 640, 480, -1, -1);
    window.runOnThread([](auto &window)
    {
        window.setScene(std::make_shared<ExampleScene>(window));
    });
    window.run();
}
```

###### event on update (whilepressed@winloop) window keypresses and move an entity

```cpp
zg::UniqueIdentifier leftKeyPressID = window.addKeyUpdateHandler(20, [&]{
    view.position.x -= 1.f * window.deltaTime;
    view.update(); });
zg::UniqueIdentifier rightKeyPressID = window.addKeyUpdateHandler(19, [&]{
    view.position.x += 1.f * window.deltaTime;
    view.update(); });
zg::UniqueIdentifier downKeyPressID = window.addKeyUpdateHandler(18, [&](){
    view.position.y -= 1.f * window.deltaTime;
    view.update(); });
zg::UniqueIdentifier upKeyPressID = window.addKeyUpdateHandler(17, [&](){
    view.position.y += 1.f * window.deltaTime;
    view.update(); });
// store IDs
// later ...
window.removeKeyUpdateHandler(20, leftKeyPressID);
window.removeKeyUpdateHandler(19, rightKeyPressID);
window.removeKeyUpdateHandler(18, downKeyPressID);
window.removeKeyUpdateHandler(17, upKeyPressID);
```

See [tests](/tests) for more usage examples

## License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.
