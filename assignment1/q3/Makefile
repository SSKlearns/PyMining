CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall
TARGETS = identify convert generate_candidates

all: $(TARGETS)

identify: identify.cpp graph.h graph_io.h gspan.h subgraph_iso.h
	$(CXX) $(CXXFLAGS) -o identify identify.cpp

convert: convert.cpp graph.h graph_io.h subgraph_iso.h
	$(CXX) $(CXXFLAGS) -o convert convert.cpp

generate_candidates: generate_candidates.cpp
	$(CXX) $(CXXFLAGS) -o generate_candidates generate_candidates.cpp

clean:
	rm -f $(TARGETS)
	rm -f *.o

test: all
	@echo "Testing on small dataset..."
	@echo "1. Identifying fragments..."
	./identify.sh q3_datasets/Mutagenicity/graphs.txt discriminative_subgraphs.txt
	@echo "2. Converting database to features..."
	./convert.sh q3_datasets/Mutagenicity/graphs.txt discriminative_subgraphs.txt db_features.npy
	@echo "3. Converting queries to features..."
	./convert.sh query_dataset/muta_final_visible discriminative_subgraphs.txt query_features.npy
	@echo "4. Generating candidates..."
	./generate_candidates.sh db_features.npy query_features.npy candidates.dat
	@echo "Done!"

.PHONY: all clean test
