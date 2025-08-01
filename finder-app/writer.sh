#!/bin/sh
# Writer script wrapper for AESD assignment
# This script calls the writer C application

if [ $# -ne 2 ]; then
    echo "Usage: $0 <file> <string>"
    exit 1
fi

writefile=$1
writestr=$2

# Get the directory of the script
SCRIPT_DIR=`dirname "$0"`

# Call the writer C application
"$SCRIPT_DIR/writer" "$writefile" "$writestr"