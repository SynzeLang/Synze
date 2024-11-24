cmake_minimum_required(VERSION 3.5.0)
project(Syntaxium VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Syntaxium main.cpp Interpreter.cpp)

target_include_directories(Syntaxium PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(CTest)
enable_testing()