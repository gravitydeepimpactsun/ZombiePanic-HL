# Building this SDK

We will not go over how to use and/or operate CMake. If you are new to CMake, please read trough it's [documentation](https://cmake.org/documentation/) before you continue.

## Requirements

* CMake 3.15 or newer

### Windows

* Windows 10 or newer
* Visual Studio 2022 or newer (latest C++17 MSVC compilers are required)

### Linux

* GCC 9 or newer (C++17 support or newer required)
* Packages:
    * `gcc-9`
    * `g++-9`
    * `g++-9-multilib`
    * `libgl1-mesa-dev` (or an equivalent package that provides the OpenGL headers)

## Building (Linux)

In the source code's root folder, write the following in your terminal:  
`cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/ToolchainLinuxGCC9.cmake -B build -S .`

Once it's done generating, run the following:  
`cmake --build build`

Once the build is complete, you will find `client.so` and `hl.se` within the `/build` folder.
