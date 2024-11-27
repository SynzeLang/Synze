#include "Interpreter.hpp"
#include <iostream>

int main() {
    Interpreter interpreter;
    std::cout << "\n#######  ##    ##  ###    ##  ########   #####   ##   ##  ##  ##    ##  ###    ### \n";
    std::cout << "##        ##  ##   ####   ##     ##     ##   ##   ## ##   ##  ##    ##  ####  #### \n";
    std::cout << "#######    ####    ## ##  ##     ##     #######    ###    ##  ##    ##  ## #### ## \n";
    std::cout << "     ##     ##     ##  ## ##     ##     ##   ##   ## ##   ##  ##    ##  ##  ##  ## \n";
    std::cout << "#######     ##     ##   ####     ##     ##   ##  ##   ##  ##   ######   ##      ## \n\n";
    std::cout << "The Syntaxium Interpreter is active.\n\nType 'exit' to quit.\nType 'help' for more help.\n";

    std::string line;
    while (true) {
        try {
            std::cout << ">> ";
            std::getline(std::cin, line);
            interpreter.execute(line);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}