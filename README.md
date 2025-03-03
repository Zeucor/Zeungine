# Zeungine

<img src="/images/zeungine-logo.png" alt="Zeungine Logo" width="400" height="400">

A library that simplifies 3D Game abstraction written in C++. Supports on Windows 10/11, Linux (X11) and MacOS

Uses CMake for it's build system and comes with some included tests

### Cloning

```bash
git clone git@github.com:Zeungine/Zeungine.git
```

### Build Dependencies


###### All platforms

 - `cmake` and `ninja` are required in your path
 - run configure and build, as user s, and, install as an admin

###### Windows

 - `Visual Studio 2022` or newer for MSVC
 - Run all commands inside 'xXX Native Tools Command Prompt for VS20XX'
 - also, required for swiftshader dependency, run once, as admin,
```bash
powershell -Command Set-ItemProperty -Path HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem -Name LongPathsEnabled -Value 1
```

###### Linux

```bash
apt install libx11-dev uuid-dev libglx-dev libgl1-mesa-dev libxfixes-dev libxrandr-dev libxkbcommon-dev
```

###### MacOS

 - *XCode* for `clang` compiler
 - `brew install cmake`

#### Compiling, Linking, Installing Dependencies (only need to do this as often as deps are updated)

```bash
cd Zeungine/cmake/Dependencies
cmake -B build -GNinja
cmake --build build
```

### Compiling Zeungine (library, tests and editor)

```bash
cd Zeungine
cmake -B build -GNinja
cmake --build build
```

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
