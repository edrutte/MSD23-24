# Building

## Requirements

* Make
* GCC
* [wiringPi](https://github.com/WiringPi/WiringPi)

## Clone Stockfish submodule

```
git submodule update --init
cd Stockfish/src
make clean
make -j build ARCH=armv8-dotprod EXTRACXXFLAGS=-mcpu=cortex-a76
cd ../..
```

## Setting up the build

```
make clean
make -j build ARCH=armv8-dotprod EXTRACXXFLAGS="-mcpu=cortex-a76 -fexceptions"
```
