# Archived Code

This directory contains legacy Python code from the original EVE OFFLINE prototype.

The project has evolved from Python to C++ and this code is kept for reference only.

## Contents

- **`python/`** — Legacy Python implementation
  - `client/` — Original text/GUI client (pygame-based)
  - `server/` — Original Python game server
  - `engine/` — Core game engine (Python)
  - `client_3d/` — 3D client prototype (Panda3D-based)
  - `tests/` — Python test files
  - `tests_suite/` — Additional test suite
  - `demos/` — Demo and showcase scripts
  - `build_and_test.py` — Legacy Python build automation
  - `build_cpp_client.py` — Legacy C++ build helper script
  - `requirements.txt` — Python dependencies

## Current Active Code

The active C++ implementation is in:
- `cpp_client/` — C++ OpenGL client
- `cpp_server/` — C++ dedicated server
- `data/` — Game data (JSON configs, shared between implementations)
