import subprocess
import time
import os
import sys
import json
import shutil
from convert_to_fsg import convert_file_to_sgmining_input
from convert_to_gaston import convert_file_to_gaston_input
from matplotlib import pyplot as plt

TIMEOUT = 3600  # 1 hour

def run_and_time(cmd):
    try:
        start = time.time()
        subprocess.run(
            cmd,
            timeout=TIMEOUT,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        return time.time() - start
    except subprocess.TimeoutExpired:
        return float(TIMEOUT)

def ensure_empty_file(path):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if not os.path.exists(path):
        open(path, "w").close()

def main():
    if len(sys.argv) != 6:
        print(
            "Usage: python run_q2.py <gspan_bin> <fsg_bin> <gaston_bin> <dataset> <out_dir>"
        )
        sys.exit(1)

    gspan_bin = sys.argv[1]
    fsg_bin = sys.argv[2]
    gaston_bin = sys.argv[3]
    dataset = sys.argv[4]
    out_dir = sys.argv[5]

    convert_file_to_sgmining_input(dataset, ".".join(dataset.split(".")[:-1]) + "_processed.txt")
    convert_file_to_gaston_input(dataset, ".".join(dataset.split(".")[:-1]) + "gaston_processed.txt")

    dataset_fsg = ".".join(dataset.split(".")[:-1]) + "_processed.txt"
    dataset_gaston = ".".join(dataset.split(".")[:-1]) + "gaston_processed.txt"

    supports = [95, 50, 25, 10, 5]

    os.makedirs(out_dir, exist_ok=True)

    results = {
        "gspan": {},
        "fsg": {},
        "gaston": {}
    }

    for s in supports:
        print(f"\n=== minsup {s}% ===")

        # ---------------- gSpan ----------------
        gspan_out = os.path.join(out_dir, f"gspan{s}")
        gspan_cmd = [
            gspan_bin,
            "-f", dataset_gaston,
            "-s", str(s / 100.0),
            "-o",
        ]

        t = run_and_time(gspan_cmd)
        results["gspan"][s] = t
        ensure_empty_file(gspan_out)
        if os.path.exists(dataset_gaston + ".fp"):
            os.rename(dataset_gaston + ".fp", gspan_out)

        print(f"gSpan   | {s}% | {t:.2f}s")

        # ---------------- FSG ----------------
        fsg_cmd = [
            fsg_bin,
            "-s", str(s),
            dataset_fsg
        ]

        t = run_and_time(fsg_cmd)
        results["fsg"][s] = t

        auto_fp = ".".join(dataset_fsg.split(".")[:-1]) + ".fp"
        fsg_out = os.path.join(out_dir, f"fsg{s}.fp")

        if os.path.exists(auto_fp):
            shutil.move(auto_fp, fsg_out)
        else:
            ensure_empty_file(fsg_out)

        print(f"FSG     | {s}% | {t:.2f}s")

        # ---------------- Gaston ----------------
        gaston_out = os.path.join(out_dir, f"gaston{s}.fp")
        gaston_cmd = [
            gaston_bin,
            str((64119 * s) // 100),
            dataset_gaston,
            gaston_out,
        ]

        t = run_and_time(gaston_cmd)
        results["gaston"][s] = t
        ensure_empty_file(gaston_out)
        print(f"Gaston  | {s}% | {t:.2f}s")

    # Save runtimes
    with open(os.path.join(out_dir, "runtimes.json"), "w") as f:
        json.dump(results, f, indent=2)

    with open(os.path.join(out_dir, "runtimes.json"), "r") as f:
        data = json.load(f)

    # Support thresholds (sorted numerically)
    supports = sorted(int(s) for s in data["gaston"].keys())

    apriori_times = [data["gspan"][str(s)] for s in supports]
    fptree_times  = [data["fsg"][str(s)] for s in supports]
    gaston_times  = [data["gaston"][str(s)] for s in supports]

    plt.figure(figsize=(8, 6))

    plt.plot(supports, apriori_times, marker="o", label="Apriori")
    plt.plot(supports, fptree_times, marker="s", label="FP-Tree")
    plt.plot(supports, gaston_times, marker="x", label="FP-Tree")

    plt.xlabel("Minimum Support Threshold (%)")
    plt.ylabel("Runtime (seconds)")
    plt.title("Runtime Comparison of FSG, GSpan and Gaston.")
    plt.legend()
    plt.grid(True)

    plt.savefig(os.path.join(out_dir, "plot.png"), dpi=300, bbox_inches="tight")
    plt.close()


if __name__ == "__main__":
    main()
