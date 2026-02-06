# Quick Reference Card

## After Pulling from Git

```bash
# Test everything works
python build_and_test.py --quick

# Or use the convenience script
./build.sh --quick          # Linux/Mac
build.bat --quick           # Windows

# Or use make
make quick
```

## Running the Game

```bash
# 3D Client (with new EVE UI!)
python client_3d.py "YourName"

# 2D GUI Client
python client/gui_client.py "YourName"

# Server (in separate terminal)
python server/server.py
```

## Development Commands

```bash
make help           # Show all commands
make build          # Full build and test
make quick          # Quick tests
make install        # Install dependencies
make test           # Run all tests
make lint           # Code quality
make clean          # Clean artifacts
make client-3d      # Start 3D client
make server         # Start server
```

## Git Hooks (One-time Setup)

```bash
# Install automatic checks
python setup_hooks.py

# Now git commit and git push will test automatically!
```

## Testing Specific Parts

```bash
# UI tests only
python build_and_test.py --ui

# Code quality only
python build_and_test.py --lint

# Install dependencies only
python build_and_test.py --install

# Full build with verbose output
python build_and_test.py --verbose
```

## Documentation

- **EVE UI**: `docs/features/EVE_UI_ENHANCEMENTS.md`
- **Build System**: `docs/development/BUILD_SYSTEM.md`
- **C++ Design**: `docs/design/CPP_CLIENT_ARCHITECTURE.md`
- **AI Design**: `docs/design/AI_COMPANION_SYSTEM.md`
- **Session Summary**: `SESSION_COMPLETE.md`

## What's New This Session

1. ✅ **EVE-styled UI** - Looks like real EVE Online!
2. ✅ **Build System** - One-command testing
3. 📋 **C++ Migration Plan** - 15-25 week roadmap
4. 📋 **AI Companions Design** - Automated helpers

## Recommended Workflow

```bash
# 1. Pull latest changes
git pull

# 2. Test everything
python build_and_test.py --quick

# 3. Make your changes
# ... edit code ...

# 4. Test again
python build_and_test.py --quick

# 5. Commit (hooks run automatically)
git commit -m "Your changes"

# 6. Push (hooks run automatically)
git push
```

## Need Help?

```bash
# Show help
python build_and_test.py --help
make help

# Check dependencies
make check-deps

# View documentation
ls docs/
```

## Quick Fixes

**Python not found?**
```bash
python3 build_and_test.py --quick
```

**Dependencies missing?**
```bash
python build_and_test.py --install
```

**Tests failing?**
Read the error messages - they tell you what's wrong!

## That's It!

After pulling from Git, just run:
```bash
python build_and_test.py --quick
```

Everything else is documented! 📚
