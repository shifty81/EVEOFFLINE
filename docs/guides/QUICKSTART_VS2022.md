# Quick Start - Visual Studio 2022 / 2019

This is a quick reference for building EVE OFFLINE in Visual Studio 2022 or 2019. For complete instructions, see [VS2022_SETUP_GUIDE.md](VS2022_SETUP_GUIDE.md).

## ⚠️ Important: Install Dependencies FIRST

Before running any build scripts, you must install the required dependencies using vcpkg. The build scripts will detect vcpkg automatically and use it.

## 📋 Prerequisites Checklist

- [ ] Visual Studio 2019 or 2022 installed with "Desktop development with C++" workload
- [ ] Git installed
- [ ] CMake 3.15+ (usually included with Visual Studio)
- [ ] vcpkg installed with dependencies (see below)

## 🔨 Which Build Script to Use?

| Script | What It Does | When to Use |
|--------|-------------|-------------|
| **`build_vs.bat`** | Builds the **C++ client only** | **Use this one** for most development |
| `generate_solution.bat` | Generates solution for **both client + server** | Only if you need to work on the C++ server too |

**Recommended**: Use `build_vs.bat` — it builds and compiles the client in one step.

## ⚡ Quick Commands

### Initial Setup (One Time)

```cmd
# 1. Clone repository
git clone https://github.com/shifty81/EVEOFFLINE.git
cd EVEOFFLINE

# 2. Install vcpkg (if not already installed)
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 3. Install required dependencies
.\vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows imgui[glfw-binding,opengl3-binding]:x64-windows

# 4. (Optional) Install audio support
.\vcpkg install openal-soft:x64-windows

# 5. Return to repository and generate Visual Studio solution
cd C:\path\to\EVEOFFLINE
build_vs.bat
```

### Daily Development

```cmd
# Build latest changes
build_vs.bat

# Or open in Visual Studio to edit and build
start cpp_client\build_vs\EVEOfflineClient.sln
```

## 🎯 Build Options

| Command | Description |
|---------|-------------|
| `build_vs.bat` | Build C++ client in Release mode |
| `build_vs.bat --debug` | Build in Debug mode |
| `build_vs.bat --clean` | Clean rebuild from scratch |
| `build_vs.bat --open` | Open Visual Studio after build |
| `generate_solution.bat` | Generate unified client+server solution |

## 📂 Important Locations

| What | Where |
|------|-------|
| **Solution File** | `cpp_client\build_vs\EVEOfflineClient.sln` |
| **Executable** | `cpp_client\build_vs\bin\Release\eve_client.exe` |
| **Source Code** | `cpp_client\src\` and `cpp_client\include\` |
| **Shaders** | `cpp_client\shaders\` |
| **Assets** | `cpp_client\assets\` |

## 🔧 In Visual Studio

1. **Set Startup Project**: Right-click `eve_client` → "Set as Startup Project"
2. **Build**: Press `F7` or Build → Build Solution
3. **Run**: Press `F5` (with debugging) or `Ctrl+F5` (without)
4. **Build Configuration**: Select `Debug` or `Release` from toolbar dropdown

## 🐛 Common Issues & Fixes

| Issue | Fix |
|-------|-----|
| **"Does not match the generator used previously"** | **Fixed in latest version.** Run `build_vs.bat --clean` to clear old cache, or delete `cpp_client\build_vs\` folder |
| **"Could NOT find GLEW"** | **Install dependencies FIRST**: See Initial Setup above |
| "ImGui not found" | Install ImGui: `vcpkg install imgui[glfw-binding,opengl3-binding]:x64-windows` |
| "Cannot find glfw3.h" | Install dependencies: `vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows imgui[glfw-binding,opengl3-binding]:x64-windows` |
| "CMake not found" | Add to PATH or use the one bundled with Visual Studio |
| "Build succeeds but crashes" | Check Working Directory in project properties |
| "OpenAL not found" | Optional, ignore or install: `vcpkg install openal-soft:x64-windows` |

**For more detailed troubleshooting**: See [TROUBLESHOOTING_VS2022.md](TROUBLESHOOTING_VS2022.md)

## 🚀 Next Steps

After building successfully:

1. ✅ Run the client: Press `F5` in Visual Studio
2. ✅ Try the Python demos: `python launcher.py`
3. ✅ Start the server: `python server/server.py`
4. ✅ Read the full guide: [VS2022_SETUP_GUIDE.md](VS2022_SETUP_GUIDE.md)

## 💡 Pro Tips

- Use `Release` build for testing performance (Debug is slow)
- Use `RelWithDebInfo` for debugging with good performance
- Run tests by right-clicking test projects and selecting "Set as Startup Project"
- Copy vcpkg DLLs to exe directory if using dynamic linking
- Use static linking (`x64-windows-static`) for standalone executables

## 📚 Full Documentation

For complete details, troubleshooting, and advanced topics:
- **[VS2022_SETUP_GUIDE.md](VS2022_SETUP_GUIDE.md)** - Complete setup guide
- **[docs/development/VISUAL_STUDIO_BUILD.md](docs/development/VISUAL_STUDIO_BUILD.md)** - Advanced build options
- **[cpp_client/README.md](cpp_client/README.md)** - C++ client documentation
- **[README.md](README.md)** - Main project documentation
