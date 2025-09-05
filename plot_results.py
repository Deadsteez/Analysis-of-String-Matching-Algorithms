# plot_dashboard.py
import pandas as pd
import matplotlib.pyplot as plt

# Load CSV
df = pd.read_csv("results.csv")

# Aggregate with median
agg = df.groupby(["algorithm", "text_type", "n", "m"]).median().reset_index()

# Custom styles
styles = {
    "Naive": {"marker": "o", "linestyle": ":", "color": "tab:red"},   # dotted red
    "KMP": {"marker": "s", "linestyle": "-", "color": "tab:green"},
    "Rabin-Karp": {"marker": "x", "linestyle": "--", "color": "tab:orange"},
    "Boyer-Moore-Horspool": {"marker": "^", "linestyle": "-.", "color": "tab:blue"},
}

def plot_dashboard(m_value, text_type="random"):
    sel = agg[(agg["m"] == m_value) & (agg["text_type"] == text_type)]
    if sel.empty:
        print(f"No data for m={m_value}, {text_type}")
        return
    
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    fig.suptitle(f"Algorithm Comparison (m={m_value}, {text_type})", fontsize=14)

    # ---- Comparisons ----
    ax = axes[0]
    for alg in sel["algorithm"].unique():
        s = sel[sel["algorithm"] == alg]
        st = styles.get(alg, {"marker": "o", "linestyle": "-", "color": None})
        ax.plot(s["n"], s["comparisons"], label=alg, **st)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Text length n (log)")
    ax.set_ylabel("Comparisons (log)")
    ax.set_title("Comparisons")
    ax.grid(True, which="both", ls="--", alpha=0.6)

    # ---- Match Time ----
    ax = axes[1]
    for alg in sel["algorithm"].unique():
        s = sel[sel["algorithm"] == alg].copy()
        s["match_ms"] = s["match_ms"].replace(0, 1e-6).fillna(1e-6)
        st = styles.get(alg, {"marker": "o", "linestyle": "-", "color": None})
        ax.plot(s["n"], s["match_ms"], label=alg, **st)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Text length n (log)")
    ax.set_ylabel("Match Time (ms, log)")
    ax.set_title("Match Time")
    ax.grid(True, which="both", ls="--", alpha=0.6)

    # ---- Shifts (BMH only) ----
    ax = axes[2]
    sel_bmh = sel[sel["algorithm"] == "Boyer-Moore-Horspool"]
    if not sel_bmh.empty:
        st = styles["Boyer-Moore-Horspool"]
        ax.plot(sel_bmh["n"], sel_bmh["shifts"], label="BMH", **st)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Text length n (log)")
    ax.set_ylabel("Shifts (log)")
    ax.set_title("BMH Shifts")
    ax.grid(True, which="both", ls="--", alpha=0.6)

    # ---- Legend ----
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper right")

    # Save
    fname = f"dashboard_m{m_value}_{text_type}.png"
    plt.tight_layout(rect=[0, 0, 0.95, 0.95])
    plt.savefig(fname, dpi=200)
    print(f"Saved: {fname}")
    plt.close()

# Generate dashboards
for m in [3, 10, 50]:
    for text_type in ["random", "repetitive"]:
        plot_dashboard(m, text_type)

print("All dashboard plots generated.")
