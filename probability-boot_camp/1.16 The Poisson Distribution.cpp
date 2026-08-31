// ============================================================================
//  1.16 - THE POISSON DISTRIBUTION
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.16 The Poisson Distribution.cpp" -o p116
//      ./p116
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.13  Bernoulli and binomial, mean np and variance np(1-p)
//  1.14  the normal - what the binomial becomes when n is large
//  1.15  the standard normal, one table for every normal
//  1.16  the OTHER limit of the binomial                 <- you are here
//
//  1.14 was careful to tell you when the normal approximation is allowed:
//  roughly np >= 10 and n(1-p) >= 10. It needs a fat middle to look like a
//  bell.
//
//  But some of the most important questions in engineering and finance sit
//  exactly where that rule fails - n enormous, p tiny, np small:
//
//       typos on a page          thousands of chances, each tiny
//       calls to a helpdesk      thousands of customers, most silent
//       flaws in a fibre         millions of microns, few defective
//       market crashes           252 trading days, few disasters
//
//  Push a binomial into that corner and it does NOT become a bell. It
//  becomes something else, with its own name.
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA
//  ---------------------------------------------------------------------------
//  Let n grow and p shrink together, holding the average np fixed at some
//  number lambda. Everything about n and p separately washes out, and only
//  lambda survives:
//
//                        lambda^k
//       P(X = k)  =  e^-lambda ---------
//                          k!
//
//  ONE parameter. Not n, not p - just lambda, the average count. You do not
//  even need to know how many chances there were, which is exactly the
//  situation you are usually in.
//
//  And its signature, which no other distribution has:
//
//       E[X] = lambda        Var(X) = lambda        mean EQUALS variance
// ============================================================================

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// ============================================================================
//  THE POISSON
// ============================================================================

// P(X = k) for a Poisson with mean lambda.
//
// Written in logs on purpose. The direct form e^-L * L^k / k! computes L^k
// and k! separately, and both explode long before the answer does - for
// lambda = 50 and k = 50, L^k is 8.9e84 and k! is 3.0e64, yet the answer is
// a perfectly ordinary 0.056. Same discipline as 1.5's lgamma fix.
double poissonPmf(int k, double lambda) {
    if (k < 0) return 0.0;
    if (lambda <= 0.0) return k == 0 ? 1.0 : 0.0;
    return std::exp(-lambda + k * std::log(lambda) - std::lgamma(k + 1.0));
}

// P(X <= k), summed from the bottom.
double poissonCdf(int k, double lambda) {
    double total = 0.0;
    for (int i = 0; i <= k; ++i) total += poissonPmf(i, lambda);
    return std::min(1.0, total);
}

// P(X >= k). Never sum an infinite tail - complement it (1.4).
double poissonAtLeast(int k, double lambda) {
    if (k <= 0) return 1.0;
    return 1.0 - poissonCdf(k - 1, lambda);
}

// The exact binomial, carried over from 1.13, so we can watch it converge.
double binomialPmf(int n, int k, double p) {
    if (k < 0 || k > n) return 0.0;
    return std::exp(std::lgamma(n + 1.0) - std::lgamma(k + 1.0)
                    - std::lgamma(n - k + 1.0)
                    + k * std::log(p) + (n - k) * std::log1p(-p));
}

// The normal CDF from 1.15, for the head-to-head in Part 9.
double Phi(double z) { return 0.5 * std::erfc(-z / std::sqrt(2.0)); }

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.15)
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

// The distribution as a picture. Poisson is skewed for small lambda and only
// straightens out into a bell when lambda gets large, so the shape is the
// lesson - print it rather than describe it.
void showPoisson(double lambda, int upTo) {
    std::cout << "\n          k     P(X = k)\n";
    for (int k = 0; k <= upTo; ++k) {
        const double p = poissonPmf(k, lambda);
        std::cout << "        " << std::setw(3) << k << std::setw(13)
                  << std::fixed << std::setprecision(4) << p
                  << bar(p, 50) << "\n";
    }
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ========================================================================
    part(1, "WHERE THE NORMAL GIVES UP", "1.14's fine print");
    // ========================================================================
    question("A book has 100,000 characters. Each one is mistyped with");
    std::cout << "      probability 0.00002. How many typos will the book have?\n";

    note("This is a binomial - 100,000 independent trials, each a");
    std::cout << "      Bernoulli with p = 0.00002. From 1.13:\n\n"
                 "         mean     np       = 100,000 x 0.00002  =  2\n"
                 "         variance np(1-p)  =  1.99996, near enough 2\n";

    note("So 1.14 says: use a normal with mu = 2, sigma = 1.414?");
    std::cout << "      Look at 1.14's own condition before you do:\n\n"
                 "         np >= 10 and n(1-p) >= 10\n"
                 "         np = 2.  It fails, and badly.\n";

    note("Watch it fail. The normal is a curve over the whole real");
    std::cout << "      line; the count of typos cannot be negative:\n\n";
    showP("normal says P(typos < 0)", Phi((0.0 - 2.0) / std::sqrt(2.0)));
    std::cout << "\n"
        "      Eight percent of the probability is sitting on impossible\n"
        "      outcomes. A bell needs room on both sides of its mean, and\n"
        "      when the mean is 2 and zero is a hard wall, there is none.\n";

    answer("Large n with tiny p is a different limit, with its own law.");

    // ========================================================================
    part(2, "THE LIMIT", "let n grow and p shrink together");
    // ========================================================================
    question("What does a binomial converge to if np stays fixed?");

    note("Hold the average at lambda = 2 and push n up. Every one of");
    std::cout << "      these has exactly the same mean:\n\n"
                 "         n =    10,  p = 0.2\n"
                 "         n =    50,  p = 0.04\n"
                 "         n = 1,000,  p = 0.002\n"
                 "         n = infinity, p -> 0     the limit\n";

    const double LAMBDA = 2.0;
    std::cout << "\n          k    n=10      n=50     n=1000     POISSON\n"
              << "        " << repeat("-", 56) << "\n";
    for (int k = 0; k <= 6; ++k) {
        std::cout << "        " << std::setw(3) << k
                  << std::setw(9) << binomialPmf(10, k, 0.2)
                  << std::setw(10) << binomialPmf(50, k, 0.04)
                  << std::setw(11) << binomialPmf(1000, k, 0.002)
                  << std::setw(12) << poissonPmf(k, LAMBDA) << "\n";
    }

    note("The columns march to the right and settle. n and p have");
    std::cout << "      vanished as separate quantities - only their product\n"
                 "      survives. That limit is the POISSON DISTRIBUTION:\n\n"
        "                              lambda^k\n"
        "         P(X = k)  =  e^-lambda ---------\n"
        "                                   k!\n";

    double worst = 0.0;
    for (int k = 0; k <= 12; ++k)
        worst = std::max(worst, std::fabs(binomialPmf(1000, k, 0.002)
                                          - poissonPmf(k, LAMBDA)));
    verifyClose("n=1000 vs the limit", worst, 0.0, 0.001, "gap    ");

    note("ONE parameter, not two. That matters more than it looks:");
    std::cout << "      to use a binomial you must know n AND p. To use a\n"
                 "      Poisson you only need the average - and the average is\n"
                 "      usually the one thing you can actually measure.\n"
                 "      Nobody knows how many chances there were for a typo.\n";

    // ========================================================================
    part(3, "IT IS A REAL DISTRIBUTION", "the k's go on forever");
    // ========================================================================
    question("k runs 0, 1, 2, ... with no upper limit. Does it still");
    std::cout << "      add up to 1?\n";

    note("A binomial stops at n - you cannot get 11 heads in 10");
    std::cout << "      flips. A Poisson has no n to stop at, so k is unbounded.\n"
                 "      The terms have to die fast enough on their own:\n";

    showPoisson(LAMBDA, 8);

    double total = 0.0;
    for (int k = 0; k <= 200; ++k) total += poissonPmf(k, LAMBDA);
    verifyClose("all k from 0 to 200", total, 1.0, 1e-12, "must be");

    note("They die because of the k! underneath. Each step up");
    std::cout << "      multiplies by lambda and divides by k, so once k passes\n"
                 "      lambda every term is smaller than the one before, and\n"
                 "      shrinking faster all the time:\n\n"
        "         P(k)     lambda\n"
        "        ------ = --------\n"
        "        P(k-1)       k\n";
    verifyClose("P(3)/P(2) = lambda/3", poissonPmf(3, LAMBDA) / poissonPmf(2, LAMBDA),
                LAMBDA / 3.0, 1e-12, "ratio  ");

    // ========================================================================
    part(4, "MEAN EQUALS VARIANCE", "the fingerprint");
    // ========================================================================
    question("What are E[X] and Var(X)?");

    note("Inherit them from 1.13 and take the limit. The binomial");
    std::cout << "      has mean np and variance np(1-p). Fix np = lambda and\n"
                 "      let p go to 0:\n\n"
                 "         mean      np        ->  lambda\n"
                 "         variance  np(1-p)   ->  lambda x 1  =  lambda\n"
                 "\n"
                 "      The (1-p) that made the binomial's variance smaller\n"
                 "      than its mean just goes to 1 and stops mattering.\n";

    // Computed from the definitions in 1.12, not quoted.
    double mean = 0.0, second = 0.0;
    for (int k = 0; k <= 200; ++k) {
        mean   += k * poissonPmf(k, LAMBDA);
        second += static_cast<double>(k) * k * poissonPmf(k, LAMBDA);
    }
    const double variance = second - mean * mean;

    std::cout << "\n      Computed straight from 1.12's definitions:\n\n";
    showV("E[X]   = sum k P(k)", mean);
    showV("Var(X) = E[X^2] - E[X]^2", variance);
    verifyClose("E[X] = lambda", mean, LAMBDA, 1e-10, "lambda ");
    verifyClose("Var(X) = lambda", variance, LAMBDA, 1e-10, "lambda ");

    answer("Mean and variance are the SAME number. That is diagnostic.");
    note("It gives you a free test. Measure a count, compute its");
    std::cout << "      mean and its variance, and compare:\n\n"
        "         variance ~ mean     Poisson is plausible\n"
        "         variance > mean     OVERDISPERSED - events are clumping,\n"
        "                             so they are not independent\n"
        "         variance < mean     UNDERDISPERSED - something is\n"
        "                             regulating them, too evenly spaced\n"
        "\n"
        "      Part 8 is a case where that test fails loudly, and the\n"
        "      failure is the interesting part.\n";

    // ========================================================================
    part(5, "WHEN IS SOMETHING POISSON?", "three conditions");
    // ========================================================================
    question("You have a count. When are you allowed to use this?");

    std::cout << "\n"
        "      1.  INDEPENDENCE (1.11)\n"
        "          One event happening does not make the next more or\n"
        "          less likely. Typos do not summon other typos.\n"
        "\n"
        "      2.  CONSTANT RATE\n"
        "          The average per unit of time, length or area does not\n"
        "          drift. Same lambda at the start as at the end.\n"
        "\n"
        "      3.  NO TWO AT ONCE\n"
        "          Chop the interval fine enough and each piece holds at\n"
        "          most one event. Events do not arrive in bundles.\n";

    note("Those three are exactly what 'binomial with n huge and p");
    std::cout << "      tiny' means, written in the language of the thing you\n"
                 "      are counting instead of the language of trials.\n";

    note("And each one has a classic way of breaking:");
    std::cout << "\n"
        "         goals in a match      a goal changes how both teams\n"
        "                               play - independence gone\n"
        "\n"
        "         calls to a helpdesk   9am is not 3am\n"
        "                               - constant rate gone\n"
        "\n"
        "         injuries in a crash   one accident hurts four people\n"
        "                               at once - no-two-at-once gone\n"
        "\n"
        "      Break a condition and you do not get a slightly wrong\n"
        "      answer. You get variance > mean, every time.\n";

    // ========================================================================
    part(6, "THE CLASSIC", "Prussian cavalry, 1875-1894");
    // ========================================================================
    // Bortkiewicz's data. It is the standard example for a reason: the fit
    // is extraordinary, and the underlying process is genuinely absurd.
    question("Does any of this survive contact with real data?");

    note("Bortkiewicz, 1898. He counted deaths from horse kicks in");
    std::cout << "      14 Prussian cavalry corps over 20 years. Ten of those\n"
                 "      corps are the standard data set: 200 corps-years, 122\n"
                 "      deaths in total.\n";

    const int cyears = 200;
    const int observed[] = {109, 65, 22, 3, 1};    // corps-years with k deaths
    int deaths = 0, checkYears = 0;
    for (int k = 0; k < 5; ++k) { deaths += k * observed[k]; checkYears += observed[k]; }
    const double horseLambda = static_cast<double>(deaths) / cyears;

    std::cout << "\n        corps-years        " << checkYears << "\n"
              << "        deaths in total    " << deaths << "\n"
              << "        lambda = 122/200   " << horseLambda << " deaths per corps-year\n";

    note("Nobody fitted a curve here. lambda is just the average.");
    std::cout << "      Everything below follows from that one number:\n";

    std::cout << "\n        deaths   observed   Poisson says\n"
              << "        " << repeat("-", 50) << "\n";
    for (int k = 0; k < 5; ++k) {
        const double expected = cyears * poissonPmf(k, horseLambda);
        std::cout << "        " << std::setw(6) << k << std::setw(11) << observed[k]
                  << std::setw(15) << std::setprecision(1) << expected
                  << std::setprecision(4) << "\n";
    }

    double chi = 0.0;
    for (int k = 0; k < 5; ++k) {
        const double e = cyears * poissonPmf(k, horseLambda);
        chi += (observed[k] - e) * (observed[k] - e) / e;
    }
    showV("total squared mismatch", chi);

    note("109 against 108.7. 65 against 66.3. A man being kicked to");
    std::cout << "      death by a horse is about as far from mathematics as an\n"
                 "      event can get - and yet the counts land on the curve,\n"
                 "      because the three conditions in Part 5 all happen to\n"
                 "      hold. Many soldiers, each almost never kicked.\n";

    // ========================================================================
    part(7, "AT LEAST ONE", "1.4, one more time");
    // ========================================================================
    question("A page averages 0.3 typos. What is the chance a page has");
    std::cout << "      at least one?\n";

    note("Every 'at least one' question in this course has had the");
    std::cout << "      same answer since 1.4 - do not sum the tail, flip it:\n\n"
        "         P(X >= 1)  =  1 - P(X = 0)  =  1 - e^-lambda\n"
        "\n"
        "      and P(X = 0) is the easiest Poisson value there is, because\n"
        "      lambda^0 and 0! are both 1. The whole term collapses to\n"
        "      e^-lambda.\n";

    const double pageLambda = 0.3;
    std::cout << "\n";
    showP("P(a clean page)", poissonPmf(0, pageLambda));
    showP("P(at least one typo)", 1.0 - poissonPmf(0, pageLambda));
    verifyClose("1 - P(0) vs the tail sum", 1.0 - poissonPmf(0, pageLambda),
                poissonAtLeast(1, pageLambda), 1e-12, "summed ");

    note("Now the useful direction. A 300-page book at 0.3 per page");
    std::cout << "      averages 90 typos - so what does one proofreading pass\n"
                 "      have to achieve to ship a clean book?\n";

    std::cout << "\n        typos left over   P(book is clean)\n"
              << "        " << repeat("-", 50) << "\n";
    for (double lam : {90.0, 20.0, 5.0, 3.0, 1.0, 0.3, 0.05}) {
        std::cout << "        " << std::setw(13) << std::setprecision(2) << lam
                  << std::setw(18) << std::setprecision(4) << poissonPmf(0, lam)
                  << "\n";
    }
    std::cout << std::setprecision(4);

    note("e^-lambda falls off a cliff. Getting from 3 expected typos");
    std::cout << "      to 1 buys you a lot; getting from 90 to 20 buys nothing\n"
                 "      at all. Both are certainly-not-clean.\n";

    // ========================================================================
    part(8, "MARKETS", "where the assumption breaks");
    // ========================================================================
    // The four-worlds thread, and the one place in this lesson where the
    // honest answer is "do not use this model".
    question("252 trading days a year. A crash - a one-day fall of 5%");
    std::cout << "      or more - happens about twice a year. Model it?\n";

    note("It looks perfect for Poisson. Many days, each almost never");
    std::cout << "      a crash, average lambda = 2 per year:\n";

    showPoisson(2.0, 6);

    std::cout << "\n";
    showP("P(a calm year, no crash)", poissonPmf(0, 2.0));
    showP("P(four or more crashes)", poissonAtLeast(4, 2.0));

    note("Now test it the way Part 4 said to. Real market crash");
    std::cout << "      counts per year do not have variance equal to their\n"
                 "      mean - the variance is far bigger. Crashes CLUSTER:\n"
                 "      1929, 1987, 2008, 2020 each delivered several in days.\n";

    std::cout << "\n"
        "         Poisson assumes    a crash today says nothing about\n"
        "                            a crash tomorrow\n"
        "         markets do         a crash today makes tomorrow far\n"
        "                            more likely - volatility clusters\n";

    note("So condition 1 fails, and it fails in the expensive");
    std::cout << "      direction. A Poisson fitted to the average will\n"
                 "      understate the chance of a terrible year, because it\n"
                 "      cannot produce clusters:\n";

    // A crude clustered alternative: same mean, but arrivals come in pairs.
    // Same lambda, visibly fatter tail - which is the entire point.
    std::cout << "\n        crashes    Poisson(2)   if they arrive in pairs\n"
              << "        " << repeat("-", 52) << "\n";
    for (int k = 0; k <= 6; ++k) {
        const double clustered = (k % 2 == 0) ? poissonPmf(k / 2, 1.0) : 0.0;
        std::cout << "        " << std::setw(6) << k
                  << std::setw(13) << poissonPmf(k, 2.0)
                  << std::setw(17) << clustered << "\n";
    }
    std::cout << "\n"
        "      Both average 2 crashes a year. The clustered one puts\n"
        "      eight times more weight on 'four crashes' - and four\n"
        "      crashes is the year that ends your fund.\n";

    answer("Poisson is the right tool for INDEPENDENT rare events.");
    note("Use it for machine failures, arrivals, defects, decay.");
    std::cout << "      Be very careful using it for anything where fear is\n"
                 "      one of the inputs. That is not a flaw in the maths -\n"
                 "      it is Part 5 telling you which model to reach for.\n";

    // ========================================================================
    part(9, "WHICH LIMIT DO I USE?", "normal or Poisson");
    // ========================================================================
    question("Both 1.14 and 1.16 approximate a binomial. Which one?");

    note("It depends entirely on np. Same n = 1,000 throughout,");
    std::cout << "      only p changes:\n";

    std::cout << "\n           p        np    exact      Poisson     normal\n"
              << "        " << repeat("-", 56) << "\n";
    struct Row { double p; int k; };
    for (const Row& r : std::vector<Row>{{0.001, 1}, {0.005, 5}, {0.02, 20},
                                         {0.10, 100}, {0.30, 300}}) {
        const int n = 1000;
        const double np = n * r.p;
        const double exact  = binomialPmf(n, r.k, r.p);
        const double pois   = poissonPmf(r.k, np);
        const double sd     = std::sqrt(np * (1.0 - r.p));
        const double norm   = Phi((r.k + 0.5 - np) / sd) - Phi((r.k - 0.5 - np) / sd);
        std::cout << "        " << std::setw(6) << std::setprecision(3) << r.p
                  << std::setw(10) << std::setprecision(0) << np
                  << std::setw(10) << std::setprecision(4) << exact
                  << std::setw(11) << pois << std::setw(11) << norm << "\n";
    }
    std::cout << std::setprecision(4);

    note("Read the two approximation columns against the exact one.");
    std::cout << "      Poisson is excellent on the top rows and drifts as np\n"
                 "      grows. The normal is hopeless at the top and excellent\n"
                 "      at the bottom. They hand over around np = 10 - which is\n"
                 "      exactly the threshold 1.14 gave you.\n";

    std::cout << "\n"
        "         np < 10 and p tiny     ->  POISSON, lambda = np\n"
        "         np >= 10 and n(1-p) >= 10  ->  NORMAL, mu = np,\n"
        "                                    sigma = sqrt(np(1-p))\n"
        "         small n                ->  just use the binomial\n";

    note("And when lambda itself gets large the Poisson turns into a");
    std::cout << "      bell too - it has to, since both are limits of the same\n"
                 "      binomial. The two approximations agree in the middle:\n";
    std::cout << "\n        lambda    P(X <= lambda)   normal says\n"
              << "        " << repeat("-", 50) << "\n";
    for (double lam : {1.0, 5.0, 20.0, 100.0}) {
        const int k = static_cast<int>(lam);
        std::cout << "        " << std::setw(6) << std::setprecision(0) << lam
                  << std::setw(16) << std::setprecision(4) << poissonCdf(k, lam)
                  << std::setw(14) << Phi((k + 0.5 - lam) / std::sqrt(lam)) << "\n";
    }
    std::cout << std::setprecision(4);

    // ========================================================================
    part(10, "DO NOT TRUST ME, SIMULATE IT", "200,000 books");
    // ========================================================================
    question("Is a binomial with huge n really this thing?");

    note("No formulas. Flip 100,000 characters with p = 0.00002,");
    std::cout << "      count the typos, and do it 200,000 times.\n";

    std::mt19937 rng(20260831);
    const int TRIALS = 200000;
    // Simulating 100,000 Bernoulli trials 200,000 times is 2e10 flips - far
    // too slow. Sampling the binomial directly is the same distribution and
    // finishes instantly, which is the honest way to run this check.
    std::binomial_distribution<int> book(100000, 0.00002);

    std::vector<int> tally(15, 0);
    double sum = 0.0, sumSq = 0.0;
    for (int t = 0; t < TRIALS; ++t) {
        const int typos = book(rng);
        sum += typos;
        sumSq += static_cast<double>(typos) * typos;
        if (typos < 15) ++tally[static_cast<size_t>(typos)];
    }
    const double simMean = sum / TRIALS;
    const double simVar  = sumSq / TRIALS - simMean * simMean;

    std::cout << "\n          k     simulated     Poisson(2)\n"
              << "        " << repeat("-", 48) << "\n";
    for (int k = 0; k <= 7; ++k) {
        std::cout << "        " << std::setw(3) << k
                  << std::setw(13) << static_cast<double>(tally[static_cast<size_t>(k)]) / TRIALS
                  << std::setw(15) << poissonPmf(k, 2.0) << "\n";
    }

    verifyClose("simulated mean", simMean, 2.0, 0.02, "lambda    ");
    verifyClose("simulated variance", simVar, 2.0, 0.05, "lambda    ");
    note("Mean and variance both land on 2, from data that never");
    std::cout << "      once consulted the formula.\n";

    // ========================================================================
    part(11, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Put your own rate in. Every answer shows its working.\n"
                 "\n"
                 "        1   distribution  - the shape for a given lambda\n"
                 "        2   at least one  - and 'at least k'\n"
                 "        3   which limit   - Poisson or normal for your n, p\n"
                 "        4   quit\n";

    auto askInt = [](const std::string& prompt, int low, int high, int& out) {
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

    // lambda and p are not integers, so they are read as "per hundred" and
    // "per ten thousand" - it keeps every prompt integer-only, which keeps
    // the input handling simple and impossible to confuse.
    bool keepGoing = true;
    bool firstRound = true;
    while (keepGoing) {
        if (!firstRound)
            std::cout << "\n      " << repeat("-", 60) << "\n"
                         "      1 distribution   2 at least one   3 which limit"
                         "   4 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askInt("Your choice", 1, 4, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            int hundredths = 0;
            if (!askInt("lambda x 100 (so 250 means 2.5)", 1, 5000, hundredths)) {
                keepGoing = false;
                break;
            }
            const double lam = hundredths / 100.0;
            std::cout << "\n      lambda = " << lam << " events on average.\n";

            const int upTo = std::min(20, static_cast<int>(lam + 4 * std::sqrt(lam)) + 2);
            showPoisson(lam, upTo);

            std::cout << "\n        STEP 1  the fingerprint\n";
            showV("E[X]   = lambda", lam);
            showV("Var(X) = lambda", lam);
            showV("SD     = sqrt(lambda)", std::sqrt(lam));

            std::cout << "\n        STEP 2  the most likely single count\n";
            int mode = 0;
            for (int k = 0; k <= upTo; ++k)
                if (poissonPmf(k, lam) > poissonPmf(mode, lam)) mode = k;
            std::cout << "                k = " << mode << ", with probability "
                      << poissonPmf(mode, lam) << "\n";
            if (lam < 1.0)
                note("With lambda below 1 the single likeliest count is ZERO.");
            break;
        }
        case 2: {
            int hundredths = 0, atLeast = 0;
            if (!askInt("lambda x 100 (so 250 means 2.5)", 1, 5000, hundredths)) {
                keepGoing = false;
                break;
            }
            if (!askInt("at least how many", 1, 50, atLeast)) {
                keepGoing = false;
                break;
            }
            const double lam = hundredths / 100.0;

            std::cout << "\n      lambda = " << lam << ", asking for at least "
                      << atLeast << ".\n";
            std::cout << "\n        STEP 1  never sum an infinite tail - flip it (1.4)\n"
                         "                P(X >= " << atLeast << ") = 1 - P(X <= "
                      << atLeast - 1 << ")\n";
            std::cout << "\n        STEP 2  the terms being subtracted\n";
            for (int k = 0; k < atLeast && k <= 8; ++k)
                std::cout << "                P(" << k << ") = "
                          << poissonPmf(k, lam) << "\n";
            if (atLeast > 9) std::cout << "                   ... and the rest\n";
            std::cout << "\n        STEP 3  subtract from 1\n\n";
            showP("ANSWER  P(at least that many)", poissonAtLeast(atLeast, lam));
            if (atLeast == 1)
                std::cout << "\n      (for 'at least one' this is just 1 - e^-lambda\n"
                             "      = 1 - " << poissonPmf(0, lam) << ")\n";
            break;
        }
        case 3: {
            int n = 0, pTenThousandths = 0;
            if (!askInt("n, the number of trials", 1, 1000000, n)) {
                keepGoing = false;
                break;
            }
            if (!askInt("p x 10000 (so 25 means 0.0025)", 1, 9999, pTenThousandths)) {
                keepGoing = false;
                break;
            }
            const double p = pTenThousandths / 10000.0;
            const double np = n * p;

            std::cout << "\n      n = " << commas(static_cast<unsigned long long>(n))
                      << ", p = " << std::setprecision(4) << p
                      << ", so np = " << np << "\n";

            std::cout << "\n        1.14's condition   np >= 10 and n(1-p) >= 10\n"
                         "                           np = " << np
                      << ",  n(1-p) = " << n * (1.0 - p) << "\n";

            const bool normalOk = (np >= 10.0) && (n * (1.0 - p) >= 10.0);
            const bool poissonOk = (p <= 0.05) && (n >= 20);

            std::cout << "\n        VERDICT\n";
            if (poissonOk && !normalOk)
                std::cout << "                POISSON, with lambda = " << np << "\n"
                             "                p is tiny and np is small - this is\n"
                             "                exactly 1.16's corner.\n";
            else if (normalOk && !poissonOk)
                std::cout << "                NORMAL, mu = " << np << ", sigma = "
                          << std::sqrt(np * (1.0 - p)) << "\n"
                             "                fat middle, plenty of room either side.\n";
            else if (normalOk && poissonOk)
                std::cout << "                EITHER works here - you are in the\n"
                             "                handover zone. Poisson is simpler.\n";
            else
                std::cout << "                NEITHER is safe. n is small enough\n"
                             "                that you should just use the exact\n"
                             "                binomial from 1.13.\n";

            const int k = static_cast<int>(np);
            std::cout << "\n        Checked at k = " << k << ":\n";
            showV("exact binomial", binomialPmf(n, k, p));
            showV("Poisson approximation", poissonPmf(k, np));
            if (np > 0.0)
                showV("normal approximation",
                      Phi((k + 0.5 - np) / std::sqrt(np * (1.0 - p)))
                      - Phi((k - 0.5 - np) / std::sqrt(np * (1.0 - p))));
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
    part(12, "WHAT YOU LEARNED", "1.16 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE OTHER LIMIT OF THE BINOMIAL\n"
        "          1.14  n large, p moderate, np >= 10   ->  NORMAL\n"
        "          1.16  n large, p tiny,  np modest     ->  POISSON\n"
        "          same binomial, two different corners of it\n"
        "\n"
        "      THE FORMULA\n"
        "                              lambda^k\n"
        "          P(X = k)  =  e^-lambda ---------\n"
        "                                    k!\n"
        "          ONE parameter. You never need n or p separately -\n"
        "          only the average, which is what you can measure.\n"
        "\n"
        "      THE FINGERPRINT\n"
        "          E[X] = lambda        Var(X) = lambda\n"
        "          mean EQUALS variance, and that is a free test:\n"
        "          variance > mean means the events are clumping and\n"
        "          the independence assumption has failed\n"
        "\n"
        "      THE THREE CONDITIONS\n"
        "          independent   one event does not summon another\n"
        "          steady rate   lambda does not drift\n"
        "          one at a time no simultaneous arrivals\n"
        "\n"
        "      AT LEAST ONE - still 1.4\n"
        "          P(X >= 1) = 1 - e^-lambda\n"
        "          P(X = 0) is the easiest term there is\n"
        "\n"
        "      WHERE IT WORKS AND WHERE IT LIES\n"
        "          horse kicks, typos, arrivals, decay, defects: superb\n"
        "          market crashes: fitted to the mean, understates the\n"
        "          bad years, because crashes cluster and Poisson\n"
        "          cannot cluster by construction\n"
        "\n"
        "      COMING NEXT\n"
        "          everything so far has been ONE random variable.\n"
        "          Two at a time: covariance, correlation, and what\n"
        "          1.11's independence looks like as a number.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
