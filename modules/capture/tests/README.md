# capture tests

Unit tests for the capture module. GoogleTest for the framework, CTest for the runner.

## Build

```bash
cmake -B ./build
cmake --build ./build
```

The tests are built when `BUILD_TESTING` and `BUILD_CAPTURE_TESTS` are both `ON` (the default).

## Run

Through CTest, with the rest of the suite:

```bash
ctest --test-dir ./build
```

Or the binary directly, which is what you want when iterating on one test:

```bash
./build/modules/capture/tests/capture_tests
./build/modules/capture/tests/capture_tests --gtest_filter='ScreenLocatorTest.*'
```

## Dumping images for inspection

Framing is geometry, and geometry is hard to assert usefully — a test can tell you a screen was
found, but only your eyes can tell you it was framed correctly. Tests that handle images therefore
write their inputs and outputs to disk when `CAPTURE_TEST_DUMP_DIR` is set, and write nothing
otherwise, so a normal run stays clean.

```bash
mkdir -p /tmp/capture_dump

CAPTURE_TEST_DUMP_DIR=/tmp/capture_dump \
    ./build/modules/capture/tests/capture_tests --gtest_filter='ScreenLocatorTest.*'

ls /tmp/capture_dump
```

For `ScreenLocatorTest.LocatesScreenInPhoto` that gives you:

| File | Contents |
|---|---|
| `input.png` | the wide rig photo the locator was given |
| `output.png` | the rectified 512x384 top screen it produced |

Open the two side by side: `input.png` shows what the camera saw, `output.png` shows which
quadrilateral the locator picked and how it warped it. A crop that is the right size but the wrong
region still passes the assertions, so this is the only way to catch it.

### Notes

- **Create the directory first.** `cv::imwrite` returns `false` for a path that does not exist and
  reports nothing further, so a missing directory looks identical to the environment variable not
  being set.
- **Run through the binary, not CTest.** CTest passes the environment through, but `--gtest_filter`
  lets you dump one test instead of every image-handling test in the suite.
- **Filenames are per-test, not per-run.** Two tests that dump `output.png` overwrite each other,
  and a second run overwrites the first. Dump one test at a time, or copy the results out between
  runs.

The helper is `dump_if_requested` in
[`screen_locator_test.cpp`](screen_locator_test.cpp) — call it from any new test that produces an
image worth looking at.

## Test data

Fixtures live in [`test_data/`](test_data). `CAPTURE_TEST_DATA_DIR` is defined by
[`CMakeLists.txt`](CMakeLists.txt) so tests locate them without relative paths; use the
`fixture_path` and `load_fixture` helpers rather than building paths by hand.

Fixture dimensions are unconstrained — only the 512x384 BGR888 output is asserted — so a rig photo
can be whatever the camera happens to produce. Wide rig photos should cover the nominal framing plus
shifted and tilted variants, to exercise the locator's tolerance for the rig drifting or being
knocked.
