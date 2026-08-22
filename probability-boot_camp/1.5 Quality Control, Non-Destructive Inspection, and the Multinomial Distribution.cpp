// ============================================================================
//  1.5 - QUALITY CONTROL, NON-DESTRUCTIVE INSPECTION,
//        AND THE MULTINOMIAL DISTRIBUTION
//
//  Build & run (quote the name, it has spaces and commas):
//      g++ -std=c++17 -Wall -Wextra -O2 "1.5 Quality Control, Non-Destructive Inspection, and the Multinomial Distribution.cpp" -o p15
//      ./p15
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.1  P(A) = ways A can happen / things that can happen
//  1.2  counting those ways: n^r, nPr, nCr, with/without replacement
//  1.3  the language of sets
//  1.4  the complement trick, P(A) = 1 - P(A^c)
//  1.5  all of it, pointed at a factory floor                <- you are here
//
//  This is the first lesson where the maths is doing somebody's actual job.
//  A crate of 1,000 parts arrives. Some are bad. You cannot check them all.
//  How many do you open, and what does the answer actually tell you?
//
//  ---------------------------------------------------------------------------
//  DESTRUCTIVE vs NON-DESTRUCTIVE - why this is a probability problem at all
//  ---------------------------------------------------------------------------
//  NON-DESTRUCTIVE inspection leaves the part usable afterwards: X-ray,
//  ultrasound, a camera, a gauge. You could in principle test everything -
//  it just costs time and money.
//
//  DESTRUCTIVE testing wrecks the part: crash tests, tensile tests, burning
//  a fuse to see when it blows. Test all 1,000 and you have 1,000 broken
//  parts and nothing to ship.
//
//  Either way you end up sampling, and the moment you sample you are doing
//  probability. The key fact: a sample is drawn WITHOUT REPLACEMENT - once
//  you pull a part out of the crate to inspect it, it is not in the crate
//  any more. That single word decides which formula you need.
// ============================================================================

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
//  THE COUNTING TOOLS  (nCr comes straight from 1.2)
// ============================================================================
std::string commasFwd(unsigned long long value);   // defined with the layout helpers

// C(n,r) built up step by step so the running value never explodes.
// Good to well past C(200,10); beyond that you would want long double.
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

// THE MULTINOMIAL COEFFICIENT
//
//      n!  /  (n1! x n2! x ... x nk!)
//
// = the number of ways to split n labelled things into groups of the given
// sizes. Rather than build n! and cancel it, we choose the groups one at a
// time out of what is left - which is the same number and never overflows:
//
//      C(n, n1) x C(n-n1, n2) x C(n-n1-n2, n3) x ...
unsigned long long multinomial(int n, const std::vector<int>& groups) {
    unsigned long long result = 1;
    int remaining = n;
    for (int g : groups) {
        result *= nCr(remaining, g);
        remaining -= g;
    }
    return result;
}

// log of C(n,r), via lgamma (the log of the factorial).
//
// WHY THIS EXISTS: the integer nCr above is exact but it overflows fast.
// C(200,13) is already 8.8e19 and a 64-bit integer stops at 1.8e19, so a
// naive C(D,k)*C(N-D,n-k)/C(N,n) silently returns nonsense for realistic
// batch sizes. Working in logs keeps every intermediate small, and we only
// exponentiate at the very end - the same discipline as 1.4's birthday
// product, applied to combinations instead of fractions.
double logChoose(int n, int r) {
    if (r < 0 || r > n) return -std::numeric_limits<double>::infinity();
    return std::lgamma(n + 1.0) - std::lgamma(r + 1.0) - std::lgamma(n - r + 1.0);
}

// log of the multinomial coefficient, for the same reason.
double logMultinomial(int n, const std::vector<int>& groups) {
    double lg = std::lgamma(n + 1.0);
    for (int g : groups) lg -= std::lgamma(g + 1.0);
    return lg;
}

// Exact digits when they fit in 64 bits, scientific notation when they do not.
std::string bigNumber(double logValue, unsigned long long exactValue) {
    if (logValue < std::log(9.0e18)) return commasFwd(exactValue);
    std::ostringstream ss;
    ss << std::scientific << std::setprecision(3) << std::exp(logValue);
    return ss.str();
}

// THE HYPERGEOMETRIC PROBABILITY - sampling WITHOUT replacement.
//
// A batch of N parts holds D bad ones. You inspect n of them.
// P(exactly k of your n are bad):
//
//          C(D, k)  x  C(N-D, n-k)        pick k bad, then n-k good
//          -----------------------        ---------------------------
//               C(N, n)                   out of every possible sample
//
// Straight out of 1.1: ways it can happen over ways anything can happen.
double hypergeometric(int N, int D, int n, int k) {
    if (k < 0 || k > n || k > D || (n - k) > (N - D)) return 0.0;
    return std::exp(logChoose(D, k) + logChoose(N - D, n - k) - logChoose(N, n));
}

// THE BINOMIAL - the same question WITH replacement, i.e. if you put each
// part back before drawing the next, or if the batch is so huge that taking
// one out barely changes the mix.
double binomial(int n, int k, double p) {
    return std::exp(logChoose(n, k)) * std::pow(p, k) * std::pow(1.0 - p, n - k);
}

// THE MULTINOMIAL DISTRIBUTION - more than two outcomes per item.
// Each item independently lands in category i with probability p[i].
//
//      P(n1 of type 1, n2 of type 2, ...) =
//              n!/(n1! n2! ...) x p1^n1 x p2^n2 x ...
//
// The coefficient counts the orders it could have happened in; the powers
// give the probability of any one such order.
double multinomialPmf(int n, const std::vector<int>& counts,
                      const std::vector<double>& probs) {
    double lg = logMultinomial(n, counts);
    for (size_t i = 0; i < counts.size(); ++i)
        lg += counts[i] * std::log(probs[i]);
    return std::exp(lg);
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.4)
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

// Bridges the maths section above to commas() down here.
std::string commasFwd(unsigned long long value) { return commas(value); }

// A bar-chart cell. Returns "" for a bar of length zero, so a tiny
// probability never leaves trailing spaces at the end of its line.
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

    // The crate we will argue about for most of this lesson.
    const int BATCH   = 100;   // parts in the crate
    const int BAD     = 10;    // how many are actually defective
    const int INSPECT = 5;     // how many we pull out and check

    // ========================================================================
    part(1, "THE JOB", "a crate just arrived");
    // ========================================================================
    question("A crate of 100 parts lands on the dock. You happen to");
    std::cout << "      know 10 of them are defective. You pull out 5 at random\n"
                 "      and inspect them. What will you find?\n";

    note("First, why we sample at all. Two kinds of inspection:");
    std::cout << "\n"
        "         NON-DESTRUCTIVE   X-ray, ultrasound, a gauge, a camera.\n"
        "                           The part survives. You COULD test all\n"
        "                           100 - it just costs time and money.\n"
        "\n"
        "         DESTRUCTIVE       crash tests, tensile tests, burning a\n"
        "                           fuse to see when it blows. Test all 100\n"
        "                           and you have 100 broken parts and\n"
        "                           nothing left to sell.\n";

    note("Either way you sample - and the moment you sample, this");
    std::cout << "      is a probability problem. One word decides everything:\n"
                 "      a part you have pulled out is NOT still in the crate.\n"
                 "      That is sampling WITHOUT REPLACEMENT, from 1.2.\n";

    // ========================================================================
    part(2, "COUNTING THE SAMPLE", "C(D,k) x C(N-D,n-k) / C(N,n)");
    // ========================================================================
    question("What is the chance that exactly ONE of my 5 is bad?");

    note("Use 1.1's formula. Bottom first - every sample I could");
    std::cout << "      possibly have drawn. Order does not matter (a sample is\n"
                 "      a set of parts, not a sequence), so it is a combination:\n"
        "\n"
        "         C(100, 5)  =  " << commas(nCr(BATCH, INSPECT))
              << " possible samples\n";

    note("Top next - samples with exactly 1 bad part. Build one:");
    std::cout << "\n"
        "         choose 1 bad part from the 10 bad      C(10, 1) = "
              << commas(nCr(BAD, 1)) << "\n"
        "         choose 4 good parts from the 90 good   C(90, 4) = "
              << commas(nCr(BATCH - BAD, INSPECT - 1)) << "\n"
        "\n"
        "      Those are two independent choices, so MULTIPLY them\n"
        "      (the slot method from 1.2):\n"
        "\n"
        "         " << commas(nCr(BAD, 1)) << " x "
              << commas(nCr(BATCH - BAD, INSPECT - 1)) << "  =  "
              << commas(nCr(BAD, 1) * nCr(BATCH - BAD, INSPECT - 1))
              << " samples\n";

    answer("Divide, and that is the HYPERGEOMETRIC probability:");
    std::cout << "\n"
        "                   C(10,1) x C(90,4)\n"
        "         P(1 bad) = ------------------  =  "
              << std::setprecision(4) << hypergeometric(BATCH, BAD, INSPECT, 1)
              << "\n"
        "                        C(100,5)\n";

    // Trust it on a case small enough to enumerate completely.
    note("Check it where every sample can be listed. A crate of 5");
    std::cout << "      with 2 bad, inspect 2. There are C(5,2) = 10 samples;\n"
                 "      count how many hold exactly one bad part:\n";

    // Parts 0 and 1 are the bad ones; 2, 3, 4 are good.
    int matching = 0, totalSamples = 0;
    for (int a = 0; a < 5; ++a)
        for (int b = a + 1; b < 5; ++b) {         // a < b, so each set once
            ++totalSamples;
            int bad = (a < 2 ? 1 : 0) + (b < 2 ? 1 : 0);
            if (bad == 1) ++matching;
        }
    std::cout << "\n        samples in total      " << totalSamples << "\n"
              << "        with exactly one bad  " << matching << "\n";
    verifyClose("5 parts, 2 bad, inspect 2", hypergeometric(5, 2, 2, 1),
                static_cast<double>(matching) / totalSamples, 1e-12, "listed ");

    // ========================================================================
    part(3, "THE WHOLE PICTURE", "every k at once");
    // ========================================================================
    question("Do not ask about one k. Ask about all of them.");

    note("Sweep k from 0 to 5. These are disjoint events that cover");
    std::cout << "      everything that can happen, so by 1.3 they must total 1:\n";

    std::cout << "\n        bad found   probability\n"
              << "        " << repeat("-", 58) << "\n";
    double total = 0.0;
    for (int k = 0; k <= INSPECT; ++k) {
        double p = hypergeometric(BATCH, BAD, INSPECT, k);
        total += p;
        std::cout << "        " << std::setw(9) << k << std::setw(14) << p << bar(p, 40) << "\n";
    }
    verifyClose("the whole distribution", total, 1.0, 1e-12, "must be");

    note("Read the top row. Even though a tenth of the crate is");
    std::cout << "      junk, the single most likely outcome of a 5-part\n"
                 "      inspection is that you find NOTHING wrong.\n";

    // ========================================================================
    part(4, "WILL I EVEN CATCH IT?", "1.4's trick, on the job");
    // ========================================================================
    // This is the question a quality engineer actually asks, and it is
    // exactly the "at least one" shape from 1.4.
    question("What is the chance my inspection catches ANY defect?");

    note("'At least one bad part' - that is 1.4. Do not add up");
    std::cout << "      k = 1, 2, 3, 4, 5. Flip it:\n"
        "\n"
        "         P(catch something)  =  1 - P(find nothing)\n"
        "\n"
        "      and 'find nothing' is a single clean case - all 5 of your\n"
        "      picks came from the 90 good parts:\n"
        "\n"
        "                            C(90,5)      " << commas(nCr(90, 5)) << "\n"
        "         P(find nothing) = ---------  =  ----------  =  "
              << hypergeometric(BATCH, BAD, INSPECT, 0) << "\n"
        "                            C(100,5)     " << commas(nCr(100, 5)) << "\n";

    std::cout << "\n";
    showP("P(find nothing wrong)", hypergeometric(BATCH, BAD, INSPECT, 0));
    showP("P(catch at least one defect)",
          1.0 - hypergeometric(BATCH, BAD, INSPECT, 0));

    answer("A 5-part check misses a 10%-bad crate more often than not.");

    note("So how many must you open? This table is the actual");
    std::cout << "      deliverable - it is what you take to your manager:\n";
    std::cout << "\n        inspect   P(catch a defect)\n"
              << "        " << repeat("-", 58) << "\n";
    for (int n : {1, 5, 10, 15, 20, 25, 30, 40, 50}) {
        double miss = hypergeometric(BATCH, BAD, n, 0);
        std::cout << "        " << std::setw(7) << n << std::setw(14) << 1.0 - miss
                  << bar(1.0 - miss, 36) << "\n";
    }

    int needed = 1;
    while (1.0 - hypergeometric(BATCH, BAD, needed, 0) < 0.95) ++needed;
    note("To be 95% sure of catching a 10%-bad crate you must open");
    std::cout << "      " << needed << " of the 100. There is no clever way around\n"
                 "      that number - it is what the arithmetic says.\n";

    // ========================================================================
    part(5, "WITH OR WITHOUT REPLACEMENT?", "hypergeometric vs binomial");
    // ========================================================================
    // 1.2's central question shows up here as a genuine modelling decision,
    // not a textbook curiosity.
    question("Does it matter that the crate shrinks as I inspect?");

    note("Two models of the same inspection:");
    std::cout << "\n"
        "         HYPERGEOMETRIC   the truth. Parts leave the crate, so\n"
        "                          the mix changes with every draw.\n"
        "                          WITHOUT replacement.\n"
        "\n"
        "         BINOMIAL         the shortcut. Pretend each part is bad\n"
        "                          with a fixed probability p = D/N, every\n"
        "                          time. WITH replacement.\n";

    note("Side by side on our crate (100 parts, 10 bad, inspect 5):");
    std::cout << "\n        bad found   hypergeometric      binomial   difference\n"
              << "        " << repeat("-", 58) << "\n";
    const double rate = static_cast<double>(BAD) / BATCH;
    for (int k = 0; k <= INSPECT; ++k) {
        double h = hypergeometric(BATCH, BAD, INSPECT, k);
        double b = binomial(INSPECT, k, rate);
        std::cout << "        " << std::setw(9) << k << std::setw(16) << h
                  << std::setw(14) << b << std::setw(13) << std::fabs(h - b) << "\n";
    }

    note("Close, but not equal. Now watch the gap vanish as the");
    std::cout << "      crate grows, holding the defect rate at 10%:\n";
    std::cout << "\n        batch size   hypergeometric   binomial   gap\n"
              << "        " << repeat("-", 58) << "\n";
    for (int N : {20, 50, 100, 500, 2000, 10000}) {
        double h = hypergeometric(N, N / 10, INSPECT, 0);
        double b = binomial(INSPECT, 0, 0.10);
        std::cout << "        " << std::setw(10) << commas(static_cast<unsigned long long>(N))
                  << std::setw(17) << h << std::setw(11) << b
                  << std::setw(9) << std::fabs(h - b) << "\n";
    }

    answer("Small batch: use hypergeometric. Huge batch: binomial is fine.");
    note("Rule of thumb: if your sample is under about 10% of the");
    std::cout << "      batch, removing parts barely changes the mix and the\n"
                 "      easier formula is close enough to ship.\n";

    // ========================================================================
    part(6, "THE MULTINOMIAL COEFFICIENT", "n! / (n1! n2! ... nk!)");
    // ========================================================================
    // Real inspection is rarely pass/fail. Two categories was the easy case;
    // this is the general one.
    question("Parts are not just good or bad. What if there are three");
    std::cout << "      verdicts - PASS, REWORK and SCRAP?\n";

    note("First a pure counting question. In how many different");
    std::cout << "      ORDERS could an inspection of 10 parts produce 7 passes,\n"
                 "      2 reworks and 1 scrap?\n";

    std::cout << "\n"
        "      Pick which 7 of the 10 slots are passes, then which 2 of\n"
        "      the remaining 3 are reworks, then the last 1 is scrap:\n"
        "\n"
        "         C(10,7) x C(3,2) x C(1,1)  =  "
              << commas(nCr(10, 7)) << " x " << commas(nCr(3, 2)) << " x 1  =  "
              << commas(multinomial(10, {7, 2, 1})) << "\n"
        "\n"
        "      Written as factorials that is the MULTINOMIAL COEFFICIENT:\n"
        "\n"
        "                10!                 3,628,800\n"
        "         ---------------  =  ---------------------  =  "
              << commas(multinomial(10, {7, 2, 1})) << "\n"
        "          7!  x 2! x 1!         5,040 x 2 x 1\n";

    note("It is nCr's big brother. With only TWO groups it IS nCr:");
    std::cout << "\n        C(10,3)                 = " << commas(nCr(10, 3)) << "\n"
              << "        10! / (3! x 7!)         = " << commas(multinomial(10, {3, 7}))
              << "\n";
    verifyCount("2 groups is just nCr", nCr(10, 3), multinomial(10, {3, 7}));

    // Brute force on a word short enough to enumerate.
    note("Same idea, checked by listing. How many distinct ways can");
    std::cout << "      you arrange the letters of PASS? 4 letters, but the two\n"
                 "      S's are identical:  4!/(1! 1! 2!) = "
              << commas(multinomial(4, {1, 1, 2})) << "\n";

    std::vector<char> word = {'A', 'P', 'S', 'S'};
    std::sort(word.begin(), word.end());
    std::vector<std::string> seen;
    do {
        seen.push_back(std::string(word.begin(), word.end()));
    } while (std::next_permutation(word.begin(), word.end()));
    std::cout << "\n      ";
    for (size_t i = 0; i < seen.size(); ++i) {
        if (i && i % 6 == 0) std::cout << "\n      ";
        std::cout << " " << seen[i];
    }
    std::cout << "\n";
    verifyCount("spellings of PASS", multinomial(4, {1, 1, 2}),
                seen.size());

    note("The classic showpiece, same formula: MISSISSIPPI has 11");
    std::cout << "      letters - 1 M, 4 I, 4 S, 2 P:\n\n"
                 "         11! / (1! 4! 4! 2!)  =  "
              << commas(multinomial(11, {1, 4, 4, 2})) << " spellings\n";

    // ========================================================================
    part(7, "THE MULTINOMIAL DISTRIBUTION", "counts x probabilities");
    // ========================================================================
    question("Now the real question: how LIKELY is 7 pass, 2 rework,");
    std::cout << "      1 scrap - if the line runs at 80% pass, 15% rework,\n"
                 "      5% scrap?\n";

    const std::vector<double> lineRates = {0.80, 0.15, 0.05};
    const std::vector<std::string> verdicts = {"pass", "rework", "scrap"};

    note("Two pieces, and you already have both:");
    std::cout << "\n"
        "         HOW MANY ORDERS could it happen in?\n"
        "             the multinomial coefficient  =  "
              << commas(multinomial(10, {7, 2, 1})) << "\n"
        "\n"
        "         HOW LIKELY is any ONE such order?\n"
        "             0.80^7 x 0.15^2 x 0.05^1  =  "
              << std::setprecision(8)
              << std::pow(0.80, 7) * std::pow(0.15, 2) * 0.05 << "\n"
        << std::setprecision(4) <<
        "\n"
        "      Multiply them together and that is the MULTINOMIAL\n"
        "      DISTRIBUTION:\n"
        "\n"
        "             n!\n"
        "         ----------- x p1^n1 x p2^n2 x p3^n3\n"
        "          n1! n2! n3!\n";

    std::cout << "\n";
    showP("P(7 pass, 2 rework, 1 scrap)",
          multinomialPmf(10, {7, 2, 1}, lineRates));

    note("A few other outcomes from the same batch of 10:");
    std::cout << "\n        pass  rework  scrap   probability\n"
              << "        " << repeat("-", 58) << "\n";
    const std::vector<std::vector<int>> outcomes = {
        {10, 0, 0}, {8, 2, 0}, {8, 1, 1}, {7, 2, 1}, {6, 3, 1}, {5, 3, 2}};
    for (const auto& o : outcomes) {
        double p = multinomialPmf(10, o, lineRates);
        std::cout << "        " << std::setw(4) << o[0] << std::setw(8) << o[1]
                  << std::setw(7) << o[2] << std::setw(14) << p
                  << bar(p, 100) << "\n";
    }

    // Every possible split must add to 1 - the 1.3 partition idea again.
    double grand = 0.0;
    int splits = 0;
    for (int a = 0; a <= 10; ++a)
        for (int b = 0; a + b <= 10; ++b) {
            grand += multinomialPmf(10, {a, b, 10 - a - b}, lineRates);
            ++splits;
        }
    note("There are " + std::to_string(splits) + " possible splits of 10 parts into three");
    std::cout << "      buckets. They are disjoint and cover everything, so by\n"
                 "      1.3 their probabilities must total exactly 1:\n";
    verifyClose("all splits of 10 parts", grand, 1.0, 1e-12, "must be");

    // ========================================================================
    part(8, "THE SAME MATHS, A PORTFOLIO", "quality control for money");
    // ========================================================================
    // 1.1's fourth world. A portfolio is a batch and stocks are the parts.
    question("What has any of this got to do with markets?");

    note("A portfolio IS a batch, and stocks are the parts. Say 8");
    std::cout << "      of your 40 holdings will miss earnings this quarter, and\n"
                 "      you have time to research only 6 of them properly.\n";

    const int NAMES = 40, MISSES = 8, RESEARCH = 6;
    std::cout << "\n";
    showP("P(your 6 catch no problem at all)",
          hypergeometric(NAMES, MISSES, RESEARCH, 0));
    showP("P(you spot at least one)",
          1.0 - hypergeometric(NAMES, MISSES, RESEARCH, 0));
    showP("P(you spot exactly two)",
          hypergeometric(NAMES, MISSES, RESEARCH, 2));

    note("Exactly the crate on the dock, with tickers instead of");
    std::cout << "      parts. Sampling without replacement, same formula.\n";

    note("And the multinomial version: each holding ends the day");
    std::cout << "      UP (45%), FLAT (20%) or DOWN (35%). In a 10-stock book:\n";
    const std::vector<double> dayRates = {0.45, 0.20, 0.35};
    std::cout << "\n        up  flat  down   probability\n"
              << "        " << repeat("-", 58) << "\n";
    for (const auto& o : std::vector<std::vector<int>>{
             {10, 0, 0}, {7, 1, 2}, {5, 2, 3}, {4, 2, 4}, {0, 0, 10}}) {
        double p = multinomialPmf(10, o, dayRates);
        std::cout << "        " << std::setw(2) << o[0] << std::setw(6) << o[1]
                  << std::setw(6) << o[2] << std::setw(14) << p
                  << bar(p, 200) << "\n";
    }
    note("Note how tiny 'all ten up' is - and 'all ten down' too.");
    std::cout << "      Extreme uniform outcomes are rare precisely because\n"
                 "      there is only ONE arrangement that produces them, while\n"
                 "      the mixed ones have thousands.\n";

    // ========================================================================
    part(9, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Put your own crate in. Every answer shows its working.\n"
                 "\n"
                 "        1   inspection   - batch of N, D bad, inspect n\n"
                 "        2   how many     - how many to open to be X% sure\n"
                 "        3   three-way    - pass / rework / scrap counts\n"
                 "        4   quit\n";

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
                         "      1 inspection   2 how many   3 three-way   4 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 4, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            int N = 0, D = 0, n = 0;
            if (!askNumber("Parts in the batch", 2, 200, N)) { keepGoing = false; break; }
            if (!askNumber("How many are defective", 0, N, D)) { keepGoing = false; break; }
            if (!askNumber("How many will you inspect", 1, N, n)) { keepGoing = false; break; }

            std::cout << "\n      Batch of " << N << ", " << D << " defective, inspecting "
                      << n << ".\n";
            std::cout << "\n        STEP 1  every sample you could draw\n"
                         "                C(" << N << "," << n << ") = "
                      << bigNumber(logChoose(N, n), nCr(N, n)) << "\n";
            std::cout << "\n        STEP 2  the full distribution\n\n"
                         "                bad found   probability\n";
            double sum = 0.0;
            for (int k = 0; k <= n && k <= D; ++k) {
                double p = hypergeometric(N, D, n, k);
                sum += p;
                std::cout << "                " << std::setw(9) << k
                          << std::setw(14) << p << bar(p, 30) << "\n";
            }
            std::cout << "\n        STEP 3  flip it, the 1.4 move\n"
                         "                P(catch something) = 1 - P(find none)\n"
                         "                                   = 1 - "
                      << hypergeometric(N, D, n, 0) << "\n\n";
            showP("ANSWER  P(catch at least one)", 1.0 - hypergeometric(N, D, n, 0));
            std::cout << "\n      (the distribution above sums to " << sum << ")\n";
            break;
        }
        case 2: {
            int N = 0, D = 0, confidence = 0;
            if (!askNumber("Parts in the batch", 2, 200, N)) { keepGoing = false; break; }
            if (!askNumber("How many are defective", 1, N, D)) { keepGoing = false; break; }
            if (!askNumber("How sure do you want to be, in %", 1, 99, confidence)) {
                keepGoing = false;
                break;
            }
            const double target = confidence / 100.0;
            int open = 1;
            while (open < N && 1.0 - hypergeometric(N, D, open, 0) < target) ++open;

            std::cout << "\n      Batch of " << N << " with " << D
                      << " defective, want " << confidence << "% confidence.\n";
            std::cout << "\n        Opening more parts only ever helps. Walk up\n"
                         "        until P(catch one) clears the bar:\n\n"
                         "                inspect   P(catch a defect)\n";
            for (int n : {1, open / 2 > 0 ? open / 2 : 1, open - 1, open}) {
                if (n < 1 || n > N) continue;
                std::cout << "                " << std::setw(7) << n << std::setw(16)
                          << 1.0 - hypergeometric(N, D, n, 0)
                          << (n == open ? "   <- clears it" : "") << "\n";
            }
            std::cout << "\n";
            showP("ANSWER  parts you must inspect",
                  static_cast<double>(open) / N);
            std::cout << "        that is " << open << " of the " << N
                      << " parts.\n";
            break;
        }
        case 3: {
            int nPass = 0, nRework = 0, nScrap = 0;
            if (!askNumber("How many PASS", 0, 30, nPass)) { keepGoing = false; break; }
            if (!askNumber("How many REWORK", 0, 30, nRework)) { keepGoing = false; break; }
            if (!askNumber("How many SCRAP", 0, 30, nScrap)) { keepGoing = false; break; }
            const int n = nPass + nRework + nScrap;
            if (n == 0) { note("Nothing to inspect."); break; }

            std::cout << "\n      " << n << " parts: " << nPass << " pass, "
                      << nRework << " rework, " << nScrap << " scrap.\n";
            std::cout << "\n        STEP 1  how many ORDERS give that split\n"
                         "                " << n << "! / (" << nPass << "! "
                      << nRework << "! " << nScrap << "!)  =  "
                      << bigNumber(logMultinomial(n, {nPass, nRework, nScrap}),
                                   multinomial(n, {nPass, nRework, nScrap}))
                      << "\n";
            double one = std::pow(lineRates[0], nPass) *
                         std::pow(lineRates[1], nRework) *
                         std::pow(lineRates[2], nScrap);
            std::cout << "\n        STEP 2  how likely is ONE such order\n"
                         "                0.80^" << nPass << " x 0.15^" << nRework
                      << " x 0.05^" << nScrap << "  =  " << std::setprecision(10)
                      << one << "\n" << std::setprecision(4);
            std::cout << "\n        STEP 3  multiply\n\n";
            showP("ANSWER  P(exactly that split)",
                  multinomialPmf(n, {nPass, nRework, nScrap}, lineRates));
            std::cout << "\n      (line rates fixed at 80% pass, 15% rework, 5% scrap)\n";
            break;
        }
        case 4:
        default:
            note("Done. Run it again any time.");
            keepGoing = false;
            break;
        }
    }

    // ========================================================================
    part(10, "WHAT YOU LEARNED", "1.5 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      WHY YOU SAMPLE\n"
        "          non-destructive testing leaves the part usable, so you\n"
        "          COULD test everything - it just costs\n"
        "          destructive testing wrecks it, so you never can\n"
        "          either way you sample WITHOUT replacement\n"
        "\n"
        "      THE HYPERGEOMETRIC - k bad in a sample of n\n"
        "                   C(D,k) x C(N-D,n-k)\n"
        "          P(k)  =  -------------------\n"
        "                        C(N,n)\n"
        "\n"
        "      THE PRACTICAL QUESTION IS ALWAYS 1.4\n"
        "          P(catch a defect) = 1 - P(find nothing)\n"
        "          and 'find nothing' is one clean term, C(N-D,n)/C(N,n)\n"
        "\n"
        "      WHICH MODEL\n"
        "          small batch   hypergeometric, the crate really shrinks\n"
        "          huge batch    binomial is close enough\n"
        "          rule of thumb: sample under 10% of the batch -> binomial\n"
        "\n"
        "      THE MULTINOMIAL COEFFICIENT - more than two verdicts\n"
        "               n!\n"
        "          -----------   ways to split n items into those groups\n"
        "           n1! n2! ...\n"
        "          with two groups it is exactly nCr\n"
        "\n"
        "      THE MULTINOMIAL DISTRIBUTION\n"
        "          coefficient x p1^n1 x p2^n2 x ...\n"
        "          = how many orders, times how likely one order\n"
        "\n"
        "      COMING LATER\n"
        "          conditional probability P(A given B), independence,\n"
        "          and Bayes' theorem.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
