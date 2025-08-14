# Blur ASM Project

## Overview
This project is a learning exercise that demonstrates how to blur images by combining C and Assembly (ASM) programming. The main goal is to use C for file management and image handling, while leveraging Assembly for the computationally intensive blur operation.

## Features
- **Image Blurring:** Applies a blur effect to images using an efficient algorithm implemented in Assembly.
- **Hybrid Approach:** C is used for reading/writing image files and managing data, while Assembly handles the core blur processing for performance.
- **Educational Focus:** Designed to help understand the interaction between C and ASM, and how to optimize critical code sections.

## Project Structure
- `main.c` — Entry point, manages program flow.
- `src/` — Source files for C and ASM code.
- `include/` — Header files for C code.
- `obj/` — Compiled object files.
- `test_img/` — Sample images for testing.
- `Makefile` — Build instructions.

## How It Works
1. **C Code:** Handles loading and saving images, memory management, and calling the blur function.
2. **Assembly Code:** Performs the blur operation on image data for maximum speed.
3. **Integration:** C and ASM communicate via function calls, passing image data for processing.

## Getting Started
1. Clone the repository.
2. Place your images in the `test_img/` folder.
3. Build the project using `make`.
4. Run the program and provide an image to blur.

## Requirements
- GCC (for compiling C code)
- NASM or compatible assembler (for ASM code)
- Make

## Usage
```sh
make
./blur <input_image> <output_image>
```

## Learning Objectives
- Understand how to mix C and Assembly in a single project
- Learn how to optimize image processing tasks
- Practice file I/O and memory management in C

## License
This project is for educational purposes.
