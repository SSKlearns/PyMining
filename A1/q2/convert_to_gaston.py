import sys

inp = sys.argv[1]
out = sys.argv[2]


def convert_file_to_gaston_input(inp, out):
    mapper = ['Br', 'C', 'Cl', 'F', 'H', 'I', 'N', 'O', 'P', 'S', 'Si']

    with open(inp, "r") as f, open(out, "w") as g:
        lines = [line.strip() for line in f if line.strip()]
        i = 0
        graph_id = 0

        while i < len(lines):
            # ---- Transaction header (comment) ----
            if not lines[i].startswith("#"):
                raise ValueError(f"Expected transaction header, got: {lines[i]}")
            # g.write(f"# {lines[i][1:].strip()}\n")   # preserve comment
            g.write(f"t # {graph_id}\n")
            graph_id += 1
            i += 1

            # ---- Nodes ----
            num_nodes = int(lines[i])
            i += 1

            for node_id in range(num_nodes):
                label = lines[i]
                g.write(f"v {node_id} {mapper.index(label)}\n")
                i += 1

            # ---- Edges ----
            num_edges = int(lines[i])
            i += 1

            for _ in range(num_edges):
                src, dst, edge_label = lines[i].split(" ")
                g.write(f"e {src} {dst} {edge_label}\n")
                i += 1
