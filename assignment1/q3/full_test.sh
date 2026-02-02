#!/bin/bash

# Full integration test for Q3

echo "========================================="
echo "Q3 Full Integration Test"
echo "========================================="
echo ""

# Clean previous outputs
rm -f test_*.txt test_*.npy test_*.dat 2>/dev/null

# Step 1: Create small test dataset
echo "Step 1: Creating test dataset (first 500 lines)..."
head -500 q3_datasets/Mutagenicity/graphs.txt > test_db.txt
head -100 query_dataset/muta_final_visible > test_queries.txt
echo "✓ Test dataset created"
echo ""

# Step 2: Identify fragments
echo "Step 2: Identifying discriminative subgraphs..."
./identify.sh test_db.txt test_fragments.txt
if [ $? -ne 0 ]; then
    echo "✗ identify.sh failed!"
    exit 1
fi

FRAG_COUNT=$(grep -c '^#' test_fragments.txt)
echo "✓ Created $FRAG_COUNT fragments"

if [ "$FRAG_COUNT" -ne 50 ]; then
    echo "✗ ERROR: Expected 50 fragments, got $FRAG_COUNT"
    exit 1
fi
echo ""

# Step 3: Convert database to features
echo "Step 3: Converting database to features..."
./convert.sh test_db.txt test_fragments.txt test_db.npy
if [ $? -ne 0 ]; then
    echo "✗ convert.sh failed for database!"
    exit 1
fi
echo "✓ Database features created"
echo ""

# Step 4: Convert queries to features
echo "Step 4: Converting queries to features..."
./convert.sh test_queries.txt test_fragments.txt test_queries.npy
if [ $? -ne 0 ]; then
    echo "✗ convert.sh failed for queries!"
    exit 1
fi
echo "✓ Query features created"
echo ""

# Step 5: Generate candidates
echo "Step 5: Generating candidates..."
./generate_candidates.sh test_db.npy test_queries.npy test_candidates.dat
if [ $? -ne 0 ]; then
    echo "✗ generate_candidates.sh failed!"
    exit 1
fi
echo "✓ Candidates generated"
echo ""

# Step 6: Run correctness checks
echo "Step 6: Running correctness checks..."
echo ""

# Rename files for correctness checker
cp test_fragments.txt discriminative_subgraphs.txt
cp test_db.npy db_features.npy
cp test_queries.npy query_features.npy
cp test_candidates.dat candidates.dat

./correctness_check.sh
CHECK_RESULT=$?

# Clean up temporary renamed files
rm -f discriminative_subgraphs.txt db_features.npy query_features.npy candidates.dat

if [ $CHECK_RESULT -ne 0 ]; then
    echo "✗ Correctness checks failed!"
    exit 1
fi

echo ""
echo "========================================="
echo "ALL TESTS PASSED! ✓✓✓"
echo "========================================="
echo ""
echo "Summary:"
echo "  - Fragments: $FRAG_COUNT (exactly 50)"
echo "  - Database graphs converted to features"
echo "  - Query graphs converted to features"
echo "  - Candidates generated with filtering"
echo "  - All correctness checks passed"
echo ""
echo "You can now run on full datasets:"
echo "  ./identify.sh q3_datasets/Mutagenicity/graphs.txt muta_fragments.txt"
echo "  ./convert.sh q3_datasets/Mutagenicity/graphs.txt muta_fragments.txt muta_db.npy"
echo "  ./convert.sh query_dataset/muta_final_visible muta_fragments.txt muta_query.npy"
echo "  ./generate_candidates.sh muta_db.npy muta_query.npy muta_candidates.dat"
echo ""
