import numpy as np
import sys

def generate_candidates(db_vectors, query_vectors):
    """
    Filters database graphs based on the condition: v_q <= v_i.
    """
    all_candidates = []
    
    for v_q in query_vectors:
        # Identify indices where the query fragment is present (bit is 1)
        # Using numpy's where for efficiency on large feature vectors
        ones = np.where(v_q == 1)[0]
        
        candidates = []
        for i, v_i in enumerate(db_vectors):
            # Pruning: v_i must have 1s at every position where v_q has a 1
            if np.all(v_i[ones] == 1):
                candidates.append(i)
        
        all_candidates.append(candidates)
    
    return all_candidates

def save_output(Cq, output_path):
    """
    Saves the results in the required 1-indexed candidates.dat format.
    """
    with open(output_path, 'w') as f:
        for q_idx, candidates in enumerate(Cq):
            # q # [1-indexed query number] [cite: 272]
            f.write(f"q # {q_idx + 1}\n")
            # c # [1-indexed space-separated candidate numbers] [cite: 273]
            candidate_str = " ".join(str(c_idx + 1) for c_idx in candidates)
            f.write(f"c # {candidate_str}\n")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python generate_candidates.py <db_features.npy> <query_features.npy> <out_file>")
        sys.exit(1)

    # Loading the 2D numpy arrays as specified in the assignment 
    db_feats = np.load(sys.argv[1])
    query_feats = np.load(sys.argv[2])
    output_file = sys.argv[3]

    results = generate_candidates(db_feats, query_feats)
    save_output(results, output_file)