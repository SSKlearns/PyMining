#!/bin/bash

# identify.sh - Mine and select discriminative subgraphs
# Usage: ./identify.sh <path_graph_dataset> <path_discriminative_subgraphs>

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <path_graph_dataset> <path_discriminative_subgraphs>"
    exit 1
fi

DATASET_PATH="$1"
OUTPUT_PATH="$2"

# Compile if needed
if [ ! -f "./identify" ] || [ "identify.cpp" -nt "./identify" ]; then
    echo "Compiling identify.cpp..."
    g++ -std=c++17 -O3 -o identify identify.cpp
    if [ $? -ne 0 ]; then
        echo "Compilation failed!"
        exit 1
    fi
fi

# Run the identification
echo "Running fragment identification..."
./identify "$DATASET_PATH" "$OUTPUT_PATH"

exit $?
