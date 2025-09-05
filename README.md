# Analysis-of-String-Matching-Algorithms

This project benchmarks and compares four classical string matching algorithms:

- Naïve Search
- Knuth–Morris–Pratt (KMP)
- Rabin–Karp (rolling hash)
- Boyer–Moore–Horspool (BMH)

The results are saved as CSV and plotted with Python.

🔧 How to Run
1. Compile and Run Benchmarks
Run the string_match_bench.cpp
This generates results.csv.

2. Generate Plots
Run the plot_results.py(python plot_dashboard.py)
dashboards (3-in-1 comparison plots):






--------------------------------------------------------------------------------------------------------------------------------------------------------------------
📂 Output Files
- results.csv – Raw measurements (per algorithm, per run).
- comparisons_mX_random.png / time_mX_random.png / shifts_mX_random.png – Individual plots.
- dashboard_mX_random.png – Combined plots (comparisons, time, shifts).
- Same for repetitive text.
📊 What the CSV Contains
Column	Meaning
algorithm	Naïve, KMP, Rabin-Karp, Boyer-Moore-Horspool
text_type	`random` (uniform characters) or `repetitive` (`aaaaa...`)
n	Text length
m	Pattern length
run	Repetition index (1–5)
pre_ms	Preprocessing time (ms)
match_ms	Matching time (ms)
comparisons	Character comparisons (useful for Naïve, KMP, RK)
shifts	Number of shifts (only meaningful for BMH)
matches_count	Number of matches found
📈 What the Plots Show
- Comparisons vs Text Length
  - On random text, Naïve grows linearly with n, KMP stabilizes, BMH often wins for longer m.
  - On repetitive text, Naïve degenerates, KMP is efficient, BMH performs poorly.
  - Rabin–Karp may disappear unless we map 0 comparisons → 1e-6 (log-scale cannot plot 0).

- Match Time vs Text Length
  - Shows real runtime cost in milliseconds.
  - Values are aggregated as median over 5 runs to reduce noise.
  - Very small times (0) are mapped to 1e-6 to avoid log(0).

- BMH Shifts vs Text Length
  - Shows how far Boyer–Moore–Horspool skips per step.
  - On random text, shifts grow nicely. On repetitive text, shifts degrade.

- Dashboards combine all three into one figure for a fixed m.
⚙️ Technical Notes & Caveats
1. Naïve is dotted red in plots (circle markers, ':' linestyle) to distinguish it easily.
2. Rabin–Karp in repetitive text:
   - Pattern = 'aaaa...b' against text 'aaaa...a'.
   - Hash almost never matches → zero comparisons.
   - On log-scale, 0 is invisible, so we replace with 1e-6 to keep the line.
3. Preprocessing time:
   - KMP builds the LPS table.
   - RK builds initial hash.
   - BMH builds shift table.
   - Naïve has no preprocessing.
4. Log–log plots:
   - X-axis = n (text length).
   - Y-axis = comparisons/time/shifts.
   - Flattened/vertical-looking lines usually mean identical values across all n (e.g., RK with no matches).
5. Runs per case = 5 → results are aggregated with median (not mean) for robustness.
📌 Example Insights
- Naïve vs KMP: Similar on random text, but KMP dominates on repetitive input.
- Rabin–Karp: Fast hashing, but weak on repetitive text unless collisions occur.
- BMH: Very efficient for longer patterns in random text, poor on repetitive input.
🧑‍💻 Environment
- Language: C++17
- Compiler: g++ on Linux/Windows
- Plots: Python 3, pandas, matplotlib
