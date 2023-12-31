# 🖥️ JIT-8 

JIT 8 is a CHIP-8 just-in-time compiler to x86-64 (currently only supporting Linux).

Everything that is executed is compiled (no interpreter). No further optimizaion is occuring.
On the Benchmarks it is faster the reference interpreter by a factor of ~10.

For programs that use self-modifying code a lot, it can perform (much) worse than a basic interpreter.

## Dependencies

* [ASMJIT](https://asmjit.com/) for JIT compilation
* [SFML](https://www.sfml-dev.org/) for Graphics

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

### References

The Interpreter that was used for comparison:
https://bisqwit.iki.fi/jutut/kuvat/programming_examples/chip8/chip8.cc

Basic parsing and some structure was taken from this project:
https://github.com/shlomnissan/chip8