# Contributing to MultiButton

Thank you for your interest in contributing!

## Building

```bash
# Make
make all       # build library + examples
make test      # run unit tests

# CMake
cmake -B build -DMULTIBUTTON_BUILD_TESTS=ON -DMULTIBUTTON_BUILD_EXAMPLES=ON
cmake --build build
cd build && ctest
```

## Code Style

- C99 standard
- K&R brace style (opening brace on same line)
- Tab indentation in source files
- `button_` prefix for all public API functions
- Null pointer checks on all public API entry points

## Pull Request Process

1. Fork the repository and create a feature branch
2. Ensure `make test` passes with no failures
3. Ensure `make all` compiles with no warnings (`-Wall -Wextra`)
4. Update documentation if the public API changes
5. Describe your changes clearly in the PR description

## Reporting Bugs

Please use the [bug report template](.github/ISSUE_TEMPLATE/bug_report.md) and include:
- Hardware platform (MCU, board)
- Compiler version
- Steps to reproduce
- Expected vs actual behavior
