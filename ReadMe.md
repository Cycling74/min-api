# Min-API

This folder contains the support files you will need to compile an external object written in C++ using a high-level declarative application programming interface. It is distributed as a part of the [Min-DevKit Package](https://github.com/Cycling74/min-devkit). Please refer to that package for additional documentation and best practices.

## Overview of Contents

* `include` : header files
* `doc` : documentation
* `script` : resources to be included and used by CMake
* `test` : supporting code and resources for unit testing
* `max-api` : a git submodule that provides the low-level bindings to the Max application
* [Conan.md](Conan.md) : information about how to create and use conan packages with `min-api`

## License

Use of this Max-API distribution is governed by the MIT License as stated in the accompanying `License.md` file.

## Breaking changes

April 9, 2021 - Min-API now requires CMake 3.19.0 or later. Use the pre-cmake-3_19 branch if you depend on an earlier CMake version.
