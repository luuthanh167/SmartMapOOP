#include "Server/HttpServer.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    int port = 5000;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }

    // Khoi dong Web Server SmartMap tren cong 5000 (giong het ban goc C#)
    SmartMap::Server::HttpServer server(port);
    server.Start();

    return 0;
}