// Import WebSrv Library
#include "WebSrv.h"
// #include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Set input arguments
    if (argc > 1 && std::string(argv[1]) == "help") {
        WebSrv::help();
        return 0;
    }
    // set Port
    int port = 8080;
    // Set file to serve
    std::string file = "index.html";

    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    if (argc > 2) {
        file = argv[2];
    }

    // Initialize server object
    WebSrv server(port, file);

    // Start the server
    server.start();

    // Stop the server
    // server.stop();

    return 0;
}
