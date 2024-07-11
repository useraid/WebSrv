#ifndef WEBSRV_H
#define WEBSRV_H

#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <ctime>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <signal.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#endif

// Trick to change close function to closesocket for windows compatibility
#ifdef _WIN32
#define close closesocket
#endif

class WebSrv {
public:
    // Init Functions
    WebSrv(int port, const std::string& file);
    ~WebSrv();
    void start();
    void stop();
    static void help();

private:
    // Init Variables
    int srvPort;
    std::string filePath;
    std::atomic<bool> runStatus;
    int socketMain;
    std::vector<std::thread> thread;
    std::ofstream logFile;
    static WebSrv* instance;

    // Init Functions
    void clientHandler(int clntSocket, const std::string& clntIP, int clntPort);
    static void interuptHandler(int interupt);
    void log(const std::string& clntIP, int clntPort, const std::string& method, const std::string& resource, int response_status, std::size_t response_size);
    std::vector<char> getFileContent(const std::string& filePath);
    std::string getContentType(const std::string& filePath);
    bool endsWith(const std::string& str, const std::string& suffix);
};
#endif
