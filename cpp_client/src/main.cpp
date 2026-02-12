#include "core/application.h"
#include "core/file_logger.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
    // Initialize file logging so all output is captured to logs/atlas_client.log.
    // This ensures error messages are preserved even if the console window
    // closes immediately on crash or exit.
    atlas::FileLogger::init();

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
        atlas::Application app("EVE OFFLINE - " + characterName, 1280, 720);
        app.run();

        std::cout << "Client shutting down gracefully." << std::endl;
        atlas::FileLogger::shutdown();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        std::cerr << "See logs/atlas_client.log for details." << std::endl;
        atlas::FileLogger::shutdown();
        return EXIT_FAILURE;
    }
}
