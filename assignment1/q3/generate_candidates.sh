#!/bin/bash

# generate_candidates.sh - Generate candidate sets for queries
# Usage: ./generate_candidates.sh <db_features.npy> <query_features.npy> <path_out_file>

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <db_features.npy> <query_features.npy> <path_out_file>"
    exit 1
fi

DB_FEATURES="$1"
QUERY_FEATURES="$2"
OUTPUT_PATH="$3"

# Compile if needed
if [ ! -f "./generate_candidates" ] || [ "generate_candidates.cpp" -nt "./generate_candidates" ]; then
    echo "Compiling generate_candidates.cpp..."
    g++ -std=c++17 -O3 -o generate_candidates generate_candidates.cpp
    if [ $? -ne 0 ]; then
        echo "Compilation failed!"
        exit 1
    fi
fi

# Run candidate generation
echo "Running candidate generation..."
./generate_candidates "$DB_FEATURES" "$QUERY_FEATURES" "$OUTPUT_PATH"

exit $?
