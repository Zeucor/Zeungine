# Zeungine

<img src="/images/zeungine-logo.jpg" alt="Zeungine Logo" width="400" height="400">

A library that simplifies Game abstraction written in C++

Comes with some modules, A `Fenster` module for 2D graphics drawing, and a `GL` module for 3D rendering

Uses CMake for it's build system and comes with some included tests

## Cloning

```bash
git clone git@github.com:ZeunO8/Zeungine.git --recurse-submodules
```

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
#include <zg/modules/fenster/Fenster.hpp>
using namespace zg::modules::fenster;
int main()
{
    FensterWindow game(640, 480);
    game.awaitWindowThread();
};
```

See [tests](/tests) for more usage examples

## License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.
