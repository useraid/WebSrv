# WebSrv
A simple HTTP server library written in C++

## Features
- Concurrency - Handle mutiple connections at once
- Logging - Console and File based Detailed Logging
- Standalone Mode - Compile and Run the binary as is to use as a simple HTTP server
- Lightweight - Low resource usage

## Quick Start
### Using as library
- Include Library
```
#include "WebSrv.h"
```
- Initialize server object
```
WebSrv server(8080, index.html);
```
- Start the server
```
server.start();
```
- Stop the server
```
server.stop();
```
### Using Standalone
- Compile and run the executable
For linux
```
./websrv
```
For Windows
```
websrv.exe
```
- Set custom port (Default: 8080)
```
./websrv 8000
```
- Serve custom pages (Default index.html)
```
./websrv 8000 page.html
```
- Help menu
```
./websrv help
```
- Add Custom arguments by modifying `argc` inputs

## Precompiled Binaries - Standalone mode
[Linux](https://github.com/useraid/WebSrv/releases/tag/v0.1)
