# Visual Studio 2022 Setup Guide for EVE OFFLINE

This guide will help you set up and build the EVE OFFLINE project in Visual Studio 2022 after cloning from Git.

## Quick Setup (5 Minutes)

### 1. Prerequisites

Before starting, ensure you have:

- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Download: https://visualstudio.microsoft.com/vs/
  - During installation, select: **"Desktop development with C++"**
  - Required components:
    - MSVC v143 or later (VS 2022 C++ compiler)
    - Windows 10 or 11 SDK
    - C++ CMake tools for Windows
    - C++ ATL for latest v143 build tools (optional but recommended)

- **Git** (to clone the repository)
  - Download: https://git-scm.com/download/win
  - Or use Git included with Visual Studio

- **CMake 3.15+** (usually included with Visual Studio)
  - If not included, download: https://cmake.org/download/
  - Add to PATH during installation

### 2. Clone the Repository

```cmd
git clone https://github.com/shifty81/EVEOFFLINE.git
cd EVEOFFLINE
```

### 3. Install Dependencies (vcpkg - Recommended)

The C++ client requires several libraries. The easiest way to install them is using vcpkg:

#### Option A: Quick Install with vcpkg

```cmd
# Clone vcpkg (if you don't have it already)
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Add vcpkg to your PATH (optional but convenient)
# Or note the location for later use

# Install required packages
.\vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows

# Optional: Install audio support
.\vcpkg install openal-soft:x64-windows
```

#### Option B: Static Linking (Single EXE, no DLLs)

If you want a standalone executable with no external DLL dependencies:

```cmd
.\vcpkg install glfw3:x64-windows-static glm:x64-windows-static glew:x64-windows-static nlohmann-json:x64-windows-static openal-soft:x64-windows-static
```

### 4. Build with Visual Studio 2022

#### Method 1: Automated Build Script (Easiest)

**Option A: Build just the C++ client**

From the repository root directory:

```cmd
# Navigate to repository root
cd C:\path\to\EVEOFFLINE

# Run the build script
build_vs.bat

# Or for a clean build:
build_vs.bat --clean

# Or to open Visual Studio after building:
build_vs.bat --open
```

This will:
1. Generate the Visual Studio 2022 solution for the C++ client
2. Build the project in Release mode
3. Show you where the executable is located

The solution file will be created at: `cpp_client\build_vs\EVEOfflineClient.sln`

**Option B: Build both C++ client and server (unified solution)**

From the repository root directory:

```cmd
# Generate a unified solution with both client and server
generate_solution.bat

# Or to open Visual Studio after generation:
generate_solution.bat --open
```

This creates a single solution at: `build_vs\EVEOffline.sln` that includes both the C++ client and server projects.

#### Method 2: Manual CMake Configuration

If you prefer manual control or the script doesn't work:

```cmd
cd cpp_client
mkdir build_vs
cd build_vs

# If you used vcpkg:
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DBUILD_TESTS=ON

# Or without vcpkg (if using bundled libraries):
cmake .. -G "Visual Studio 17 2022" -A x64 -DUSE_SYSTEM_LIBS=OFF -DBUILD_TESTS=ON
```

Then open the solution:
```cmd
start EVEOfflineClient.sln
```

### 5. Configure and Build in Visual Studio

Once the solution is open in Visual Studio 2022:

1. **Set Startup Project**
   - Right-click on `eve_client` project in Solution Explorer
   - Select **"Set as Startup Project"**

2. **Select Build Configuration**
   - In the toolbar, select `Release` or `Debug` from the dropdown
   - Use `Release` for better performance
   - Use `Debug` if you need to debug code

3. **Build the Solution**
   - Press **F7** or select **Build → Build Solution**
   - Wait for compilation to complete

4. **Run the Client**
   - Press **F5** to run with debugging
   - Or **Ctrl+F5** to run without debugging

### 6. Running Tests

The solution includes several test projects:

- `test_asteroid_field` - Asteroid field rendering test
- `test_lighting` - Dynamic lighting system test
- `test_shadow_mapping` - Shadow mapping test
- `test_deferred_rendering` - Deferred rendering pipeline test
- `test_ui_system` - UI system test
- `test_network` - Network communication test
- `test_entity_sync` - Entity synchronization test
- `test_post_processing` - Post-processing effects test
- `test_audio_system` - Audio system test (if OpenAL is installed)

To run a test:
1. Right-click the test project
2. Select **"Set as Startup Project"**
3. Press **F5** or **Ctrl+F5**

## Project Structure

After building, you'll have:

```
EVEOFFLINE/
├── cpp_client/
│   ├── build_vs/              # Build directory (created by CMake)
│   │   ├── EVEOfflineClient.sln  # Visual Studio solution file
│   │   └── bin/
│   │       ├── Release/
│   │       │   └── eve_client.exe  # Main executable (Release)
│   │       └── Debug/
│   │           └── eve_client.exe  # Main executable (Debug)
│   │
│   ├── include/               # Header files
│   ├── src/                   # Source files
│   ├── shaders/               # GLSL shaders
│   ├── assets/                # Game assets
│   └── CMakeLists.txt         # CMake configuration
│
├── cpp_server/                # C++ dedicated server
├── server/                    # Python server
├── engine/                    # Python game engine
├── data/                      # Game data (JSON files)
└── build_vs.bat              # VS2022 build script
```

## Troubleshooting

### "CMake not found"

**Solution:** Install CMake from https://cmake.org/download/ or ensure Visual Studio's CMake is in PATH:
```cmd
# Add to PATH (example):
set PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
```

### "Cannot find glfw3.h" or "Cannot find GL/glew.h"

**Solution:** Dependencies not found. Make sure you:
1. Installed packages via vcpkg
2. Specified the toolchain file in cmake command:
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
   ```

### "MSBuild version is too old" or "Platform Toolset v143 not installed"

**Solution:** Update Visual Studio or install the C++ build tools:
1. Open **Visual Studio Installer**
2. Click **Modify** on your VS2022 installation
3. Ensure **"Desktop development with C++"** is checked
4. In the right panel, verify **"MSVC v143"** is selected

### Build succeeds but executable crashes on startup

**Common causes:**

1. **Missing DLLs** (if using dynamic linking)
   - Copy DLLs from `C:\vcpkg\installed\x64-windows\bin\` to your exe directory
   - Or use static linking (see Option B in dependencies)

2. **Working directory not set correctly**
   - Right-click `eve_client` → **Properties** → **Debugging**
   - Set **Working Directory** to: `$(TargetDir)` or `$(ProjectDir)\..\bin\$(Configuration)`

3. **Shaders or assets not copied**
   - Verify `shaders/` and `assets/` folders exist in the same directory as the .exe
   - CMake should copy these automatically

### "OpenAL not found" warning

This is **normal** and **optional**. The client will build without audio support. To enable audio:
```cmd
vcpkg install openal-soft:x64-windows
```
Then re-run CMake configuration.

### Visual Studio can't find the solution file

Make sure you're opening the correct file:
- Location: `cpp_client\build_vs\EVEOfflineClient.sln`
- If it doesn't exist, run `build_vs.bat` first to generate it

## Python Components

The repository also includes Python components (game server, demos, etc.):

### Python Setup

1. **Install Python 3.11+**
   - Download: https://www.python.org/downloads/

2. **Install Python dependencies** (optional for enhanced features):
   ```cmd
   pip install -r requirements.txt
   ```

3. **Run Python server** (for multiplayer):
   ```cmd
   python server/server.py
   ```

4. **Try Python demos**:
   ```cmd
   python launcher.py      # Interactive launcher
   python gui_demo.py      # Visual 2D demo
   python interactive_demo.py  # Text-based gameplay
   ```

## Development Workflow

### Typical Development Cycle

1. **Make code changes** in Visual Studio
2. **Build** (F7)
3. **Run/Debug** (F5 or Ctrl+F5)
4. **Test specific features** using test projects
5. **Commit changes** using Git

### Using Git with Visual Studio

Visual Studio 2022 has built-in Git support:

- **View changes**: **Git Changes** window (Ctrl+0, G)
- **Commit**: Write message and click **Commit All**
- **Push/Pull**: Use buttons in Git Changes window
- **Create branch**: Branch dropdown in status bar

Or use Git command line:
```cmd
git status
git add .
git commit -m "Your commit message"
git push
```

## Building the C++ Server

The repository also includes a C++ dedicated server:

```cmd
cd cpp_server
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The server executable will be at: `cpp_server\build\bin\Release\eve_dedicated_server.exe`

## Additional Resources

- **Main README**: [README.md](README.md) - Project overview
- **C++ Client README**: [cpp_client/README.md](cpp_client/README.md) - Detailed client documentation
- **Visual Studio Build Guide**: [docs/development/VISUAL_STUDIO_BUILD.md](docs/development/VISUAL_STUDIO_BUILD.md) - Advanced VS build options
- **Dependencies Guide**: [cpp_client/DEPENDENCIES.md](cpp_client/DEPENDENCIES.md) - Detailed dependency information
- **Contributing**: [CONTRIBUTING.md](CONTRIBUTING.md) - How to contribute

## Quick Reference Commands

```cmd
# Clone repository
git clone https://github.com/shifty81/EVEOFFLINE.git
cd EVEOFFLINE

# Build C++ client only
build_vs.bat

# Build both client and server (unified solution)
generate_solution.bat

# Open solution in VS2022
start cpp_client\build_vs\EVEOfflineClient.sln
# or (if using unified solution)
start build_vs\EVEOffline.sln

# Run Python server
python server/server.py

# Run Python demos
python launcher.py
python gui_demo.py
```

## Getting Help

If you encounter issues:

1. Check the **Troubleshooting** section above
2. Review [cpp_client/SETUP.md](cpp_client/SETUP.md) for Linux/macOS setup
3. Check [docs/development/VISUAL_STUDIO_BUILD.md](docs/development/VISUAL_STUDIO_BUILD.md) for advanced topics
4. Search existing [GitHub Issues](https://github.com/shifty81/EVEOFFLINE/issues)
5. Create a new issue with:
   - Visual Studio version
   - CMake version
   - Build command or script used
   - Full error message
   - Steps to reproduce

## Success! What's Next?

Once you have successfully built and run the client:

1. **Explore the codebase** - Check out the architecture in `cpp_client/include/`
2. **Try the demos** - Run Python demos to see gameplay features
3. **Modify and experiment** - Make changes and rebuild
4. **Read the documentation** - Learn about systems and features
5. **Contribute** - See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines

Enjoy developing EVE OFFLINE! 🚀
