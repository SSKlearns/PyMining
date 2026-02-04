from collections import defaultdict
from itertools import combinations
import sys
import numpy as np


def canonical_2edge_labels(graph_text):
    """
    Input: graph as a single string in Gaston format
    Output: set of canonical 2-edge labels (tuples of 3 node ids)
    """

    edges = []
    
    # Parse edges (ignore labels)
    for line in graph_text.strip().splitlines():
        line = line.strip()
        if line.startswith("e"):
            try:
                _, u, v, _ = line.split()
                u, v = int(u), int(v)
            except Exception as e:
                continue
            edges.append((u, v))

    # Build adjacency list
    adj = defaultdict(set)
    for u, v in edges:
        adj[u].add(v)
        adj[v].add(u)

    # Collect 2-edge canonical labels
    canonical_labels = set()

    for center in adj:
        neighbors = list(adj[center])
        for u, v in combinations(neighbors, 2):
            label = tuple(sorted((u, center, v)))
            canonical_labels.add(label)

    return canonical_labels


def canonical_3edge_labels(graph_text):
    """
    Input: graph as a single string in Gaston format
    Output: set of canonical 3-edge labels (tuples of 4 node ids)
    """

    edges = []

    # Parse edges
    for line in graph_text.strip().splitlines():
        line = line.strip()
        if line.startswith("e"):
            try:
                _, u, v, _ = line.split()
                u, v = int(u), int(v)
            except Exception as e:
                continue
            edges.append((u, v))

    # Build adjacency list
    adj = defaultdict(set)
    for u, v in edges:
        adj[u].add(v)
        adj[v].add(u)

    canonical_labels = set()

    # ---- Case 1: 3-edge STAR (center with 3 neighbors) ----
    for center in adj:
        neighbors = list(adj[center])
        if len(neighbors) >= 3:
            for u, v, w in combinations(neighbors, 3):
                label = tuple(sorted((center, u, v, w)))
                canonical_labels.add(label)

    # ---- Case 2: 3-edge PATH (u - v - w - x) ----
    for v in adj:
        for u in adj[v]:
            for w in adj[v]:
                if w == u:
                    continue
                for x in adj[w]:
                    if x == v:
                        continue
                    label = tuple(sorted((u, v, w, x)))
                    canonical_labels.add(label)

    return canonical_labels


def read_gaston_graphs_as_strings(filepath):
    """
    Reads a Gaston-style graph file and returns a list of graph strings.
    """

    graphs = []
    current_graph = []

    with open(filepath, "r") as f:
        for line in f:
            line = line.rstrip("\n")

            if line.startswith("#"):
                if current_graph:
                    graphs.append("\n".join(current_graph))
                    current_graph = []
                current_graph.append(line)
            else:
                if current_graph:
                    current_graph.append(line)

        if current_graph and current_graph not in graphs:
            graphs.append("\n".join(current_graph))

    return graphs


def build_db_feature_vectors(db_graphs, features):
    """
    Returns a list of binary vectors, one per database graph.
    """
    features = list(features)          # fixed column order
    feature_set = set(features)

    db_vectors = []

    for graph in db_graphs:
        canonicals_2 = canonical_2edge_labels(graph)  # set
        canonicals_3 = canonical_3edge_labels(graph)  # set
        present = (canonicals_2 | canonicals_3) & feature_set

        v = [1 if f in present else 0 for f in features]
        db_vectors.append(v)

    return db_vectors

if __name__ == "__main__":

    print("entered script")
    input_graph = sys.argv[1]
    gaston_out = sys.argv[2]
    output_npy = sys.argv[3]

    gaston_graphs = read_gaston_graphs_as_strings(gaston_out)

    canonical_counts = defaultdict(int)
    canonical_3_counts = defaultdict(int)
    canonical_graph_list = []
    canonical_3_graph_list = []

    for graph in gaston_graphs:
        canonical_graph = canonical_2edge_labels(graph)  # set of labels in this graph
        canonical_3_graph = canonical_3edge_labels(graph)  # set of labels in this graph
        canonical_graph_list.append(canonical_graph)
        canonical_3_graph_list.append(canonical_3_graph)

        for label in canonical_graph:
            canonical_counts[label] += 1

        for label in canonical_3_graph:
            canonical_3_counts[label] += 1


    import math

    ranked = sorted(
        canonical_counts.items(),
        key=lambda x: math.log(4337 / x[1]),
        reverse=True
    )
    ranked_3 = sorted(
        canonical_3_counts.items(),
        key=lambda x: math.log(4337 / x[1]),
        reverse=True
    )


    features = [f for f, count in ranked][:12]
    features2 = [f for f, count in ranked][27:40]
    features_3 = [f for f, count in ranked_3][:12]
    features_32 = [f for f, count in ranked_3][27:40]

    combined_features = features + features_3 + features2 + features_32

    graph_db_content = read_gaston_graphs_as_strings(input_graph)
    db_vectors = build_db_feature_vectors(graph_db_content, combined_features)

    np.save(output_npy, np.array(db_vectors))