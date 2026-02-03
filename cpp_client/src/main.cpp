#include "core/application.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
    try {
        // Parse command line arguments
        std::string characterName = "Player";
        if (argc > 1) {
            characterName = argv[1];
        }

        std::cout << "EVE OFFLINE C++ Client" << std::endl;
        std::cout << "=======================" << std::endl;
        std::cout << "Character: " << characterName << std::endl;
        std::cout << std::endl;

        // Create and run application
        eve::Application app("EVE OFFLINE - " + characterName, 1280, 720);
        app.run();

        std::cout << "Client shutting down gracefully." << std::endl;
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
