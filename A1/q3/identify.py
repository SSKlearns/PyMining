import subprocess
import time
import os
import sys
import json

TIMEOUT = 3600  # 1 hour

def convert_file_to_gaston_input(inp, out):

    with open(inp, "r") as f, open(out, "w") as g:
        content = f.read()
        if content.count("t #") < 10:
            g.write(content.replace("#", "t #"))


def run_and_time(cmd):
    try:
        start = time.time()
        subprocess.run(
            cmd,
            timeout=TIMEOUT,
            # stdout=subprocess.DEVNULL,
            # stderr=subprocess.DEVNULL
        )
        return time.time() - start
    except subprocess.TimeoutExpired:
        return float(TIMEOUT)

def ensure_empty_file(path):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if not os.path.exists(path):
        open(path, "w").close()

def main():
    # if len(sys.argv) != 4:
    #     print(
    #         "Usage: python run_q3.py <dataset> <out_dir>"
    #     )
    #     sys.exit(1)

    dataset = sys.argv[1]
    out_dir = sys.argv[2]

    print("dataset: ", dataset)
    print("outdir: ", out_dir)

    convert_file_to_gaston_input(dataset, "gaston_input.txt")

    dataset_gaston = "gaston_input.txt"
    with open(dataset_gaston, "r") as f:
        content = f.read()
    total_graphs = content.count("t #")
    print("Total graphs: ", total_graphs)

    supports = [50]

    results = {
        "gaston": {}
    }

    for s in supports:
        print(f"\n=== minsup {s}% ===")

        # ---------------- Gaston ----------------
        if os.path.isdir(out_dir):
            print("creating directory")
            os.makedirs(out_dir, exist_ok=True)
            gaston_out = os.path.join(out_dir, f"gaston.fp")
        else:
            gaston_out = out_dir
        gaston_cmd = [
            "./gaston",
            str((total_graphs * s) // 100),
            dataset_gaston,
            gaston_out,
        ]
        print(gaston_cmd)
        t = run_and_time(gaston_cmd)
        results["gaston"][s] = t
        ensure_empty_file(gaston_out)
        print(f"Gaston  | {s}% | {t:.2f}s")

if __name__ == "__main__":

    print("entered script")
    main()
