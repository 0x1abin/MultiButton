# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/).

## [1.1.0] - 2026-03-17

### Added
- Version macros (`MULTIBUTTON_VERSION_MAJOR/MINOR/PATCH`)
- Compile-time check for `DEBOUNCE_TICKS` exceeding 3-bit field maximum
- Optional thread-safety support via `MULTIBUTTON_THREAD_SAFE` compile flag
- Unit tests with 10 test cases covering all major functionality
- CMakeLists.txt for CMake-based build support
- English README (`README.md`), Chinese README moved to `README_CN.md`
- GitHub Actions CI (gcc + clang, Make + CMake)
- CONTRIBUTING.md, issue templates, and PR template
- CHANGELOG.md

### Fixed
- BTN_STATE_REPEAT to BTN_STATE_PRESS transition: ticks and repeat counter now properly reset, preventing incorrect immediate long-press detection after a repeat sequence

### Changed
- Examples: replaced emoji characters with ASCII markers for terminal compatibility

## [1.0.0] - 2024-01-01

### Added
- Initial optimized release
- Multi-button state machine with 7 event types
- Hardware debounce filtering
- Callback and polling event modes
- `button_detach()`, `button_reset()`, `button_is_pressed()`, `button_get_repeat_count()` APIs
- Parameter validation and NULL pointer safety
- Basic, advanced, and polling examples
- Makefile build system
