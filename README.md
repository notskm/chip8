# Chip8

[![Build Status](https://github.com/notskm/chip8/workflows/CI/badge.svg)](https://github.com/notskm/chip8/actions?query=workflow%3ACI)
[![Clang-Format Status](https://github.com/notskm/chip8/workflows/clang-format/badge.svg)](https://github.com/notskm/chip8/actions?query=workflow%3Aclang-format)
[![CMake-Format Status](https://github.com/notskm/chip8/workflows/cmake-format/badge.svg)](https://github.com/notskm/chip8/actions?query=workflow%3Acmake-format)
[![License](https://img.shields.io/github/license/notskm/chip8)](./LICENSE)

A Chip8 interpreter written in C++

![Chip8 screenshot](./data/screenshots/chip8.png)

## Building

### Prerequisites

* C++17 compliant compiler
* [CMake](https://cmake.org/download/)
* [Conan](https://conan.io/downloads.html)

### Setting up

```sh
git clone --recurse-submodules https://github.com/notskm/chip8
cd chip8
```

```sh
mkdir build
cd build
```

### Makefile generators (Make, Ninja, etc.)

```sh
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Multi generators (Visual Studio, etc.)

```sh
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### CMake options

|       Option       | Default | Description                        |
| :----------------: | :-----: | ---------------------------------- |
|     RUN_CONAN      |   ON    | Runs `conan install` automatically |
|    BUILD_TESTS     |   OFF   | Builds the tests                   |
| WARNINGS_AS_ERRORS |   OFF   | Treat compiler warnings as errors  |

## Authors

* [@notskm](https://github.com/notskm)

See also the list of [contributors](https://github.com/notskm/chip8) who participated in this project.
