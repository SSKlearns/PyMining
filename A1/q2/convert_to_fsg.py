import sys

# inp = sys.argv[1]
# out = sys.argv[2]

def convert_file_to_sgmining_input(inp, out):
    with open(inp, "r") as f, open(out, "w") as g:
        lines = [line.strip() for line in f if line.strip()]
        i = 0

        while i < len(lines):
            # ---- Transaction header (comment) ----
            if not lines[i].startswith("#"):
                raise ValueError(f"Expected transaction header, got: {lines[i]}")
            i += 1

            # ---- Start graph ----
            g.write("t\n")

            # ---- Nodes ----
            num_nodes = int(lines[i])
            i += 1

            for node_id in range(num_nodes):
                label = lines[i]
                g.write(f"v {node_id} {label}\n")
                i += 1

            # ---- Edges ----
            num_edges = int(lines[i])
            i += 1

            for _ in range(num_edges):
                src, dst, edge_label = lines[i].split(" ")
                g.write(f"u {src} {dst} {edge_label}\n")
                i += 1
