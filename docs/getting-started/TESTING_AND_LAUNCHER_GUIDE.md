# EVE OFFLINE - Testing & Launcher Guide

**Version**: 1.0  
**Date**: February 2, 2026

---

## Overview

This guide explains the new automated testing and launcher tools that make it easy to run, test, and debug EVE OFFLINE.

---

## Quick Links

- **Easy Launcher**: `python launcher.py`
- **Automated Tests**: `python automated_tests.py`
- **3D Client with Logging**: `python client_3d.py "YourName" --debug`

---

## 1. Easy Launcher (`launcher.py`)

### What It Does

The Easy Launcher provides a user-friendly menu interface to access all features of EVE OFFLINE without memorizing commands.

### How to Use

```bash
python launcher.py
```

### Features

1. **Testing & Demos**:
   - Run automated tests
   - Run 3D client standalone test
   - Run interactive gameplay demo
   - Run GUI demo (2D)

2. **Multiplayer**:
   - Start game server
   - Connect text client
   - Connect 2D GUI client
   - Connect 3D client

3. **Development**:
   - Check dependencies
   - View logs

### Example Session

```
===========================================================
           EVE OFFLINE - Easy Launcher
===========================================================

What would you like to do?

Testing & Demos:
  1. Run automated tests (verify everything works)
  2. Run 3D client standalone test (no server needed)
  3. Run interactive gameplay demo
  4. Run GUI demo (2D graphics)

Multiplayer:
  5. Start game server
  6. Connect text client to server
  7. Connect 2D GUI client to server
  8. Connect 3D client to server (NEW!)

Development:
  9. Check dependencies
  10. View logs

  0. Exit

Select option (0-10): 2

▶ Running 3D client standalone test...
Command: python test_3d_client.py
Press Ctrl+C to stop

[Test runs...]
```

---

## 2. Automated Testing (`automated_tests.py`)

### What It Does

Runs comprehensive tests to verify all components of EVE OFFLINE are working correctly.

### How to Use

**Interactive Mode** (recommended):
```bash
python automated_tests.py
```

**Quick Test Mode** (fast check):
```bash
python automated_tests.py --quick
```

**Help**:
```bash
python automated_tests.py --help
```

### What It Tests

1. **Dependencies**: Python version, pygame, panda3d
2. **Unit Tests**: All test files in `tests/` directory
3. **Engine**: Core game engine functionality
4. **Demos**: Demo scripts (demo.py, showcase_gameplay.py, etc.)
5. **3D Client**: 3D client components and imports
6. **Network**: Network protocol and messaging

### Example Output

```
===========================================================
Step 1: Checking Dependencies
===========================================================

▶ Checking Python version...
✓ Python 3.11.5 (>= 3.11 required)

▶ Checking required Python modules...
✓ pygame is installed
✓ panda3d is installed

===========================================================
Step 2: Running Unit Tests
===========================================================

▶ Running test suite with run_tests.py...
✓ All unit tests passed!

[... more tests ...]

===========================================================
Test Report
===========================================================
Total Tests: 6
Passed: 6
Failed: 0

  Dependencies                   PASS
  Unit Tests                     PASS
  Engine                         PASS
  Demos                          PASS
  3D Client                      PASS
  Network                        PASS

✓ All tests passed! ✓

Software is ready to use!

Next steps:
  1. Run server: python server/server.py
  2. Run client: python client/client.py "YourName"
  3. Try 3D client: python client_3d.py "YourName"
  4. Try standalone 3D: python test_3d_client.py
```

### Menu Options

When running interactively:

```
Options:
  1. Run all tests (recommended)
  2. Quick test (dependencies + unit tests only)
  3. Test specific component
  4. Exit
```

### Testing Individual Components

```
Components:
  1. Dependencies
  2. Unit Tests
  3. Engine
  4. Demos
  5. 3D Client
  6. Network
```

---

## 3. Error Logging System

### What It Does

Automatically logs all events, errors, and debug information to help troubleshoot issues.

### Log Files

Logs are saved to the `logs/` directory with timestamps:

```
logs/
├── eve_offline_3d_20260202_033800.log
├── eve_offline_3d_20260202_034512.log
└── eve_offline_3d_20260202_035223.log
```

### Log Levels

- **DEBUG**: Detailed information for debugging
- **INFO**: General informational messages
- **WARNING**: Warning messages
- **ERROR**: Error messages with stack traces
- **CRITICAL**: Critical errors

### Using Logging

**Normal Mode** (INFO and above):
```bash
python client_3d.py "TestPilot"
```

**Debug Mode** (DEBUG and above):
```bash
python client_3d.py "TestPilot" --debug
```

**Custom Log Directory**:
```bash
python client_3d.py "TestPilot" --log-dir my_logs
```

### Log Format

**Console** (simple):
```
15:38:00 - INFO - Character: TestPilot
15:38:01 - INFO - Connecting to server...
15:38:02 - INFO - Connected successfully
```

**File** (detailed):
```
2026-02-02 15:38:00 - eve_offline_3d - INFO - [client_3d.py:75] - Character: TestPilot
2026-02-02 15:38:01 - eve_offline_3d - INFO - [network_client.py:45] - Connecting to server...
2026-02-02 15:38:02 - eve_offline_3d - INFO - [network_client.py:70] - Connected successfully
```

### Viewing Logs

**Via Launcher**:
```bash
python launcher.py
# Select option 10: View logs
```

**Via Command Line**:
```bash
# View most recent log
cat logs/eve_offline_3d_*.log | tail -50

# View specific log
cat logs/eve_offline_3d_20260202_033800.log

# Search logs for errors
grep ERROR logs/*.log
```

### What Gets Logged

1. **Startup**: Configuration, initialization
2. **Network**: Connection, disconnection, messages
3. **Rendering**: Entity creation, camera updates
4. **Input**: Keyboard/mouse events
5. **Errors**: Exceptions with full stack traces
6. **Performance**: Frame rates, timing info (DEBUG mode)

---

## 4. Troubleshooting with Logs

### Problem: Client Won't Start

**Step 1**: Run with debug logging
```bash
python client_3d.py "TestPilot" --debug
```

**Step 2**: Check the log file
```bash
cat logs/eve_offline_3d_*.log | tail -50
```

**Step 3**: Look for ERROR or CRITICAL messages

### Problem: Connection Issues

**Check logs for**:
```
ERROR - Connection failed
ERROR - Timeout
ERROR - Cannot connect to server
```

**Solution**: Ensure server is running on correct port

### Problem: Performance Issues

**Step 1**: Run with debug logging
```bash
python client_3d.py "TestPilot" --debug
```

**Step 2**: Look for performance warnings
```
WARNING - Frame rate dropped to 30 FPS
DEBUG - Entity count: 500
DEBUG - Render time: 50ms
```

### Problem: Crashes

**Check the end of the log file**:
```bash
tail -100 logs/eve_offline_3d_*.log
```

Look for:
- Exception stack traces
- CRITICAL errors
- Last action before crash

---

## 5. Best Practices

### For Users

1. **Always run automated tests first**:
   ```bash
   python automated_tests.py
   ```

2. **Use the launcher for ease**:
   ```bash
   python launcher.py
   ```

3. **Enable debug logging when troubleshooting**:
   ```bash
   python client_3d.py "YourName" --debug
   ```

4. **Check logs when something goes wrong**:
   ```bash
   python launcher.py  # Select option 10
   ```

### For Developers

1. **Run tests before committing**:
   ```bash
   python automated_tests.py
   ```

2. **Check logs during development**:
   ```bash
   python client_3d.py "Dev" --debug --log-dir dev_logs
   ```

3. **Add logging to new code**:
   ```python
   from client_3d.utils import get_logger
   
   logger = get_logger(__name__)
   logger.debug("Debug message")
   logger.info("Info message")
   logger.error("Error message", exc_info=True)
   ```

4. **Use exception context manager**:
   ```python
   from client_3d.utils import ExceptionLogger
   
   with ExceptionLogger(logger, "Operation failed"):
       # Your code here
       pass
   ```

---

## 6. FAQ

### Q: Where are the logs stored?

**A**: In the `logs/` directory in the project root. Each run creates a timestamped log file.

### Q: How do I delete old logs?

**A**: Simply delete files from the `logs/` directory:
```bash
rm logs/*.log
```

### Q: Can I change the log level?

**A**: Yes, use the `--debug` flag for DEBUG level, or edit the code to use WARNING, ERROR, or CRITICAL levels.

### Q: Do tests require the server to be running?

**A**: No, the automated tests are designed to run standalone.

### Q: Can I run tests on specific components only?

**A**: Yes, select option 3 in the automated tests menu.

### Q: What if tests fail?

**A**: Check the error messages in the output. Common issues:
- Missing dependencies (run `pip install -r requirements.txt`)
- Python version < 3.11
- Corrupted installation

---

## 7. Command Reference

### Testing

```bash
# Interactive testing
python automated_tests.py

# Quick tests
python automated_tests.py --quick

# View help
python automated_tests.py --help
```

### Launcher

```bash
# Start launcher
python launcher.py
```

### 3D Client with Logging

```bash
# Normal mode
python client_3d.py "YourName"

# Debug mode
python client_3d.py "YourName" --debug

# Custom log directory
python client_3d.py "YourName" --log-dir my_logs

# Custom server
python client_3d.py "YourName" 192.168.1.100 8765 --debug
```

### Log Management

```bash
# View recent logs
ls -lt logs/

# View last 50 lines of most recent log
tail -50 logs/eve_offline_3d_*.log | tail -50

# Search for errors
grep ERROR logs/*.log

# Count log files
ls logs/*.log | wc -l

# Delete old logs
rm logs/*.log
```

---

## 8. Integration with Development Workflow

### Pre-Commit Checklist

1. ✅ Run automated tests: `python automated_tests.py`
2. ✅ Fix any failures
3. ✅ Test manually with: `python launcher.py`
4. ✅ Check logs for errors
5. ✅ Commit changes

### Release Checklist

1. ✅ Run full test suite
2. ✅ Test on clean Python environment
3. ✅ Test on multiple platforms (if possible)
4. ✅ Review logs for any warnings
5. ✅ Update documentation

---

## Conclusion

These tools make EVE OFFLINE much easier to use and debug:

- **Launcher**: Easy access to all features
- **Automated Tests**: Verify everything works
- **Logging**: Debug issues quickly

Use them to ensure a smooth experience!

---

**Last Updated**: February 2, 2026  
**Author**: EVE OFFLINE Development Team
