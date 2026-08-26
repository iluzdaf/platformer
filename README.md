# 🎮 Platformer

A simple 2D platformer built with OpenGL. This project is designed as a learning and experimentation sandbox to explore:

- **Modern C++**
- **Test-Driven Development (TDD)**
- **Data-driven design**
- **Platformer mechanics**

![alt text](https://media2.giphy.com/media/v1.Y2lkPTc5MGI3NjExYnltbXlwdmw0eWM5OGo4ZzRjd3d3NXQzMXdxd3hhaGN5dzl4NHdweiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/AYZJkrDUkgunRRcqNP/giphy.gif)

## 🖥 Supported Setups

Targets **C++23**. CI builds and tests all three platforms on every push.

| | Windows | macOS | Linux |
|---|---|---|---|
| Compiler | MSVC, Visual Studio 2022 17.8+ | LLVM clang 20+ | LLVM clang 20+ with libc++ |
| Editor | Visual Studio 2022 | VS Code | VS Code |
| Build | the editor's CMake support | the editor's CMake support | the editor's CMake support |
| Debug | Visual Studio's Run button | CMake Tools' debug button | CMake Tools' debug button |
| Code intelligence | Visual Studio's own | clangd | clangd |

Terminal commands below assume a Unix shell. On Windows use Git Bash, and `python`
where they say `python3`.

## 🧰 What The Repository Configures

Every platform builds the same way, from `CMakeLists.txt`, which defines the
`platformer` and `tests` targets. No editor project files.

Visual Studio needs nothing further: it reads `CMakeLists.txt` directly and brings its
own IntelliSense and debugger.

VS Code divides the work between three extensions. **CMake Tools** configures and
builds, **clangd** provides completion, navigation and diagnostics, and the **C/C++**
extension provides the debugger. clangd reads the compile database CMake writes, so it
sees the same flags the compiler does; the C/C++ extension parses with a front end of
its own and can disagree, so its IntelliSense is switched off.

- `.vscode/extensions.json` — the extensions to install.
- `.vscode/settings.json` — the generator, the build directory, and clangd in place of
  the C/C++ extension.
- `.clang-format`, `.clang-tidy` — formatting and naming, applied on save.

CMake writes the compile database and links it to the repository root, so clangd reads
whichever configuration you last configured.

One thing to check: clangd must come from the LLVM you build with — `PATH` often finds
a different one first, so check that `clangd --version` matches your compiler, and name
it in your user settings if it does not. Machine paths do not belong in the
repository.

```json
"clangd.path": "/opt/homebrew/opt/llvm/bin/clangd"
```

No `launch.json` is needed; CMake Tools runs and debugs the selected target from its
own build directory, which is where the game looks for assets. Format on save is
off for json only, since no formatter has been found that reproduces the format the
game writes.

## 🛠 Setup

1. Install the prerequisites.

    For Windows, Visual Studio 2022 with **CMake** and **Git for Windows** added
    through *Tools and Features → Individual Components*.

    For macOS and Linux, **CMake**, **Ninja**, **git-lfs** and **LLVM clang**.

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
[.clang-tidy](.clang-tidy). Both editors apply them on save; to sweep the whole tree:

```bash
clang-format -i $(find src include tests -name '*.cpp' -o -name '*.hpp')
clang-tidy -p build/Debug $(find src -name '*.cpp')
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

## 🔭 Future Plans

- Fast, precise platforming
- Explorable interconnected world
- Engaging enemy AI

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
