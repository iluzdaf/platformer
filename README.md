# 🎮 Platformer

A simple 2D platformer built with OpenGL. This project is designed as a learning and experimentation sandbox to explore:

- **Modern C++**
- **Test-Driven Development (TDD)**
- **Data-driven design**
- **Platformer mechanics**

![alt text](https://media2.giphy.com/media/v1.Y2lkPTc5MGI3NjExYnltbXlwdmw0eWM5OGo4ZzRjd3d3NXQzMXdxd3hhaGN5dzl4NHdweiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/AYZJkrDUkgunRRcqNP/giphy.gif)

## 🖥 Supported Setups

Targets **C++23**. Developed on macOS and Linux. It [builds on
Windows](#-building-on-windows) too, which CI checks, but the tooling is LLVM only.

| | macOS | Linux |
|---|---|---|
| Compiler | LLVM clang | LLVM clang with libc++ |
| Editor | VS Code | VS Code |
| Build | the editor's CMake support | the editor's CMake support |
| Debug | CMake Tools' debug button | CMake Tools' debug button |
| Code intelligence | clangd | clangd |

## 🛠 Setup

1. Install **CMake**, **Ninja**, **git-lfs**, and **LLVM** with its tools, which
    install separately from the compiler:

    ```bash
    brew install llvm clang-format                       # macOS
    sudo apt-get install clang clang-format clang-tidy   # Linux
    ```

    [.llvm-version](.llvm-version) is the version CI builds and checks with, and
    the one the pre commit hook looks for.

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

## 🪟 Building On Windows

The game builds and runs on Windows, and CI checks that on every pull request. The
checks the project enforces are LLVM tools, which Windows does not compile with, so this
is for building and playing rather than contributing.

Install Visual Studio 2022 with **CMake** and **Git for Windows** from *Tools and
Features → Individual Components*, clone with submodules as above, and open the
repository folder. Visual Studio reads `CMakeLists.txt` directly, so pick `platformer`
and use the Run button.

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

## 🎨 Style Guide

Formatting is defined by [.clang-format](.clang-format) and naming by
[.clang-tidy](.clang-tidy). Two conventions those cannot express:

- File names are `snake_case`, as in `tile_map.hpp`.
- `glz::meta::value` must keep that name because Glaze requires it, so it carries a
  `NOLINTNEXTLINE`.

### Json Assets

Levels and `game_data.json` share one format, written both by `TileMap::save` and by
`tools/format_json.py`:

```json
"indices":[
    [14, 0, 0, 0, 0,14],
    [14,34,34,34,34,14]
]
```

No formatter has been found that reproduces this, so json formatting is turned off in
the workspace settings.

## 🤖 What CI Does

Every pull request runs two jobs at once. All four results must be green to merge.

| job | runs on | description |
|---|---|---|
| `build-and-test` | Windows, macOS, Linux | configures, builds under `-Werror`, runs the test suite |
| `checks` | Linux | the four checks below |

| check | run by |
|---|---|
| json assets are formatted | `tools/format_json.py --check` |
| sources are formatted | `clang-format` |
| naming, and includes that are used | `clang-tidy` |
| headers compile on their own | the `header_self_containment` target |

## 🧰 What The Repository Configures

Every platform builds the same way, from `CMakeLists.txt`.

| | |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | the `platformer_lib`, `platformer` and `tests` targets, and the compile database, linked to the repository root from whichever of `build/Debug` or `build/Release` you configured last |
| [.vscode/extensions.json](.vscode/extensions.json) | CMake Tools to build, clangd for code intelligence, C/C++ for the debugger |
| [.vscode/settings.json](.vscode/settings.json) | the generator, the build directory, and clangd in place of the C/C++ extension's IntelliSense |
| [.clang-format](.clang-format) | formatting, applied on save |
| [.clang-tidy](.clang-tidy) | naming and includes, applied on save |
| [.llvm-version](.llvm-version) | the LLVM the tools must come from |
| [tools/hooks/pre-commit](tools/hooks/pre-commit) | formats staged json and sources |

The C/C++ extension's IntelliSense is off because it parses with a front end of its own
and disagrees with the compiler, and no `launch.json` is needed because CMake Tools runs
the target from its build directory, which is where the game looks for its assets.

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

**The hook only fixes what is safe to fix.** It formats staged json and sources,
because a formatter's answer is never wrong. It does not run clang-tidy, whose answer
sometimes is: dropping an include it calls unused takes out `glz::meta`
specializations, which are reached by instantiation rather than by name, and nothing
fails until two translation units disagree about how a type serializes. Checks whose fix
can be wrong are left to CI, where they are read rather than applied. A file with
unstaged edits is reported rather than formatted, since staging the fix would sweep the
rest of your work into the commit.

**Developed on two platforms, built on three.** Every check the project enforces —
formatting, naming, includes — is an LLVM tool pinned to one version. Windows compiles
with MSVC, so a contributor there would install a second toolchain purely to run tools
that never compile anything, and still have to match a clang-format version Visual
Studio does not ship. CI keeps building and testing Windows, because MSVC reads the same
code differently and finds what clang does not.

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
