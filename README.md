# Automated Shiny Hunting for Pokemon HGSS

An embedded system that automates shiny hunting in Pokemon HeartGold/SoulSilver on a Nintendo DS Lite. The system captures the DS screen via camera, classifies the game state using OpenCV, and drives automated button presses on the DS hardware to perform repeated shiny hunting attempts — stopping when a shiny Pokemon is detected.

## System Overview

A single Raspberry Pi 4 handles everything: vision, decision-making, and button control. The Pi captures the DS top screen via a Pi Camera Module, classifies the current game state (title screen, dialogue, battle encounter, shiny sparkle), and drives GPIO pins to toggle N-channel MOSFETs soldered to the DS Lite button pads, emulating physical button presses.

```
┌──────────┐   CSI    ┌──────────────────────┐  GPIO/MOSFET  ┌──────────┐
│ Pi Camera├──────────►│       Pi 4           ├──────────────►│  DS Lite │
│ Module   │  frames  │ OpenCV + Strategy +   │  button       │  (HGSS)  │
│          │          │ GPIO Button Control   │  emulation    │          │
└──────────┘          └──────────────────────┘               └──────────┘
```

## Shiny Hunt Method

The primary supported method is **soft resetting** (for starters and legendaries):

1. Press L+R+Start+Select simultaneously to soft reset the game
2. Mash A through title screen and dialogue
3. Wait for the battle/encounter screen
4. Check for the shiny sparkle animation
5. If shiny — stop and log. If not — repeat from step 1

The architecture is modular, designed to support additional hunt methods (random encounters, breeding) and more complex automation in the future.

## Hardware

| Component | Role |
|---|---|
| Raspberry Pi 4 | Vision, decision-making, and button control (Linux + OpenCV) |
| Pi Camera Module v2/v3 | Screen capture |
| N-channel MOSFETs | Button emulation (one per button) |
| Pull-down resistors | Gate pull-downs for safe default state (buttons release if Pi loses power) |
| 3D-printed camera mount | Fixed camera positioning over DS screen |
| Nintendo DS Lite | Target hardware (wires soldered to button pads) |

## Software Architecture

The Pi 4 software is split into independent C++ modules:

- **vision** — Captures frames via Pi Camera, uses OpenCV to classify the DS screen state. Other modules only see a `ScreenState` enum — OpenCV types stay inside this module.
- **strategy** — Takes a `ScreenState`, returns a `ButtonAction`. Each hunt method is a concrete `IHuntStrategy` implementation (e.g., `SoftResetStrategy`).
- **gpio** — Drives MOSFET gates via GPIO to emulate DS button presses. Exposes an `IButtonDriver` interface so tests can mock it without hardware.
- **app** — Wires vision + strategy + gpio together, runs the hunt loop, handles startup/shutdown.

Modules may depend on each other's public interfaces (e.g., strategy uses vision's `ScreenState` enum), but core modules do not depend on app.

## Build

C++17, CMake minimum 3.9:

```bash
cmake -B ./build
cmake --build ./build
```

## Testing

Each module has its own unit tests using GoogleTest, run via CTest:

```bash
cmake -B ./build
cmake --build ./build
ctest --test-dir ./build
```
