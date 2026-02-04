#!/bin/bash
set -e
echo "Starting"
python3 generate_candidates.py "$1" "$2" "$3"