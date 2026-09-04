# Automated Shiny Hunting for Pokemon HGSS

An embedded system that automates shiny hunting in Pokemon HeartGold/SoulSilver on a Nintendo DS Lite. The system captures the DS screen via camera, classifies the game state using OpenCV, and drives automated button presses on the DS hardware to perform repeated shiny hunting attempts — stopping when a shiny Pokemon is detected.

## System Overview

A single Raspberry Pi 4 handles everything: vision, decision-making, and button control. The Pi captures the DS top screen via a Pi Camera Module, classifies the current game state (title screen, dialogue, battle encounter, Pokemon sprite on screen), and drives GPIO pins to toggle N-channel MOSFETs soldered to the DS Lite button pads, emulating physical button presses.

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
3. Wait for the battle/encounter screen, then for the Pokemon sprite to appear
4. Compare the sprite against stored reference images of the target species, normal and shiny
5. If shiny — stop and log. If confirmed not shiny — repeat from step 1. If uncertain — stop rather than risk resetting over a shiny

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

- **vision** — "What do I see?" Captures frames via Pi Camera and classifies them with OpenCV into a single `ScreenState`. Screen classification and shiny classification are separate stages: the screen classifier runs on every frame (title, dialogue, encounter, sprite visible), and only a stable encounter advances to the shiny classifier, which compares the sprite against stored reference images of the target species. Every other screen state short-circuits and skips the shiny check entirely. Requiring the encounter to be stable keeps the comparison off half-faded sprites. Not the sparkle animation — the sprite persists, so it can be sampled repeatedly. OpenCV stays inside this module; other modules only see a `ScreenState` enum, with the internal `ShinyVerdict` kept in `private_include/`.
- **strategy** — "What should I do?" Takes a `ScreenState`, returns a `ButtonAction` — a *set* of buttons, so simultaneous presses are one action (soft reset = `{L, R, Start, Select}`) and the empty set means no buttons are pressed. Carries no timing. Each hunt method is a concrete `IHuntStrategy` implementation (e.g., `SoftResetStrategy`), fixed at construction rather than passed in at runtime. Adding a new method means adding a new class — no changes elsewhere. Soft reset only on a confirmed non-shiny state; an unknown or uncertain state halts rather than resets.
- **gpio** — "Press the button." Drives MOSFET gates via Pi 4 GPIO to emulate DS button presses. Takes a `ButtonAction` and drives its buttons together, owning press duration so strategy stays timing-free. Exposes an `IButtonDriver` interface so tests can mock it without hardware.
- **app** — The main application. Wires vision + strategy + gpio together, selects the hunt method, runs the hunt loop at a fixed poll interval, and handles startup/shutdown/signal handling.

### Hunt Loop

Every iteration follows the same path. The only branch is whether the shiny classifier runs.

```
        ┌──────────────┐
        │ capture frame│◄──────── poll interval (app)
        └──────┬───────┘
               ▼
        ┌──────────────┐
        │    screen    │
        │  classifier  │
        └──────┬───────┘
               ▼
        stable encounter?
         │            │
      no │            │ yes
         │            ▼
         │     ┌──────────────┐
         │     │    shiny     │
         │     │  classifier  │
         │     └──────┬───────┘
         ▼            ▼
    ScreenState   ScreenState
    (as-is)       (shiny result replaces Encounter)
         │            │
         └─────┬──────┘
               ▼
        ┌──────────────┐
        │   strategy   │
        └──────┬───────┘
               ▼
         ButtonAction
         │          │
   empty │          │ non-empty
         ▼          ▼
    no presses  ┌────────┐
                │  gpio  │
                └────────┘
```

The shiny verdict replaces the encounter state rather than travelling beside it, so strategy
always switches on exactly one value:

| ScreenState | ButtonAction | Outcome |
|---|---|---|
| `Unknown` | `{}` | halt — screen unrecognized, log for review |
| `LoadingScreen` | `{}` | nothing — next poll |
| `Encounter` (sprite not stable yet) | `{}` | nothing — next poll |
| `TrainerInFrontOfPokemon` | `{A}` | advance dialogue |
| `EncounterShiny` | `{}` | halt — preserve the shiny |
| `EncounterNonShiny` | `{L, R, Start, Select}` | soft reset, next attempt |
| `EncounterUncertain` | `{}` | halt — never risk resetting over a shiny |

An empty `ButtonAction` only ever means "press nothing"; it does not say whether to keep
hunting. Loop termination is app's decision, based on which state was reached — strategy
stays concerned with buttons alone.

### Dependency Graph

```
app ──► vision     (reads screen state)
    ──► strategy   (decides next action)
    ──► gpio       (executes button presses)

Modules may depend on each other's public interfaces (e.g., strategy uses vision's ScreenState enum).
```

Core modules do not depend on app. To extend: add new `IHuntStrategy` impls for hunt methods, new `IButtonDriver` impls to swap button control, or change vision internals without affecting other modules.

## File Structure

```
CMakeLists.txt                  — top-level CMake, aggregates modules
modules/                        — C++ modules (static libraries)
  vision/                       — screen capture and classification
    include/vision/             — public headers (ScreenState enum)
    private_include/            — internal headers (screen + shiny classifiers)
    src/                        — implementation + CMakeLists.txt
    test/                       — unit tests for this module
  strategy/                     — hunt logic (IHuntStrategy + implementations)
  gpio/                         — GPIO button driver (IButtonDriver + implementation)
  app/                          — main application entry points and orchestration
docs/                           — Schematics, 3D models, and documentation
```

Library modules (vision, strategy, gpio) build as static libraries. App builds as an executable that links against them.

## Dependencies

- OpenCV
- GoogleTest

## Build

C++17, CMake minimum 3.10:

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
