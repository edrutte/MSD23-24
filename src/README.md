# Building

## Requirements

* CMake
* Make
* GCC/Clang
* Ninja (optional, but recommended)

## Clone Stockfish submodule

```
git submodule update --init
```

## Setting up the build

```
mkdir -p build && cd build
cmake .. # -GNinja if installed
make -j `nproc` # ninja if installed
```
