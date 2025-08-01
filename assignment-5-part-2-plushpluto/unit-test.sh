#!/bin/bash
# Script to run unit tests
set -e

# Get the directory of the script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Ensure build directory exists
mkdir -p "$SCRIPT_DIR/build"

# Change to the build directory
cd "$SCRIPT_DIR/build"

# Remove existing CMakeCache.txt to avoid path conflicts
rm -f CMakeCache.txt

# Run CMake to generate build files, using the script's directory as source
cmake "$SCRIPT_DIR"

# Build the project
make

# Run the assignment-autotest executable
./assignment-autotest/assignment-autotest