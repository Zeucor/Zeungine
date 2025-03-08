# Zeungine

<img src="/images/zeungine-logo.png" alt="Zeungine Logo" width="400" height="400">

A library that simplifies 3D Game abstraction written in C++. Supports on Windows 10/11, Linux (X11) and MacOS

Uses CMake for it's build system and comes with some included tests

### Cloning

```bash
git clone git@github.com:Zeungine/Zeungine.git
```

### Releases

Releases are available on GitHub, see [here](https://github.com/Zeucor/Zeungine/releases), ideally for development you'll need all **5** packages `(2*(debug&release)+headers)`, when shipping production packages built using Zeungine you _can_ ship with just **2** (release(zeungine+dependencies)) installers

### Builing from Source

If ultimately you want a installed copy of Zeungine and dependencies then you'll be best off using...

`win-package.bat` or `unx-package.sh`

You can analyze these files for their configure, compile, install and package commands

###### All platforms

 - `cmake` and `ninja` are required in your path

###### Windows

 - `Visual Studio 2022` or newer for MSVC
 - `msys64` installed at `C:\msys64` for ffmpeg shell consistency
 - Run all commands inside 'xXX Native Tools Command Prompt for VS20XX' as admin

###### Linux

```bash
apt install libx11-dev uuid-dev libglx-dev libgl1-mesa-dev libxfixes-dev libxrandr-dev libxkbcommon-dev
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
zg::EventIdentifier leftKeyPressID = window.addKeyUpdateHandler(20, [&]{
    view.position.x -= 1.f * window.deltaTime;
    view.update(); });
zg::EventIdentifier rightKeyPressID = window.addKeyUpdateHandler(19, [&]{
    view.position.x += 1.f * window.deltaTime;
    view.update(); });
zg::EventIdentifier downKeyPressID = window.addKeyUpdateHandler(18, [&](){
    view.position.y -= 1.f * window.deltaTime;
    view.update(); });
zg::EventIdentifier upKeyPressID = window.addKeyUpdateHandler(17, [&](){
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
