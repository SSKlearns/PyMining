#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <iostream>

struct Edge {
    int from;
    int to;
    int label;
    
    Edge(int f, int t, int l) : from(f), to(t), label(l) {}
};

class Graph {
public:
    int graph_id;
    std::map<int, int> vertices;  // vertex_id -> label
    std::vector<Edge> edges;
    std::map<int, std::vector<std::pair<int, int>>> adj_list;  // vertex -> [(neighbor, edge_label)]
    
    Graph(int id = 0) : graph_id(id) {}
    
    void add_vertex(int vertex_id, int label) {
        vertices[vertex_id] = label;
    }
    
    void add_edge(int v1, int v2, int label) {
        edges.emplace_back(v1, v2, label);
        adj_list[v1].emplace_back(v2, label);
        adj_list[v2].emplace_back(v1, label);
    }
    
    int num_vertices() const {
        return vertices.size();
    }
    
    int num_edges() const {
        return edges.size();
    }
    
    std::map<int, int> get_vertex_label_multiset() const {
        std::map<int, int> counts;
        for (const auto& [vid, label] : vertices) {
            counts[label]++;
        }
        return counts;
    }
    
    std::map<int, int> get_edge_label_multiset() const {
        std::map<int, int> counts;
        for (const auto& e : edges) {
            counts[e.label]++;
        }
        return counts;
    }
    
    std::string to_string() const {
        return "Graph(id=" + std::to_string(graph_id) + 
               ", V=" + std::to_string(num_vertices()) + 
               ", E=" + std::to_string(num_edges()) + ")";
    }
};

// DFS Code for canonical representation
struct DFSCode {
    int from;
    int to;
    int from_label;
    int edge_label;
    int to_label;
    
    DFSCode(int f, int t, int fl, int el, int tl) 
        : from(f), to(t), from_label(fl), edge_label(el), to_label(tl) {}
    
    bool operator<(const DFSCode& other) const {
        if (from != other.from) return from < other.from;
        if (to != other.to) return to < other.to;
        if (from_label != other.from_label) return from_label < other.from_label;
        if (edge_label != other.edge_label) return edge_label < other.edge_label;
        return to_label < other.to_label;
    }
    
    bool operator==(const DFSCode& other) const {
        return from == other.from && to == other.to && 
               from_label == other.from_label && 
               edge_label == other.edge_label && 
               to_label == other.to_label;
    }
    
    std::string to_string() const {
        return "(" + std::to_string(from) + "," + std::to_string(to) + "," +
               std::to_string(from_label) + "," + std::to_string(edge_label) + "," +
               std::to_string(to_label) + ")";
    }
};

#endif
