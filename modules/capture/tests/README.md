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

Fixtures live in `test_data/` at the project root, shared by every module's tests. They are raw
rig photos (1280x960), grouped by time of day and screen state:

```
test_data/
├── day/
│   ├── both_health_bars/
│   └── one_health_bar/
├── morning/
│   ├── both_health_bars/
│   └── one_health_bar/
├── night/
│   ├── both_health_bars/
│   └── one_health_bar/
├── non_encounter/
├── screen_found/
└── screen_not_found/
```

`TEST_DATA_DIR` and `CAPTURE_TEST_OUTPUT_DIR` are defined in
[`CMakeLists.txt`](CMakeLists.txt) so tests find these folders.
