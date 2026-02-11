# Naming Convention — UI Framework & Components

**Date**: February 11, 2026  
**Status**: ✅ Applied

---

## Overview

All UI framework classes, files, and identifiers have been renamed to use
original names unique to the EVE OFFLINE project. Names that were
previously pulled directly from EVE Online (Photon UI, Neocom, etc.)
have been replaced with project-specific alternatives.

## Naming Mapping

| Old Name (EVE Online) | New Name (EVE OFFLINE) | Scope |
|----------------------|------------------------|-------|
| `photon` namespace | `atlas` namespace | Namespace |
| `PhotonContext` | `AtlasContext` | Class |
| `PhotonRenderer` | `AtlasRenderer` | Class |
| `PhotonHUD` | `AtlasHUD` | Class |
| `photon_types.h` | `atlas_types.h` | File |
| `photon_context.h` | `atlas_context.h` | File |
| `photon_renderer.h` | `atlas_renderer.h` | File |
| `photon_widgets.h` | `atlas_widgets.h` | File |
| `photon_hud.h` | `atlas_hud.h` | File |
| `photon_ui.rcss` | `atlas_ui.rcss` | CSS File |
| `EVEColors` | `SpaceColors` | Struct |
| `eve_colors.h` | `space_colors.h` | File |
| `EVEPanels` | `HUDPanels` | Namespace |
| `eve_panels.h` | `hud_panels.h` | File |
| `EVETargetList` | `TargetList` | Class |
| `eve_target_list.h` | `target_list.h` | File |
| `NeocomPanel` | `SidebarPanel` | Class |
| `neocom_panel.h` | `sidebar_panel.h` | File |
| `neocom.rml` | `sidebar.rml` | RML File |
| `neocomBar()` | `sidebarBar()` | Function |
| `setNeocomCallback()` | `setSidebarCallback()` | Method |
| `m_neocomPanel` | `m_sidebarPanel` | Variable |
| `m_photonCtx` | `m_atlasCtx` | Variable |
| `m_photonHUD` | `m_atlasHUD` | Variable |

## Guidelines for New Code

1. **Do not use trademarked game terminology** as class or file names.
   Use descriptive, generic alternatives.

2. **UI Framework**: Use the `atlas` namespace and `Atlas*` prefix for
   the custom UI rendering system.

3. **Panel naming**: Use descriptive names (`SidebarPanel`,
   `OverviewPanel`, `FittingPanel`).  Avoid game-specific marketing
   terms.

4. **Color/theme structs**: Use generic terms (`SpaceColors`, `Theme`)
   rather than branded UI names.

5. **The `eve` namespace** remains for the core game namespace, as the
   project itself is titled "EVE OFFLINE".  Only UI component names
   needed renaming.

## Directory Structure (After Rename)

```
cpp_client/include/ui/
├── atlas/               # Atlas UI framework (was photon/)
│   ├── atlas_types.h
│   ├── atlas_context.h
│   ├── atlas_renderer.h
│   ├── atlas_widgets.h
│   └── atlas_hud.h
├── space_colors.h       # Color palette (was eve_colors.h)
├── hud_panels.h         # HUD panel utilities (was eve_panels.h)
├── target_list.h        # Target list (was eve_target_list.h)
├── sidebar_panel.h      # Left sidebar (was neocom_panel.h)
├── layout_manager.h     # NEW: Layout save/load system
├── ui_manager.h         # Central UI coordinator
└── ...                  # Other panel headers unchanged
```
