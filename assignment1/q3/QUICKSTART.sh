#!/bin/bash

# Quick Start Guide for Q3

echo "=================================================="
echo "Q3: Graph Indexing - Quick Start Guide"
echo "=================================================="
echo ""

echo "Files created:"
echo "  - graph.h, graph_io.h, subgraph_iso.h, gspan.h (C++ headers)"
echo "  - identify.cpp, convert.cpp, generate_candidates.cpp (C++ programs)"
echo "  - identify.sh, convert.sh, generate_candidates.sh (shell scripts)"
echo "  - Makefile, test_pipeline.sh, README.md, q3.md"
echo ""

echo "=================================================="
echo "QUICK START"
echo "=================================================="
echo ""

echo "1. COMPILE:"
echo "   make"
echo ""

echo "2. FULL PIPELINE (Mutagenicity):"
echo "   # Mine discriminative fragments"
echo "   ./identify.sh q3_datasets/Mutagenicity/graphs.txt discriminative_muta.txt"
echo ""
echo "   # Convert database to features"
echo "   ./convert.sh q3_datasets/Mutagenicity/graphs.txt discriminative_muta.txt db_muta_features.npy"
echo ""
echo "   # Convert queries to features"
echo "   ./convert.sh query_dataset/muta_final_visible discriminative_muta.txt query_muta_features.npy"
echo ""
echo "   # Generate candidates"
echo "   ./generate_candidates.sh db_muta_features.npy query_muta_features.npy candidates_muta.dat"
echo ""

echo "3. FULL PIPELINE (NCI-H23):"
echo "   # Mine discriminative fragments"
echo "   ./identify.sh q3_datasets/NCI-H23/graphs.txt discriminative_nci.txt"
echo ""
echo "   # Convert database to features"
echo "   ./convert.sh q3_datasets/NCI-H23/graphs.txt discriminative_nci.txt db_nci_features.npy"
echo ""
echo "   # Convert queries to features"
echo "   ./convert.sh query_dataset/nci_final_visible discriminative_nci.txt query_nci_features.npy"
echo ""
echo "   # Generate candidates"
echo "   ./generate_candidates.sh db_nci_features.npy query_nci_features.npy candidates_nci.dat"
echo ""

echo "4. TEST ON SMALL SAMPLE:"
echo "   ./test_pipeline.sh"
echo ""

echo "=================================================="
echo "OUTPUT FILES"
echo "=================================================="
echo ""
echo "  discriminative_*.txt  - 50 selected subgraph fragments (graph format)"
echo "  db_*_features.npy     - Database feature matrix (N×50 binary)"
echo "  query_*_features.npy  - Query feature matrix (M×50 binary)"
echo "  candidates_*.dat      - Candidate sets for each query"
echo ""

echo "=================================================="
echo "DOCUMENTATION"
echo "=================================================="
echo ""
echo "  README.md  - Implementation overview"
echo "  q3.md      - Detailed report (convert to PDF for submission)"
echo ""

echo "=================================================="
echo "NOTES"
echo "=================================================="
echo ""
echo "  - Fragment mining may take several minutes"
echo "  - Feature conversion is the slowest step (many isomorphism tests)"
echo "  - Candidate generation is very fast (vectorized filtering)"
echo "  - Output format: 1-indexed graph serial numbers"
echo ""

echo "Ready to run? (y/n)"
read -r response
if [ "$response" = "y" ]; then
    echo ""
    echo "Starting compilation..."
    make
    if [ $? -eq 0 ]; then
        echo ""
        echo "Compilation successful!"
        echo "Run './test_pipeline.sh' to test on small dataset"
    fi
fi
