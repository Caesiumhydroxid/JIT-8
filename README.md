# JIT 8

JIT 8 is a CHIP-8 just-in-time compiler to x86-64 (currently only supporting Linux, the only platform specific code is a sleep syscall). It is built using ASMJIT and SFML.

## Building
The Project uses CMake:

```
mkdir build
cd build
cmake ..
make -j8
```

All dependencies should be fetched by CMake.

## Running
To run the JIT, start the program using:

```
./jit8 <-t slowdown in ns> <-s shift quirks> <-i I register quirks> filepath_rom
```

The slowdown is the time the program sleeps between instructions (given in nanoseconds).
For most common games a value of 500000 - 1000000 seems optimal.



e.g to play the Blinky ROM you need to enable the quirks using flags -s -i

```
./jit8 -t 500000 -s -i  ../rom/c8games/BLINKY
```
