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

Terminal commands below assume a Unix shell. On Windows use Git Bash.

## 🛠 Setup

1. Install the prerequisites.

    For Windows, Visual Studio 2022 with **CMake** and **Git for Windows** added
    through *Tools and Features → Individual Components*, and clang-format, which
    Visual Studio does not ship at the version this project pins:

    ```bash
    pip install clang-format==$(cat .llvm-version).*
    ```

    For macOS and Linux, **CMake**, **Ninja**, **git-lfs**, and **LLVM** with its
    tools, which install separately from the compiler:

    ```bash
    brew install llvm clang-format                       # macOS
    sudo apt-get install clang clang-format clang-tidy   # Linux
    ```

    [.llvm-version](.llvm-version) is the version CI builds and checks with.

    On macOS and Linux, `PATH` often finds a different clangd first, so check that
    `clangd --version` matches your compiler. Name it in your user settings if it
    does not:

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

## 🧱 Project Structure

`include` and `src` mirror each other, one folder per subject.

```bash
platformer/
├── assets/          json configuration, textures, tile maps
├── include/         headers, by subject
│   ├── actor/       movement abilities, behaviors, animation state
│   ├── tile_map/    the grid, its tiles, and what they do on contact
│   ├── navigation/  the graph actors path over, and how it is built
│   ├── player/      npc/            the two kinds of actor
│   ├── physics/     rendering/      animations/     cameras/
│   ├── input/       scripting/      reloading/      serialization/
│   └── game/        the game itself: Game, Level, and the data they load
├── src/             implementations, same folders
├── tests/           Catch2 cases, same folders
├── external/        third party libraries
├── tools/           formatting and hook scripts
└── CMakeLists.txt
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

## 🪝 Pre Commit Hook

Installed by step 4 above, and run on every commit.

- Formats staged json under `assets`, and staged `.cpp` and `.hpp` under `src`,
  `include` and `tests`, then stages what it changed.
- Leaves a file alone and says so if it has unstaged edits, since staging the fix would
  sweep the rest of your work into the commit.
- Formats nothing and says so if it cannot find clang-format at the version in
  [.llvm-version](.llvm-version), because formatting to another version is what CI would
  then reject.
- Never runs clang-tidy. Its answer is sometimes wrong: dropping a `glz::meta` include
  it calls unused compiles fine, and surfaces later as two translation units disagreeing
  about how a type serializes. Checks whose fix can be wrong are left to CI, which
  reports rather than applies them.

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

In VS Code the C/C++ extension's IntelliSense is off because it parses with a front end
of its own and disagrees with the compiler, and no `launch.json` is needed because CMake
Tools runs the target from its build directory, which is where the game looks for its
assets. Visual Studio reads `CMakeLists.txt` directly and needs none of it.

## 🧭 Decisions

**Loading throws, and the caller decides what that means.**

- At startup nothing catches, so bad data stops the game with the error.
- A file watcher catches and logs instead.
- Each reload builds the replacement before assigning it, so a failure leaves what you
  had.

**No C++20 modules.**

- The tooling is not ready. clangd, which this repo leans on for code intelligence,
  handles them poorly.
- Every dependency is a header library, so modules would sit beside includes rather than
  replace them.
- Scanning for them writes an argument into every compile command naming a file that
  exists only after a build, which breaks anything reading the compile database.

## 🔭 Future Plans

- Fast, precise platforming
- Explorable interconnected world
- Engaging enemy AI

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
