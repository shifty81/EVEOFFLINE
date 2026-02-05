# Continuous Integration and Deployment (CI/CD)

## Overview

EVE OFFLINE uses GitHub Actions for automated building and testing of the C++ client across multiple platforms.

## Workflows

### C++ Client Build and Test (`cpp-client-ci.yml`)

**Triggers:**
- Push to `main`, `develop`, or `copilot/**` branches
- Pull requests to `main` or `develop`
- Manual workflow dispatch

**Platforms:**
- Ubuntu Linux (latest)
- Windows (latest with MSVC)
- macOS (latest)

**Build Steps:**
1. Checkout repository with submodules
2. Install platform-specific dependencies
3. Configure CMake with system libraries
4. Build in Release mode
5. Run non-graphical tests
6. Upload build artifacts

**Artifacts:**
- Linux: `eve-client-linux` (executable + assets)
- Windows: `eve-client-windows` (executable + assets)
- macOS: `eve-client-macos` (executable + assets)
- Retention: 7 days

## Build Matrix

| Platform | Compiler | OpenGL | Audio | Status |
|----------|----------|--------|-------|--------|
| Ubuntu 22.04 | GCC 11+ | Mesa | OpenAL | ✅ |
| Windows Server 2022 | MSVC 2022 | Native | OpenAL | ✅ |
| macOS 12+ | Clang | Native | OpenAL | ✅ |

## Dependencies

### Linux (Ubuntu/Debian)
```bash
build-essential cmake libgl1-mesa-dev libglew-dev
libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev
libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
libxkbcommon-dev wayland-protocols libwayland-dev
```

### macOS (Homebrew)
```bash
cmake glfw glm glew nlohmann-json openal-soft
```

### Windows (vcpkg)
```bash
glfw3:x64-windows glm:x64-windows glew:x64-windows
nlohmann-json:x64-windows openal-soft:x64-windows
```

## Test Execution

### Automated Tests

Tests run automatically during CI:

1. **test_network** - TCP client and protocol
   - Tests connection handling
   - Tests JSON message parsing
   - Expected to fail gracefully without server

2. **test_entity_sync** - Entity synchronization
   - Tests entity lifecycle
   - Tests interpolation
   - Tests state management

### Manual Tests

Some tests require a display/GPU and must be run manually:

- `test_asteroid_field` - Asteroid rendering
- `test_lighting` - Dynamic lighting
- `test_shadow_mapping` - Shadow system
- `test_deferred_rendering` - Deferred pipeline
- `test_ui_system` - UI panels
- `test_post_processing` - Post-effects
- `test_audio_system` - Audio (if OpenAL available)

## Build Artifacts

After successful builds, artifacts are available for download:

1. Go to Actions tab in GitHub
2. Select the workflow run
3. Download artifacts at the bottom

**Contents:**
- `eve_client` (or `eve_client.exe`)
- `shaders/` directory
- `assets/` directory

**Usage:**
```bash
# Linux/macOS
./eve_client "YourName"

# Windows
eve_client.exe "YourName"
```

## Build Status Badges

Add to README.md:

```markdown
[![C++ Client Build](https://github.com/shifty81/EVEOFFLINE/actions/workflows/cpp-client-ci.yml/badge.svg)](https://github.com/shifty81/EVEOFFLINE/actions/workflows/cpp-client-ci.yml)
```

## Local CI Testing

### Using act (GitHub Actions locally)

Install [act](https://github.com/nektos/act):

```bash
# Linux/macOS
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# macOS with Homebrew
brew install act

# Windows with Chocolatey
choco install act-cli
```

Run workflows locally:

```bash
# Run the C++ client workflow
act -W .github/workflows/cpp-client-ci.yml

# Run only Linux build
act -j build-linux
```

## Troubleshooting

### Build Fails on One Platform

**Check:**
1. Platform-specific dependencies in workflow
2. CMake configuration for that platform
3. Compiler version compatibility

**Solutions:**
- Update dependency versions
- Add platform-specific CMake flags
- Check CMakeLists.txt for platform issues

### Tests Fail

**Non-graphical tests should pass:**
- test_network (may warn about no server)
- test_entity_sync

**If they fail:**
1. Check for C++ compilation errors
2. Verify test logic
3. Check for platform-specific issues

**Graphical tests are not run in CI** because they require display/GPU.

### Artifacts Not Uploaded

**Check:**
1. Build succeeded
2. Paths in upload-artifact action are correct
3. Files exist in expected locations

**Fix:**
```yaml
- name: List build artifacts
  run: find build/bin -type f
  
- name: Upload artifacts
  uses: actions/upload-artifact@v4
  with:
    name: eve-client-linux
    path: cpp_client/build/bin/
```

### Out of Storage

GitHub Actions has storage limits:

**Solutions:**
1. Reduce artifact retention days
2. Use `if: github.event_name == 'release'` for artifacts
3. Delete old workflow runs

## Advanced Configuration

### Matrix Builds

To test multiple configurations:

```yaml
strategy:
  matrix:
    os: [ubuntu-latest, windows-latest, macos-latest]
    build_type: [Debug, Release]
```

### Caching Dependencies

Speed up builds with caching:

```yaml
- name: Cache vcpkg
  uses: actions/cache@v3
  with:
    path: C:\vcpkg\installed
    key: ${{ runner.os }}-vcpkg-${{ hashFiles('**/CMakeLists.txt') }}
```

### Conditional Steps

Run steps only on specific platforms:

```yaml
- name: Install Linux dependencies
  if: runner.os == 'Linux'
  run: sudo apt-get install ...
```

### Secrets and Environment Variables

For future integration with servers/services:

```yaml
env:
  SERVER_HOST: ${{ secrets.SERVER_HOST }}
  API_KEY: ${{ secrets.API_KEY }}
```

## Release Automation

### Automatic Releases

To create releases automatically:

```yaml
on:
  push:
    tags:
      - 'v*'

jobs:
  release:
    # ... build steps ...
    
    - name: Create Release
      uses: softprops/action-gh-release@v1
      with:
        files: |
          eve-client-linux.tar.gz
          eve-client-windows.zip
          eve-client-macos.tar.gz
```

## Monitoring

### Build Times

Typical build times on GitHub Actions:

| Platform | Configure | Build | Total |
|----------|-----------|-------|-------|
| Linux | ~1 min | ~3 min | ~4 min |
| Windows | ~2 min | ~5 min | ~7 min |
| macOS | ~2 min | ~4 min | ~6 min |

### Optimization Tips

1. **Use system libraries**: Faster than compiling from source
2. **Parallel builds**: Use all available cores
3. **Cache dependencies**: Reuse between runs
4. **Disable unused tests**: Reduce build time

## Security

### CodeQL Analysis

Add security scanning:

```yaml
- name: Initialize CodeQL
  uses: github/codeql-action/init@v2
  with:
    languages: cpp

- name: Perform CodeQL Analysis
  uses: github/codeql-action/analyze@v2
```

### Dependency Scanning

Use Dependabot to keep dependencies updated:

```yaml
# .github/dependabot.yml
version: 2
updates:
  - package-ecosystem: "github-actions"
    directory: "/"
    schedule:
      interval: "weekly"
```

## Contributing

When adding new features:

1. Ensure builds pass on all platforms
2. Add tests if applicable
3. Update workflow if new dependencies needed
4. Test locally before pushing

## See Also

- [Build Automation](../../cpp_client/README.md#building)
- [Visual Studio Build](VISUAL_STUDIO_BUILD.md)
- [CMake Configuration](../../cpp_client/CMakeLists.txt)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
