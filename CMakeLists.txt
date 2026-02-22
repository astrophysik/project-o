cmake_minimum_required(VERSION 3.10)

set(REQUIRED_CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_COMPILER clang++)

project(project-o)

include(compiler/compiler.cmake)
