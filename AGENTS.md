# AGENTS.md

Read [`README.md`](README.md) first. It is the source of truth for the architecture, module
responsibilities, hunt loop, and file structure — this file does not repeat them. What
follows is how to work in the codebase.

Current scope is soft-reset shiny hunting (starters and legendaries). Keep the module
boundaries intact so later work — additional hunt methods, a monitoring GUI, LED/buzzer
peripherals — drops in without touching unrelated modules.

## Build & Test

```bash
cmake -B ./build
cmake --build ./build
ctest --test-dir ./build
```

## Boundaries

These are design decisions, not incidental structure. Do not work around them silently — if
one is genuinely blocking, say so rather than routing past it.

- OpenCV types (`cv::Mat`) stay inside vision and never cross a public interface.
- vision publishes one `ScreenState` enum. `ShinyVerdict` is internal, in `private_include/`.
- strategy carries no timing. gpio owns press duration; app owns the poll interval.
- Dependencies point inward: vision and strategy must not depend on app.
- Core modules depend on abstract interfaces (`IHuntStrategy`, `IButtonDriver`), not concrete
  implementations, so tests can run without hardware.
- Public interfaces use standard C++ types.

## Module Layout

Each library module under `modules/` uses the same structure:
`include/<module_name>/`, `private_include/`, `src/`, `test/`. App is the exception — only
`src/`, with one executable entry point per hunt scenario.

All code uses the `autoshinyhunthgss` namespace.

## Coding Conventions

- snake_case for functions and variables.
- PascalCase for classes and structs.
- `is`, `has`, `can` prefixes for booleans.
- Verb-based names for actions (e.g., `send_command`, `capture_frame`, `classify_sprite`).
- Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers. Use RAII for
  resource management.

## Testing

Each module has its own unit tests — test images for vision, a mock `IButtonDriver` for gpio,
direct input/output for strategy. GoogleTest for the framework, CTest for the runner.
