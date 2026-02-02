#ifndef GSPAN_H
#define GSPAN_H

#include "graph.h"
#include "subgraph_iso.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <queue>

// Simplified frequent pattern mining - generates small connected subgraphs
class SimpleMiner {
private:
    std::vector<Graph> database;
    int min_support;
    int max_edges;
    std::vector<Graph> frequent_subgraphs;
    
    int count_support(const Graph& pattern) {
        int support = 0;
        for (const auto& g : database) {
            if (SubgraphIsomorphism::is_subgraph(pattern, g)) {
                support++;
            }
        }
        return support;
    }
    
    // Generate all single-edge patterns from database
    std::vector<Graph> generate_single_edge_patterns() {
        std::set<std::tuple<int, int, int>> unique_patterns;
        
        for (const auto& g : database) {
            for (const auto& edge : g.edges) {
                // Safely get labels
                auto vit = g.vertices.find(edge.from);
                auto uit = g.vertices.find(edge.to);
                
                if (vit == g.vertices.end() || uit == g.vertices.end()) {
                    continue; // Skip invalid edges
                }
                
                int v_label = vit->second;
                int u_label = uit->second;
                int e_label = edge.label;
                
                // Canonical form: smaller label first
                if (v_label > u_label) {
                    std::swap(v_label, u_label);
                }
                
                unique_patterns.insert(std::make_tuple(v_label, e_label, u_label));
            }
        }
        
        std::vector<Graph> patterns;
        int pattern_id = 0;
        for (const auto& [v_label, e_label, u_label] : unique_patterns) {
            Graph pattern(pattern_id++);
            pattern.add_vertex(0, v_label);
            pattern.add_vertex(1, u_label);
            pattern.add_edge(0, 1, e_label);
            
            int support = count_support(pattern);
            if (support >= min_support) {
                patterns.push_back(pattern);
            }
        }
        
        return patterns;
    }
    
    // Generate two-edge patterns (simplified and safe)
    std::vector<Graph> generate_two_edge_patterns() {
        std::vector<Graph> patterns;
        
        // Collect label statistics from database safely
        std::set<int> vertex_labels, edge_labels;
        for (const auto& g : database) {
            for (const auto& [vid, label] : g.vertices) {
                vertex_labels.insert(label);
            }
            for (const auto& e : g.edges) {
                edge_labels.insert(e.label);
            }
        }
        
        // Limit to small label sets for efficiency
        std::vector<int> vlabels(vertex_labels.begin(), vertex_labels.end());
        std::vector<int> elabels(edge_labels.begin(), edge_labels.end());
        
        if (vlabels.size() > 10) vlabels.resize(10);
        if (elabels.size() > 5) elabels.resize(5);
        
        int pattern_id = 0;
        int checked = 0;
        int max_checks = 500; // Limit total patterns to try
        
        // Generate simple paths: v0-e0-v1-e1-v2
        for (size_t i = 0; i < vlabels.size() && checked < max_checks; i++) {
            for (size_t j = 0; j < vlabels.size() && checked < max_checks; j++) {
                for (size_t k = 0; k < vlabels.size() && checked < max_checks; k++) {
                    for (size_t ei = 0; ei < elabels.size() && checked < max_checks; ei++) {
                        for (size_t ej = 0; ej < elabels.size() && checked < max_checks; ej++) {
                            checked++;
                            
                            Graph pattern(pattern_id++);
                            pattern.add_vertex(0, vlabels[i]);
                            pattern.add_vertex(1, vlabels[j]);
                            pattern.add_vertex(2, vlabels[k]);
                            pattern.add_edge(0, 1, elabels[ei]);
                            pattern.add_edge(1, 2, elabels[ej]);
                            
                            try {
                                int support = count_support(pattern);
                                if (support >= min_support) {
                                    patterns.push_back(pattern);
                                    if (patterns.size() >= 100) return patterns; // Enough
                                }
                            } catch (...) {
                                // Skip patterns that cause errors
                                continue;
                            }
                        }
                    }
                }
            }
        }
        
        return patterns;
    }
    
public:
    SimpleMiner(const std::vector<Graph>& db, int minsup, int max_e = 4) 
        : database(db), min_support(minsup), max_edges(max_e) {}
    
    std::vector<Graph> mine() {
        frequent_subgraphs.clear();
        
        std::cout << "  Mining 1-edge patterns..." << std::endl;
        auto edge_patterns = generate_single_edge_patterns();
        std::cout << "  Found " << edge_patterns.size() << " frequent 1-edge patterns" << std::endl;
        
        for (auto& p : edge_patterns) {
            frequent_subgraphs.push_back(p);
        }
        
        if (max_edges >= 2 && frequent_subgraphs.size() < 100) {
            std::cout << "  Mining 2-edge patterns..." << std::endl;
            auto two_edge_patterns = generate_two_edge_patterns();
            std::cout << "  Found " << two_edge_patterns.size() << " frequent 2-edge patterns" << std::endl;
            
            for (auto& p : two_edge_patterns) {
                frequent_subgraphs.push_back(p);
                if (frequent_subgraphs.size() >= 300) break;
            }
        }
        
        return frequent_subgraphs;
    }
    
    const std::vector<Graph>& get_frequent_subgraphs() const {
        return frequent_subgraphs;
    }
};

// Legacy name for compatibility
using GSpan = SimpleMiner;

#endif
