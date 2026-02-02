#!/bin/bash

# test_pipeline.sh - Test the complete Q3 pipeline on a small sample

echo "========================================="
echo "Testing Q3 Graph Indexing Pipeline"
echo "========================================="

# Create a small test dataset
echo ""
echo "Step 0: Creating small test dataset..."
head -1000 q3_datasets/Mutagenicity/graphs.txt > test_db.txt
head -200 query_dataset/muta_final_visible > test_queries.txt

# Step 1: Identify discriminative subgraphs
echo ""
echo "Step 1: Identifying discriminative subgraphs..."
./identify.sh test_db.txt test_fragments.txt

if [ $? -ne 0 ]; then
    echo "Error: identify.sh failed!"
    exit 1
fi

echo "Fragments identified. Checking output..."
if [ -f "test_fragments.txt" ]; then
    NUM_FRAGMENTS=$(grep -c "^#" test_fragments.txt)
    echo "Found $NUM_FRAGMENTS fragments"
else
    echo "Error: test_fragments.txt not created!"
    exit 1
fi

# Step 2: Convert database to features
echo ""
echo "Step 2: Converting database graphs to feature vectors..."
./convert.sh test_db.txt test_fragments.txt test_db_features.npy

if [ $? -ne 0 ]; then
    echo "Error: convert.sh failed for database!"
    exit 1
fi

# Step 3: Convert queries to features
echo ""
echo "Step 3: Converting query graphs to feature vectors..."
./convert.sh test_queries.txt test_fragments.txt test_query_features.npy

if [ $? -ne 0 ]; then
    echo "Error: convert.sh failed for queries!"
    exit 1
fi

# Step 4: Generate candidates
echo ""
echo "Step 4: Generating candidate sets..."
./generate_candidates.sh test_db_features.npy test_query_features.npy test_candidates.dat

if [ $? -ne 0 ]; then
    echo "Error: generate_candidates.sh failed!"
    exit 1
fi

echo ""
echo "Checking output format..."
if [ -f "test_candidates.dat" ]; then
    echo "First 20 lines of output:"
    head -20 test_candidates.dat
    
    NUM_QUERIES=$(grep -c "^q #" test_candidates.dat)
    echo ""
    echo "Total queries processed: $NUM_QUERIES"
else
    echo "Error: test_candidates.dat not created!"
    exit 1
fi

echo ""
echo "========================================="
echo "Pipeline test completed successfully!"
echo "========================================="
echo ""
echo "Cleanup test files? (y/n)"
read -r response
if [ "$response" = "y" ]; then
    rm -f test_*.txt test_*.npy test_*.dat
    echo "Test files cleaned up."
fi
