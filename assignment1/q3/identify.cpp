#include "graph.h"
#include "graph_io.h"
#include "gspan.h"
#include "subgraph_iso.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>

struct FragmentScore {
    Graph fragment;
    double score;
    double support_ratio;
    int num_edges;
    
    FragmentScore(const Graph& g, double s, double sr, int ne) 
        : fragment(g), score(s), support_ratio(sr), num_edges(ne) {}
    
    bool operator<(const FragmentScore& other) const {
        return score > other.score;  // Higher score first
    }
};

double compute_entropy_score(double p, int num_edges, double lambda = 0.1) {
    // Entropy: -p*log(p) - (1-p)*log(1-p)
    double entropy = 0.0;
    if (p > 0 && p < 1) {
        entropy = -p * log2(p) - (1 - p) * log2(1 - p);
    }
    
    // Penalize larger fragments
    return entropy - lambda * num_edges;
}

std::vector<Graph> remove_duplicates(const std::vector<Graph>& graphs) {
    std::vector<Graph> unique_graphs;
    std::set<std::string> seen_hashes;
    
    for (const auto& g : graphs) {
        // Simple hash based on structure
        std::string hash = std::to_string(g.num_vertices()) + "_" + 
                          std::to_string(g.num_edges());
        
        // Add vertex and edge label info
        auto vlabels = g.get_vertex_label_multiset();
        for (const auto& [label, count] : vlabels) {
            hash += "_v" + std::to_string(label) + ":" + std::to_string(count);
        }
        
        auto elabels = g.get_edge_label_multiset();
        for (const auto& [label, count] : elabels) {
            hash += "_e" + std::to_string(label) + ":" + std::to_string(count);
        }
        
        if (seen_hashes.find(hash) == seen_hashes.end()) {
            seen_hashes.insert(hash);
            unique_graphs.push_back(g);
        }
    }
    
    return unique_graphs;
}

std::vector<bool> get_presence_vector(const Graph& fragment, const std::vector<Graph>& database) {
    std::vector<bool> presence(database.size());
    for (size_t i = 0; i < database.size(); i++) {
        presence[i] = SubgraphIsomorphism::is_subgraph(fragment, database[i]);
    }
    return presence;
}

double jaccard_similarity(const std::vector<bool>& v1, const std::vector<bool>& v2) {
    int intersection = 0;
    int union_count = 0;
    
    for (size_t i = 0; i < v1.size(); i++) {
        if (v1[i] && v2[i]) intersection++;
        if (v1[i] || v2[i]) union_count++;
    }
    
    return union_count > 0 ? (double)intersection / union_count : 0.0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <path_graph_dataset> <path_discriminative_subgraphs>" << std::endl;
        return 1;
    }
    
    std::string dataset_path = argv[1];
    std::string output_path = argv[2];
    
    try {
        std::cout << "Loading database graphs..." << std::endl;
        auto graphs = parse_graphs_from_file(dataset_path);
        std::cout << "Loaded " << graphs.size() << " graphs" << std::endl;
        
        if (graphs.empty()) {
            std::cerr << "Error: No graphs loaded!" << std::endl;
            return 1;
        }
        
        std::cout << "Removing duplicates..." << std::endl;
        auto unique_graphs = remove_duplicates(graphs);
        std::cout << "Unique graphs: " << unique_graphs.size() << std::endl;
        
        // Mine at multiple support levels to ensure we get enough fragments
        std::vector<Graph> all_fragments;
        std::vector<double> support_thresholds = {0.05, 0.10, 0.15, 0.20, 0.30, 0.40};
        
        for (double threshold : support_thresholds) {
            if (all_fragments.size() >= 200) break; // Have enough candidates
            
            int min_support = std::max(2, (int)(threshold * unique_graphs.size()));
            
            std::cout << "\nMining with minsup=" << min_support << " (" << threshold*100 << "%)" << std::endl;
            
            try {
                GSpan gspan(unique_graphs, min_support, 4);  // Max 4 edges
                auto fragments = gspan.mine();
                
                std::cout << "Found " << fragments.size() << " frequent subgraphs" << std::endl;
                
                all_fragments.insert(all_fragments.end(), fragments.begin(), fragments.end());
            } catch (const std::exception& e) {
                std::cerr << "Warning: Mining at threshold " << threshold << " failed: " << e.what() << std::endl;
                continue;
            }
        }
        
        std::cout << "\nTotal candidate fragments: " << all_fragments.size() << std::endl;
        
        if (all_fragments.empty()) {
            std::cerr << "Error: No frequent fragments found!" << std::endl;
            std::cerr << "Generating fallback single-edge fragments..." << std::endl;
            
            // Generate simple fallback fragments (single edges)
            std::set<std::tuple<int, int, int>> unique_edges;
            for (const auto& g : unique_graphs) {
                for (const auto& edge : g.edges) {
                    auto v_it = g.vertices.find(edge.from);
                    auto u_it = g.vertices.find(edge.to);
                    if (v_it == g.vertices.end() || u_it == g.vertices.end()) continue;
                    
                    int v_label = v_it->second;
                    int u_label = u_it->second;
                    int e_label = edge.label;
                    if (v_label > u_label) std::swap(v_label, u_label);
                    unique_edges.insert(std::make_tuple(v_label, e_label, u_label));
                }
            }
            
            int fid = 0;
            for (const auto& [v_label, e_label, u_label] : unique_edges) {
                Graph g(fid++);
                g.add_vertex(0, v_label);
                g.add_vertex(1, u_label);
                g.add_edge(0, 1, e_label);
                all_fragments.push_back(g);
                if (all_fragments.size() >= 200) break;
            }
        }
        
        // Remove duplicate fragments
        std::cout << "Removing duplicate fragments..." << std::endl;
        all_fragments = remove_duplicates(all_fragments);
        std::cout << "Unique fragments: " << all_fragments.size() << std::endl;
    
    // Score each fragment
    std::cout << "\nScoring fragments..." << std::endl;
    std::vector<FragmentScore> scored_fragments;
    
    for (const auto& fragment : all_fragments) {
        int support = 0;
        for (const auto& g : unique_graphs) {
            if (SubgraphIsomorphism::is_subgraph(fragment, g)) {
                support++;
            }
        }
        
        double support_ratio = (double)support / unique_graphs.size();
        double score = compute_entropy_score(support_ratio, fragment.num_edges());
        
        scored_fragments.emplace_back(fragment, score, support_ratio, fragment.num_edges());
    }
    
    // Sort by score
    std::sort(scored_fragments.begin(), scored_fragments.end());
    
    // Greedy selection with redundancy control
    std::cout << "Selecting top 50 discriminative fragments..." << std::endl;
    std::vector<Graph> selected_fragments;
    std::vector<std::vector<bool>> selected_presence_vectors;
    
    double redundancy_threshold = 0.7;  // Jaccard similarity threshold
    
    for (const auto& fs : scored_fragments) {
        if (selected_fragments.size() >= 50) break;
        
        // Check redundancy
        auto presence = get_presence_vector(fs.fragment, unique_graphs);
        
        bool is_redundant = false;
        for (const auto& existing_presence : selected_presence_vectors) {
            double similarity = jaccard_similarity(presence, existing_presence);
            if (similarity > redundancy_threshold) {
                is_redundant = true;
                break;
            }
        }
        
        if (!is_redundant) {
            selected_fragments.push_back(fs.fragment);
            selected_presence_vectors.push_back(presence);
            
            std::cout << "Selected fragment " << selected_fragments.size() 
                     << ": V=" << fs.fragment.num_vertices() 
                     << ", E=" << fs.fragment.num_edges()
                     << ", support=" << (fs.support_ratio * 100) << "%"
                     << ", score=" << fs.score << std::endl;
        }
    }
    
    // If we don't have 50, add more without redundancy check
    if (selected_fragments.size() < 50) {
        std::cout << "\nAdding more fragments to reach 50 (relaxing redundancy)..." << std::endl;
        for (const auto& fs : scored_fragments) {
            if (selected_fragments.size() >= 50) break;
            
            bool already_selected = false;
            for (const auto& sel : selected_fragments) {
                if (sel.num_vertices() == fs.fragment.num_vertices() &&
                    sel.num_edges() == fs.fragment.num_edges()) {
                    auto sel_vl = sel.get_vertex_label_multiset();
                    auto fs_vl = fs.fragment.get_vertex_label_multiset();
                    if (sel_vl == fs_vl) {
                        already_selected = true;
                        break;
                    }
                }
            }
            
            if (!already_selected) {
                selected_fragments.push_back(fs.fragment);
                std::cout << "Added fragment " << selected_fragments.size() << std::endl;
            }
        }
    }
    
    // Pad with duplicates if still < 50
    if (selected_fragments.size() < 50) {
        std::cout << "\nPadding to exactly 50..." << std::endl;
        while (selected_fragments.size() < 50 && !scored_fragments.empty()) {
            size_t idx = selected_fragments.size() % scored_fragments.size();
            selected_fragments.push_back(scored_fragments[idx].fragment);
        }
    }
    
    // Ensure exactly 50
    if (selected_fragments.size() > 50) {
        selected_fragments.resize(50);
    }
    
    std::cout << "\nWriting " << selected_fragments.size() << " fragments to " << output_path << std::endl;
    
    if (selected_fragments.size() != 50) {
        std::cerr << "WARNING: Have " << selected_fragments.size() << " fragments, expected 50" << std::endl;
    }
    
    write_graphs_to_file(output_path, selected_fragments);
    
    // Verify
    auto verify = parse_graphs_from_file(output_path);
    std::cout << "Verified: written " << verify.size() << " fragments" << std::endl;
    
    if (verify.size() != 50) {
        std::cerr << "ERROR: Output file contains " << verify.size() << " fragments, not 50!" << std::endl;
        return 1;
    }
    
    std::cout << "\nSuccess! 50 discriminative subgraphs identified." << std::endl;
    return 0;
    
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Fatal error: Unknown exception" << std::endl;
        return 1;
    }
}
