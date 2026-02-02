#!/bin/bash

# correctness_check.sh - Verify no false negatives on small subset

echo "========================================="
echo "Q3 Correctness Checker"
echo "========================================="
echo ""

# Compile if needed
if [ ! -f "./identify" ]; then
    echo "Compiling programs..."
    make
    if [ $? -ne 0 ]; then
        echo "Compilation failed!"
        exit 1
    fi
fi

# Check fragment file
if [ ! -f "discriminative_subgraphs.txt" ]; then
    echo "ERROR: discriminative_subgraphs.txt not found!"
    echo "Run ./identify.sh first"
    exit 1
fi

FRAG_COUNT=$(grep -c '^#' discriminative_subgraphs.txt)
echo "1. Fragment Count Check:"
echo "   Found: $FRAG_COUNT fragments"
if [ "$FRAG_COUNT" -eq 50 ]; then
    echo "   ✓ Correct (exactly 50)"
else
    echo "   ✗ ERROR: Expected 50, got $FRAG_COUNT"
    exit 1
fi
echo ""

# Check feature file shapes using Python
echo "2. Feature Matrix Shape Check:"

python3 << 'PYCHECK'
import sys
try:
    import numpy as np
    
    try:
        db = np.load("db_features.npy")
        print(f"   Database features: {db.shape}")
        if db.shape[1] == 50:
            print("   ✓ Database has 50 features")
        else:
            print(f"   ✗ ERROR: Database has {db.shape[1]} features, expected 50")
            sys.exit(1)
    except FileNotFoundError:
        print("   ! db_features.npy not found (run convert.sh first)")
        sys.exit(1)
    
    try:
        query = np.load("query_features.npy")
        print(f"   Query features: {query.shape}")
        if query.shape[1] == 50:
            print("   ✓ Query has 50 features")
        else:
            print(f"   ✗ ERROR: Query has {query.shape[1]} features, expected 50")
            sys.exit(1)
    except FileNotFoundError:
        print("   ! query_features.npy not found (run convert.sh first)")
        sys.exit(1)
        
except ImportError:
    print("   ! NumPy not available, skipping shape check")
    print("   Install numpy: pip install numpy")
PYCHECK

if [ $? -ne 0 ]; then
    exit 1
fi

echo ""

# Check candidates file
if [ ! -f "candidates.dat" ]; then
    echo "ERROR: candidates.dat not found!"
    echo "Run ./generate_candidates.sh first"
    exit 1
fi

echo "3. Candidate File Format Check:"
QUERY_COUNT=$(grep -c '^q #' candidates.dat)
CAND_COUNT=$(grep -c '^c #' candidates.dat)
echo "   Query lines: $QUERY_COUNT"
echo "   Candidate lines: $CAND_COUNT"

if [ "$QUERY_COUNT" -eq "$CAND_COUNT" ]; then
    echo "   ✓ Format correct (matching q/c pairs)"
else
    echo "   ✗ ERROR: Mismatched query/candidate counts"
    exit 1
fi

echo ""

# Check that not all candidates are full database
echo "4. Candidate Set Size Check:"
FIRST_CAND=$(head -2 candidates.dat | tail -1)
CAND_SIZE=$(echo $FIRST_CAND | wc -w)
CAND_SIZE=$((CAND_SIZE - 2))  # Remove "c #"

echo "   First query has $CAND_SIZE candidates"

# Get total database size from npy
python3 << 'PYCHECK2'
try:
    import numpy as np
    db = np.load("db_features.npy")
    print(f"   Database size: {db.shape[0]}")
    
    # Simple heuristic: if all queries return full DB, something is wrong
    import re
    with open("candidates.dat") as f:
        lines = f.readlines()
    
    cand_sizes = []
    for line in lines:
        if line.startswith("c #"):
            parts = line.strip().split()[2:]  # Skip "c #"
            cand_sizes.append(len(parts))
    
    if cand_sizes:
        avg_size = sum(cand_sizes) / len(cand_sizes)
        max_size = max(cand_sizes)
        min_size = min(cand_sizes)
        
        print(f"   Candidate set sizes: min={min_size}, max={max_size}, avg={avg_size:.1f}")
        
        if min_size == db.shape[0] and max_size == db.shape[0]:
            print("   ✗ ERROR: All queries return entire database!")
            print("   This means features are not discriminative.")
            exit(1)
        else:
            print("   ✓ Candidate sets are filtered (not returning entire DB)")
            
            if avg_size < db.shape[0] * 0.5:
                print("   ✓ Good filtering: average candidate set < 50% of database")
            else:
                print("   ! Warning: average candidate set is large (>50% of database)")
                print("   Consider improving fragment selection")
                
except Exception as e:
    print(f"   ! Check failed: {e}")
    exit(1)
PYCHECK2

if [ $? -ne 0 ]; then
    exit 1
fi

echo ""
echo "========================================="
echo "All basic checks passed! ✓"
echo "========================================="
echo ""
echo "Note: This verifies format and basic sanity."
echo "For full correctness (no false negatives), run"
echo "brute-force spot-check on a small subset."
