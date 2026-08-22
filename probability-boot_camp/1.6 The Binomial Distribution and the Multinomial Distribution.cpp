// ============================================================================
//  1.6 - THE BINOMIAL DISTRIBUTION AND THE MULTINOMIAL DISTRIBUTION
//
//  Build & run (quote the name, it has spaces):
//      g++ -std=c++17 -Wall -Wextra -O2 "1.6 The Binomial Distribution and the Multinomial Distribution.cpp" -o p16
//      ./p16
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.1  P(A) = ways A can happen / things that can happen
//  1.2  counting those ways: n^r, nPr, nCr, with/without replacement
//  1.3  the language of sets
//  1.4  the complement trick, P(A) = 1 - P(A^c)
//  1.5  a crate on a dock: hypergeometric, with binomial as the shortcut
//  1.6  the binomial for its own sake                        <- you are here
//
//  In 1.5 the binomial turned up as a convenience - pretend the crate never
//  shrinks and the arithmetic gets easy. That undersold it badly. The
//  binomial is the most useful distribution in this whole course, and you
//  get it any time you repeat the same yes/no experiment a fixed number of
//  times. Coin flips, defective parts, clicks on an ad, free throws made,
//  days the market closed up. Same formula every time.
//
//  ---------------------------------------------------------------------------
//  THE FOUR THINGS THAT MAKE SOMETHING BINOMIAL
//  ---------------------------------------------------------------------------
//  Check these before you reach for the formula. If any one fails, you have
//  a different distribution on your hands.
//
//      1  FIXED n         you decided in advance how many trials to run
//      2  TWO OUTCOMES    each trial is a yes/no - success or failure
//      3  CONSTANT p      the success probability never changes
//      4  INDEPENDENT     no trial tells you anything about another
//
//  Condition 3 is exactly the one the crate in 1.5 broke: pull a bad part
//  out and the remaining crate is cleaner than it was, so p moved. That is
//  why 1.5 needed the hypergeometric and this lesson does not.
//
//  ---------------------------------------------------------------------------
//  WHERE THE FORMULA COMES FROM
//  ---------------------------------------------------------------------------
//  Everything below is assembled from two pieces you already own.
//
//      p^k (1-p)^(n-k)     how likely ONE particular sequence is
//      C(n, k)             how many sequences have k successes  (1.2)
//
//  Multiply them and you have the binomial. That really is the entire
//  derivation - the rest of this file is showing you it is true.
// ============================================================================

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
//  THE COUNTING TOOLS  (nCr and the multinomial coefficient, from 1.2 / 1.5)
// ============================================================================
std::string commasFwd(unsigned long long value);   // defined with the layout helpers

// C(n,r) built up step by step so the running value never explodes.
unsigned long long nCr(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r > n - r) r = n - r;
    unsigned long long result = 1;
    for (int i = 1; i <= r; ++i) {
        result *= static_cast<unsigned long long>(n - r + i);
        result /= static_cast<unsigned long long>(i);
    }
    return result;
}

// The multinomial coefficient  n! / (n1! n2! ... nk!), built by choosing each
// group out of what is left so no intermediate ever overflows.
unsigned long long multinomial(int n, const std::vector<int>& groups) {
    unsigned long long result = 1;
    int remaining = n;
    for (int g : groups) {
        result *= nCr(remaining, g);
        remaining -= g;
    }
    return result;
}

// log of C(n,r) via lgamma. Same discipline as 1.4 and 1.5: keep every
// intermediate small and exponentiate once, at the very end.
double logChoose(int n, int r) {
    if (r < 0 || r > n) return -std::numeric_limits<double>::infinity();
    return std::lgamma(n + 1.0) - std::lgamma(r + 1.0) - std::lgamma(n - r + 1.0);
}

double logMultinomial(int n, const std::vector<int>& groups) {
    double lg = std::lgamma(n + 1.0);
    for (int g : groups) lg -= std::lgamma(g + 1.0);
    return lg;
}

std::string bigNumber(double logValue, unsigned long long exactValue) {
    if (logValue < std::log(9.0e18)) return commasFwd(exactValue);
    std::ostringstream ss;
    ss << std::scientific << std::setprecision(3) << std::exp(logValue);
    return ss.str();
}

// ============================================================================
//  THE BINOMIAL DISTRIBUTION - the whole lesson in one function
//
//      P(X = k)  =  C(n, k)  x  p^k  x  (1-p)^(n-k)
//                   -------     ----------------
//                   how many     how likely
//                   sequences    any ONE of them is
//
//  Careful with the edges: p = 0 or p = 1 makes std::pow fine but
//  0 * log(0) turns into a NaN if you go through logs, so the powers stay
//  as powers here and only the coefficient goes through lgamma.
// ============================================================================
double binomialPmf(int n, int k, double p) {
    if (k < 0 || k > n) return 0.0;
    if (p <= 0.0) return (k == 0) ? 1.0 : 0.0;
    if (p >= 1.0) return (k == n) ? 1.0 : 0.0;
    return std::exp(logChoose(n, k) + k * std::log(p) + (n - k) * std::log1p(-p));
}

// P(X <= k), the cumulative version. Almost every real question ("at most",
// "at least", "between") is answered with this rather than a single P(X = k).
double binomialCdf(int n, int k, double p) {
    double total = 0.0;
    for (int i = 0; i <= k && i <= n; ++i) total += binomialPmf(n, i, p);
    return total > 1.0 ? 1.0 : total;
}

// P(X >= k). Written as the complement of the CDF - 1.4's trick, still
// paying for itself.
double binomialAtLeast(int n, int k, double p) {
    if (k <= 0) return 1.0;
    return 1.0 - binomialCdf(n, k - 1, p);
}

// THE MULTINOMIAL DISTRIBUTION - the same idea with more than two outcomes.
//
//      P(n1, n2, ..., nk) = n!/(n1! ... nk!) x p1^n1 x ... x pk^nk
//
// Set k = 2 and this IS the binomial: n!/(n1! n2!) is C(n, n1), and
// p2 = 1 - p1. Part 8 checks exactly that, in code.
double multinomialPmf(int n, const std::vector<int>& counts,
                      const std::vector<double>& probs) {
    double lg = logMultinomial(n, counts);
    for (size_t i = 0; i < counts.size(); ++i) {
        if (counts[i] == 0) continue;              // 0 * log(0) would be NaN
        if (probs[i] <= 0.0) return 0.0;           // impossible outcome, seen
        lg += counts[i] * std::log(probs[i]);
    }
    return std::exp(lg);
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.5)
// ============================================================================
const int PAGE = 70;

std::string repeat(const std::string& unit, int times) {
    std::string s;
    for (int i = 0; i < times; ++i) s += unit;
    return s;
}

std::string commas(unsigned long long value) {
    std::string digits = std::to_string(value), out;
    int count = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        out += digits[static_cast<size_t>(i)];
        if (++count % 3 == 0 && i > 0) out += ',';
    }
    return std::string(out.rbegin(), out.rend());
}

std::string commasFwd(unsigned long long value) { return commas(value); }

std::string bar(double p, int scale) {
    const int n = static_cast<int>(p * scale + 0.5);
    return n > 0 ? "   " + repeat("#", n) : "";
}

void part(int number, const std::string& name, const std::string& tag) {
    std::string left = " PART " + std::to_string(number) + "   " + name;
    int pad = PAGE - static_cast<int>(left.size() + tag.size());
    if (pad < 1) pad = 1;
    std::cout << "\n" << repeat("=", PAGE) << "\n"
              << left << std::string(static_cast<size_t>(pad), ' ') << tag
              << "\n" << repeat("=", PAGE) << "\n";
}

void question(const std::string& text) { std::cout << "\n  Q.  " << text << "\n"; }
void answer  (const std::string& text) { std::cout << "\n  A.  " << text << "\n"; }
void note    (const std::string& text) { std::cout << "\n      " << text << "\n"; }

void showP(const std::string& label, double p) {
    std::cout << "        " << std::left << std::setw(38) << label << std::right
              << std::fixed << std::setprecision(4) << p
              << std::setw(9) << std::setprecision(1) << p * 100.0 << "%\n"
              << std::setprecision(4);
}

void verifyClose(const std::string& claim, double formula, double other,
                 double tolerance, const std::string& otherLabel = "counted") {
    bool ok = std::fabs(formula - other) < tolerance;
    std::cout << "\n  " << (ok ? "ok " : "XX ") << std::left << std::setw(26)
              << claim << std::right << "formula " << std::fixed
              << std::setprecision(4) << formula
              << "   " << otherLabel << " " << other << "\n";
}

void verifyCount(const std::string& claim, unsigned long long formula,
                 unsigned long long listed) {
    std::cout << "\n  " << (formula == listed ? "ok " : "XX ") << std::left
              << std::setw(25) << claim << std::right
              << "formula " << std::setw(11) << commas(formula)
              << "  listed " << std::setw(11) << commas(listed) << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // The experiment we will argue about for most of this lesson.
    const int    FLIPS = 10;     // trials
    const double PHEAD = 0.60;   // a coin weighted 60/40, so the maths shows

    // ========================================================================
    part(1, "THE SETUP", "n trials, one yes/no each");
    // ========================================================================
    question("I flip a weighted coin 10 times. It comes up heads 60% of");
    std::cout << "      the time. What is the chance I see exactly 6 heads?\n";

    note("Before any formula, check this is even a binomial. Four");
    std::cout << "      boxes have to tick:\n"
        "\n"
        "         1  FIXED n        I said 10 flips up front, not\n"
        "                           'keep flipping until I get bored'\n"
        "\n"
        "         2  TWO OUTCOMES   heads or tails, nothing else\n"
        "\n"
        "         3  CONSTANT p     the coin does not get more heads-y\n"
        "                           because it just landed heads\n"
        "\n"
        "         4  INDEPENDENT    flip 7 knows nothing about flip 3\n";

    note("All four tick, so the binomial applies. Compare that with");
    std::cout << "      1.5's crate, where pulling a bad part out left the rest\n"
                 "      of the crate cleaner - box 3 failed, and that single\n"
                 "      failure is the whole reason 1.5 needed a different\n"
                 "      formula. The word for what a coin has and a crate has\n"
                 "      not is REPLACEMENT.\n";

    // ========================================================================
    part(2, "BUILDING THE FORMULA", "two pieces you already own");
    // ========================================================================
    question("Take it in two halves. First: how likely is ONE particular");
    std::cout << "      run of 10 flips - say H H T H H T H H H T?\n";

    note("Independent, so multiply (1.2's slot method). That run has");
    std::cout << "      7 heads and 3 tails, in that exact order:\n"
        "\n"
        "         0.6 x 0.6 x 0.4 x 0.6 x 0.6 x 0.4 x 0.6 x 0.6 x 0.6 x 0.4\n"
        "\n"
        "      Multiplication does not care about order, so gather them:\n"
        "\n"
        "         0.6^7 x 0.4^3  =  " << std::setprecision(10)
              << std::pow(0.6, 7) * std::pow(0.4, 3) << "\n" << std::setprecision(4);

    note("Here is the part that matters. EVERY run with 7 heads has");
    std::cout << "      that same probability, because it is the same ten numbers\n"
                 "      multiplied in a different order. HHHHHHHTTT, TTTHHHHHHH,\n"
                 "      HTHTHHHHHT - all of them, 0.6^7 x 0.4^3, every time.\n";

    question("Second half: how MANY runs of 10 flips have 7 heads?");

    note("That is 1.2, and nothing more. Choose which 7 of the 10");
    std::cout << "      slots hold a head; the tails fill in whatever is left:\n"
        "\n"
        "         C(10, 7)  =  " << commas(nCr(10, 7)) << " different runs\n";

    answer("Multiply the two halves. That is the binomial distribution.");
    std::cout << "\n"
        "         P(X = k)  =  C(n, k)  x  p^k  x  (1-p)^(n-k)\n"
        "                      --------    --------------------\n"
        "                      how many     how likely any\n"
        "                      runs have    single one of\n"
        "                      k successes  them is\n"
        "\n"
        "      For 7 heads in 10 flips of a 60% coin:\n"
        "\n"
        "         " << commas(nCr(10, 7)) << " x 0.6^7 x 0.4^3  =  "
              << binomialPmf(FLIPS, 7, PHEAD) << "\n";

    std::cout << "\n";
    showP("P(exactly 6 heads)", binomialPmf(FLIPS, 6, PHEAD));
    showP("P(exactly 7 heads)", binomialPmf(FLIPS, 7, PHEAD));

    note("Notice the shape of that answer: a COUNT times a");
    std::cout << "      PROBABILITY. Every distribution in this course is built\n"
                 "      the same way, and 1.5's multinomial was the same trick\n"
                 "      with three buckets instead of two.\n";

    // ========================================================================
    part(3, "DO NOT TRUST ME, LIST THEM", "all 2^n runs, by hand");
    // ========================================================================
    // The formula is short enough to be suspicious of. Four flips is small
    // enough to print every single outcome, so let us just look.
    question("Is that really right? Four flips is small enough to check");
    std::cout << "      by writing down all of them. 2^4 = 16 runs:\n";

    const int SMALL = 4;
    std::vector<std::string> runs;
    for (int mask = 0; mask < (1 << SMALL); ++mask) {
        std::string s;
        for (int bit = SMALL - 1; bit >= 0; --bit)
            s += ((mask >> bit) & 1) ? 'H' : 'T';
        runs.push_back(s);
    }
    std::cout << "\n      ";
    for (size_t i = 0; i < runs.size(); ++i) {
        if (i && i % 8 == 0) std::cout << "\n      ";
        std::cout << " " << runs[i];
    }
    std::cout << "\n";

    note("Now sort them into piles by how many heads they hold, and");
    std::cout << "      compare each pile size against C(4, k):\n";
    std::cout << "\n        heads k   runs listed   C(4,k)   the runs\n"
              << "        " << repeat("-", 58) << "\n";
    for (int k = 0; k <= SMALL; ++k) {
        std::vector<std::string> pile;
        for (const auto& r : runs)
            if (std::count(r.begin(), r.end(), 'H') == k) pile.push_back(r);
        std::cout << "        " << std::setw(7) << k << std::setw(14) << pile.size()
                  << std::setw(9) << commas(nCr(SMALL, k)) << "   ";
        for (const auto& r : pile) std::cout << " " << r;
        std::cout << "\n";
    }
    verifyCount("piles add to 2^4", 1ULL << SMALL, runs.size());

    note("Every pile size is exactly C(4,k) - the counting half is");
    std::cout << "      right. Now the probability half. Walk all 16 runs, work\n"
                 "      out each one's probability the long way, and add up the\n"
                 "      ones with k heads:\n";

    std::cout << "\n        heads k   added up by hand    the formula\n"
              << "        " << repeat("-", 58) << "\n";
    double listedTotal = 0.0;
    for (int k = 0; k <= SMALL; ++k) {
        double byHand = 0.0;
        for (const auto& r : runs) {
            int heads = static_cast<int>(std::count(r.begin(), r.end(), 'H'));
            if (heads != k) continue;
            double one = 1.0;                       // multiply out this one run
            for (char c : r) one *= (c == 'H') ? PHEAD : (1.0 - PHEAD);
            byHand += one;
        }
        listedTotal += byHand;
        std::cout << "        " << std::setw(7) << k << std::setw(18) << byHand
                  << std::setw(15) << binomialPmf(SMALL, k, PHEAD) << "\n";
    }
    verifyClose("every run, added up", listedTotal, 1.0, 1e-12, "must be");

    answer("Column by column, the formula and the brute count agree.");

    // ========================================================================
    part(4, "THE WHOLE DISTRIBUTION", "not one k, all of them");
    // ========================================================================
    // A single P(X = k) is rarely the interesting object. The DISTRIBUTION -
    // the whole row of them - is what you actually reason with.
    question("Stop asking about one k. What does the whole thing look");
    std::cout << "      like for our 10 flips of a 60% coin?\n";

    std::cout << "\n        heads   probability\n"
              << "        " << repeat("-", 58) << "\n";
    double total = 0.0;
    for (int k = 0; k <= FLIPS; ++k) {
        double p = binomialPmf(FLIPS, k, PHEAD);
        total += p;
        std::cout << "        " << std::setw(5) << k << std::setw(14) << p
                  << bar(p, 90) << "\n";
    }
    verifyClose("the whole distribution", total, 1.0, 1e-12, "must be");

    note("These are disjoint events covering everything that can");
    std::cout << "      happen - 1.3's partition - so they total 1 exactly.\n"
                 "      That is not a coincidence, it is a guarantee, and it is\n"
                 "      the cheapest sanity check you own.\n";

    note("Now the questions people actually ask. All three come off");
    std::cout << "      that one table, and none of them needs a new formula:\n";
    std::cout << "\n";
    showP("P(exactly 6 heads)",      binomialPmf(FLIPS, 6, PHEAD));
    showP("P(at most 6 heads)",      binomialCdf(FLIPS, 6, PHEAD));
    showP("P(at least 6 heads)",     binomialAtLeast(FLIPS, 6, PHEAD));
    showP("P(at least 1 head)",      binomialAtLeast(FLIPS, 1, PHEAD));
    showP("P(between 4 and 7 heads)", binomialCdf(FLIPS, 7, PHEAD)
                                      - binomialCdf(FLIPS, 3, PHEAD));

    note("Look at 'at least 1 head'. Adding k = 1..10 works, but it");
    std::cout << "      is ten terms. 1.4 says flip it - the only way to fail is\n"
                 "      all ten tails:\n"
        "\n"
        "         1 - 0.4^10  =  1 - " << std::setprecision(8) << std::pow(0.4, 10)
              << "  =  " << 1.0 - std::pow(0.4, 10) << "\n" << std::setprecision(4);
    verifyClose("at least one head", binomialAtLeast(FLIPS, 1, PHEAD),
                1.0 - std::pow(0.4, 10), 1e-12, "1 - 0.4^10");

    note("Same lesson as 1.4 and 1.5: the complement turns a long");
    std::cout << "      sum into one term. It keeps working because it is not a\n"
                 "      trick about coins, it is a fact about sets.\n";

    // ========================================================================
    part(5, "PASCAL'S TRIANGLE", "where the coefficients live");
    // ========================================================================
    // The C(n,k) row is worth seeing all at once - it explains the symmetry
    // of the fair-coin case and it connects straight to algebra.
    question("Those C(n,k) numbers - is there a pattern to them?");

    note("Stack the rows. Row n holds C(n,0) up to C(n,n):");
    std::cout << "\n";
    const int ROWS = 10;
    for (int n = 0; n <= ROWS; ++n) {
        std::string line;
        for (int k = 0; k <= n; ++k) {
            std::ostringstream cell;
            cell << std::setw(4) << commas(nCr(n, k));
            line += cell.str();
        }
        int lead = 8 + (ROWS - n) * 2;
        std::cout << std::string(static_cast<size_t>(lead), ' ') << line << "\n";
    }

    note("Every number is the two above it added together, because");
    std::cout << "      C(n,k) = C(n-1,k-1) + C(n-1,k). That identity is just a\n"
                 "      question about one particular coin: either flip n was a\n"
                 "      head (so k-1 heads came from the first n-1 flips) or it\n"
                 "      was a tail (so all k did). Two disjoint cases - 1.3.\n";

    std::cout << "\n        n    row of C(n,k)                      row total\n"
              << "        " << repeat("-", 58) << "\n";
    for (int n : {3, 5, 8, 10}) {
        unsigned long long sum = 0;
        std::ostringstream row;
        for (int k = 0; k <= n; ++k) { sum += nCr(n, k); row << " " << nCr(n, k); }
        std::cout << "        " << std::setw(2) << n << std::setw(38) << row.str()
                  << std::setw(11) << commas(sum) << "\n";
    }

    note("Each row totals 2^n. Of course it does - the row counts");
    std::cout << "      every run of n flips, sorted into piles by head count,\n"
                 "      and there are 2^n runs. That is 1.2's n^r with r = n and\n"
                 "      n = 2, arriving from the other direction.\n";

    note("Same numbers, third disguise - the BINOMIAL THEOREM:");
    std::cout << "\n"
        "         (a + b)^n  =  sum over k of  C(n,k) a^k b^(n-k)\n"
        "\n"
        "      Set a = p and b = 1-p and the left side is (p + 1 - p)^n = 1,\n"
        "      which is precisely the 'distribution adds to 1' check from\n"
        "      Part 4. The binomial distribution is named after this theorem,\n"
        "      not the other way round.\n";

    // Verify the theorem numerically on an arbitrary pair.
    {
        const double A = 2.0, B = 3.0;
        const int N = 7;
        double expanded = 0.0;
        for (int k = 0; k <= N; ++k)
            expanded += static_cast<double>(nCr(N, k)) * std::pow(A, k)
                        * std::pow(B, N - k);
        verifyClose("(2+3)^7 by expansion", expanded, std::pow(A + B, N),
                    1e-6, "direct  ");
    }

    // ========================================================================
    part(6, "MEAN AND SPREAD", "np and np(1-p)");
    // ========================================================================
    // Two numbers summarise the whole table, and both are worth trusting
    // more than the table itself once n gets large.
    question("Where does the distribution sit, and how wide is it?");

    note("Guess the centre first. Ten flips, heads 60% of the time,");
    std::cout << "      so you expect about 6 heads. That guess is exactly right,\n"
                 "      and it is right for a reason worth knowing:\n"
        "\n"
        "         MEAN  =  n x p\n"
        "\n"
        "      Each flip contributes p heads on average. Ten flips, add\n"
        "      them up, 10 x 0.6 = 6. Expected values add even when the\n"
        "      things they describe are tangled together.\n";

    // Compute the mean and variance the slow, honest way and compare.
    double mean = 0.0, meanSq = 0.0;
    for (int k = 0; k <= FLIPS; ++k) {
        double p = binomialPmf(FLIPS, k, PHEAD);
        mean   += k * p;
        meanSq += static_cast<double>(k) * k * p;
    }
    double variance = meanSq - mean * mean;

    verifyClose("mean, summed k x P(k)", mean, FLIPS * PHEAD, 1e-9, "n x p   ");
    verifyClose("variance, summed",      variance,
                FLIPS * PHEAD * (1.0 - PHEAD), 1e-9, "np(1-p) ");

    note("The spread is np(1-p), and the (1-p) is doing something");
    std::cout << "      you can feel. A coin at p = 0.5 is the most uncertain\n"
                 "      coin there is; push p towards 0 or 1 and the outcome\n"
                 "      gets easier to call, so the variance collapses:\n";
    std::cout << "\n        p       mean    variance   std dev\n"
              << "        " << repeat("-", 58) << "\n";
    for (double pp : {0.01, 0.10, 0.30, 0.50, 0.70, 0.90, 0.99}) {
        double v = FLIPS * pp * (1.0 - pp);
        std::cout << "        " << std::setw(4) << pp << std::setw(9) << FLIPS * pp
                  << std::setw(11) << v << std::setw(10) << std::sqrt(v)
                  << bar(v / 2.5, 18) << "\n";
    }

    answer("Widest at p = 0.5, vanishing at both ends.");
    note("A coin that always lands heads has no spread at all -");
    std::cout << "      there is nothing left to be uncertain about.\n";

    // ========================================================================
    part(7, "THE SHAPE AS n GROWS", "a bell turns up uninvited");
    // ========================================================================
    // This is the single most important picture in elementary probability,
    // and it costs nothing to draw here.
    question("What happens to that ragged little table when n gets big?");

    note("Fair coin, three different n. Watch the outline:");
    for (int n : {4, 10, 30}) {
        std::cout << "\n        n = " << n << "   (mean " << n * 0.5
                  << ", std dev " << std::sqrt(n * 0.25) << ")\n";
        for (int k = 0; k <= n; ++k) {
            double p = binomialPmf(n, k, 0.5);
            if (p < 0.002) continue;               // skip the invisible tails
            std::cout << "        " << std::setw(5) << k << std::setw(12) << p
                      << bar(p, 120) << "\n";
        }
    }

    answer("It is turning into a bell, and it did not ask permission.");

    note("That bell is the NORMAL DISTRIBUTION, and the fact that a");
    std::cout << "      pile of yes/no trials drifts towards it is the central\n"
                 "      limit theorem in its first disguise. It is the last\n"
                 "      video in this chapter - all we need today is to have\n"
                 "      seen it happen.\n";

    note("Here is the practical half of the same fact. The spread");
    std::cout << "      grows like sqrt(n) while the count grows like n, so the\n"
                 "      spread RELATIVE to n shrinks. More trials means a\n"
                 "      proportion you can actually trust:\n";
    std::cout << "\n        n         mean    std dev   std dev as % of n\n"
              << "        " << repeat("-", 58) << "\n";
    for (int n : {10, 100, 1000, 10000, 100000}) {
        double sd = std::sqrt(n * 0.25);
        std::cout << "        " << std::setw(7) << commas(static_cast<unsigned long long>(n))
                  << std::setw(11) << n * 0.5 << std::setw(11) << sd
                  << std::setw(14) << std::setprecision(2) << 100.0 * sd / n << "%\n"
                  << std::setprecision(4);
    }
    note("This is why polls sample thousands and why one more coin");
    std::cout << "      flip tells you almost nothing. To halve your error you\n"
                 "      have to QUADRUPLE your sample. That trade is the whole\n"
                 "      economics of measurement.\n";

    // ========================================================================
    part(8, "MORE THAN TWO OUTCOMES", "the multinomial");
    // ========================================================================
    // 1.5 met the multinomial on a factory floor. Here it earns its place as
    // the honest generalisation of everything above.
    question("A coin has two faces. What about a die, or a survey with");
    std::cout << "      five answers, or 1.5's pass/rework/scrap?\n";

    note("Nothing changes except the bookkeeping. Same two halves:");
    std::cout << "\n"
        "         HOW MANY orders give that split?\n"
        "               n! / (n1! n2! ... nk!)     the multinomial coefficient\n"
        "\n"
        "         HOW LIKELY is any one such order?\n"
        "               p1^n1 x p2^n2 x ... x pk^nk\n"
        "\n"
        "      Multiply. That is the MULTINOMIAL DISTRIBUTION, and the\n"
        "      binomial is just the case where k = 2.\n";

    // Prove the claim rather than asserting it.
    note("Prove that last sentence instead of taking it on faith.");
    std::cout << "      Run our 10 flips through BOTH formulas - binomial with\n"
                 "      p = 0.6, multinomial with two buckets {0.6, 0.4}:\n";
    std::cout << "\n        heads   binomial      multinomial\n"
              << "        " << repeat("-", 58) << "\n";
    double worstGap = 0.0;
    for (int k = 0; k <= FLIPS; ++k) {
        double b = binomialPmf(FLIPS, k, PHEAD);
        double m = multinomialPmf(FLIPS, {k, FLIPS - k}, {PHEAD, 1.0 - PHEAD});
        worstGap = std::max(worstGap, std::fabs(b - m));
        std::cout << "        " << std::setw(5) << k << std::setw(13) << b
                  << std::setw(16) << m << "\n";
    }
    verifyClose("binomial IS multinomial", worstGap, 0.0, 1e-12, "gap of  ");

    note("Now a genuinely three-way case. Roll a fair die 12 times");
    std::cout << "      and call it LOW (1-2), MID (3-4), HIGH (5-6) - each a\n"
                 "      third. How likely is a tidy 4 / 4 / 4?\n";

    const std::vector<double> thirds = {1.0 / 3, 1.0 / 3, 1.0 / 3};
    std::cout << "\n"
        "         12! / (4! 4! 4!)  =  " << commas(multinomial(12, {4, 4, 4}))
              << " orders\n"
        "         (1/3)^4 x (1/3)^4 x (1/3)^4  =  " << std::setprecision(10)
              << std::pow(1.0 / 3, 12) << "\n" << std::setprecision(4);
    std::cout << "\n";
    showP("P(4 low, 4 mid, 4 high)", multinomialPmf(12, {4, 4, 4}, thirds));
    showP("P(12 low, 0 mid, 0 high)", multinomialPmf(12, {12, 0, 0}, thirds));
    showP("P(6 low, 4 mid, 2 high)",  multinomialPmf(12, {6, 4, 2}, thirds));

    note("The perfectly even split is the most likely SINGLE split,");
    std::cout << "      but it is still only about 1 chance in 12. 'Most likely'\n"
                 "      and 'likely' are different words - there are simply so\n"
                 "      many ways for 12 rolls to come out that no one of them\n"
                 "      gets much of the probability.\n";

    // The partition check again, now over every three-way split.
    double grand = 0.0;
    int splits = 0;
    for (int a = 0; a <= 12; ++a)
        for (int b = 0; a + b <= 12; ++b) {
            grand += multinomialPmf(12, {a, b, 12 - a - b}, thirds);
            ++splits;
        }
    note("There are " + std::to_string(splits) + " ways to split 12 rolls into three");
    std::cout << "      buckets, and they are disjoint and exhaustive, so:\n";
    verifyClose("all splits of 12 rolls", grand, 1.0, 1e-12, "must be");

    // ========================================================================
    part(9, "A REAL PROBLEM", "the binomial earns its keep");
    // ========================================================================
    // 1.1's fourth world again - the same arithmetic, pointed at money.
    question("A strategy wins 55% of its trades. You take 100 trades.");
    std::cout << "      How often does a genuinely good strategy still lose?\n";

    const int    TRADES = 100;
    const double EDGE   = 0.55;

    note("Four boxes first, honestly. n is fixed at 100, each trade");
    std::cout << "      wins or loses, the edge is assumed constant, and trades\n"
                 "      are assumed independent. The last two are ASSUMPTIONS,\n"
                 "      not facts - real edges decay and real trades correlate.\n"
                 "      Say so out loud, then use the model anyway.\n";

    std::cout << "\n";
    showP("mean wins in 100 trades", TRADES * EDGE);
    showP("P(exactly 55 wins)",  binomialPmf(TRADES, 55, EDGE));
    showP("P(at least 50 wins)", binomialAtLeast(TRADES, 50, EDGE));
    showP("P(a losing 100 - under 50)", binomialCdf(TRADES, 49, EDGE));

    answer("A real 55% edge still shows a losing hundred about 1 time in 6.");

    note("Which is the useful number here, and it is not the mean.");
    std::cout << "      Anyone can quote 55%. The question that decides whether\n"
                 "      you survive is how bad the normal bad stretch looks:\n";
    std::cout << "\n        trades    P(you are behind at the end)\n"
              << "        " << repeat("-", 58) << "\n";
    for (int n : {10, 20, 50, 100, 500, 1000, 5000}) {
        int half = (n - 1) / 2;                    // strictly fewer than half
        double behind = binomialCdf(n, half, EDGE);
        std::cout << "        " << std::setw(6) << commas(static_cast<unsigned long long>(n))
                  << std::setw(15) << behind << bar(behind, 40) << "\n";
    }

    note("The edge only becomes visible through repetition. At ten");
    std::cout << "      trades a 55% strategy and a coin are indistinguishable;\n"
                 "      by five thousand the question is settled. That is Part 7's\n"
                 "      sqrt(n) again, wearing a suit.\n";

    // The other side of the same coin: how many trials to detect the edge.
    int needed = 10;
    while (needed < 20000 && binomialCdf(needed, (needed - 1) / 2, EDGE) > 0.05)
        needed += 10;
    note("Turn it around - how many trades before a 55% edge is");
    std::cout << "      ahead 95% of the time? About " << commas(static_cast<unsigned long long>(needed))
              << ". Below that number you are\n"
                 "      not measuring the strategy, you are measuring noise.\n";

    // ========================================================================
    part(10, "DO NOT TRUST ME, SIMULATE IT", "200,000 runs");
    // ========================================================================
    // Same discipline as 1.4: the formula is only convincing once something
    // dumb and mechanical agrees with it.
    question("The formula says P(6 heads in 10) is what it is. Does a");
    std::cout << "      machine that just flips coins agree?\n";

    std::mt19937 rng(1729);
    std::bernoulli_distribution flip(PHEAD);
    const int TRIALS = 200000;

    std::vector<int> tally(static_cast<size_t>(FLIPS) + 1, 0);
    for (int t = 0; t < TRIALS; ++t) {
        int heads = 0;
        for (int f = 0; f < FLIPS; ++f) if (flip(rng)) ++heads;
        ++tally[static_cast<size_t>(heads)];
    }

    std::cout << "\n        heads    formula     simulated    difference\n"
              << "        " << repeat("-", 58) << "\n";
    for (int k = 0; k <= FLIPS; ++k) {
        double f = binomialPmf(FLIPS, k, PHEAD);
        double s = static_cast<double>(tally[static_cast<size_t>(k)]) / TRIALS;
        std::cout << "        " << std::setw(5) << k << std::setw(12) << f
                  << std::setw(13) << s << std::setw(14) << std::fabs(f - s) << "\n";
    }
    verifyClose("P(6 heads), 200k runs", binomialPmf(FLIPS, 6, PHEAD),
                static_cast<double>(tally[6]) / TRIALS, 5e-3, "simulated");

    // The mean too, since Part 6 claimed it without proof.
    double simMean = 0.0;
    for (int k = 0; k <= FLIPS; ++k)
        simMean += k * static_cast<double>(tally[static_cast<size_t>(k)]) / TRIALS;
    verifyClose("mean heads, 200k runs", FLIPS * PHEAD, simMean, 5e-3, "simulated");

    answer("Agreement to three decimals, from a machine that knows no maths.");

    // ========================================================================
    part(11, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Put your own experiment in. Every answer shows working.\n"
                 "\n"
                 "        1   distribution  - n trials at p%, the whole table\n"
                 "        2   one question  - P(exactly / at most / at least k)\n"
                 "        3   how many      - trials needed to be X% sure\n"
                 "        4   three-way     - multinomial with three buckets\n"
                 "        5   quit\n";

    auto askNumber = [](const std::string& prompt, int low, int high, int& out) {
        while (true) {
            std::cout << "\n      " << prompt << " (" << low << "-" << high << "): ";
            std::cout.flush();
            if (std::cin >> out) {
                if (out >= low && out <= high) return true;
                std::cout << "      Needs to be between " << low << " and "
                          << high << ".\n";
                continue;
            }
            if (std::cin.eof()) return false;       // stream ended, stop cleanly
            std::cin.clear();
            std::string junk;
            std::getline(std::cin, junk);
            std::cout << "      That is not a number.\n";
        }
    };

    bool keepGoing = true;
    bool firstRound = true;
    while (keepGoing) {
        if (!firstRound)
            std::cout << "\n      " << repeat("-", 60) << "\n"
                         "      1 table   2 question   3 how many   4 three-way   5 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 5, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            int n = 0, pct = 0;
            if (!askNumber("Number of trials", 1, 60, n)) { keepGoing = false; break; }
            if (!askNumber("Success chance, in %", 0, 100, pct)) { keepGoing = false; break; }
            const double p = pct / 100.0;

            std::cout << "\n      " << n << " trials at " << pct << "% each.\n";
            std::cout << "\n        successes   probability\n"
                      << "        " << repeat("-", 58) << "\n";
            double sum = 0.0;
            double best = 0.0;
            int bestK = 0;
            for (int k = 0; k <= n; ++k) {
                double q = binomialPmf(n, k, p);
                sum += q;
                if (q > best) { best = q; bestK = k; }
                if (q >= 0.0005 || n <= 20)
                    std::cout << "        " << std::setw(9) << k << std::setw(14) << q
                              << bar(q, 90) << "\n";
            }
            if (n > 20) note("(rows under 0.05% left out to keep the table short)");
            std::cout << "\n";
            showP("mean, n x p",          n * p);
            showP("variance, np(1-p)",    n * p * (1.0 - p));
            showP("std dev",              std::sqrt(n * p * (1.0 - p)));
            std::cout << "\n      Most likely single outcome: " << bestK
                      << " success(es).\n";
            verifyClose("table adds to 1", sum, 1.0, 1e-9, "must be");
            break;
        }
        case 2: {
            int n = 0, pct = 0, k = 0;
            if (!askNumber("Number of trials", 1, 400, n)) { keepGoing = false; break; }
            if (!askNumber("Success chance, in %", 0, 100, pct)) { keepGoing = false; break; }
            if (!askNumber("How many successes, k", 0, n, k)) { keepGoing = false; break; }
            const double p = pct / 100.0;

            std::cout << "\n      " << n << " trials at " << pct << "%, asking about k = "
                      << k << ".\n";
            std::cout << "\n        STEP 1  how many runs have exactly " << k << "?\n"
                         "                C(" << n << "," << k << ")  =  "
                      << bigNumber(logChoose(n, k), nCr(n, k)) << "\n";
            std::cout << "\n        STEP 2  how likely is any ONE of them?\n"
                         "                " << p << "^" << k << " x " << 1.0 - p
                      << "^" << n - k << "  =  " << std::setprecision(10)
                      << std::pow(p, k) * std::pow(1.0 - p, n - k) << "\n"
                      << std::setprecision(4);
            std::cout << "\n        STEP 3  multiply\n\n";
            showP("P(exactly k)",   binomialPmf(n, k, p));
            showP("P(at most k)",   binomialCdf(n, k, p));
            showP("P(at least k)",  binomialAtLeast(n, k, p));
            showP("P(fewer than k)", k > 0 ? binomialCdf(n, k - 1, p) : 0.0);
            showP("P(more than k)", binomialAtLeast(n, k + 1, p));
            break;
        }
        case 3: {
            int pct = 0, confidence = 0;
            if (!askNumber("Success chance per trial, in %", 1, 99, pct)) {
                keepGoing = false; break;
            }
            if (!askNumber("How sure of at least one success, in %", 1, 99, confidence)) {
                keepGoing = false; break;
            }
            const double p = pct / 100.0;
            const double want = confidence / 100.0;

            int trials = 1;
            while (trials < 100000 && binomialAtLeast(trials, 1, p) < want) ++trials;

            std::cout << "\n      Each trial succeeds " << pct << "% of the time.\n";
            std::cout << "\n        P(no success in n tries)  =  " << 1.0 - p
                      << "^n\n"
                         "        P(at least one)           =  1 - " << 1.0 - p
                      << "^n      <- 1.4\n";
            std::cout << "\n      Solve that for n:\n";
            std::cout << "\n        n  >=  log(1 - " << want << ") / log(" << 1.0 - p
                      << ")  =  " << std::setprecision(2)
                      << std::log(1.0 - want) / std::log(1.0 - p) << "\n"
                      << std::setprecision(4);
            std::cout << "\n      So you need " << trials << " trial(s).\n\n";
            showP("P(at least one) at that n", binomialAtLeast(trials, 1, p));
            if (trials > 1)
                showP("P(at least one) one fewer", binomialAtLeast(trials - 1, 1, p));
            break;
        }
        case 4: {
            int n1 = 0, n2 = 0, n3 = 0, q1 = 0, q2 = 0;
            if (!askNumber("Count in bucket A", 0, 40, n1)) { keepGoing = false; break; }
            if (!askNumber("Count in bucket B", 0, 40, n2)) { keepGoing = false; break; }
            if (!askNumber("Count in bucket C", 0, 40, n3)) { keepGoing = false; break; }
            const int n = n1 + n2 + n3;
            if (n == 0) { note("Nothing to count."); break; }
            if (!askNumber("Chance of A, in %", 1, 98, q1)) { keepGoing = false; break; }
            if (!askNumber("Chance of B, in % (A+B must stay under 100)",
                           1, 99 - q1, q2)) { keepGoing = false; break; }

            const std::vector<double> probs = {q1 / 100.0, q2 / 100.0,
                                               (100 - q1 - q2) / 100.0};
            std::cout << "\n      " << n << " items: " << n1 << " A, " << n2
                      << " B, " << n3 << " C.\n"
                         "      Rates " << q1 << "% / " << q2 << "% / "
                      << 100 - q1 - q2 << "%.\n";
            std::cout << "\n        STEP 1  how many ORDERS give that split\n"
                         "                " << n << "! / (" << n1 << "! " << n2
                      << "! " << n3 << "!)  =  "
                      << bigNumber(logMultinomial(n, {n1, n2, n3}),
                                   multinomial(n, {n1, n2, n3})) << "\n";
            double one = std::pow(probs[0], n1) * std::pow(probs[1], n2)
                       * std::pow(probs[2], n3);
            std::cout << "\n        STEP 2  how likely is ONE such order\n"
                         "                " << std::setprecision(10) << one << "\n"
                      << std::setprecision(4);
            std::cout << "\n        STEP 3  multiply\n\n";
            showP("P(exactly that split)", multinomialPmf(n, {n1, n2, n3}, probs));
            note("And the binomial hiding inside it - A against not-A:");
            std::cout << "\n";
            showP("P(exactly " + std::to_string(n1) + " of A)",
                  binomialPmf(n, n1, probs[0]));
            break;
        }
        case 5:
        default:
            note("Done. Run it again any time.");
            keepGoing = false;
            break;
        }
    }

    // ========================================================================
    part(12, "WHAT YOU LEARNED", "1.6 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      WHEN IT IS BINOMIAL - all four, or it is not\n"
        "          fixed n, two outcomes, constant p, independent trials\n"
        "          break 'constant p' and you are back in 1.5\n"
        "\n"
        "      THE DISTRIBUTION\n"
        "          P(X = k)  =  C(n,k) x p^k x (1-p)^(n-k)\n"
        "                       ------   ----------------\n"
        "                       how many   how likely\n"
        "                       runs       any one run is\n"
        "\n"
        "      THE QUESTIONS PEOPLE ACTUALLY ASK\n"
        "          exactly k     one term\n"
        "          at most k     add k' = 0..k              the CDF\n"
        "          at least k    1 - (at most k-1)          1.4 again\n"
        "          at least 1    1 - (1-p)^n                one term\n"
        "\n"
        "      MEAN AND SPREAD\n"
        "          mean      n x p\n"
        "          variance  n x p x (1-p)      widest at p = 0.5\n"
        "          std dev   sqrt(np(1-p))      grows like sqrt(n)\n"
        "\n"
        "      WHY sqrt(n) MATTERS\n"
        "          count grows like n, spread grows like sqrt(n), so the\n"
        "          spread as a FRACTION shrinks - to halve your error you\n"
        "          must quadruple your sample\n"
        "\n"
        "      PASCAL AND THE BINOMIAL THEOREM\n"
        "          C(n,k) = C(n-1,k-1) + C(n-1,k)   each entry, two above\n"
        "          row n adds to 2^n                every run of n flips\n"
        "          (a+b)^n = sum C(n,k) a^k b^(n-k) where the name comes from\n"
        "\n"
        "      THE MULTINOMIAL - two outcomes becomes k outcomes\n"
        "          n!/(n1!...nk!) x p1^n1 x ... x pk^nk\n"
        "          set k = 2 and it IS the binomial\n"
        "\n"
        "      THE SHAPE\n"
        "          large n pushes the binomial towards a bell - that is\n"
        "          the normal distribution, and it closes this chapter\n"
        "\n"
        "      COMING NEXT\n"
        "          1.7 conditional probability - what happens to P(A)\n"
        "          once somebody tells you B happened.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
