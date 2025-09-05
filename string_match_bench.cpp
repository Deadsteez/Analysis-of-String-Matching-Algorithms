// string_match_bench.cpp
// Compile: g++ -O2 -std=c++17 string_match_bench.cpp -o bench
#include <bits/stdc++.h>
using namespace std;
using u64 = uint64_t;
struct Metrics {
    double preprocess_ms = 0.0;
    double match_ms = 0.0;
    u64 comparisons = 0;
    u64 shifts = 0;           // meaningful for BM-Horspool
    vector<int> matches;
};

static double now_ms() {
    using namespace std::chrono;
    return duration_cast<duration<double, std::milli>>(high_resolution_clock::now().time_since_epoch()).count();
}

/* ---------- Naive ---------- */
Metrics naive_search(const string &text, const string &pat) {
    Metrics R;
    int n = (int)text.size(), m = (int)pat.size();
    double t0 = now_ms();
    R.comparisons = 0;
    for (int i = 0; i + m <= n; ++i) {
        int j = 0;
        while (j < m) {
            R.comparisons++;
            if (text[i + j] != pat[j]) break;
            ++j;
        }
        if (j == m) R.matches.push_back(i);
    }
    double t1 = now_ms();
    R.match_ms = t1 - t0;
    return R;
}

/* ---------- KMP ---------- */
vector<int> build_lps(const string &p, u64 &comparisons, double &pre_ms) {
    double t0 = now_ms();
    int m = (int)p.size();
    vector<int> lps(m);
    lps[0] = 0;
    int len = 0;
    int i = 1;
    while (i < m) {
        comparisons++;
        if (p[i] == p[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
    pre_ms = now_ms() - t0;
    return lps;
}

Metrics kmp_search(const string &text, const string &pat) {
    Metrics R;
    int n = (int)text.size(), m = (int)pat.size();
    R.comparisons = 0;
    double pre_ms = 0.0;
    R.comparisons = 0;
    vector<int> lps = build_lps(pat, R.comparisons, pre_ms); // lps building counts comparisons
    R.preprocess_ms = pre_ms;

    double t0 = now_ms();
    int i = 0, j = 0;
    while (i < n) {
        R.comparisons++;
        if (pat[j] == text[i]) {
            i++; j++;
            if (j == m) {
                R.matches.push_back(i - j);
                j = lps[j - 1];
            }
        } else {
            if (j != 0) j = lps[j - 1];
            else i++;
        }
    }
    R.match_ms = now_ms() - t0;
    return R;
}

/* ---------- Rabin-Karp (modular) ---------- */
const u64 RK_MOD = 1000000007ULL;
const u64 RK_BASE = 257ULL;

u64 modmul(u64 a, u64 b) {
    return (a * b) % RK_MOD;
}
u64 modadd(u64 a, u64 b) {
    u64 s = a + b;
    if (s >= RK_MOD) s -= RK_MOD;
    return s;
}

Metrics rabin_karp_search(const string &text, const string &pat) {
    Metrics R;
    int n = (int)text.size(), m = (int)pat.size();
    if (m == 0 || m > n) return R;
    R.comparisons = 0;
    double t0pre = now_ms();
    u64 pat_hash = 0, txt_hash = 0, power = 1;
    for (int i = 0; i < m; ++i) {
        pat_hash = modadd(modmul(pat_hash, RK_BASE), (u64)(unsigned char)pat[i]);
        txt_hash = modadd(modmul(txt_hash, RK_BASE), (u64)(unsigned char)text[i]);
        if (i) power = modmul(power, RK_BASE);
    }
    R.preprocess_ms = now_ms() - t0pre;

    double t0 = now_ms();
    for (int i = 0; i + m <= n; ++i) {
        if (pat_hash == txt_hash) {
            // verify to avoid false positive due to collision
            bool equal = true;
            for (int j = 0; j < m; ++j) {
                R.comparisons++;
                if (text[i + j] != pat[j]) { equal = false; break; }
            }
            if (equal) R.matches.push_back(i);
        }
        if (i + m < n) {
            // remove text[i], add text[i+m]
            u64 left = modmul((u64)(unsigned char)text[i], power);
            u64 cur = txt_hash;
            // cur = (cur - left) * base + newchar  mod
            if (cur < left) cur += RK_MOD;
            cur = (cur - left) % RK_MOD;
            cur = modmul(cur, RK_BASE);
            cur = modadd(cur, (u64)(unsigned char)text[i + m]);
            txt_hash = cur;
        }
    }
    R.match_ms = now_ms() - t0;
    return R;
}

/* ---------- Boyer-Moore-Horspool (bad-character variant used in practice) ---------- */
Metrics bm_horspool_search(const string &text, const string &pat) {
    Metrics R;
    int n = (int)text.size(), m = (int)pat.size();
    if (m == 0 || m > n) return R;
    R.comparisons = 0;
    R.shifts = 0;
    double t0pre = now_ms();
    const int SIG = 256;
    vector<int> shift(SIG, m);
    for (int i = 0; i < m - 1; ++i) {
        shift[(unsigned char)pat[i]] = m - 1 - i;
    }
    R.preprocess_ms = now_ms() - t0pre;

    double t0 = now_ms();
    int i = 0;
    while (i + m <= n) {
        int j = m - 1;
        while (j >= 0) {
            R.comparisons++;
            if (pat[j] != text[i + j]) break;
            --j;
        }
        if (j < 0) {
            R.matches.push_back(i);
            i += 1; // shift by 1 after full match (can be improved), count as a shift
            R.shifts++;
        } else {
            int s = shift[(unsigned char)text[i + m - 1]];
            if (s <= 0) s = 1;
            i += s;
            R.shifts++;
        }
    }
    R.match_ms = now_ms() - t0;
    return R;
}

/* ---------- Utilities for tests ---------- */
string gen_random_text(int n, mt19937 &rng) {
    string s;
    s.resize(n);
    uniform_int_distribution<int> d('a', 'z');
    for (int i = 0; i < n; ++i) s[i] = (char)d(rng);
    return s;
}
string gen_repetitive_text(int n, char c = 'a') {
    return string((size_t)n, c);
}

/* ---------- Single-run helper ---------- */
struct RunRecord {
    string algorithm;
    string text_type;
    int n;
    int m;
    int run_id;
    double preprocess_ms;
    double match_ms;
    u64 comparisons;
    u64 shifts;
    int matches_count;
};

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Experiment configuration (you can edit these)
    vector<int> Ns = {1000, 5000, 10000, 50000, 100000}; // text sizes
    vector<int> Ms = {3, 10, 50};                       // pattern sizes
    int runs_per_case = 5;

    vector<RunRecord> records;
    mt19937 rng(123456);

    for (const string &text_type : {"random", "repetitive"}) {
        for (int n : Ns) {
            // generate text
            string text;
            if (text_type == "random") text = gen_random_text(n, rng);
            else text = gen_repetitive_text(n, 'a');

            for (int m : Ms) {
                if (m > n) continue;
                // choose pattern: for random -> pick a substring so pattern exists;
                // for repetitive -> pick a repetitive pattern that causes stress
                string pat;
                if (text_type == "random") {
                    uniform_int_distribution<int> di(0, n - m);
                    int pos = di(rng);
                    pat = text.substr(pos, m);
                } else {
                    // repetitive sample with a slight tail to create matches or near-misses
                    pat = string((size_t)m, 'a');
                    if (m >= 2) pat.back() = 'b'; // creates near-misses
                }

                for (int run = 0; run < runs_per_case; ++run) {
                    // Naive
                    {
                        Metrics R = naive_search(text, pat);
                        records.push_back({"Naive", text_type, n, m, run+1, R.preprocess_ms, R.match_ms, R.comparisons, R.shifts, (int)R.matches.size()});
                    }
                    // KMP
                    {
                        Metrics R = kmp_search(text, pat);
                        records.push_back({"KMP", text_type, n, m, run+1, R.preprocess_ms, R.match_ms, R.comparisons, R.shifts, (int)R.matches.size()});
                    }
                    // Rabin-Karp
                    {
                        Metrics R = rabin_karp_search(text, pat);
                        records.push_back({"Rabin-Karp", text_type, n, m, run+1, R.preprocess_ms, R.match_ms, R.comparisons, R.shifts, (int)R.matches.size()});
                    }
                    // Boyer-Moore-Horspool
                    {
                        Metrics R = bm_horspool_search(text, pat);
                        records.push_back({"Boyer-Moore-Horspool", text_type, n, m, run+1, R.preprocess_ms, R.match_ms, R.comparisons, R.shifts, (int)R.matches.size()});
                    }
                } // runs
            } // m
        } // n
    } // text_type

    // write CSV
    ofstream ofs("results.csv");
    ofs << "algorithm,text_type,n,m,run,pre_ms,match_ms,comparisons,shifts,matches_count\n";
    for (auto &r : records) {
        ofs << r.algorithm << ',' << r.text_type << ',' << r.n << ',' << r.m << ',' << r.run_id << ',';
        ofs << fixed << setprecision(6) << r.preprocess_ms << ',' << r.match_ms << ',';
        ofs << r.comparisons << ',' << r.shifts << ',' << r.matches_count << '\n';
    }
    ofs.close();

    cout << "Finished benchmark. CSV written to results.csv (" << records.size() << " rows).\n";
    cout << "Run: bench (compiled binary) in terminal. Use plot_results.py to visualize.\n";
    return 0;
}
