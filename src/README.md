# Building

## Requirements

* CMake
* Make
* GCC/Clang
* Ninja (optional, but recommended)
* Ccache (optional, but recommended)
* libgpiod-dev

## Clone Stockfish submodule

```
git submodule update --init
cd Stockfish
git apply ../suppress_stockfish_info.patch
```

## Setting up the build

```
mkdir -p build && cd build
cmake .. -GNinja -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
ninja
```
