// ============================================================================
//  1.13 - BERNOULLI AND BINOMIAL RANDOM VARIABLES
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.13 Bernoulli and Binomial Random Variables.cpp" -o p113
//      ./p113
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.6   the binomial distribution - which ASSERTED mean = np and
//        variance = np(1-p), checked them numerically, and moved on
//  1.11  independence, and the difference between pairwise and mutual
//  1.12  random variables, E[X], Var(X), and linearity of expectation
//  1.13  the debt from 1.6, paid                     <- you are here
//
//  This lesson has one job. In 1.6 we computed the binomial's mean and
//  variance by summing the whole distribution and noticing the answers
//  came out at np and np(1-p). That is evidence, not a proof, and it
//  explains nothing about WHY those are the answers.
//
//  With 1.12's linearity and 1.11's independence, both results fall out
//  in about four lines - and the derivation shows exactly which one needs
//  independence and which one does not.
//
//  ---------------------------------------------------------------------------
//  THE BUILDING BLOCK
//  ---------------------------------------------------------------------------
//  A BERNOULLI random variable is the simplest one that exists:
//
//      X = 1 with probability p        "it happened"
//      X = 0 with probability 1-p      "it did not"
//
//  That is the entire definition. One trial, one yes/no, encoded as a
//  number so it can be added up.
//
//  The whole point of the 0/1 encoding is that ADDING Bernoullis COUNTS
//  successes. If X1..Xn are the individual flips, then X1 + ... + Xn is
//  the number of heads - and that sum is a BINOMIAL. Everything in this
//  lesson comes from taking that sentence seriously.
// ============================================================================

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
//  DISTRIBUTIONS, as in 1.12
// ============================================================================
struct Distribution {
    std::string name;
    std::vector<double> values;
    std::vector<double> probs;
};

double probSum(const Distribution& d) {
    double s = 0.0;
    for (double p : d.probs) s += p;
    return s;
}

double expectation(const Distribution& d) {
    double e = 0.0;
    for (size_t i = 0; i < d.values.size(); ++i) e += d.values[i] * d.probs[i];
    return e;
}

double expectationOf(const Distribution& d, const std::function<double(double)>& g) {
    double e = 0.0;
    for (size_t i = 0; i < d.values.size(); ++i) e += g(d.values[i]) * d.probs[i];
    return e;
}

double variance(const Distribution& d) {
    const double mu = expectation(d);
    double v = 0.0;
    for (size_t i = 0; i < d.values.size(); ++i)
        v += (d.values[i] - mu) * (d.values[i] - mu) * d.probs[i];
    return v;
}

double stdDev(const Distribution& d) { return std::sqrt(variance(d)); }

// A BERNOULLI: takes 1 with probability p, 0 otherwise.
Distribution bernoulli(double p) {
    return Distribution{"Bernoulli(" + std::to_string(p) + ")", {0.0, 1.0}, {1.0 - p, p}};
}

// A BINOMIAL, built from the 1.6 formula so we can check the sum-of-
// Bernoullis story against the distribution we already trusted.
Distribution binomial(int n, double p) {
    Distribution d{"Binomial", {}, {}};
    for (int k = 0; k <= n; ++k) {
        d.values.push_back(k);
        double logp;
        if (p <= 0.0)      logp = (k == 0) ? 0.0 : -INFINITY;
        else if (p >= 1.0) logp = (k == n) ? 0.0 : -INFINITY;
        else logp = std::lgamma(n + 1.0) - std::lgamma(k + 1.0)
                  - std::lgamma(n - k + 1.0)
                  + k * std::log(p) + (n - k) * std::log1p(-p);
        d.probs.push_back(std::exp(logp));
    }
    return d;
}

// Add two INDEPENDENT distributions - the distribution of X + Y.
// Every pair of values (x,y) contributes p(x)q(y) to the total x+y.
// This is only correct BECAUSE of independence, which is Part 6's point.
Distribution convolve(const Distribution& a, const Distribution& b) {
    std::vector<double> vals, ps;
    for (size_t i = 0; i < a.values.size(); ++i)
        for (size_t j = 0; j < b.values.size(); ++j) {
            const double v = a.values[i] + b.values[j];
            const double p = a.probs[i] * b.probs[j];
            auto it = std::find_if(vals.begin(), vals.end(),
                                   [&](double u) { return std::fabs(u - v) < 1e-9; });
            if (it == vals.end()) { vals.push_back(v); ps.push_back(p); }
            else ps[static_cast<size_t>(it - vals.begin())] += p;
        }
    // Sort by value so the table prints in order.
    std::vector<size_t> order(vals.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(),
              [&](size_t x, size_t y) { return vals[x] < vals[y]; });
    Distribution out{"sum", {}, {}};
    for (size_t i : order) { out.values.push_back(vals[i]); out.probs.push_back(ps[i]); }
    return out;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.12)

// Distribution values are usually whole numbers stored as doubles. Printing
// them as "10.0000" overflows the column and shifts the table, so drop the
// decimals whenever there is nothing after the point.
std::string fmtValue(double v) {
    std::ostringstream ss;
    if (std::fabs(v - std::llround(v)) < 1e-9) ss << std::llround(v);
    else ss << std::fixed << std::setprecision(3) << v;
    return ss.str();
}

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

void showV(const std::string& label, double v) {
    std::cout << "        " << std::left << std::setw(38) << label << std::right
              << std::fixed << std::setprecision(4) << v << "\n";
}

void verifyClose(const std::string& claim, double formula, double other,
                 double tolerance, const std::string& otherLabel = "counted") {
    bool ok = std::fabs(formula - other) < tolerance;
    std::cout << "\n  " << (ok ? "ok " : "XX ") << std::left << std::setw(26)
              << claim << std::right << "formula " << std::fixed
              << std::setprecision(4) << formula
              << "   " << otherLabel << " " << other << "\n";
}

void showDist(const Distribution& d, int scale = 60) {
    std::cout << "\n        value    P(X = x)\n"
              << "        " << repeat("-", 58) << "\n";
    for (size_t i = 0; i < d.values.size(); ++i)
        std::cout << "        " << std::setw(6) << fmtValue(d.values[i])
                  << std::setw(12) << d.probs[i] << bar(d.probs[i], scale) << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    const double P = 0.6;      // a weighted coin, so the algebra shows
    const int    N = 10;

    // ========================================================================
    part(1, "THE SIMPLEST RANDOM VARIABLE", "one trial, 0 or 1");
    // ========================================================================
    question("What is the smallest interesting random variable?");

    note("One trial, two outcomes, written as numbers:");
    std::cout << "\n"
        "         X = 1   with probability p        it happened\n"
        "         X = 0   with probability 1-p      it did not\n"
        "\n"
        "      That is a BERNOULLI random variable. It is the whole\n"
        "      definition - there is nothing else to it.\n";

    const Distribution bern = bernoulli(P);
    showDist(bern);
    verifyClose("it is a distribution", probSum(bern), 1.0, 1e-12, "must be");

    note("The 0/1 encoding looks arbitrary and is not. It is");
    std::cout << "      chosen so that ADDING Bernoullis COUNTS successes:\n"
        "\n"
        "         1 + 0 + 1 + 1 + 0  =  3 successes out of 5\n"
        "\n"
        "      Any other pair of labels - heads/tails, true/false, -1/+1\n"
        "      - loses that property. Counting by addition is the entire\n"
        "      reason the convention exists, and Part 4 cashes it in.\n";

    note("Bernoulli is the atom. Almost every distribution in an");
    std::cout << "      introductory course is built out of these:\n"
        "\n"
        "         BINOMIAL     n of them, added up          this lesson\n"
        "         GEOMETRIC    how many until the first success\n"
        "         NEGATIVE     how many until the r-th success\n"
        "           BINOMIAL\n"
        "         POISSON      the limit of a binomial as n grows and\n"
        "                      p shrinks\n";

    // ========================================================================
    part(2, "ITS MEAN AND VARIANCE", "two lines each, and worth doing by hand");
    // ========================================================================
    question("What is E[X] for a Bernoulli?");

    note("Apply 1.12's definition. There are only two terms:");
    std::cout << "\n"
        "         E[X]  =  1 x p  +  0 x (1-p)  =  p\n"
        "\n"
        "      The mean of a Bernoulli IS its probability. That is not a\n"
        "      coincidence of notation - it is the bridge between\n"
        "      probability and averaging, and Part 3 builds on it.\n";
    verifyClose("E[Bernoulli] = p", expectation(bern), P, 1e-12, "p       ");

    question("And the variance?");

    note("Use the shortcut form, Var = E[X^2] - (E[X])^2, and note");
    std::cout << "      the trick that makes it easy: X only takes the values 0\n"
                 "      and 1, and for both of those, x^2 = x. So X^2 IS X:\n"
        "\n"
        "         E[X^2]  =  E[X]  =  p\n"
        "\n"
        "         Var(X)  =  E[X^2] - (E[X])^2\n"
        "                 =  p - p^2\n"
        "                 =  p(1 - p)\n";
    std::cout << "\n";
    showV("E[X]",            expectation(bern));
    showV("E[X^2]",          expectationOf(bern, [](double x) { return x * x; }));
    showV("Var(X) = p(1-p)", variance(bern));
    showV("sd(X)",           stdDev(bern));
    verifyClose("E[X^2] = E[X] for 0/1",
                expectationOf(bern, [](double x) { return x * x; }),
                expectation(bern), 1e-12, "E[X]    ");
    verifyClose("Var = p(1-p)", variance(bern), P * (1.0 - P), 1e-12, "p(1-p)  ");

    note("The shape of p(1-p) is worth knowing by sight. It is a");
    std::cout << "      downward parabola, zero at both ends and largest in the\n"
                 "      middle:\n";
    std::cout << "\n        p       Var = p(1-p)    sd\n"
              << "        " << repeat("-", 58) << "\n";
    for (double p : {0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 1.0}) {
        const double v = p * (1 - p);
        std::cout << "        " << std::setw(5) << p << std::setw(14) << v
                  << std::setw(10) << std::sqrt(v) << bar(v, 100) << "\n";
    }

    answer("Maximum uncertainty at p = 0.5, none at all at 0 or 1.");

    note("Which matches the meaning. A coin that always lands");
    std::cout << "      heads is not uncertain about anything, so its variance\n"
                 "      had better be zero - and p(1-p) delivers that without\n"
                 "      being told.\n";

    // ========================================================================
    part(3, "THE INDICATOR TRICK", "turning any event into a number");
    // ========================================================================
    // The idea that makes linearity applicable to problems that look
    // nothing like coin flips.
    question("Can I make a Bernoulli out of ANY event?");

    note("Yes, and this is one of the most useful moves in the");
    std::cout << "      subject. For any event A, define its INDICATOR:\n"
        "\n"
        "         1_A  =  1 if A happens,  0 if it does not\n"
        "\n"
        "      That is a Bernoulli with p = P(A). And so:\n"
        "\n"
        "         E[1_A]  =  P(A)\n"
        "\n"
        "      An expectation and a probability are the same object,\n"
        "      seen from two sides.\n";

    note("Why that matters: expectation is LINEAR (1.12 Part 7)");
    std::cout << "      and probability is not. Convert your events into\n"
                 "      indicators, add them up freely, and convert back at the\n"
                 "      end. Problems that are painful to count become easy.\n";

    // The classic demonstration - a problem that is genuinely hard by
    // counting and trivial by indicators.
    note("Here is the standard demonstration. n people check coats");
    std::cout << "      and the coats are returned COMPLETELY at random. How\n"
                 "      many people expect to get their own coat back?\n";

    std::cout << "\n"
        "      By counting: you would need the distribution of the number\n"
        "      of fixed points of a random permutation, which involves\n"
        "      derangements and inclusion-exclusion. It is a real fight.\n"
        "\n"
        "      By indicators: let Xi = 1 if person i gets their own coat.\n"
        "\n"
        "         E[Xi]  =  P(person i gets their coat)  =  1/n\n"
        "         E[X]   =  E[X1] + ... + E[Xn]  =  n x (1/n)  =  1\n"
        "\n"
        "      The answer is 1, for every n. Two lines.\n";

    // Check it by brute force on small n, where all permutations fit.
    std::cout << "\n        n      E[own coat back], by listing every shuffle\n"
              << "        " << repeat("-", 58) << "\n";
    for (int n = 1; n <= 7; ++n) {
        std::vector<int> perm(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i) perm[static_cast<size_t>(i)] = i;
        long long total = 0, count = 0;
        do {
            int fixed = 0;
            for (int i = 0; i < n; ++i)
                if (perm[static_cast<size_t>(i)] == i) ++fixed;
            total += fixed;
            ++count;
        } while (std::next_permutation(perm.begin(), perm.end()));
        std::cout << "        " << std::setw(3) << n << std::setw(20)
                  << static_cast<double>(total) / count
                  << "   over " << commas(static_cast<unsigned long long>(count))
                  << " shuffles\n";
    }
    answer("Exactly 1, every time - and the indicators saw it instantly.");

    note("Notice what we did NOT need. Those Xi are heavily");
    std::cout << "      DEPENDENT - if the first n-1 people get their own coats,\n"
                 "      the last one certainly does too. Linearity did not care,\n"
                 "      because linearity never cares. That is Part 5's theme.\n";

    // ========================================================================
    part(4, "BINOMIAL = A SUM OF BERNOULLIS", "the whole lesson in one line");
    // ========================================================================
    question("What IS a binomial random variable, structurally?");

    note("A sum of n independent Bernoullis, all with the same p:");
    std::cout << "\n"
        "         X  =  X1 + X2 + ... + Xn\n"
        "\n"
        "      Each Xi is 1 if trial i succeeded. Adding them counts the\n"
        "      successes - which is exactly what 'binomial' measures.\n"
        "\n"
        "      1.6 defined the binomial by its formula, C(n,k) p^k (1-p)^(n-k).\n"
        "      This is the same object described by its CONSTRUCTION, and\n"
        "      the construction is what makes it easy to reason about.\n";

    note("Check the claim rather than asserting it. Add up two");
    std::cout << "      Bernoulli(0.6) variables and compare against Binomial(2, 0.6):\n";
    {
        const Distribution twoWay = convolve(bernoulli(P), bernoulli(P));
        const Distribution binom2 = binomial(2, P);
        std::cout << "\n        value   Bern + Bern    Binomial(2, 0.6)\n"
                  << "        " << repeat("-", 58) << "\n";
        for (size_t i = 0; i < twoWay.values.size(); ++i)
            std::cout << "        " << std::setw(6) << fmtValue(twoWay.values[i])
                      << std::setw(14) << twoWay.probs[i]
                      << std::setw(18) << binom2.probs[i] << "\n";
        double worst = 0.0;
        for (size_t i = 0; i < twoWay.probs.size(); ++i)
            worst = std::max(worst, std::fabs(twoWay.probs[i] - binom2.probs[i]));
        verifyClose("adding 2 Bernoullis", worst, 0.0, 1e-12, "gap of  ");
    }

    note("Now do it n times. Start from a single Bernoulli and");
    std::cout << "      add one more at each step:\n";
    {
        Distribution built = bernoulli(P);
        for (int i = 1; i < N; ++i) built = convolve(built, bernoulli(P));
        const Distribution formula = binomial(N, P);
        double worst = 0.0;
        for (size_t i = 0; i < built.probs.size(); ++i)
            worst = std::max(worst, std::fabs(built.probs[i] - formula.probs[i]));
        std::cout << "\n        adding " << N << " Bernoulli(0.6) variables one at a time\n"
                     "        vs the C(n,k) p^k (1-p)^(n-k) formula from 1.6:\n";
        verifyClose("10 Bernoullis = Binomial", worst, 0.0, 1e-12, "gap of  ");
        showDist(formula, 90);
    }

    answer("Same distribution, two descriptions. The sum is the useful one.");

    // ========================================================================
    part(5, "THE MEAN, PROPERLY", "np in one line, no independence needed");
    // ========================================================================
    // The debt from 1.6, part one.
    question("1.6 said the binomial mean is np. Why?");

    note("Because expectation is linear, and that is the entire");
    std::cout << "      argument:\n"
        "\n"
        "         E[X]  =  E[X1 + X2 + ... + Xn]\n"
        "               =  E[X1] + E[X2] + ... + E[Xn]     linearity, 1.12\n"
        "               =  p + p + ... + p                 n copies\n"
        "               =  np\n";

    answer("Four lines, and no factorials appeared at any point.");

    note("Compare the alternative. Without linearity you would");
    std::cout << "      have to evaluate\n"
        "\n"
        "         sum over k of  k C(n,k) p^k (1-p)^(n-k)\n"
        "\n"
        "      which needs the identity k C(n,k) = n C(n-1,k-1), an index\n"
        "      shift, and a re-application of the binomial theorem. It\n"
        "      works, and it takes a page, and it tells you nothing.\n";

    {
        const Distribution b = binomial(N, P);
        std::cout << "\n";
        showV("E[X], summed over the distribution", expectation(b));
        showV("n x p", N * P);
        verifyClose("mean = np", expectation(b), N * P, 1e-9, "n x p   ");
    }

    note("And here is the crucial detail for Part 6: that argument");
    std::cout << "      NEVER USED INDEPENDENCE. Linearity holds for dependent\n"
                 "      variables too, so the mean would still be np even if the\n"
                 "      trials were correlated. As long as each trial has\n"
                 "      probability p, the expected count is np. Full stop.\n";

    // Demonstrate with a maximally dependent set of trials.
    note("Prove it. Take the most dependent trials imaginable -");
    std::cout << "      flip ONE coin and copy the result n times. Every trial\n"
                 "      still has probability p, but they are identical:\n";
    {
        // X is either 0 (prob 1-p) or n (prob p). Nothing in between.
        Distribution copies{"all-or-nothing", {0.0, static_cast<double>(N)},
                            {1.0 - P, P}};
        std::cout << "\n";
        showV("E[X] for n copies of one flip", expectation(copies));
        showV("n x p", N * P);
        verifyClose("mean is STILL np", expectation(copies), N * P, 1e-12, "n x p   ");
        std::cout << "\n";
        showV("Var(X), independent trials",  N * P * (1 - P));
        showV("Var(X), identical trials",    variance(copies));
        note("The mean survived. The variance did not - it is "
             + std::to_string(static_cast<int>(variance(copies)
                              / (N * P * (1 - P)) + 0.5)) + "x larger.");
        std::cout << "      That gap is the whole subject of the next part.\n";
    }

    // ========================================================================
    part(6, "THE VARIANCE", "np(1-p), and here independence earns its keep");
    // ========================================================================
    // The debt from 1.6, part two - and the place where 1.11 pays off.
    question("Does variance add the way expectation does?");

    note("NO - and this is the single most important asymmetry in");
    std::cout << "      elementary probability:\n"
        "\n"
        "         E[X + Y]   =  E[X] + E[Y]           ALWAYS\n"
        "\n"
        "         Var(X + Y) =  Var(X) + Var(Y)       ONLY IF INDEPENDENT\n"
        "\n"
        "      In general the true statement carries a third term:\n"
        "\n"
        "         Var(X + Y) = Var(X) + Var(Y) + 2 Cov(X, Y)\n"
        "\n"
        "      and independence is exactly what makes that covariance\n"
        "      vanish.\n";

    note("Given that, the binomial variance is as short as the");
    std::cout << "      mean was:\n"
        "\n"
        "         Var(X)  =  Var(X1 + ... + Xn)\n"
        "                 =  Var(X1) + ... + Var(Xn)      independence\n"
        "                 =  p(1-p) + ... + p(1-p)        n copies, Part 2\n"
        "                 =  np(1-p)\n";

    {
        const Distribution b = binomial(N, P);
        std::cout << "\n";
        showV("Var(X), summed over the distribution", variance(b));
        showV("n x p x (1-p)", N * P * (1 - P));
        verifyClose("variance = np(1-p)", variance(b), N * P * (1 - P), 1e-9,
                    "np(1-p) ");
        showV("sd(X) = sqrt(np(1-p))", stdDev(b));
    }

    answer("1.6's two assertions, both now proved, in eight lines total.");

    note("And notice the division of labour, which is the real");
    std::cout << "      lesson here:\n"
        "\n"
        "         the MEAN     needed only LINEARITY        (1.12)\n"
        "         the VARIANCE needed INDEPENDENCE too      (1.11)\n"
        "\n"
        "      Every one of 1.6's four boxes now has a job. Box 4 -\n"
        "      'the trials are independent' - is what buys the variance,\n"
        "      and Part 5 showed the mean does not need it.\n";

    // Show the covariance term explicitly on the identical-trials case.
    note("Put a number on what independence was worth. For n");
    std::cout << "      identical copies of one flip, every pair is perfectly\n"
                 "      correlated, so the covariance terms all survive:\n"
        "\n"
        "         Var  =  n Var(Xi)  +  2 x C(n,2) x Cov\n"
        "\n"
        "      with Cov = p(1-p) when the trials are copies, giving\n"
        "      n^2 p(1-p) instead of n p(1-p):\n";
    std::cout << "\n"
        "         independent   " << N << " x " << P * (1 - P) << "  =  "
              << N * P * (1 - P) << "\n"
        "         identical     " << N << "^2 x " << P * (1 - P) << "  =  "
              << N * N * P * (1 - P) << "\n"
        "         ratio                        " << N << "x\n";
    {
        Distribution copies{"all-or-nothing", {0.0, static_cast<double>(N)},
                            {1.0 - P, P}};
        verifyClose("identical trials variance", variance(copies),
                    static_cast<double>(N) * N * P * (1 - P), 1e-9, "n^2p(1-p)");
    }

    note("So the variance is n times too big if you assume");
    std::cout << "      independence and the trials are actually identical. That\n"
                 "      is 1.11's correlated-defaults result arriving from the\n"
                 "      algebraic side instead of the tail side.\n";

    // ========================================================================
    part(7, "PARTIAL CORRELATION", "between the two extremes");
    // ========================================================================
    question("Real trials are neither independent nor identical.");
    std::cout << "      What happens in between?\n";

    note("Model it the way 1.11 did - a shared shock. With");
    std::cout << "      probability s every trial succeeds together at rate pHi;\n"
                 "      otherwise they behave independently at rate pLo. Pick\n"
                 "      pLo so the overall success rate stays exactly p:\n";

    {
        const double shock = 0.20;
        const double pHi = 0.95;
        const double pLo = (P - shock * pHi) / (1.0 - shock);
        std::cout << "\n"
            "         P(shock)          " << shock << "\n"
            "         success in shock  " << pHi << "\n"
            "         success otherwise " << pLo << "\n"
            "         overall rate      " << shock * pHi + (1 - shock) * pLo
                  << "     <- still " << P << "\n";

        // Mixture of two binomials, same marginal success rate.
        Distribution mix{"mixture", {}, {}};
        const Distribution hi = binomial(N, pHi), lo = binomial(N, pLo);
        for (int k = 0; k <= N; ++k) {
            mix.values.push_back(k);
            mix.probs.push_back(shock * hi.probs[static_cast<size_t>(k)]
                                + (1 - shock) * lo.probs[static_cast<size_t>(k)]);
        }
        const Distribution indep = binomial(N, P);

        std::cout << "\n        successes   independent   with a shared shock\n"
                  << "        " << repeat("-", 58) << "\n";
        for (int k = 0; k <= N; ++k)
            std::cout << "        " << std::setw(7) << k << std::setw(15)
                      << indep.probs[static_cast<size_t>(k)] << std::setw(18)
                      << mix.probs[static_cast<size_t>(k)] << "\n";

        std::cout << "\n";
        showV("E[X], independent",   expectation(indep));
        showV("E[X], shared shock",  expectation(mix));
        showV("Var(X), independent", variance(indep));
        showV("Var(X), shared shock", variance(mix));
        verifyClose("the means still match", expectation(mix), expectation(indep),
                    1e-9, "indep   ");

        answer("Identical means. The variance is "
               + std::to_string(variance(mix) / variance(indep)).substr(0, 4)
               + "x larger.");

        note("This is the pattern you should now expect. Dependence");
        std::cout << "      leaves the average alone and inflates the spread -\n"
                     "      which means it is invisible in any summary that only\n"
                     "      reports a mean, and decisive in anything that cares\n"
                     "      about the worst case.\n";
    }

    // ========================================================================
    part(8, "ADDING BINOMIALS", "when it works and when it does not");
    // ========================================================================
    question("Is the sum of two binomials a binomial?");

    note("Sometimes. The rule follows straight from the sum-of-");
    std::cout << "      Bernoullis picture - a binomial is a bag of Bernoullis,\n"
                 "      so adding two bags gives a bigger bag ONLY IF all the\n"
                 "      Bernoullis inside have the same p:\n"
        "\n"
        "         Bin(n, p) + Bin(m, p)  =  Bin(n+m, p)     same p, YES\n"
        "         Bin(n, p) + Bin(m, q)  =  not a binomial  p != q, NO\n"
        "\n"
        "      and both need the two to be independent.\n";

    {
        const Distribution sum1 = convolve(binomial(4, P), binomial(6, P));
        const Distribution big  = binomial(10, P);
        double worst = 0.0;
        for (size_t i = 0; i < sum1.probs.size(); ++i)
            worst = std::max(worst, std::fabs(sum1.probs[i] - big.probs[i]));
        verifyClose("Bin(4,p) + Bin(6,p)", worst, 0.0, 1e-12, "= Bin(10,p)");

        // Different p - the sum has the right mean but the wrong shape.
        const Distribution mixed = convolve(binomial(5, 0.2), binomial(5, 0.8));
        const double mixedMean = expectation(mixed);
        const Distribution pretend = binomial(10, mixedMean / 10.0);
        double worst2 = 0.0;
        for (size_t i = 0; i < mixed.probs.size(); ++i)
            worst2 = std::max(worst2, std::fabs(mixed.probs[i] - pretend.probs[i]));
        std::cout << "\n      Now with different p. Bin(5, 0.2) + Bin(5, 0.8) has\n"
                     "      mean " << mixedMean << ", the same as Bin(10, 0.5) - but:\n";
        std::cout << "\n        value   the real sum   Bin(10, 0.5)\n"
                  << "        " << repeat("-", 58) << "\n";
        for (size_t i = 0; i < mixed.values.size(); ++i)
            std::cout << "        " << std::setw(6) << fmtValue(mixed.values[i])
                      << std::setw(14) << mixed.probs[i]
                      << std::setw(15) << pretend.probs[i] << "\n";
        std::cout << "\n      Largest disagreement: " << worst2 << "\n";
        showV("Var, the real sum",  variance(mixed));
        showV("Var, Bin(10, 0.5)",  variance(pretend));

        answer("Same mean, different distribution. Matching means proves nothing.");

        note("The real sum is TIGHTER, because 0.2 and 0.8 are both");
        std::cout << "      more predictable than 0.5 - p(1-p) is 0.16 for each of\n"
                     "      them against 0.25 in the middle. Averaging the\n"
                     "      probabilities did not average the variances.\n";
    }

    // ========================================================================
    part(9, "A REAL PROBLEM", "how many servers do you need?");
    // ========================================================================
    question("Each of 10,000 users logs in independently with");
    std::cout << "      probability 0.02 at peak. Each server handles 50 users.\n"
                 "      How many servers do you buy?\n";

    {
        const int USERS = 10000;
        const double LOGIN = 0.02;
        const double meanUsers = USERS * LOGIN;
        const double sdUsers = std::sqrt(USERS * LOGIN * (1 - LOGIN));

        std::cout << "\n";
        showV("E[simultaneous users] = np", meanUsers);
        showV("sd = sqrt(np(1-p))",         sdUsers);

        note("Buying for the mean means buying 4 servers - and being");
        std::cout << "      down roughly half the time, because demand is above\n"
                     "      its average about half the time. You size for a TAIL,\n"
                     "      not a mean:\n";

        const Distribution load = binomial(USERS, LOGIN);
        auto tailAbove = [&](double users) {
            double t = 0.0;
            for (size_t i = 0; i < load.values.size(); ++i)
                if (load.values[i] > users) t += load.probs[i];
            return t;
        };
        std::cout << "\n        servers   capacity   P(overloaded)\n"
                  << "        " << repeat("-", 58) << "\n";
        for (int s = 4; s <= 8; ++s) {
            const double cap = s * 50.0;
            std::cout << "        " << std::setw(7) << s << std::setw(11)
                      << static_cast<int>(cap) << std::setw(16)
                      << std::scientific << std::setprecision(2) << tailAbove(cap)
                      << std::fixed << std::setprecision(4) << "\n";
        }

        answer("Five servers already takes the risk to about 1 in 5,000.");

        note("Now the sentence that should worry you: every number");
        std::cout << "      in that table assumed users log in INDEPENDENTLY.\n"
                     "      They do not. They log in when the market opens, when\n"
                     "      the email goes out, when the show starts.\n";

        // The same shared-shock model, applied to load.
        const double eventProb = 0.05, eventRate = 0.15;
        const double calmRate = (LOGIN - eventProb * eventRate) / (1 - eventProb);
        const Distribution evt = binomial(USERS, eventRate);
        const Distribution calm = binomial(USERS, calmRate);
        auto tailMix = [&](double users) {
            double t = 0.0;
            for (size_t i = 0; i < evt.values.size(); ++i)
                if (evt.values[i] > users)
                    t += eventProb * evt.probs[i] + (1 - eventProb) * calm.probs[i];
            return t;
        };
        std::cout << "\n      Say 5% of the time there is an event and the login\n"
                     "      rate jumps to 15%. Average load is unchanged at "
                  << (eventProb * eventRate + (1 - eventProb) * calmRate) * USERS
                  << ":\n";
        std::cout << "\n        servers   independent   with events   ratio\n"
                  << "        " << repeat("-", 58) << "\n";
        for (int s : {6, 8, 12, 20, 30}) {
            const double cap = s * 50.0;
            const double a = tailAbove(cap), b = tailMix(cap);
            std::cout << "        " << std::setw(7) << s << std::setw(15)
                      << std::scientific << std::setprecision(2) << a
                      << std::setw(15) << b << std::fixed << std::setprecision(4);
            if (a > 0) std::cout << std::setw(11) << std::scientific
                                 << std::setprecision(1) << b / a
                                 << std::fixed << std::setprecision(4);
            else       std::cout << std::setw(11) << "beyond";
            std::cout << "\n";
        }
        note("Six servers went from effectively never to 1-in-20.");
        std::cout << "      Same expected load, same users, same everything except\n"
                     "      the independence assumption - which was never true.\n";
    }

    // ========================================================================
    part(10, "DO NOT TRUST ME, SIMULATE IT", "400,000 runs");
    // ========================================================================
    question("Flip 10 weighted coins, 400,000 times, and add the 1s.");
    std::cout << "      Does np and np(1-p) come out of a machine that only\n"
                 "      knows how to flip?\n";

    std::mt19937 rng(1729);
    std::bernoulli_distribution flip(P);
    const int RUNS = 400000;

    std::vector<int> tally(static_cast<size_t>(N) + 1, 0);
    double sum = 0.0, sumSq = 0.0;
    for (int t = 0; t < RUNS; ++t) {
        int heads = 0;
        for (int i = 0; i < N; ++i) heads += flip(rng) ? 1 : 0;   // adding Bernoullis
        ++tally[static_cast<size_t>(heads)];
        sum += heads;
        sumSq += static_cast<double>(heads) * heads;
    }
    const double simMean = sum / RUNS;
    const double simVar = sumSq / RUNS - simMean * simMean;

    {
        const Distribution b = binomial(N, P);
        std::cout << "\n        successes   formula      simulated    difference\n"
                  << "        " << repeat("-", 60) << "\n";
        for (int k = 0; k <= N; ++k) {
            const double sim = static_cast<double>(tally[static_cast<size_t>(k)]) / RUNS;
            std::cout << "        " << std::setw(7) << k << std::setw(14)
                      << b.probs[static_cast<size_t>(k)] << std::setw(14) << sim
                      << std::setw(14) << std::fabs(b.probs[static_cast<size_t>(k)] - sim)
                      << "\n";
        }
    }

    std::cout << "\n";
    verifyClose("mean = np", N * P, simMean, 1e-2, "simulated");
    verifyClose("variance = np(1-p)", N * P * (1 - P), simVar, 2e-2, "simulated");
    verifyClose("sd", std::sqrt(N * P * (1 - P)), std::sqrt(simVar), 1e-2, "simulated");

    answer("Both of 1.6's assertions, confirmed by something that cannot do algebra.");

    note("And look at the inner loop: it literally adds Bernoullis.");
    std::cout << "      The simulation is not modelling the binomial formula -\n"
                 "      it is enacting Part 4's definition, and the formula\n"
                 "      falls out because the definition was right.\n";

    // ========================================================================
    part(11, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Build your own Bernoulli and binomial.\n"
                 "\n"
                 "        1   the basics    - n and p, mean, variance, table\n"
                 "        2   build it up   - add Bernoullis one at a time\n"
                 "        3   dependence    - what a shared shock does\n"
                 "        4   capacity      - how much headroom do you need\n"
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
            if (std::cin.eof()) return false;
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
                         "      1 basics   2 build up   3 dependence   4 capacity   5 quit\n";
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
            if (!askNumber("Number of trials n", 1, 60, n)) { keepGoing = false; break; }
            if (!askNumber("Success chance p, in %", 0, 100, pct)) { keepGoing = false; break; }
            const double p = pct / 100.0;
            const Distribution b = binomial(n, p);
            const Distribution one = bernoulli(p);

            std::cout << "\n      ONE trial - the Bernoulli building block:\n\n";
            showV("E[Xi] = p",          expectation(one));
            showV("Var(Xi) = p(1-p)",   variance(one));

            std::cout << "\n      " << n << " of them, added up:\n\n";
            showV("E[X] = np",          expectation(b));
            showV("Var(X) = np(1-p)",   variance(b));
            showV("sd(X)",              stdDev(b));
            verifyClose("mean = n x E[Xi]", expectation(b), n * expectation(one),
                        1e-9, "n x p   ");
            verifyClose("var = n x Var(Xi)", variance(b), n * variance(one),
                        1e-9, "np(1-p) ");
            note("First check needs only linearity. Second needs independence.");
            showDist(b, n <= 20 ? 90 : 45);
            verifyClose("it is a distribution", probSum(b), 1.0, 1e-9, "must be");
            break;
        }
        case 2: {
            int n = 0, pct = 0;
            if (!askNumber("How many Bernoullis to add", 1, 12, n)) { keepGoing = false; break; }
            if (!askNumber("Each one's p, in %", 1, 99, pct)) { keepGoing = false; break; }
            const double p = pct / 100.0;

            std::cout << "\n      Adding one Bernoulli(" << p << ") at a time:\n";
            std::cout << "\n        after   E[X]      Var(X)     values it can take\n"
                      << "        " << repeat("-", 60) << "\n";
            Distribution built = bernoulli(p);
            for (int i = 1; i <= n; ++i) {
                if (i > 1) built = convolve(built, bernoulli(p));
                std::cout << "        " << std::setw(5) << i << std::setw(10)
                          << expectation(built) << std::setw(12) << variance(built)
                          << "     0 to " << built.values.back() << "\n";
            }
            std::cout << "\n      Each step adds exactly p to the mean and "
                      << p * (1 - p) << "\n      to the variance. That is the two theorems, running.\n";
            const Distribution formula = binomial(n, p);
            double worst = 0.0;
            for (size_t i = 0; i < built.probs.size(); ++i)
                worst = std::max(worst, std::fabs(built.probs[i] - formula.probs[i]));
            verifyClose("built vs the formula", worst, 0.0, 1e-9, "gap of  ");
            showDist(built, 90);
            break;
        }
        case 3: {
            int n = 0, pct = 0, shockPct = 0, shockRate = 0;
            if (!askNumber("Number of trials n", 2, 40, n)) { keepGoing = false; break; }
            if (!askNumber("Overall success rate p, in %", 2, 90, pct)) { keepGoing = false; break; }
            if (!askNumber("Chance of a shared shock, in %", 1, 50, shockPct)) {
                keepGoing = false; break;
            }
            if (!askNumber("Success rate during the shock, in %", pct, 99, shockRate)) {
                keepGoing = false; break;
            }
            const double p = pct / 100.0, s = shockPct / 100.0, pHi = shockRate / 100.0;
            const double pLo = (p - s * pHi) / (1.0 - s);
            if (pLo < 0.0) {
                note("Those cannot average to your overall rate. Lower the shock.");
                break;
            }
            const Distribution hi = binomial(n, pHi), lo = binomial(n, pLo);
            Distribution mix{"mixture", {}, {}};
            for (int k = 0; k <= n; ++k) {
                mix.values.push_back(k);
                mix.probs.push_back(s * hi.probs[static_cast<size_t>(k)]
                                    + (1 - s) * lo.probs[static_cast<size_t>(k)]);
            }
            const Distribution ind = binomial(n, p);
            std::cout << "\n      Calm-times rate works out at " << pLo
                      << ", so the overall\n      rate is still " << p << ".\n";
            std::cout << "\n";
            showV("E[X], independent",    expectation(ind));
            showV("E[X], shared shock",   expectation(mix));
            showV("Var(X), independent",  variance(ind));
            showV("Var(X), shared shock", variance(mix));
            verifyClose("means still agree", expectation(mix), expectation(ind),
                        1e-6, "indep   ");
            std::cout << "\n      Variance inflated by " << variance(mix) / variance(ind)
                      << "x, with the mean untouched.\n";
            std::cout << "\n        value   independent   shared shock\n"
                      << "        " << repeat("-", 58) << "\n";
            for (int k = 0; k <= n; ++k) {
                if (ind.probs[static_cast<size_t>(k)] < 5e-4
                    && mix.probs[static_cast<size_t>(k)] < 5e-4) continue;
                std::cout << "        " << std::setw(5) << k << std::setw(14)
                          << ind.probs[static_cast<size_t>(k)] << std::setw(15)
                          << mix.probs[static_cast<size_t>(k)] << "\n";
            }
            note("Dependence leaves the average alone and fattens the tails.");
            break;
        }
        case 4: {
            int users = 0, ratePerMille = 0, per = 0;
            if (!askNumber("How many users", 10, 100000, users)) { keepGoing = false; break; }
            if (!askNumber("Each active with probability, per 1,000", 1, 999, ratePerMille)) {
                keepGoing = false; break;
            }
            if (!askNumber("Capacity per server", 1, 10000, per)) { keepGoing = false; break; }
            const double p = ratePerMille / 1000.0;
            const double mean = users * p;
            const double sd = std::sqrt(users * p * (1 - p));
            const Distribution load = binomial(users, p);
            auto over = [&](double cap) {
                double t = 0.0;
                for (size_t i = 0; i < load.values.size(); ++i)
                    if (load.values[i] > cap) t += load.probs[i];
                return t;
            };
            std::cout << "\n";
            showV("E[active users] = np", mean);
            showV("sd = sqrt(np(1-p))",   sd);
            std::cout << "\n      Sizing for the mean alone needs "
                      << static_cast<int>(std::ceil(mean / per)) << " server(s) - and\n"
                         "      overloads about " << over(std::ceil(mean / per) * per)
                      << " of the time.\n";
            std::cout << "\n        servers   capacity   P(overloaded)\n"
                      << "        " << repeat("-", 58) << "\n";
            const int base = static_cast<int>(std::ceil(mean / per));
            for (int s2 = base; s2 <= base + 8; ++s2) {
                const double cap = static_cast<double>(s2) * per;
                std::cout << "        " << std::setw(7) << s2 << std::setw(11) << cap
                          << std::setw(16) << over(cap) << "\n";
                if (over(cap) < 1e-6) break;
            }
            note("And every row assumes users act INDEPENDENTLY.");
            std::cout << "      Try option 3 to see what one shared event does to\n"
                         "      numbers that look this comfortable.\n";
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
    part(12, "WHAT YOU LEARNED", "1.13 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE BERNOULLI - the atom\n"
        "          X = 1 with probability p, 0 otherwise\n"
        "          E[X]   = p\n"
        "          Var(X) = p(1-p)      because X^2 = X when X is 0 or 1\n"
        "          widest at p = 0.5, zero at both ends\n"
        "\n"
        "      WHY 0 AND 1\n"
        "          so that ADDING Bernoullis COUNTS successes\n"
        "          any other labels lose that, and it is the whole trick\n"
        "\n"
        "      THE INDICATOR TRICK\n"
        "          1_A = 1 if A happens, 0 if not - a Bernoulli with p = P(A)\n"
        "          E[1_A] = P(A):  expectation and probability, one object\n"
        "          convert events to indicators, add freely, convert back\n"
        "          n coats returned at random -> E[own coat] = 1, always\n"
        "\n"
        "      THE BINOMIAL IS A SUM\n"
        "          X = X1 + ... + Xn, independent Bernoullis, same p\n"
        "          same object as 1.6's C(n,k) p^k (1-p)^(n-k), described\n"
        "          by its construction instead of its formula\n"
        "\n"
        "      THE MEAN - needs only LINEARITY\n"
        "          E[X] = E[X1] + ... + E[Xn] = np\n"
        "          holds even for DEPENDENT trials - proved on n copies\n"
        "          of a single flip, where the mean is still np\n"
        "\n"
        "      THE VARIANCE - needs INDEPENDENCE\n"
        "          Var(X+Y) = Var(X) + Var(Y) + 2Cov(X,Y)\n"
        "          independence kills the covariance, leaving np(1-p)\n"
        "          n identical trials instead give n^2 p(1-p) - n times\n"
        "          bigger, with the same mean\n"
        "\n"
        "      SO 1.6's FOUR BOXES ALL HAVE JOBS NOW\n"
        "          fixed n, two outcomes  -> the object exists\n"
        "          constant p             -> every E[Xi] is the same p\n"
        "          independent            -> buys the variance, not the mean\n"
        "\n"
        "      ADDING BINOMIALS\n"
        "          Bin(n,p) + Bin(m,p) = Bin(n+m,p)   same p, independent\n"
        "          different p is NOT binomial, even with a matching mean\n"
        "\n"
        "      THE RECURRING LESSON\n"
        "          dependence leaves the MEAN alone and inflates the SPREAD\n"
        "          invisible in any summary that reports only an average,\n"
        "          decisive in anything that cares about the worst case\n"
        "\n"
        "      COMING NEXT\n"
        "          the normal distribution - what a binomial turns into\n"
        "          when n gets large, which 1.6 Part 7 already showed you\n"
        "          happening without naming it.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
