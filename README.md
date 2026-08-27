# 🎮 Platformer

A simple 2D platformer built with OpenGL. This project is designed as a learning and experimentation sandbox to explore:

- **Modern C++**
- **Test-Driven Development (TDD)**
- **Data-driven design**
- **Platformer mechanics**

![alt text](https://media2.giphy.com/media/v1.Y2lkPTc5MGI3NjExYnltbXlwdmw0eWM5OGo4ZzRjd3d3NXQzMXdxd3hhaGN5dzl4NHdweiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/AYZJkrDUkgunRRcqNP/giphy.gif)

## 🖥 Supported Setups

Targets **C++23**.

| | Windows | macOS | Linux |
|---|---|---|---|
| Compiler | MSVC | LLVM clang | LLVM clang with libc++ |
| Editor | Visual Studio 2022 | VS Code | VS Code |
| Build | the editor's CMake support | the editor's CMake support | the editor's CMake support |
| Debug | Visual Studio's Run button | CMake Tools' debug button | CMake Tools' debug button |
| Code intelligence | Visual Studio's own | clangd | clangd |

Terminal commands below assume a Unix shell. On Windows use Git Bash, and `python`
where they say `python3`.

## 🧰 What The Repository Configures

Every platform builds the same way, from `CMakeLists.txt`.

| | |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | the `platformer_lib`, `platformer` and `tests` targets |
| [.vscode/extensions.json](.vscode/extensions.json) | the extensions VS Code should install |
| [.vscode/settings.json](.vscode/settings.json) | the generator, the build directory, and clangd in place of the C/C++ extension |
| [.clang-format](.clang-format) | formatting, applied on save |
| [.clang-tidy](.clang-tidy) | naming and includes, applied on save |
| [.llvm-version](.llvm-version) | the LLVM the tools must come from |
| [tools/hooks/pre-commit](tools/hooks/pre-commit) | formats staged json and sources |

Visual Studio needs nothing further: it reads `CMakeLists.txt` directly and brings its
own IntelliSense and debugger. VS Code divides the work between three extensions.
**CMake Tools** configures and builds, **clangd** provides completion, navigation and
diagnostics, and the **C/C++** extension provides the debugger. clangd reads the compile
database CMake writes, so it sees the same flags the compiler does; the C/C++ extension
parses with a front end of its own and can disagree, so its IntelliSense is switched off.

CMake writes that database and links it to the repository root, so clangd reads
whichever of `build/Debug` or `build/Release` you configured last. Other build
directories leave the link alone, so a one off build cannot take it.

No `launch.json` is needed; CMake Tools runs and debugs the selected target from its
own build directory, which is where the game looks for assets. Format on save is
off for json only, since no formatter has been found that reproduces the format the
game writes.

## 🛠 Setup

1. Install the prerequisites.

    For Windows, Visual Studio 2022 with **CMake** and **Git for Windows** added
    through *Tools and Features → Individual Components*.

    For macOS and Linux, **CMake**, **Ninja**, **git-lfs**, and **LLVM** with its
    tools, which install separately from the compiler:

    ```bash
    brew install llvm clang-format                       # macOS
    sudo apt-get install clang clang-format clang-tidy   # Linux
    ```

    [.llvm-version](.llvm-version) is the version CI builds and checks with, and
    the one the pre commit hook looks for. Match it, and `CMakeLists.txt` will
    tell you if what you have is too old.

    `PATH` often finds a different clangd first, so check that `clangd --version`
    matches your compiler. Name it in your user settings if it does not:

    ```json
    "clangd.path": "/opt/homebrew/opt/llvm/bin/clangd"
    ```

2. Clone with submodules:

    ```bash
    git clone https://github.com/iluzdaf/platformer.git
    git submodule update --init --recursive
    ```

3. Open the repository folder in your editor. Both configure CMake on open; VS Code
    also offers to install the recommended extensions.

4. Install the git hooks:

    ```bash
    python3 tools/hooks/install.py
    ```

5. Pick `platformer` or `tests` as the target, choose **Debug** or **Release**, and
    use the run and debug buttons. The game reads its assets relative to the build
    directory, which is where both editors run it from.

## 🎨 Style Guide

Formatting is defined by [.clang-format](.clang-format) and naming by
[.clang-tidy](.clang-tidy), which also asks whether a file includes what it uses.
Editors apply them on save and the pre commit hook formats what you stage. Both
tools must be the version CI uses, since others disagree. To sweep the whole tree,
and to check that every header compiles on its own:

```bash
clang-format -i $(find src include tests -name '*.cpp' -o -name '*.hpp')
clang-tidy -p build/Debug $(find src tests -name '*.cpp')
cmake --build build/Debug --target header_self_containment
```

Two conventions those tools cannot express:

- File names are `snake_case`, as in `tile_map.hpp`.
- `glz::meta::value` must keep that name because Glaze requires it, so it carries a
  `NOLINTNEXTLINE`.

### Json Assets

Levels and `game_data.json` share one format, written both by `TileMap::save` and by
`tools/format_json.py`. Structure goes on its own lines, leaves stay compact:

```json
"indices":[
    [14, 0, 0, 0, 0,14],
    [14,34,34,34,34,14]
]
```

No formatter has been found that reproduces this, so json formatting is turned off in
the workspace settings.
The pre commit hook normalises anything that drifts, and CI checks it:

```bash
python3 tools/format_json.py --format   # write the format
python3 tools/format_json.py --check    # verify it
```

## 🤖 What CI Does

Every pull request runs two jobs at once. All four results must be green to merge.

| job | runs on | does |
|---|---|---|
| `build-and-test` | Windows, macOS, Linux | configures, builds under `-Werror`, runs the test suite |
| `checks` | Linux | the four questions below, which are about source text and so only need answering once |

| check | run by |
|---|---|
| json assets are formatted | `tools/format_json.py --check` |
| sources are formatted | `clang-format` |
| naming, and includes that are used | `clang-tidy` |
| headers compile on their own | the `header_self_containment` target |

`checks` needs no build. clang-tidy reads the compile database CMake writes at
configure, and the header target compiles files of its own.

## 🧱 Project Structure

```bash
platformer/
├── assets/              # JSON configuration, textures, tile maps
├── include/             # Header files
│   └── game/            # Game-related classes (Player, TileMap, etc.)
├── src/                 # Source files
│   └── game/            # Implementations of game logic
├── tests/               # Catch2 test cases
├── external/            # Third-party libraries (e.g., GLFW, Glaze)
├── CMakeLists.txt       # Build configuration
├── README.md            # Project documentation
└── tools/               # scripts and toolin utilities
```

## 🧭 Decisions

**Loading and reloading.** Loading anything from `assets` throws when the data is wrong.
What that means is left to whoever asked for the load. At startup nothing catches, so a
bad level, script or game data stops the game with the error; the assets are expected to
be right and there is nothing to fall back to. A file watcher catches and logs instead,
and each reload builds the replacement before assigning it, so a failure leaves what you
had untouched.

**One library, two executables.** Everything but `main.cpp` is compiled once into
`platformer_lib`, which the game and the tests both link. Listing those sources in both
executables built them twice, linted them twice, and meant adding a new file in two
places.

**No C++20 modules.** Every dependency is a header library and clangd is still catching
up, so scanning for modules buys nothing and costs a compiler pass per file. It also
writes an argument into every compile command naming a file that only exists once you
have built, which leaves anything reading the database — clang-tidy, or your own scripts
— broken until then. Turn it back on with `-DCMAKE_CXX_SCAN_FOR_MODULES=ON` to
experiment.

## 🔭 Future Plans

- Fast, precise platforming
- Explorable interconnected world
- Engaging enemy AI

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
