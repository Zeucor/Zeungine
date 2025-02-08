# Zeungine

<img src="/images/zeungine-logo.png" alt="Zeungine Logo" width="400" height="400">

A library that simplifies 3D Game abstraction written in C++. Supports on Windows 10/11, Linux (X11) and MacOS

Uses CMake for it's build system and comes with some included tests

### Cloning

```bash
git clone git@github.com:Zeungine/Zeungine.git --recursive
```

### Dependencies

###### All platforms

`cmake`

###### Windows

`Visual Studio 2022` or newer for MSVC

###### Linux

```bash
apt install libx11-dev uuid-dev libglx-dev libgl1-mesa-dev libxfixes-dev libxrandr-dev libxkbcommon-dev
```

also, you may need these libs to build some of the vendor examples

```bash
apt install libxinerama-dev libxcursor-dev libxi-dev
```

###### MacOS

`XCode` for `clang` compiler
`cmake`: `brew install cmake`


### Building

Either use your preferred IDE of choice, or run the following commands to build

```bash
cmake -B build .
cmake --build build
```

### Testing

```bash
ctest --test-dir build --rerun-failed -VV -C Debug
```

### Usage

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
            (Window &)window, // reference to window
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
        window.setIScene(std::make_shared<ExampleScene>(window));
    });
    window.run();
}
```

See [tests](/tests) for more usage examples

## License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.
