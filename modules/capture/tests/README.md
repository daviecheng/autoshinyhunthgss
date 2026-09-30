# capture tests

Unit tests for the capture module. GoogleTest for the framework, CTest for the runner.

## Build and run

```bash
cmake -B ./build
cmake --build ./build
ctest --test-dir ./build
```

To run a single test:

```bash
./build/modules/capture/tests/capture_tests --gtest_filter='ScreenLocatorTest.*'
```

## Test output

Tests that handle images write what went in and what came out, so you can compare the two:

```
test_output/
└── screen_locator/
    ├── input_frame.png
    └── output_frame.png
```

`test_output/` is ignored by git.

## Test data

Fixtures live in `test_data/`, one subfolder per screen state, plus `bad_rig/` for shots the
rig should never produce:

```
test_data/
├── bad_rig/
├── encounter_non_shiny/
├── intro/
├── intro_b_roll/
├── off/
└── overworld_idle/
```

`CAPTURE_TEST_DATA_DIR` and `CAPTURE_TEST_OUTPUT_DIR` are defined in
[`CMakeLists.txt`](CMakeLists.txt) so tests find these folders.
