# Server Admin Console GUI — Design Document

> **Status**: Future Work — Planned  
> **Priority**: Low (focus is on client GUI first)  
> **Document Version**: 1.0

## Overview

The EVE OFFLINE dedicated server currently runs as a headless console application.
This document outlines the design for a future **Server Admin Console GUI** that will
provide real-time monitoring, on-the-fly configuration, and administrative controls
for server operators.

## Goals

1. **Console Output** — Live scrolling log viewer with filtering by log level
   (DEBUG, INFO, WARN, ERROR, FATAL) and search.
2. **On-the-fly Configuration** — Ability to change server settings at runtime
   without restarting (tick rate, max players, auto-save interval, etc.).
3. **Player Management** — View connected players, kick/ban, inspect entity state.
4. **World Monitoring** — Entity counts, system performance metrics, ECS system timings.
5. **Lightweight** — The GUI should be optional and not add significant dependencies
   to the dedicated server build.

## Proposed Architecture

### Option A: Terminal UI (TUI) — Recommended for Phase 1

Use a lightweight terminal UI library (e.g., FTXUI or ncurses) to build a
rich console interface that works in any terminal emulator. No windowing
system required.

```
┌─── EVE OFFLINE Server Console ──────────────────────────────────────┐
│ Status: Running │ Players: 12/20 │ Tick: 30Hz │ Uptime: 02:45:13   │
├─────────────────────────────────────────────────────────────────────-┤
│ [Console Output]                                          [Filter ▾]│
│ 14:32:01.123 [INFO]  Player 'CaptainX' connected                   │
│ 14:32:01.456 [INFO]  Spawned entity for CaptainX in Jita            │
│ 14:32:02.789 [WARN]  High tick time: 45ms (target 33ms)            │
│ 14:32:03.012 [INFO]  Auto-save completed (234 entities)            │
│ 14:32:05.234 [DEBUG] Capacitor tick: 2847 entities processed       │
│                                                                     │
├──────────────────────────────────────────────────────────────────────┤
│ > command: _                                                        │
└──────────────────────────────────────────────────────────────────────┘
```

**Commands:**
- `status` — Show server status summary
- `players` — List connected players
- `kick <name>` — Disconnect a player
- `ban <name>` — Add player to ban list
- `config get <key>` — Show a configuration value
- `config set <key> <value>` — Change a configuration value at runtime
- `save` — Force world save
- `reload` — Reload configuration from file
- `shutdown` — Graceful server shutdown
- `metrics` — Show performance metrics
- `loglevel <level>` — Change log verbosity

### Option B: Graphical GUI — Phase 2

If a full graphical interface is desired, use a lightweight windowing
library. The recommended approach:

- **Framework**: SDL2 + Dear ImGui (ImGui is already available in the client)
- **Layout**: Multi-panel dashboard with tabs
- **Panels**:
  - **Dashboard** — Overview metrics, player count, tick rate graph
  - **Console** — Scrolling log output with filters
  - **Players** — Connected player list with admin actions
  - **Config** — Editable server configuration with apply/revert
  - **World** — Entity inspector, system map, solar system state
  - **Metrics** — Performance graphs (tick time, memory, network)

## Planned Interface

```cpp
// cpp_server/include/ui/server_console.h

class ServerConsole {
public:
    ServerConsole();
    ~ServerConsole();

    /// Initialize the console (call after server init).
    bool init(Server& server, const ServerConfig& config);

    /// Process one frame of console I/O. Call from the server main loop.
    void update();

    /// Shutdown the console.
    void shutdown();

    /// Add a log message to the console output buffer.
    void addLogMessage(LogLevel level, const std::string& message);

    /// Execute a command string.
    std::string executeCommand(const std::string& command);

    /// Set whether the console captures input (vs. raw terminal).
    void setInteractive(bool interactive);
};
```

## Configuration Changes (Runtime-adjustable)

The following `ServerConfig` fields should be modifiable at runtime via
the admin console:

| Setting | Config Key | Type | Range | Default |
|---------|-----------|------|-------|---------|
| Tick Rate | `tick_rate` | float | 10–120 Hz | 30.0 |
| Max Connections | `max_connections` | int | 1–200 | 20 |
| Auto-save Interval | `auto_save_interval` | int | 30–3600 sec | 300 |
| Server Password | `password` | string | — | "" |
| Public Visibility | `public_server` | bool | — | true |
| Whitelist Enabled | `whitelist_enabled` | bool | — | false |
| Log Level | `log_level` | enum | DEBUG–FATAL | INFO |

## Implementation Plan

### Phase 1: Command-line Console (Near-term)
1. Add `ServerConsole` class with command parsing
2. Integrate with existing `Logger` for output capture
3. Add runtime config modification via `ServerConfig`
4. Support basic admin commands (status, players, kick, save)

### Phase 2: TUI Dashboard (Mid-term)
1. Add FTXUI or ncurses dependency (optional build flag)
2. Build multi-pane terminal layout
3. Add real-time metrics display
4. Log level filtering and search

### Phase 3: Graphical Dashboard (Long-term)
1. Add SDL2 + ImGui build option for server
2. Build graphical dashboard panels
3. Add performance graphs and world visualization
4. Entity inspector with live state editing

## Dependencies

| Phase | Library | License | Notes |
|-------|---------|---------|-------|
| Phase 1 | None | — | Pure C++ stdin/stdout |
| Phase 2 | FTXUI | MIT | Header-only TUI library |
| Phase 3 | SDL2 + ImGui | zlib/MIT | Already available in client |

## Notes

- The server GUI is entirely optional. The server must always be runnable
  as a pure headless process for Docker/cloud deployment.
- Build flag: `-DSERVER_CONSOLE=ON` (default OFF for headless builds)
- The console should not block the server's game loop; all I/O should be
  non-blocking or on a separate thread.
