# CHIP8 interpreter
---
<img src="https://user-images.githubusercontent.com/102764781/192083200-28c7557d-8afb-422f-9d67-02196d312c87.png">

## Getting Started
### Dependencies
  - C++11
  - SDL2 -> https://github.com/libsdl-org/SDL

### Compiling program
```
# clone repo
git clone https://github.com/01-michael/chip8_interpreter.git

# go into directory
cd chip8_interpreter

# Compile (to use debug dump, use c++20 or newer since std::format is used to structure the output)
g++ -std=c++11 -o chip8 src/*.cpp -I<SDL2 INCLUDE DIRECTORY> -L<SDL2 LIB DIRECTORY> -lSDL2main -lSDL2

```
---
### Running program
Chip8 takes two required arguments: **`-rom`** and **`-scale`**
|**arg**      |  description                                                                                                       |
|-------------|--------------------------------------------------------------------------------------------------------------------|
| **`rom`**   | File path to the rom file                                                                                          |
| **`scale`** | Enlarges game view by scale value. Since chip8 has a 64x32 display, a scale of 7-20 tend to yield the best result. |

```
# run command
./chip8 -rom <ROM FILE PATH> -scale <INTEGER VALUE>

# Example of running pong
./chip8 -rom Pong.ch8 -scale 10
```
