# Platformer

A simple 2D platformer built in modern C++ using OpenGL. This project is designed as a learning and experimentation sandbox to explore:

- **Test-Driven Development (TDD)** in game development
- **Physics and platformer mechanics**
- **Data-driven design** (e.g., loading player and tile data from JSON)
- **Defensive programming practices**

## 🚀 Motivation

As a game developer and educator, I'm building this platformer to:

- Reinforce principles of **TDD** and **clean architecture**
- Create a reference project for **teaching and mentoring**
- Experiment with **modern C++**

## 🛠 Setup

### macOS (VS Code, Homebrew, CMake)
WIP

### Windows (Visual Studio, vcpkg, Chocolatey)
WIP

## 🎨 Style Guide

### Naming Conventions

| Element         | Style         | Example              |
|------------------|---------------|-----------------------|
| Variable         | `camelCase`   | `tileIndex`          |
| Function         | `camelCase`   | `moveLeft()`         |
| Class/Struct     | `PascalCase`  | `Player`, `TileMap`  |
| Enum             | `PascalCase`  | `TileKind::Solid`    |
| Constant         | `kCamelCase`  | `kGravity`           |

### Code Formatting

Use `clang-format` to automatically format your code:

```bash
brew install clang-format         # macOS
choco install llvm                # Windows (via Chocolatey)
clang-format -i src/**/*.cpp include/**/*.hpp
```

You can also install the [Clang-Format extension](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format) for VS Code to format on save.


## 🧭 Principles

This project follows these key development principles:

- **Test-Driven Development (TDD)**: Write tests before implementation to guide design and catch regressions early.
- **Defensive Programming**: Use assertions and checks to prevent invalid states and improve robustness.
- **Separation of Concerns**: Each class or module should have a single, well-defined responsibility.
- **Data-Driven Design**: Game elements like tiles and player configuration are loaded from external data (e.g., JSON files) for flexibility and ease of iteration.
- **Continuous Refactoring**: Improve readability and maintainability without changing behavior, guided by tests.

## 🧱 Project Structure

```
platformer/
├── assets/              # JSON configuration, textures, tile maps
├── include/             # Header files
│   └── game/            # Game-related classes (Player, TileMap, etc.)
├── src/                 # Source files
│   └── game/            # Implementations of game logic
├── tests/               # Catch2 test cases
├── external/            # Third-party libraries (e.g., GLFW, Glaze)
├── CMakeLists.txt       # Build configuration
└── README.md            # Project documentation
```

### Assets and Textures

This repository does not currently use Git LFS. If you're looking for the textures required to run the platformer, you can download them from the following link:

[Download Textures](https://drive.google.com/file/d/1JdVLy9ghLwf-Uz4zopB9EdoCs_skeKEO/view?usp=sharing)

## 🔭 Future Plans

- Add enemy AI with basic pathfinding
- Implement level transitions and checkpoints
- Support controller input
- Animate background and parallax scrolling
- Expand tile editor tools for easier level creation
- Port to WebAssembly for browser play

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.