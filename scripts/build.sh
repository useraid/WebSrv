#!/bin/bash

# Helper
if [ "$1" == "help" ]
then
    echo "This script builds the server using CMake configuration files."
    echo "If you don't want to use CMake, you can build the server executable directly with:"
    echo "g++ main.cpp WebSrv.cpp -o server -pthread"
    exit 1
fi

if [ "$1" == "build" ]
then
    echo "Starting Build"
fi


# Dependency Checks
if ! command -v cmake &> /dev/null
then
    echo "CMake Not Found"
    exit 1
fi

if ! command -v g++ &> /dev/null
then
    echo "G++ Not Found"
    exit 1
fi

# Project directory
cd ..

# Create build folder
if [ ! -d "build" ]; then
    mkdir build
fi

# Change to working directory
cd build

# Generate Makefile 
cmake ..
if [ $? -ne 0 ]; then
    echo "Makefile failed"
    exit 1
fi

# Build server
cmake --build .
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

echo "Build successfully, run it with ./server"
