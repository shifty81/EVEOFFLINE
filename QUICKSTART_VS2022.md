# Quick Start - Visual Studio 2022

This is a quick reference for building EVE OFFLINE in Visual Studio 2022. For complete instructions, see [VS2022_SETUP_GUIDE.md](VS2022_SETUP_GUIDE.md).

## 📋 Prerequisites Checklist

- [ ] Visual Studio 2022 installed with "Desktop development with C++" workload
- [ ] Git installed
- [ ] CMake 3.15+ (usually included with VS2022)
- [ ] vcpkg installed (recommended for dependencies)

## ⚡ Quick Commands

### Initial Setup (One Time)

```cmd
# 1. Clone repository
git clone https://github.com/shifty81/EVEOFFLINE.git
cd EVEOFFLINE

# 2. Install dependencies via vcpkg
cd C:\vcpkg
.\vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows
cd C:\path\to\EVEOFFLINE

# 3. Generate Visual Studio solution
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
| "Cannot find glfw3.h" | Install dependencies: `vcpkg install glfw3:x64-windows` |
| "CMake not found" | Add to PATH: `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin` |
| "Build succeeds but crashes" | Check Working Directory in project properties |
| "OpenAL not found" | Optional, ignore or install: `vcpkg install openal-soft:x64-windows` |
| "\Microsoft was unexpected at this time" | Fixed in latest version, run `git pull` |

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
