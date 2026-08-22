# AGENTS.md

## Project

Automated shiny hunting for Pokemon HeartGold/SoulSilver on a Nintendo DS Lite. Single-board system: a Raspberry Pi 4 handles vision, strategy, and button control. C++17, CMake.

Current scope: soft-reset shiny hunting (starters and legendaries). The architecture is modular to support more complex automation in the future if desired:

- Additional hunt methods (random encounters, breeding)
- GUI for monitoring and control
- External peripherals (LEDs, buzzer for shiny notification)

## Build

C++17, CMake minimum 3.10:

```bash
cmake -B ./build
cmake --build ./build
```

## Dependencies

### Hardware
- Raspberry Pi 4 running Raspberry Pi OS
- Pi Camera Module v2 or v3
- N-channel MOSFETs (one per DS button) with pull-down resistors on each gate
- Nintendo DS Lite with wires soldered to button pads

### Software
- OpenCV
- GoogleTest

## Architecture

Single Pi 4 does everything: captures the DS screen via camera, classifies game state with OpenCV, decides actions via hunt strategy, and drives GPIO pins to toggle MOSFETs that emulate DS button presses.

```
┌──────────┐   CSI    ┌──────────────────────┐  GPIO/MOSFET  ┌──────────┐
│ Pi Camera├──────────►│       Pi 4           ├──────────────►│  DS Lite │
│ Module   │  frames  │ OpenCV + Strategy +   │  button       │  (HGSS)  │
│          │          │ GPIO Button Control   │  emulation    │          │
└──────────┘          └──────────────────────┘               └──────────┘
```

### Software Modules

- **vision** — "What do I see?" Captures frames via Pi Camera, uses OpenCV to classify the DS screen state (title, dialogue, encounter, shiny sparkle). OpenCV stays inside this module; other modules only see a `ScreenState` enum.
- **strategy** — "What should I do?" Takes a `ScreenState`, returns a `ButtonAction`. Each hunt method is a concrete `IHuntStrategy` implementation (e.g., `SoftResetStrategy`). Adding a new method means adding a new class — no changes elsewhere.
- **gpio** — "Press the button." Drives MOSFET gates via Pi 4 GPIO to emulate DS button presses. Exposes an `IButtonDriver` interface so tests can mock it without hardware.
- **app** — The main application. Wires vision + strategy + gpio together, runs the hunt loop, and handles startup/shutdown/signal handling.

## File Structure

```
CMakeLists.txt                  — top-level CMake, aggregates modules
modules/                        — C++ modules (static libraries)
  vision/                       — screen capture and classification
    include/vision/             — public headers (ScreenState enum)
    private_include/            — internal headers (OpenCV-specific helpers)
    src/                        — implementation + CMakeLists.txt
    test/                       — unit tests for this module
  strategy/                     — hunt logic (IHuntStrategy + implementations)
  gpio/                         — GPIO button driver (IButtonDriver + implementation)
  app/                          — main application entry points and orchestration
docs/                           — Schematics, 3D models, and documentation
```

### Module Layout Convention

Each library module under `modules/` follows the same structure as vision above:
`include/<module_name>/`, `private_include/`, `src/`, `test/`.
App is the exception — it only has `src/` with executable entry points (one per hunt scenario).

Library modules (vision, strategy, gpio) build as static libraries. App builds as an executable that links against them. All code uses the `autoshinyhunthgss` namespace.

### Dependency Graph

```
app ──► vision     (reads screen state)
    ──► strategy   (decides next action)
    ──► gpio       (executes button presses)

Modules may depend on each other's public interfaces (e.g., strategy uses vision's ScreenState enum).
```

To extend: add new `IHuntStrategy` impls for hunt methods, new `IButtonDriver` impls to swap button control, or change vision internals without affecting other modules.

## Coding Conventions

- snake_case for functions and variables.
- PascalCase for classes and structs.
- `is`, `has`, `can` prefixes for booleans.
- Verb-based names for actions (e.g., `send_command`, `capture_frame`, `detect_sparkle`).
- Core modules depend on abstract interfaces, not concrete implementations.
- Dependencies point inward: core modules (strategy, vision) must not depend on app.
- Framework-specific types (OpenCV `cv::Mat`) stay within their respective modules and do not leak across public interfaces.
- Standard C++ types in public interfaces.
- Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers. Use RAII for resource management.

## Testing

- Each module has its own unit tests. Use test images for vision, mock `IButtonDriver` for gpio, direct input/output for strategy.
- GoogleTest for test framework, CTest for test runner.
