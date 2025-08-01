#!/bin/sh
# Finder script for AESD assignment
# Author: Assignment Implementation

if [ $# -ne 2 ]; then
    echo "Usage: $0 <directory> <search_string>"
    exit 1
fi

filesdir=$1
searchstr=$2

# Check if directory exists
if [ ! -d "$filesdir" ]; then
    echo "Directory $filesdir does not exist"
    exit 1
fi

# Count files and matching lines
num_files=$(find "$filesdir" -type f | wc -l)
num_matching_lines=$(grep -r "$searchstr" "$filesdir" 2>/dev/null | wc -l)

echo "The number of files are $num_files and the number of matching lines are $num_matching_lines"