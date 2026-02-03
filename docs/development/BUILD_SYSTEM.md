# Automated Build and Test System

## Overview

EVE OFFLINE now has a comprehensive automated build and test system that can be run after pulling from Git to verify everything works correctly.

## Quick Start

### Option 1: Python Script (Cross-platform)
```bash
# Full build and test
python build_and_test.py

# Quick tests only
python build_and_test.py --quick

# Install dependencies only
python build_and_test.py --install
```

### Option 2: Convenience Scripts

**Windows:**
```cmd
build.bat
```

**Linux/macOS:**
```bash
chmod +x build.sh
./build.sh
```

### Option 3: Make Commands
```bash
# Show all available commands
make help

# Common commands
make build          # Full build and test
make quick          # Quick tests
make install        # Install dependencies
make test           # Run all tests
make lint           # Code quality checks
```

## What It Does

The build system automatically:

1. ✅ **Checks Python version** (3.11+ required)
2. ✅ **Checks Git status** (branch, commits, changes)
3. ✅ **Installs dependencies** from requirements.txt
4. ✅ **Verifies critical imports** (engine, client, server)
5. ✅ **Checks 3D dependencies** (Panda3D)
6. ✅ **Checks GUI dependencies** (Pygame)
7. ✅ **Runs code quality checks** (pylint, flake8)
8. ✅ **Runs unit tests** (all test_*.py files)
9. ✅ **Runs integration tests** (automated_tests.py)
10. ✅ **Runs UI tests** (test_eve_ui_components.py)
11. ✅ **Generates summary** with pass/fail report

## Command Reference

### build_and_test.py

Full-featured Python script with many options:

```bash
# Full build and test (recommended after git pull)
python build_and_test.py

# Quick mode - skip long tests (faster)
python build_and_test.py --quick

# Install dependencies only
python build_and_test.py --install

# Run code quality checks only
python build_and_test.py --lint

# Run UI tests only
python build_and_test.py --ui

# Verbose output for debugging
python build_and_test.py --verbose

# Disable colored output
python build_and_test.py --no-color

# Combine options
python build_and_test.py --quick --verbose
```

### Makefile Commands

```bash
# Development
make install        # Install dependencies
make install-dev    # Install dev dependencies (pylint, flake8, etc.)
make build          # Full build and test
make quick          # Quick build
make clean          # Clean cache and artifacts

# Testing
make test           # Run all tests
make test-quick     # Run quick tests
make test-ui        # Run UI tests
make lint           # Code quality checks
make format         # Format code with black

# Running the game
make server         # Start server
make client         # Start text client
make gui            # Start 2D GUI client
make client-3d      # Start 3D client
make demo           # Run interactive demo
make gui-demo       # Run GUI demo

# Utilities
make check-deps     # Check installed dependencies
make venv           # Create virtual environment
make docs           # Show documentation
make git-pull       # Pull and rebuild
make pre-commit     # Run pre-commit checks
make all            # Clean, install, and build everything
```

## Example Workflow

### After Pulling from Git

```bash
# Option 1: Full check (recommended first time)
python build_and_test.py

# Option 2: Quick check (when in a hurry)
python build_and_test.py --quick

# Option 3: Use make
make git-pull  # Pulls and runs tests automatically
```

### Before Committing

```bash
# Check your changes
python build_and_test.py --quick

# Or use make
make pre-commit
```

### Clean Build

```bash
# Clean everything and rebuild
make clean
make install
make build
```

## Output Examples

### Successful Build

```
======================================================================
                   EVE OFFLINE - BUILD AND TEST                      
======================================================================

Platform: Linux
Python: 3.11.0
Directory: /home/user/EVEOFFLINE

▶ Checking Python version...
✓ Python 3.11.0

▶ Checking Git status...
  Branch: main
  Latest: abc1234 - Add EVE-styled UI
✓ Working directory clean

▶ Installing dependencies...
✓ Dependencies installed

▶ Verifying critical imports...
  ✓ Engine module
  ✓ Client module
  ✓ Server module
✓ All critical imports successful

▶ Checking 3D client dependencies...
✓ Panda3D available (version: 1.10.13)

▶ Checking GUI client dependencies...
✓ Pygame available (version: 2.5.0)

▶ Running code quality checks...
  Running pylint...
✓ Pylint checks passed
  Running flake8...
✓ Flake8 checks passed

▶ Running unit tests...
  Found 12 test file(s)
  Running test_engine.py...
  Running test_network.py...
  Running test_eve_ui_components.py...
✓ All 12 test(s) passed

▶ Running integration tests...
✓ Integration tests passed

▶ Running UI tests...
✓ UI tests passed

======================================================================
                         BUILD SUMMARY                               
======================================================================

Total time: 45.32s

Passed (10):
  ✓ Python Version
  ✓ Git Status
  ✓ Dependencies
  ✓ Critical Imports
  ✓ 3D Dependencies
  ✓ GUI Dependencies
  ✓ Code Quality
  ✓ Unit Tests
  ✓ Integration Tests
  ✓ UI Tests

✓ BUILD SUCCESSFUL
```

### Failed Build

```
======================================================================
                   EVE OFFLINE - BUILD AND TEST                      
======================================================================

...

▶ Running unit tests...
  Found 12 test file(s)
  Running test_engine.py...
  ⚠    Failed: test_combat_system.py
✗ 1 test(s) failed, 11 passed

...

======================================================================
                         BUILD SUMMARY                               
======================================================================

Total time: 42.18s

Passed (9):
  ✓ Python Version
  ✓ Git Status
  ...

Failed (1):
  ✗ Unit Tests

✗ BUILD FAILED
```

## Git Hooks (Optional)

Automatically run checks before commits and pushes:

### Setup

```bash
python setup_hooks.py
```

### What It Does

- **pre-commit hook**: Runs quick tests before each commit
- **pre-push hook**: Runs full tests before pushing to remote

### Skip Hooks

If you need to bypass the hooks:

```bash
git commit --no-verify
git push --no-verify
```

## Continuous Integration

The build system is designed to work with CI/CD:

### GitHub Actions Example

```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - uses: actions/setup-python@v2
        with:
          python-version: '3.11'
      - name: Build and Test
        run: python build_and_test.py
```

### GitLab CI Example

```yaml
test:
  image: python:3.11
  script:
    - python build_and_test.py
```

## Customization

### Skip Certain Tests

Edit `build_and_test.py` and modify the `steps` list in `full_build()`:

```python
# Skip code quality checks
steps = [
    ("Python Version", self.check_python_version),
    # ("Code Quality", self.run_linters),  # Commented out
    ("Unit Tests", lambda: self.run_unit_tests(quick=False)),
]
```

### Add Custom Tests

Add your test to the `BuildSystem` class:

```python
def my_custom_test(self) -> bool:
    """Run my custom test"""
    self.print_step("Running custom test")
    
    # Your test logic here
    success = True
    
    if success:
        self.print_success("Custom test passed")
    else:
        self.print_error("Custom test failed")
    
    return success
```

Then add it to the steps:

```python
steps.append(("Custom Test", self.my_custom_test))
```

## Troubleshooting

### "Python not found"

**Solution**: Install Python 3.11+ and ensure it's in your PATH

```bash
# Check Python installation
python --version
python3 --version

# On Linux/Mac, you might need python3
python3 build_and_test.py
```

### "pip not found"

**Solution**: Install pip

```bash
# Windows
python -m ensurepip

# Linux
sudo apt install python3-pip

# Mac
python3 -m ensurepip
```

### "Module not found"

**Solution**: Install dependencies

```bash
python build_and_test.py --install

# Or manually
pip install -r requirements.txt
```

### Tests fail on fresh clone

**Solution**: This is expected! The build system tells you what's wrong:

1. Check the error messages
2. Install missing dependencies
3. Fix any code issues
4. Run again

### Permission denied (Linux/Mac)

**Solution**: Make scripts executable

```bash
chmod +x build.sh
chmod +x setup_hooks.py
```

### Git hooks not working

**Solution**: Ensure you're in a Git repository and reinstall

```bash
# Check if in Git repo
git status

# Reinstall hooks
python setup_hooks.py
```

## Best Practices

### Daily Workflow

1. **Morning**: Pull and test
   ```bash
   git pull
   python build_and_test.py --quick
   ```

2. **Before commit**: Quick check
   ```bash
   python build_and_test.py --quick
   ```

3. **Before push**: Full check
   ```bash
   python build_and_test.py
   ```

### Working on Features

```bash
# Create branch
git checkout -b my-feature

# Make changes
# ... edit code ...

# Test frequently
python build_and_test.py --quick

# Full test before commit
python build_and_test.py
git commit -m "Add feature"
```

### Reviewing Pull Requests

```bash
# Checkout PR branch
git fetch origin pull/123/head:pr-123
git checkout pr-123

# Test it
python build_and_test.py

# If it passes, it's good!
```

## Performance

### Timing

- **Quick mode**: ~10-30 seconds
- **Full mode**: ~30-120 seconds (depending on hardware)
- **Install only**: ~5-60 seconds (depending on connection)

### Optimization

```bash
# Use quick mode for iterative development
python build_and_test.py --quick

# Use full mode before commits
python build_and_test.py

# Use specific checks when debugging
python build_and_test.py --lint
python build_and_test.py --ui
```

## IDE Integration

### VS Code

Add to `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build and Test",
            "type": "shell",
            "command": "python",
            "args": ["build_and_test.py"],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "Quick Test",
            "type": "shell",
            "command": "python",
            "args": ["build_and_test.py", "--quick"]
        }
    ]
}
```

### PyCharm

1. Run → Edit Configurations
2. Add new Python configuration
3. Script path: `build_and_test.py`
4. Parameters: (leave empty for full, or add `--quick`)

## Summary

The automated build and test system provides:

✅ **One-command verification** after pulling from Git  
✅ **Comprehensive checks** for all components  
✅ **Clear pass/fail indicators** with colored output  
✅ **Multiple usage options** (Python, Make, scripts)  
✅ **Git hooks** for pre-commit/pre-push automation  
✅ **CI/CD ready** for automated pipelines  
✅ **Customizable** for your needs  

**Recommended Usage:**
```bash
# After git pull
python build_and_test.py --quick

# Before important commits
python build_and_test.py

# That's it!
```
