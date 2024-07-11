#include "WebSrv.h"

WebSrv* WebSrv::instance = nullptr;

// Init 
WebSrv::WebSrv(int port, const std::string& file) : srvPort(port), filePath(file), runStatus(false), socketMain(-1), logFile("log.txt", std::ios::app) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    // Link object
    instance = this;
}
// Destructor 
WebSrv::~WebSrv() {
    // Stop server
    stop();
    // Close log file
    if (logFile.is_open()) {
        logFile.close();
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

// Start Server - user exposed
void WebSrv::start() {
    signal(SIGINT, WebSrv::interuptHandler);
    runStatus = true;

    // Create Socket
    socketMain = socket(AF_INET, SOCK_STREAM, 0);

    // To reuse already used port again after termination of server,
    // prevents non initialization due to timeout for reuse of ports
    int opt = 1;
    setsockopt(socketMain, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // Set Server Address
    sockaddr_in srvAddress;
    // IPV4
    srvAddress.sin_family = AF_INET;
    // Accept all addresses
    // Blocklist may be added 
    srvAddress.sin_addr.s_addr = INADDR_ANY;
    // Defining port for the socket
    srvAddress.sin_port = htons(srvPort);
    // Bind the socket to the server address and port
    bind(socketMain, (sockaddr*)&srvAddress, sizeof(srvAddress));
    // Start Listening to connection with connection queue set to 10
    // If requests are dropped, try increasing connection queue limit
    // Might expose all these when TUI or GUI front created
    listen(socketMain, 10);

    // Logging to terminal
    std::cout << "[Running]" << "[" << srvPort << "]" << " [File] " << "[" << filePath << "]" << std::endl;

    // Running server till interupt occurs and kills process
    while (runStatus) {
        // Find Client Address and incoming port
        sockaddr_in clntAddress;
        socklen_t clntLen = sizeof(clntAddress);
        // Await connection on socketMain and open a new socket
        int clntSocket = accept(socketMain, (sockaddr*)&clntAddress, &clntLen);

        // Check if connection established
        if (clntSocket < 0) {
            if (runStatus) {
                // Terminal logging for failed connection
                std::cerr << "[Failed][Client Connection]" << std::endl;
            }
            continue;
        }

        // Extract Client IP and port for Logging
        char clntIP[INET_ADDRSTRLEN];
        // Internet address converters
        inet_ntop(AF_INET, &clntAddress.sin_addr, clntIP, INET_ADDRSTRLEN);
        int clntPort = ntohs(clntAddress.sin_port);
        
        // Terminal Logging
        std::cout << "[Connection] " << "[" << clntIP << "]" << "[" << clntPort << "]" << std::endl;

        // Push connections into thread vector for parallel processing
        // Allows Multiple connections 
        thread.emplace_back(&WebSrv::clientHandler, this, clntSocket, std::string(clntIP), clntPort);
    }

    for (auto& thread : thread) {
        // Check if process still running
        if (thread.joinable()) {
            // If the process is running, block current thread
            // so threads executed before moving on forward
            thread.join();
        }
    }
}

// Stop server - user exposed
void WebSrv::stop() {
    // Set running status to false;
    runStatus = false;
    // Close socket
    close(socketMain);
    // Terminal logging server stop
    std::cout << "[Server][Stopped]" << std::endl;
}

// Client Handler
// For defining threads required for concurrency
void WebSrv::clientHandler(int clntSocket, const std::string& clntIP, int clntPort) {
    // Character buffer for HTML ingestion
    char buffer[1024];
    // Write to buffer from client socket
    int received = recv(clntSocket, buffer, sizeof(buffer) - 1, 0);
    // Check if request recieved fully
    if (received > 0) {
        // Check if buffer terminated
        buffer[received] = '\0';
        // Terminal Logging request
        std::cout << "[Request]" << "[" << buffer << "]" << std::endl;
    }

    // Save client request
    std::string request(buffer);
    std::string method;
    std::string resource;

    // Interpret the request
    // Find the first empty char and save in methodPos
    size_t methodPos = request.find(' ');
    // If not 'not found', that is request is valid, copy it into method
    if (methodPos != std::string::npos) {
        // Copy substring into method till empty char
        method = request.substr(0, methodPos);
        // After method, finding resource path
        size_t resourceStart = methodPos + 1;
        // Find till empty char found
        size_t resourceEnd = request.find(' ', resourceStart);
        if (resourceEnd != std::string::npos) {
            // Copy substring into resource from resouceEnd
            resource = request.substr(resourceStart, resourceEnd - resourceStart);
        }
    }

    // Handling File Serving
    // Create File object for manuplation
    std::ifstream file(filePath, std::ios::binary);
    std::string response;
    // Reference 
    // 200 (Success/OK) - 301 (Permanent Redirect) - 302 (Temporary Redirect) - 304 (Not Modified)
    // 400 (Bad Request) - 401 (Unauthorized Error) - 403 (Forbidden) - 404 (Not Found)
    // Set to OK from server side
    int responseCode = 200;

    // Check if file exists, serve
    // else, send NOT FOUND
    if (file) {
        // Find size of file
        // ## Can be optimized ##
        file.seekg(0, std::ios::end);
        std::size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // Resize response according to file size
        response.resize(fileSize);
        // Read the file with destination buffer being response[0]
        file.read(&response[0], fileSize);

        // Create HTTP 1.1 Response
        std::string HTTPRes = "HTTP/1.1 200 OK\r\n";
        HTTPRes += "Content-Type: text/html\r\n";
        HTTPRes += "Content-Length: " + std::to_string(fileSize) + "\r\n";
        HTTPRes += "\r\n";
        HTTPRes += response;

        // Serve the created HTTP response to the client socket
        send(clntSocket, HTTPRes.c_str(), HTTPRes.size(), 0);

        // File logging the interaction to log.txt as server logs
        log(clntIP, clntPort, method, resource, responseCode, fileSize);

    } else {
        // Create NOT FOUND HTTP response
        std::string HTTPRes = "HTTP/1.1 404 Not Found\r\n\r\n";
        // Serve the created HTTP response to the client socket
        send(clntSocket, HTTPRes.c_str(), HTTPRes.size(), 0);

        // Set the response code to not found
        responseCode = 404;
        // File logging for server logs
        log(clntIP, clntPort, method, resource, responseCode, 0);
    }
    // Close socket when done serving
    close(clntSocket);
}

// Interupt handler
// Gracefully close server on keyboard or external Interrupts
void WebSrv::interuptHandler(int interupt) {
    // Check for interrupt, if server running, run stop function
    if (interupt == SIGINT && instance) {
        instance->stop();
    }
}

// File Logging Handler
// Logs server activity in log.txt
void WebSrv::log(const std::string& clntIP, int clntPort, const std::string& method, const std::string& resource, int responseCode, std::size_t response_size) {
    // Timestamp
    std::time_t now = std::time(nullptr);
    char timestamp[100];
    // Format Timestamp
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    // Check if log file open, if it is, write the logs
    if (logFile.is_open()) {
        logFile << "[" << timestamp << "][" << clntIP << "][" << clntPort << "][" << method << " " << resource << "][" << responseCode << "][" << response_size << " bytes]" << std::endl;
    }
}

// Helper
// Prints usage and examples on console
void WebSrv::help() {
    std::cout << "Usage: ./server [port] [file]\n";
    std::cout << "Default port: 8080 ";
    std::cout << "Default file: index.html\n";
    std::cout << "./server                 # Uses default port 8080 and default file path index.html\n";
    std::cout << "./server 8000            # Uses port 8000 and default file path index.html\n";
    std::cout << "./server 8000 page.html  # Uses port 8000 and serves page.html\n";
}
