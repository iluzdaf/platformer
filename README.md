# 🎮 Platformer

A simple 2D platformer built in modern C++ using OpenGL. This project is designed as a learning and experimentation sandbox to explore:

- **Test-Driven Development (TDD)**
- **Data-driven design**
- **Platformer mechanics**

![alt text](https://media2.giphy.com/media/v1.Y2lkPTc5MGI3NjExYnltbXlwdmw0eWM5OGo4ZzRjd3d3NXQzMXdxd3hhaGN5dzl4NHdweiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/AYZJkrDUkgunRRcqNP/giphy.gif)

## 🛠 Setup

1. Install dependencies if needed:
    - CMake — for configuring builds
    - Ninja — for making faster builds
    - git-lfs — for downloading large assets
    - llvm — for compiling mordern c++

    For macOS and Linux, use your package manager of choice.

    For Windows, make sure CMake and Git for Windows are installed in Visual Studio via:

    ```markdown
    Tools and Features -> Individual Components
    ```

2. Clone the repository:

    For macOS and Linux,

    ```bash
    git clone https://github.com/iluzdaf/platformer.git
    git submodule update --init --recursive
    ```

    For Windows, use the Visual Studio UI or your favourite git client.

3. Build the project:

    For macOS and Linux,

    ```bash
    cmake -G Ninja -B build/debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="/opt/homebrew/opt/llvm/bin/clang" \
    -DCMAKE_CXX_COMPILER="/opt/homebrew/opt/llvm/bin/clang++"

    cmake --build build/debug --config Debug
    ```

    For Windows,
    - once you open the repository folder using Visual Studio, it will automatically detect the CMake settings and setup the project folder accordingly.
    - You can choose **Debug** or **Release** in the dropdown near the Run button.

4. Run tests:

    For macOS and Linux,

    ```bash
    ./tests
    ```

    For Windows, choose the project in the Visual Studio **Startup Project** selector near the run button and run it.

5. Run the game executable:

    For macOS and Linux,

    ```bash
    ./platformer 
    ```

    For Windows, choose the project in the Visual Studio **Startup Project** near the run button and run it.

## 🎨 Style Guide

### Naming Conventions

| Element         | Style         | Example              |
|------------------|---------------|-----------------------|
| Variable         | `camelCase`   | `tileIndex`          |
| Function         | `camelCase`   | `moveLeft()`         |
| Class/Struct     | `PascalCase`  | `Player`, `TileMap`  |
| Enum             | `PascalCase`  | `TileKind::Solid`    |
| Constant         | `CamelCase`  | `Gravity`           |
| File Name       | `snake_case`  | `tile_map.hpp`          |

### Code Formatting

Install and setup the ms-vscode.cpptools for VS Code to format on save.

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
