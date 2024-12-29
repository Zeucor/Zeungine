# AbstractNexus

<img src="/images/abstractnexus-logo.jpg" alt="AbstractNexus Logo" width="400" height="400">

A library that simplifies Game abstraction written in C++

Comes with some modules, starting with a `Fenster` module for simple graphics drawing

Uses CMake for it's build system and comes with some included tests

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
#include <anex/modules/fenster/Fenster.hpp>
using namespace anex::modules::fenster;
int main()
{
    FensterWindow game(640, 480);
    game.awaitWindowThread();
};
```

See [tests](/tests) for more usage examples

## License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.
