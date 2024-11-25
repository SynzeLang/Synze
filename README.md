# Syntaxium

Syntaxium is a simple interpreted programming language implemented in C++. It provides basic programming constructs including variable declarations, mathematical operations, and string manipulation.

## Features

- Variable declarations (string, number, boolean)
- Basic arithmetic operations (+, -, *, /)
- String concatenation
- File execution support (.syntax files)
- Real-time command interpretation
- Error handling and reporting

## Building

To build Syntaxium, you need:
- CMake (3.5.0 or higher)
- C++ compiler with C++11 support

mkdir build
cd build
cmake ..
cmake --build .

## Usage

After building, you can run Syntaxium in two ways:

1. Interactive Mode:
./Syntaxium

2. File Execution:
# In the interactive mode
>> run example.syntax

## Syntax

### Variable Declaration
variable name = value

### Output
send expression

### File Execution
run filename.syntax

## Examples

variable message = "Hello, World!"
send message

send 5 + 3 * 2

variable name = "John"
send "Hello, " + name

variable isActive = true

## License

This project is open source and available under the MIT License.