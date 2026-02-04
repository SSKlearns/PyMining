import sys
import random

NUM_TRANSACTIONS = 15000
OUTPUT_FILE = "generated_transactions.dat"

def main():
    if len(sys.argv) != 2:
        print("Usage: python generate_dataset.py <universal_itemset_size>")
        sys.exit(1)

    U = int(sys.argv[1])
    random.seed(42)

    items = list(range(1, U + 1))

    # ---- Structure ----
    core = items[: U // 3]          # very frequent
    mid  = items[U // 3 : 2*U // 3] # medium frequent
    tail = items[2*U // 3 :]        # rare

    transactions = set()

    while len(transactions) < NUM_TRANSACTIONS:
        txn = set()

        # Core items appear together (prefix sharing)
        for i in core:
            if random.random() < 0.85:
                txn.add(i)

        # Mid items generate candidate explosion
        for i in mid:
            if random.random() < 0.45:
                txn.add(i)

        # Tail items keep uniqueness without helping Apriori
        for i in tail:
            if random.random() < 0.08:
                txn.add(i)

        # Force long transactions
        if len(txn) < U // 2:
            txn |= set(random.sample(items, U // 2))

        transactions.add(tuple(sorted(txn)))

    with open(OUTPUT_FILE, "w") as f:
        for t in transactions:
            f.write(" ".join(map(str, t)) + "\n")

    print(f"Generated {len(transactions)} transactions")
    print(f"Average txn length ≈ {sum(len(t) for t in transactions)//len(transactions)}")


if __name__ == "__main__":
    main()
