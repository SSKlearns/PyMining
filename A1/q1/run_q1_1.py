import subprocess
import time
import os
import sys
import json
import matplotlib.pyplot as plt


def run_and_time(cmd):
    try:
        start = time.time()
        subprocess.run(
            cmd,
            timeout=3600,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        ap_time = time.time() - start

        if not os.path.exists(cmd[-1]) or os.path.getsize(cmd[-1]) == 0:
            # create empty output file if not exists
            with open(cmd[-1], "a") as f:
                pass

    except subprocess.TimeoutExpired:
        ap_time = 3600.0
        with open(cmd[-1], "a") as f:
            pass  # Create empty output file on timeout

    return ap_time

def main():
    if len(sys.argv) != 5:
        print("Usage: python run_q1.py <apriori_path> <fptree_path> <dataset_path> <out_dir>")
        sys.exit(1)

    apriori_path = sys.argv[1]
    fptree_path = sys.argv[2]
    dataset_path = sys.argv[3]
    out_dir = sys.argv[4]

    supports = [90, 50, 25, 10, 5]

    os.makedirs(out_dir, exist_ok=True)

    results = {
        "apriori": {},
        "fptree": {}
    }

    for s in supports:
        # ---- Apriori ----
        ap_out_file = os.path.join(out_dir, f"ap{s}")
        # os.makedirs(ap_out_dir, exist_ok=True)
        # ap_out_file = os.path.join(ap_out_dir, "output.txt")

        ap_cmd = [
            apriori_path,
            f"-s{s}",
            dataset_path,
            ap_out_file
        ]

        ap_time = run_and_time(ap_cmd)
        results["apriori"][s] = ap_time
        print(f"Apriori | support={s}% | time={ap_time:.4f}s")

        # ---- FP-Growth ----
        fp_out_file = os.path.join(out_dir, f"fp{s}")
        # os.makedirs(fp_out_dir, exist_ok=True)
        # fp_out_file = os.path.join(fp_out_dir, "output.txt")

        fp_cmd = [
            fptree_path,
            f"-s{s}",
            dataset_path,
            fp_out_file
        ]

        fp_time = run_and_time(fp_cmd)
        results["fptree"][s] = fp_time
        print(f"FP-Growth | support={s}% | time={fp_time:.4f}s")

    # Save runtimes for plotting
    with open(os.path.join(out_dir, "runtimes.json"), "w") as f:
        json.dump(results, f, indent=2)

    # Load runtimes
    with open(os.path.join(out_dir, "runtimes.json"), "r") as f:
        data = json.load(f)

    # Support thresholds (sorted numerically)
    supports = sorted(int(s) for s in data["apriori"].keys())

    apriori_times = [data["apriori"][str(s)] for s in supports]
    fptree_times  = [data["fptree"][str(s)] for s in supports]

    plt.figure(figsize=(8, 6))

    plt.plot(supports, apriori_times, marker="o", label="Apriori")
    plt.plot(supports, fptree_times, marker="s", label="FP-Tree")

    plt.xlabel("Minimum Support Threshold (%)")
    plt.ylabel("Runtime (seconds)")
    plt.title("Runtime Comparison of Apriori and FP-Tree")
    plt.legend()
    plt.grid(True)

    plt.savefig(os.path.join(out_dir, "plot.png"), dpi=300, bbox_inches="tight")
    plt.close()

if __name__ == "__main__":
    main()
