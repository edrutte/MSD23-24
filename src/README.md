# Building

## Requirements

* CMake
* Make
* GCC/Clang
* Ninja (optional, but recommended)
* Ccache (optional, but recommended)
* [wiringPi](https://github.com/WiringPi/WiringPi)

## Clone submodules

```
git submodule update --init
cd Stockfish
git apply ../suppress_stockfish_info.patch
cd ../lcd1602_i2c
git apply ../lcd_patch.patch
```

## Setting up the build

```
mkdir -p build && cd build
cmake .. -GNinja -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
ninja
```
