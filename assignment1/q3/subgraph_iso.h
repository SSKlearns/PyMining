#ifndef SUBGRAPH_ISO_H
#define SUBGRAPH_ISO_H

#include "graph.h"
#include <vector>
#include <map>

// VF2-style subgraph isomorphism with pre-filtering
class SubgraphIsomorphism {
private:
    const Graph* pattern;
    const Graph* target;
    std::map<int, int> core_1;  // pattern -> target mapping
    std::map<int, int> core_2;  // target -> pattern mapping
    
    bool is_feasible(int pv, int tv) {
        // Check vertex label match
        auto pv_it = pattern->vertices.find(pv);
        auto tv_it = target->vertices.find(tv);
        if (pv_it == pattern->vertices.end() || tv_it == target->vertices.end() || pv_it->second != tv_it->second) {
            return false;
        }
        
        // Check edge consistency with current mapping
        auto pv_adj = pattern->adj_list.find(pv);
        if (pv_adj == pattern->adj_list.end()) return true;
        
        for (const auto& [neighbor, edge_label] : pv_adj->second) {
            if (core_1.find(neighbor) != core_1.end()) {
                int mapped_neighbor = core_1[neighbor];
                
                // Check if edge exists in target
                auto tv_adj = target->adj_list.find(tv);
                if (tv_adj == target->adj_list.end()) return false;
                
                bool found = false;
                for (const auto& [tn, tel] : tv_adj->second) {
                    if (tn == mapped_neighbor && tel == edge_label) {
                        found = true;
                        break;
                    }
                }
                if (!found) return false;
            }
        }
        
        return true;
    }
    
    bool match_recursive(const std::vector<int>& pattern_vertices, int depth) {
        if (depth == pattern_vertices.size()) {
            return true;  // All pattern vertices mapped
        }
        
        int pv = pattern_vertices[depth];
        
        for (const auto& [tv, tlabel] : target->vertices) {
            if (core_2.find(tv) != core_2.end()) continue;  // Already mapped
            
            if (is_feasible(pv, tv)) {
                core_1[pv] = tv;
                core_2[tv] = pv;
                
                if (match_recursive(pattern_vertices, depth + 1)) {
                    return true;
                }
                
                core_1.erase(pv);
                core_2.erase(tv);
            }
        }
        
        return false;
    }
    
public:
    static bool pre_filter(const Graph& pattern, const Graph& target) {
        // Quick checks before running expensive isomorphism
        if (pattern.num_vertices() > target.num_vertices()) return false;
        if (pattern.num_edges() > target.num_edges()) return false;
        
        // Check vertex label multiset
        auto p_vlabels = pattern.get_vertex_label_multiset();
        auto t_vlabels = target.get_vertex_label_multiset();
        for (const auto& [label, count] : p_vlabels) {
            if (t_vlabels[label] < count) return false;
        }
        
        // Check edge label multiset
        auto p_elabels = pattern.get_edge_label_multiset();
        auto t_elabels = target.get_edge_label_multiset();
        for (const auto& [label, count] : p_elabels) {
            if (t_elabels[label] < count) return false;
        }
        
        return true;
    }
    
    static bool is_subgraph(const Graph& pattern, const Graph& target) {
        // Pre-filtering
        if (!pre_filter(pattern, target)) {
            return false;
        }
        
        if (pattern.num_vertices() == 0) return true;
        
        SubgraphIsomorphism matcher;
        matcher.pattern = &pattern;
        matcher.target = &target;
        
        std::vector<int> pattern_vertices;
        for (const auto& [vid, label] : pattern.vertices) {
            pattern_vertices.push_back(vid);
        }
        
        return matcher.match_recursive(pattern_vertices, 0);
    }
};

#endif
