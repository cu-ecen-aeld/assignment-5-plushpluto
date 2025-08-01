#!/bin/bash
# Run unit tests for the assignment

# Automate these steps from the readme:
# Create a build subdirectory, change into it, run
# cmake .. && make && run the assignment-autotest application
rm -rf build
mkdir -p build
cd build
cmake ..
make clean
make
cd ..
./build/assignment-autotest/assignment-autotest
