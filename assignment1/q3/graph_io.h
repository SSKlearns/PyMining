#ifndef GRAPH_IO_H
#define GRAPH_IO_H

#include "graph.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

std::vector<Graph> parse_graphs_from_file(const std::string& filepath) {
    std::vector<Graph> graphs;
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return graphs;
    }
    
    Graph* current_graph = nullptr;
    int graph_id = 0;
    std::string line;
    
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        
        if (line.empty()) continue;
        
        if (line[0] == '#') {
            if (current_graph != nullptr) {
                graphs.push_back(*current_graph);
                delete current_graph;
            }
            current_graph = new Graph(graph_id++);
        } else if (line[0] == 'v') {
            std::istringstream iss(line);
            char v;
            int vertex_id, label;
            iss >> v >> vertex_id >> label;
            current_graph->add_vertex(vertex_id, label);
        } else if (line[0] == 'e') {
            std::istringstream iss(line);
            char e;
            int v1, v2, edge_label;
            iss >> e >> v1 >> v2 >> edge_label;
            current_graph->add_edge(v1, v2, edge_label);
        }
    }
    
    if (current_graph != nullptr) {
        graphs.push_back(*current_graph);
        delete current_graph;
    }
    
    file.close();
    return graphs;
}

void write_graph_to_file(std::ofstream& out, const Graph& g) {
    out << "#\n";
    
    // Write vertices in order
    std::vector<int> vertex_ids;
    for (const auto& [vid, label] : g.vertices) {
        vertex_ids.push_back(vid);
    }
    std::sort(vertex_ids.begin(), vertex_ids.end());
    
    for (int vid : vertex_ids) {
        out << "v " << vid << " " << g.vertices.at(vid) << "\n";
    }
    
    // Write edges (avoiding duplicates for undirected)
    std::set<std::pair<int, int>> written;
    for (const auto& e : g.edges) {
        int u = std::min(e.from, e.to);
        int v = std::max(e.from, e.to);
        if (written.find({u, v}) == written.end()) {
            out << "e " << e.from << " " << e.to << " " << e.label << "\n";
            written.insert({u, v});
        }
    }
}

void write_graphs_to_file(const std::string& filepath, const std::vector<Graph>& graphs) {
    std::ofstream out(filepath);
    for (const auto& g : graphs) {
        write_graph_to_file(out, g);
    }
    out.close();
}

#endif
