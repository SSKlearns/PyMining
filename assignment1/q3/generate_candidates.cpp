#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <climits>

// Simple .npy reader for 2D boolean array
std::vector<std::vector<uint8_t>> read_npy_bool_2d(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    
    if (!in.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return {};
    }
    
    // Read and verify magic number
    char magic[6];
    in.read(magic, 6);
    if (std::strncmp(magic, "\x93NUMPY", 6) != 0) {
        std::cerr << "Error: Invalid NPY file format" << std::endl;
        return {};
    }
    
    // Read version
    uint8_t major, minor;
    in.read(reinterpret_cast<char*>(&major), 1);
    in.read(reinterpret_cast<char*>(&minor), 1);
    
    // Read header length
    uint16_t header_len;
    in.read(reinterpret_cast<char*>(&header_len), 2);
    
    // Read header
    std::vector<char> header(header_len);
    in.read(header.data(), header_len);
    std::string header_str(header.begin(), header.end());
    
    // Parse shape from header (simple parsing)
    size_t shape_pos = header_str.find("'shape': (");
    if (shape_pos == std::string::npos) {
        std::cerr << "Error: Cannot parse shape from header" << std::endl;
        return {};
    }
    
    size_t start = shape_pos + 10;
    size_t comma = header_str.find(',', start);
    size_t end = header_str.find(')', comma);
    
    size_t rows = std::stoul(header_str.substr(start, comma - start));
    size_t cols = std::stoul(header_str.substr(comma + 1, end - comma - 1));
    
    // Read data
    std::vector<std::vector<uint8_t>> data(rows, std::vector<uint8_t>(cols));
    for (size_t i = 0; i < rows; i++) {
        in.read(reinterpret_cast<char*>(data[i].data()), cols);
    }
    
    in.close();
    return data;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <db_features.npy> <query_features.npy> <path_out_file>" << std::endl;
        return 1;
    }
    
    std::string db_features_path = argv[1];
    std::string query_features_path = argv[2];
    std::string output_path = argv[3];
    
    std::cout << "Loading database features from " << db_features_path << "..." << std::endl;
    auto db_features = read_npy_bool_2d(db_features_path);
    std::cout << "Database features shape: (" << db_features.size() << ", " 
              << (db_features.empty() ? 0 : db_features[0].size()) << ")" << std::endl;
    
    std::cout << "Loading query features from " << query_features_path << "..." << std::endl;
    auto query_features = read_npy_bool_2d(query_features_path);
    std::cout << "Query features shape: (" << query_features.size() << ", " 
              << (query_features.empty() ? 0 : query_features[0].size()) << ")" << std::endl;
    
    size_t num_features = db_features.empty() ? 0 : db_features[0].size();
    
    // Validation
    if (num_features == 0) {
        std::cerr << "ERROR: Feature vectors have 0 dimensions! Check convert.sh output." << std::endl;
        return 1;
    }
    
    if (num_features != 50) {
        std::cerr << "WARNING: Expected 50 features, got " << num_features << std::endl;
    }
    
    if (!query_features.empty() && query_features[0].size() != num_features) {
        std::cerr << "ERROR: Database and query feature dimensions don't match!" << std::endl;
        return 1;
    }
    
    std::ofstream out(output_path);
    if (!out.is_open()) {
        std::cerr << "Error: Cannot open output file " << output_path << std::endl;
        return 1;
    }
    
    std::cout << "\nGenerating candidates..." << std::endl;
    
    int total_candidates = 0;
    int min_candidates = INT_MAX;
    int max_candidates = 0;
    
    for (size_t q = 0; q < query_features.size(); q++) {
        out << "q # " << (q + 1) << "\n";
        out << "c # ";
        
        std::vector<int> candidates;
        
        // For each database graph
        for (size_t i = 0; i < db_features.size(); i++) {
            // Check if query features are subset of database features
            // If query has feature j=1, then database must also have j=1
            bool is_candidate = true;
            
            for (size_t j = 0; j < num_features; j++) {
                if (query_features[q][j] == 1 && db_features[i][j] == 0) {
                    is_candidate = false;
                    break;
                }
            }
            
            if (is_candidate) {
                candidates.push_back(i + 1);  // 1-indexed
            }
        }
        
        // Statistics
        total_candidates += candidates.size();
        if ((int)candidates.size() < min_candidates) min_candidates = candidates.size();
        if ((int)candidates.size() > max_candidates) max_candidates = candidates.size();
        
        // Write candidates
        for (size_t k = 0; k < candidates.size(); k++) {
            if (k > 0) out << " ";
            out << candidates[k];
        }
        out << "\n";
        
        if ((q + 1) % 10 == 0) {
            std::cout << "Processed " << (q + 1) << "/" << query_features.size() << " queries" << std::endl;
        }
    }
    
    out.close();
    
    double avg_candidates = query_features.size() > 0 ? 
        (double)total_candidates / query_features.size() : 0;
    
    std::cout << "\nDone! Candidates written to " << output_path << std::endl;
    std::cout << "Statistics:" << std::endl;
    std::cout << "  Average candidate set size: " << avg_candidates << std::endl;
    std::cout << "  Min: " << min_candidates << ", Max: " << max_candidates << std::endl;
    std::cout << "  Database size: " << db_features.size() << std::endl;
    
    // Sanity check
    if (min_candidates == (int)db_features.size() && max_candidates == (int)db_features.size()) {
        std::cerr << "\nWARNING: All queries return the entire database as candidates!" << std::endl;
        std::cerr << "This suggests fragments are not discriminative (possibly all 0 features)." << std::endl;
        return 1;
    }
    
    return 0;
}
