#!/bin/bash

# convert.sh - Convert graphs to feature vectors
# Usage: ./convert.sh <path_graphs> <path_discriminative_subgraphs> <path_features>

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <path_graphs> <path_discriminative_subgraphs> <path_features>"
    exit 1
fi

GRAPHS_PATH="$1"
FRAGMENTS_PATH="$2"
OUTPUT_PATH="$3"

# Compile if needed
if [ ! -f "./convert" ] || [ "convert.cpp" -nt "./convert" ]; then
    echo "Compiling convert.cpp..."
    g++ -std=c++17 -O3 -o convert convert.cpp
    if [ $? -ne 0 ]; then
        echo "Compilation failed!"
        exit 1
    fi
fi

# Run the conversion
echo "Running feature extraction..."
./convert "$GRAPHS_PATH" "$FRAGMENTS_PATH" "$OUTPUT_PATH"

exit $?
