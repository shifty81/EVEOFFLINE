# Celestial Navigation — Research Notes

## Background

An "icons in space" bracket system was implemented to show on-screen diamond
icons for every celestial object (stations, stargates, planets, asteroid belts,
moons, wormholes, Dyson Rings). Each bracket was projected from its 3D world
position to screen coordinates, color-coded by type, with hover tooltips
showing name/type/distance and click/right-click interactions.

## Why It Was Removed

The bracket overlay had two significant issues that could not be easily fixed
without a more thoughtful design:

1. **Camera orbit blocked** — The bracket hit-test rectangles and
   `buttonBehavior()` calls consumed mouse input broadly enough that
   left-to-right (yaw) camera orbit via right-click drag was blocked. The
   camera could only pitch up/down; horizontal rotation stopped working.

2. **Visual clutter** — Even with reduced default alpha (0.15), the sheer
   number of celestial brackets filled the viewport and became the dominant
   visual element, obscuring the 3D scene.

## Current Approach

- **Overview panel** handles on-grid interactions: target lists, locking,
  approach, orbit, keep-at-range, and context menus for nearby entities.
- **In-game solar system map** is the preferred long-term solution for
  navigating to off-grid celestials (warp-to destinations like planets,
  stargates, and stations).

## Future Research — Better Alternatives

If on-screen celestial indicators are revisited, consider these approaches:

### 1. In-Game Solar System Map (Recommended)
- A dedicated full-screen or panel-based 2D/3D map showing all celestials.
- Player selects a destination on the map to warp; no viewport clutter.
- EVE Online uses this pattern (F10 map) for off-grid navigation.

### 2. Edge-Only Indicators
- Instead of placing icons at the projected celestial position, show small
  directional arrows or chevrons only at the **screen edges** pointing
  toward off-screen destinations.
- On-screen celestials get no icon (visible as 3D objects already).
- Much less visual clutter; no interference with the center of the viewport.

### 3. Filtered / Toggle Brackets
- Allow the player to toggle brackets on/off with a keybind (e.g., `Alt+Z`).
- Show brackets only for specific types (e.g., stargates and stations only).
- Default to OFF; player enables when needed for navigation.

### 4. Compass / Heading Ribbon
- A thin ribbon along the top or bottom of the screen showing celestial
  names at their angular bearing, similar to a compass strip in flight sims.
- Minimal screen footprint; no 2D-overlay hit-test conflicts.

### 5. Input Isolation Fix
- If the bracket overlay is re-implemented, the hit-test and `buttonBehavior()`
  calls must **not** consume mouse drag events used for camera orbit.
- Separate click consumption from drag consumption: brackets should only
  consume discrete click events, never continuous drag deltas.
- Consider processing brackets in a separate input pass that does not
  set the global `m_atlasConsumedMouse` flag during drag operations.

## References
- EVE Online solar system map (F10) for off-grid navigation
- Edge-indicator patterns used in space games (Elite Dangerous, Star Citizen)
- Flight sim compass ribbons (DCS World, MSFS)
