#include "graph.h"
#include "graph_io.h"
#include "subgraph_iso.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

// Simple .npy writer for 2D boolean array
void write_npy_bool_2d(const std::string& filename, 
                       const std::vector<std::vector<uint8_t>>& data) {
    std::ofstream out(filename, std::ios::binary);
    
    if (!out.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }
    
    // NPY magic number and version
    out.write("\x93NUMPY", 6);
    out.put(0x01);  // major version
    out.put(0x00);  // minor version
    
    // Create header
    size_t rows = data.size();
    size_t cols = data.empty() ? 0 : data[0].size();
    
    std::string header = "{'descr': '|u1', 'fortran_order': False, 'shape': (" + 
                        std::to_string(rows) + ", " + std::to_string(cols) + "), }";
    
    // Pad header to make total header size (including length) divisible by 64
    size_t header_len = header.size();
    size_t padding = (64 - (10 + header_len) % 64) % 64;
    header += std::string(padding, ' ');
    header += '\n';
    
    // Write header length (2 bytes, little endian)
    uint16_t hlen = header.size();
    out.write(reinterpret_cast<char*>(&hlen), 2);
    
    // Write header
    out.write(header.c_str(), header.size());
    
    // Write data row by row
    for (const auto& row : data) {
        out.write(reinterpret_cast<const char*>(row.data()), row.size());
    }
    
    out.close();
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <path_graphs> <path_discriminative_subgraphs> <path_features>" << std::endl;
        return 1;
    }
    
    std::string graphs_path = argv[1];
    std::string fragments_path = argv[2];
    std::string output_path = argv[3];
    
    std::cout << "Loading graphs from " << graphs_path << "..." << std::endl;
    auto graphs = parse_graphs_from_file(graphs_path);
    std::cout << "Loaded " << graphs.size() << " graphs" << std::endl;
    
    std::cout << "Loading discriminative subgraphs from " << fragments_path << "..." << std::endl;
    auto fragments = parse_graphs_from_file(fragments_path);
    std::cout << "Loaded " << fragments.size() << " fragments" << std::endl;
    
    if (fragments.size() == 0) {
        std::cerr << "ERROR: No fragments loaded! Check fragments file." << std::endl;
        return 1;
    }
    
    if (fragments.size() != 50) {
        std::cerr << "WARNING: Expected 50 fragments, got " << fragments.size() << std::endl;
    }
    
    // Create feature matrix
    std::cout << "\nGenerating feature vectors..." << std::endl;
    std::vector<std::vector<uint8_t>> features(graphs.size(), 
                                                std::vector<uint8_t>(fragments.size(), 0));
    
    int total_features = 0;
    
    for (size_t i = 0; i < graphs.size(); i++) {
        if (i % 100 == 0 || i == graphs.size() - 1) {
            std::cout << "Processing graph " << i << "/" << graphs.size() 
                     << " (" << (100 * i / graphs.size()) << "%)" << std::endl;
        }
        
        for (size_t j = 0; j < fragments.size(); j++) {
            // Check if fragment is subgraph of graph
            bool is_sub = SubgraphIsomorphism::is_subgraph(fragments[j], graphs[i]);
            features[i][j] = is_sub ? 1 : 0;
            if (is_sub) total_features++;
        }
    }
    
    std::cout << "\nWriting features to " << output_path << "..." << std::endl;
    write_npy_bool_2d(output_path, features);
    
    std::cout << "Done! Feature matrix shape: (" << graphs.size() << ", " << fragments.size() << ")" << std::endl;
    std::cout << "Total features set to 1: " << total_features << " out of " << (graphs.size() * fragments.size()) << std::endl;
    
    // Verify it's not empty
    if (fragments.size() == 0) {
        std::cerr << "ERROR: Feature matrix has 0 columns (no fragments)!" << std::endl;
        return 1;
    }
    
    if (total_features == 0) {
        std::cerr << "WARNING: All features are 0! No fragments found in any graph." << std::endl;
        std::cerr << "This will cause all queries to return the entire database." << std::endl;
    }
    
    return 0;
}
