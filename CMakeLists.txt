cmake_minimum_required(VERSION 3.5.0)
project(Syntaxium VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Syntaxium syntaxium.cpp)

include(CTest)
enable_testing()