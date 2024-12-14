cmake_minimum_required(VERSION 3.5.0)
project(Synze VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Synze main.cpp Interpreter.cpp)

target_include_directories(Synze PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(CTest)
enable_testing()
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")