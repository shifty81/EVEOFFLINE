# EVE OFFLINE C++ Client - Build Automation

This directory contains automated build tools for the C++ OpenGL client.

## Quick Start

### Automated Build (Recommended)

```bash
# Build and test the C++ client
python3 build_cpp_client.py
```

This works on Linux, macOS, and Windows!

### Windows Visual Studio

```batch
# Generate Visual Studio solution and build
build_vs.bat

# Or open in Visual Studio after generation
build_vs.bat --open
```

## What's Included

### Build Scripts

1. **`build_cpp_client.py`** - Universal cross-platform build script
   - Checks dependencies
   - Configures CMake
   - Builds the project
   - Runs tests
   - Works on Linux, macOS, Windows

2. **`build_vs.bat`** - Windows Visual Studio build script
   - Generates Visual Studio solution
   - Builds using MSBuild
   - Optionally opens Visual Studio

### CI/CD

3. **`.github/workflows/cpp-client-ci.yml`** - GitHub Actions workflow
   - Automated builds on push/PR
   - Tests on Ubuntu, Windows, macOS
   - Uploads build artifacts

### Documentation

4. **`docs/development/CI_CD.md`** - CI/CD documentation
5. **`docs/development/VISUAL_STUDIO_BUILD.md`** - Visual Studio guide
6. **`docs/development/CPP_CLIENT_QUICKSTART.md`** - Quick start guide

## Usage Examples

### Basic Build

```bash
python3 build_cpp_client.py
```

### Debug Build

```bash
python3 build_cpp_client.py --debug
```

### Clean Rebuild

```bash
python3 build_cpp_client.py --clean
```

### Skip Tests

```bash
python3 build_cpp_client.py --skip-tests
```

### Verbose Output

```bash
python3 build_cpp_client.py --verbose
```

### Windows with vcpkg

```bash
python3 build_cpp_client.py --vcpkg C:\vcpkg
```

## Dependencies

### Linux (Ubuntu/Debian)

```bash
sudo apt-get install \
    build-essential cmake \
    libgl1-mesa-dev libglew-dev \
    libglfw3-dev libglm-dev \
    nlohmann-json3-dev libopenal-dev
```

### macOS

```bash
brew install cmake glfw glm glew nlohmann-json openal-soft
```

### Windows

Install Visual Studio 2019+ with C++ tools, then:

```batch
vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows openal-soft:x64-windows
```

## CI/CD

Every push to the repository automatically:
- ✅ Builds on Linux (Ubuntu)
- ✅ Builds on Windows (MSVC)
- ✅ Builds on macOS
- ✅ Runs tests
- ✅ Uploads artifacts

View build status in the Actions tab on GitHub.

## Troubleshooting

### CMake not found

Install CMake from https://cmake.org/download/

### Compiler not found

- **Linux**: `sudo apt-get install build-essential`
- **macOS**: `xcode-select --install`
- **Windows**: Install Visual Studio with C++ tools

### Library not found

Install dependencies using your platform's package manager (see Dependencies section).

### Build script fails

Try with verbose output:
```bash
python3 build_cpp_client.py --verbose
```

## Manual Build

If you prefer to build manually:

```bash
cd cpp_client
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)  # or: cmake --build . --config Release
```

## Running the Client

After building:

```bash
cd cpp_client/build/bin
./eve_client "YourName"
```

## Documentation

- [Quick Start Guide](docs/development/CPP_CLIENT_QUICKSTART.md)
- [Visual Studio Setup](docs/development/VISUAL_STUDIO_BUILD.md)
- [CI/CD Documentation](docs/development/CI_CD.md)
- [C++ Client README](cpp_client/README.md)

## Support

For issues or questions:
1. Check the documentation links above
2. Review [cpp_client/README.md](cpp_client/README.md)
3. Search existing GitHub Issues
4. Create a new issue with details

## Contributing

When contributing:
1. Ensure builds pass on all platforms
2. Run tests locally before pushing
3. Update documentation if needed
4. CI will validate your changes

---

**Happy Building!** 🚀
