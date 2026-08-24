// ============================================================================
//  1.12 - RANDOM VARIABLES AND PROBABILITY DISTRIBUTIONS
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.12 Random Variables and Probability Distributions.cpp" -o p112
//      ./p112
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.1-1.5   counting, sets, complements, and the first distributions
//  1.6       the binomial - which asserted mean = np without proving it
//  1.7-1.10  conditioning, total probability, Bayes
//  1.11      independence, taken apart
//  1.12      events become NUMBERS                    <- you are here
//
//  Everything so far has been about EVENTS - things that either happen or
//  do not. "The sum is 8." "The part is defective." "Dana tested positive."
//  You could count them, condition on them, and combine them, but you could
//  never average them, because you cannot average a happening.
//
//  This lesson attaches a number to each outcome. Once outcomes are numbers
//  you can add them, average them, and measure how spread out they are -
//  and the entire apparatus of statistics becomes available.
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA, AND THE WORST NAME IN MATHEMATICS
//  ---------------------------------------------------------------------------
//  A RANDOM VARIABLE IS A FUNCTION.
//
//  It is not random. It is not a variable. It is a perfectly ordinary,
//  deterministic function whose input is an outcome and whose output is a
//  number:
//
//      X : sample space  ->  the real numbers
//
//  Roll two dice. The outcome (4,3) is not a number - it is a pair of dice
//  showing faces. Define X = "the sum" and now X((4,3)) = 7, every time you
//  evaluate it, with no randomness whatsoever. The randomness is in WHICH
//  OUTCOME occurs, not in what X does to it.
//
//  Getting this straight fixes most of the confusion that follows, because
//  it explains why P(X = 7) means something: it is shorthand for
//  P({ all outcomes that X sends to 7 }), which is an event, which is 1.1.
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
//  A DISCRETE DISTRIBUTION
//
//  The values X can take, and how likely each one is. This pair IS the
//  probability mass function, and it is everything there is to know about
//  a discrete random variable.
// ============================================================================
struct Distribution {
    std::string name;
    std::vector<double> values;
    std::vector<double> probs;
};

// The two rules a PMF must obey. Break either and it is not a distribution.
bool probsNonNegative(const Distribution& d) {
    for (double p : d.probs) if (p < 0.0) return false;
    return true;
}

double probSum(const Distribution& d) {
    double s = 0.0;
    for (double p : d.probs) s += p;
    return s;
}

// E[X] = sum of x P(X = x).  The centre of mass of the distribution:
// put weight P(X=x) at position x and this is where it balances.
double expectation(const Distribution& d) {
    double e = 0.0;
    for (size_t i = 0; i < d.values.size(); ++i) e += d.values[i] * d.probs[i];
    return e;
}

// E[g(X)] = sum of g(x) P(X = x).
//
// Note what this does NOT do: it never builds the distribution of g(X).
// You apply g to the values and keep the ORIGINAL probabilities. That
// shortcut has a grand name - the law of the unconscious statistician -
// and Part 8 is about why it is not the same as g(E[X]).
double expectationOf(const Distribution& d, const std::function<double(double)>& g) {
    double e = 0.0;
    for (size_t i = 0; i < d.values.size(); ++i) e += g(d.values[i]) * d.probs[i];
    return e;
}

// Var(X) = E[(X - mu)^2].  The definition, computed honestly.
double variance(const Distribution& d) {
    const double mu = expectation(d);
    double v = 0.0;
    for (size_t i = 0; i < d.values.size(); ++i)
        v += (d.values[i] - mu) * (d.values[i] - mu) * d.probs[i];
    return v;
}

// The computational form, Var(X) = E[X^2] - (E[X])^2. Part 6 proves these
// agree; this one is the one you actually use.
double varianceShortcut(const Distribution& d) {
    const double mu = expectation(d);
    return expectationOf(d, [](double x) { return x * x; }) - mu * mu;
}

double stdDev(const Distribution& d) { return std::sqrt(variance(d)); }

// F(x) = P(X <= x).  Note the <= : the CDF is a running total, and it
// jumps at each value X can take.
double cdfAt(const Distribution& d, double x) {
    double c = 0.0;
    for (size_t i = 0; i < d.values.size(); ++i)
        if (d.values[i] <= x + 1e-12) c += d.probs[i];
    return c;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.11)
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

// For quantities that are NOT probabilities - a mean, a variance, a payout.
// Printing "3.5 = 350%" would be nonsense, so this one omits the percent.
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

void verifyCount(const std::string& claim, unsigned long long formula,
                 unsigned long long listed) {
    std::cout << "\n  " << (formula == listed ? "ok " : "XX ") << std::left
              << std::setw(25) << claim << std::right
              << "formula " << std::setw(11) << commas(formula)
              << "  listed " << std::setw(11) << commas(listed) << "\n";
}

// The PMF as a picture, with the CDF running alongside it so the

// Distribution values are usually whole numbers stored as doubles. Printing
// them as "10.0000" overflows the column and shifts the table, so drop the
// decimals whenever there is nothing after the point.
std::string fmtValue(double v) {
    std::ostringstream ss;
    if (std::fabs(v - std::llround(v)) < 1e-9) ss << std::llround(v);
    else ss << std::fixed << std::setprecision(3) << v;
    return ss.str();
}

// relationship between the two is visible rather than described.
void showPmfAndCdf(const Distribution& d) {
    std::cout << "\n        value    P(X = x)    F(x) = P(X <= x)\n"
              << "        " << repeat("-", 60) << "\n";
    for (size_t i = 0; i < d.values.size(); ++i) {
        std::cout << "        " << std::setw(6) << fmtValue(d.values[i])
                  << std::setw(12) << d.probs[i]
                  << std::setw(16) << cdfAt(d, d.values[i])
                  << bar(d.probs[i], 60) << "\n";
    }
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // Build the sum-of-two-dice distribution by enumerating all 36 outcomes.
    // Building it rather than typing it in is the point of Part 1: the
    // distribution is DERIVED from the sample space by the function X.
    Distribution diceSum{"sum of two dice", {}, {}};
    {
        std::vector<int> tally(13, 0);
        for (int a = 1; a <= 6; ++a)
            for (int b = 1; b <= 6; ++b)
                ++tally[static_cast<size_t>(a + b)];
        for (int s = 2; s <= 12; ++s) {
            diceSum.values.push_back(s);
            diceSum.probs.push_back(tally[static_cast<size_t>(s)] / 36.0);
        }
    }

    // ========================================================================
    part(1, "IT IS A FUNCTION", "not random, and not a variable");
    // ========================================================================
    question("What is a random variable?");

    note("It is a FUNCTION. That is the whole answer, and the name");
    std::cout << "      is actively misleading on both counts - it is not random\n"
                 "      and it is not a variable:\n"
        "\n"
        "         X : sample space  ->  the real numbers\n"
        "\n"
        "      Roll two dice. The outcome (4,3) is not a number, it is a\n"
        "      pair of dice sitting on a table. Define X = 'the sum' and\n"
        "      X((4,3)) = 7 - deterministically, every time, forever.\n";

    note("The randomness lives in WHICH OUTCOME happens, never in");
    std::cout << "      what X does to it. X is a fixed rule, applied to a\n"
                 "      random input. Once you see that, the notation stops\n"
                 "      being mysterious.\n";

    note("Here is X, laid out as the function it is:");
    std::cout << "\n           second:   1    2    3    4    5    6\n"
              << "        " << repeat("-", 56) << "\n";
    for (int a = 1; a <= 6; ++a) {
        std::cout << "        first " << a << "  ";
        for (int b = 1; b <= 6; ++b) std::cout << std::setw(5) << a + b;
        std::cout << "\n";
    }

    note("Now the key move. 'X = 7' is not an equation - it is a");
    std::cout << "      NAME FOR AN EVENT:\n"
        "\n"
        "         X = 7   means   { (1,6),(2,5),(3,4),(4,3),(5,2),(6,1) }\n"
        "\n"
        "      and so P(X = 7) is the probability of that set, which is\n"
        "      1.1 and nothing else. Random variables do not add any new\n"
        "      probability theory. They add a LANGUAGE.\n";

    std::cout << "\n";
    showP("P(X = 7)",  cdfAt(diceSum, 7) - cdfAt(diceSum, 6));
    showP("P(X <= 4)", cdfAt(diceSum, 4));
    verifyCount("outcomes with X = 7", 6, 6);

    note("Different functions on the SAME sample space give");
    std::cout << "      different random variables. Y = 'the larger die', Z =\n"
                 "      'the difference', W = '1 if a double, else 0'. Same 36\n"
                 "      outcomes underneath; three completely different tables.\n";

    // ========================================================================
    part(2, "THE NOTATION", "capital X, small x, and why both");
    // ========================================================================
    question("Why do books write P(X = x) with two different X's?");

    note("Because they are two different things, and the case");
    std::cout << "      carries the distinction:\n"
        "\n"
        "         X   CAPITAL   the random variable - the function\n"
        "                       itself, the whole mechanism\n"
        "\n"
        "         x   lowercase a particular number it might equal -\n"
        "                       an ordinary value like 7\n"
        "\n"
        "      So P(X = x) reads 'the probability that the mechanism X\n"
        "      produces the specific number x'. Once you know the case\n"
        "      convention, the formulas read as sentences.\n";

    note("The related shorthands, all meaning events:");
    std::cout << "\n"
        "         P(X = 7)        exactly 7\n"
        "         P(X <= 4)       4 or less\n"
        "         P(X > 9)        more than 9\n"
        "         P(5 <= X <= 9)  somewhere in that range\n";
    std::cout << "\n";
    showP("P(X = 7)",       cdfAt(diceSum, 7) - cdfAt(diceSum, 6));
    showP("P(X <= 4)",      cdfAt(diceSum, 4));
    showP("P(X > 9)",       1.0 - cdfAt(diceSum, 9));
    showP("P(5 <= X <= 9)", cdfAt(diceSum, 9) - cdfAt(diceSum, 4));

    note("Note the last two. 'More than 9' used 1.4's complement,");
    std::cout << "      and the range used a subtraction of two running totals.\n"
                 "      Both are old tricks; only the notation is new.\n";

    // ========================================================================
    part(3, "THE PMF", "the distribution IS the answer");
    // ========================================================================
    question("What is a probability DISTRIBUTION?");

    note("The full list of what X can be, and how likely each one");
    std::cout << "      is. For a discrete X that list is the PROBABILITY MASS\n"
                 "      FUNCTION - the PMF - and it is written p(x) = P(X = x).\n";

    showPmfAndCdf(diceSum);

    note("Two rules, and they are the only two:");
    std::cout << "\n"
        "         1   p(x) >= 0 for every x       no negative probabilities\n"
        "         2   the p(x) add to exactly 1   something must happen\n";
    verifyCount("rule 1, all non-negative", 1, probsNonNegative(diceSum) ? 1 : 0);
    verifyClose("rule 2, they add to 1", probSum(diceSum), 1.0, 1e-12, "must be");

    note("Rule 2 is 1.3's partition, one more time. The events");
    std::cout << "      {X=2}, {X=3}, ... {X=12} are disjoint and they cover\n"
                 "      everything, so their probabilities total 1. It is the\n"
                 "      same check you have been running since 1.5.\n";

    note("And notice the PMF is DERIVED, not chosen. We built it");
    std::cout << "      by walking all 36 outcomes and tallying. The sample\n"
                 "      space plus the function determine the distribution\n"
                 "      completely - there is no freedom left.\n";

    // ========================================================================
    part(4, "THE CDF", "the running total, and the more fundamental object");
    // ========================================================================
    question("What is F(x) = P(X <= x) for, when we already have p(x)?");

    note("It is the running total, and three things make it more");
    std::cout << "      useful than it first looks:\n";

    std::cout << "\n        x     p(x)      F(x)       the staircase\n"
              << "        " << repeat("-", 60) << "\n";
    for (size_t i = 0; i < diceSum.values.size(); ++i) {
        const double f = cdfAt(diceSum, diceSum.values[i]);
        std::cout << "        " << std::setw(4) << fmtValue(diceSum.values[i])
                  << std::setw(10) << diceSum.probs[i] << std::setw(10) << f
                  << bar(f, 40) << "\n";
    }

    note("ONE - every range question is a subtraction:");
    std::cout << "\n"
        "         P(a < X <= b)  =  F(b) - F(a)\n"
        "\n"
        "      P(5 <= X <= 9) = F(9) - F(4) = " << cdfAt(diceSum, 9) << " - "
              << cdfAt(diceSum, 4) << " = " << cdfAt(diceSum, 9) - cdfAt(diceSum, 4)
              << "\n";
    {
        double direct = 0.0;
        for (size_t i = 0; i < diceSum.values.size(); ++i)
            if (diceSum.values[i] >= 5 && diceSum.values[i] <= 9)
                direct += diceSum.probs[i];
        verifyClose("F(9)-F(4) vs adding up", cdfAt(diceSum, 9) - cdfAt(diceSum, 4),
                    direct, 1e-12, "summed  ");
    }

    note("TWO - the CDF's three properties hold for EVERY random");
    std::cout << "      variable, discrete or continuous, without exception:\n"
        "\n"
        "         it never decreases      you are adding non-negatives\n"
        "         F(-infinity) = 0        X is below nothing\n"
        "         F(+infinity) = 1        X is below everything\n";
    {
        bool monotone = true;
        double last = -1.0;
        for (int x = 0; x <= 14; ++x) {
            const double f = cdfAt(diceSum, x);
            if (f < last - 1e-12) monotone = false;
            last = f;
        }
        verifyCount("F never decreases", 1, monotone ? 1 : 0);
        verifyClose("F(1) = 0", cdfAt(diceSum, 1), 0.0, 1e-12, "must be");
        verifyClose("F(12) = 1", cdfAt(diceSum, 12), 1.0, 1e-12, "must be");
    }

    note("THREE - and this is why it is the more fundamental");
    std::cout << "      object: the PMF only exists for discrete X. A continuous\n"
                 "      random variable has P(X = x) = 0 for every single x, so\n"
                 "      its PMF is uselessly all zeros - but its CDF is perfectly\n"
                 "      well behaved. Part 9 comes back to this.\n";

    // ========================================================================
    part(5, "EXPECTED VALUE", "the balance point, not the expectation");
    // ========================================================================
    question("What is the 'average' of a random variable?");

    note("Weight each value by how likely it is, and add:");
    std::cout << "\n"
        "         E[X]  =  sum of  x  x  P(X = x)\n"
        "\n"
        "      For our dice sum:\n";
    std::cout << "\n        x     p(x)      x * p(x)\n"
              << "        " << repeat("-", 58) << "\n";
    for (size_t i = 0; i < diceSum.values.size(); ++i)
        std::cout << "        " << std::setw(4) << fmtValue(diceSum.values[i])
                  << std::setw(10) << diceSum.probs[i] << std::setw(14)
                  << diceSum.values[i] * diceSum.probs[i] << "\n";
    std::cout << "        " << repeat("-", 58) << "\n"
              << "        " << std::left << std::setw(13) << "E[X]" << std::right
              << std::setw(12) << expectation(diceSum) << "\n";

    answer("E[X] = 7, which is the balance point of that staircase.");

    note("Two warnings about the name, both of which cause real");
    std::cout << "      confusion:\n"
        "\n"
        "         IT IS NOT WHAT YOU EXPECT. Roll one die: E[X] = 3.5,\n"
        "         a value the die cannot possibly show. The expected\n"
        "         value need not be a possible value at all.\n"
        "\n"
        "         IT IS A CENTRE OF MASS. Put weight p(x) at position\n"
        "         x on a ruler; E[X] is where the ruler balances. That\n"
        "         is a literal physical description, not an analogy.\n";

    {
        Distribution oneDie{"one die", {1,2,3,4,5,6},
                            {1/6.0,1/6.0,1/6.0,1/6.0,1/6.0,1/6.0}};
        std::cout << "\n";
        showV("E[one die]", expectation(oneDie));
        showV("E[sum of two dice]", expectation(diceSum));
        verifyClose("E[sum] = 2 x E[one die]", expectation(diceSum),
                    2.0 * expectation(oneDie), 1e-12, "doubled ");
        note("That check is Part 7's linearity, arriving early.");
    }

    // ========================================================================
    part(6, "VARIANCE", "how far from the middle, on average");
    // ========================================================================
    question("E[X] says where. What says how spread out?");

    note("Measure the distance from the mean, square it so that");
    std::cout << "      overshoots and undershoots cannot cancel, and average\n"
                 "      that:\n"
        "\n"
        "         Var(X)  =  E[ (X - mu)^2 ]        mu = E[X]\n"
        "\n"
        "      The squaring is not decoration. E[X - mu] is ALWAYS zero -\n"
        "      the deviations cancel exactly, by the definition of the\n"
        "      mean - so you would learn nothing without it.\n";
    verifyClose("E[X - mu] is always 0",
                expectationOf(diceSum, [&](double x) { return x - expectation(diceSum); }),
                0.0, 1e-12, "must be");

    std::cout << "\n        x     p(x)     (x - 7)^2   times p(x)\n"
              << "        " << repeat("-", 58) << "\n";
    for (size_t i = 0; i < diceSum.values.size(); ++i) {
        const double dev = diceSum.values[i] - 7.0;
        std::cout << "        " << std::setw(4) << fmtValue(diceSum.values[i])
                  << std::setw(10) << diceSum.probs[i] << std::setw(12) << dev * dev
                  << std::setw(14) << dev * dev * diceSum.probs[i] << "\n";
    }
    std::cout << "        " << repeat("-", 58) << "\n"
              << "        " << std::left << std::setw(13) << "Var(X)" << std::right
              << std::setw(24) << variance(diceSum) << "\n";

    note("There is a second formula, and it is the one you will");
    std::cout << "      actually compute with:\n"
        "\n"
        "         Var(X)  =  E[X^2]  -  (E[X])^2\n"
        "\n"
        "      It falls out of expanding the square:\n"
        "         E[(X-mu)^2] = E[X^2 - 2mu X + mu^2]\n"
        "                     = E[X^2] - 2mu E[X] + mu^2\n"
        "                     = E[X^2] - 2mu^2 + mu^2\n"
        "                     = E[X^2] - mu^2\n";
    std::cout << "\n";
    showV("E[X^2]",           expectationOf(diceSum, [](double x) { return x * x; }));
    showV("(E[X])^2",         expectation(diceSum) * expectation(diceSum));
    showV("Var(X), definition", variance(diceSum));
    showV("Var(X), shortcut",   varianceShortcut(diceSum));
    verifyClose("both variance formulas", variance(diceSum),
                varianceShortcut(diceSum), 1e-9, "shortcut");

    note("One problem with variance: the UNITS are squared. If X");
    std::cout << "      is in dollars, Var(X) is in dollars-squared, which is\n"
                 "      not a thing. Take the square root and you get the\n"
                 "      STANDARD DEVIATION, back in the original units:\n";
    std::cout << "\n";
    showV("Var(X)      (squared units)", variance(diceSum));
    showV("sd(X)       (same units as X)", stdDev(diceSum));

    note("Which is why every result you will read quotes a");
    std::cout << "      standard deviation and every derivation uses a variance.\n"
                 "      Variance is the one that does algebra cleanly; standard\n"
                 "      deviation is the one that means something.\n";

    // ========================================================================
    part(7, "LINEARITY OF EXPECTATION", "the best theorem in the subject");
    // ========================================================================
    // The single most useful property in elementary probability, and the one
    // that does not need independence.
    question("If I add two random variables, what happens to the mean?");

    note("It adds. Always. With no conditions attached:");
    std::cout << "\n"
        "         E[X + Y]  =  E[X] + E[Y]\n"
        "         E[aX + b] =  a E[X] + b\n"
        "\n"
        "      And here is the part that should surprise you: X and Y do\n"
        "      NOT have to be independent. They can be perfectly\n"
        "      correlated, or one can be a function of the other, and\n"
        "      expectation still adds.\n";

    // Demonstrate on a deliberately dependent pair.
    note("Prove it on a pair that could not be more dependent -");
    std::cout << "      one die, X = the face, Y = 7 minus the face:\n";
    {
        Distribution die{"one die", {1,2,3,4,5,6},
                         {1/6.0,1/6.0,1/6.0,1/6.0,1/6.0,1/6.0}};
        Distribution comp{"7 - face", {6,5,4,3,2,1},
                          {1/6.0,1/6.0,1/6.0,1/6.0,1/6.0,1/6.0}};
        std::cout << "\n";
        showV("E[X]",     expectation(die));
        showV("E[Y]",     expectation(comp));
        showV("E[X]+E[Y]", expectation(die) + expectation(comp));
        note("X + Y is the constant 7, so E[X+Y] is exactly 7:");
        verifyClose("linearity, fully dependent",
                    expectation(die) + expectation(comp), 7.0, 1e-12, "E[X+Y]  ");
        note("Y is completely determined by X, and it made no");
        std::cout << "      difference whatsoever.\n";
    }

    note("Now use it for something. 1.6 asserted that a binomial");
    std::cout << "      has mean np and never proved it. Watch how little work\n"
                 "      that takes with linearity:\n"
        "\n"
        "         X = number of heads in n flips\n"
        "           = X1 + X2 + ... + Xn,  where Xi is 1 if flip i is\n"
        "             a head and 0 if it is not\n"
        "\n"
        "         E[Xi]  =  1(p) + 0(1-p)  =  p\n"
        "         E[X]   =  E[X1] + ... + E[Xn]  =  np\n"
        "\n"
        "      One line. The alternative is to sum k C(n,k) p^k (1-p)^(n-k)\n"
        "      over all k and fight with factorials for a page.\n";
    {
        const int n = 10;
        const double p = 0.6;
        Distribution binom{"binomial", {}, {}};
        for (int k = 0; k <= n; ++k) {
            binom.values.push_back(k);
            binom.probs.push_back(std::exp(std::lgamma(n + 1.0) - std::lgamma(k + 1.0)
                                  - std::lgamma(n - k + 1.0) + k * std::log(p)
                                  + (n - k) * std::log1p(-p)));
        }
        verifyClose("binomial mean = np", expectation(binom), n * p, 1e-9, "n x p   ");
        verifyClose("binomial var = np(1-p)", variance(binom), n * p * (1 - p),
                    1e-9, "np(1-p) ");
        note("1.13 does this properly and explains why the VARIANCE");
        std::cout << "      result needs independence when the mean did not.\n";
    }

    answer("Expectation adds unconditionally. Nothing else in probability does.");

    // ========================================================================
    part(8, "E[g(X)] IS NOT g(E[X])", "the mistake that costs money");
    // ========================================================================
    question("If E[X] = 7, is E[X^2] = 49?");

    note("No, and assuming so is one of the most expensive errors");
    std::cout << "      in applied work. The average of a function is not the\n"
                 "      function of the average:\n";
    std::cout << "\n";
    showV("E[X]",       expectation(diceSum));
    showV("(E[X])^2",   expectation(diceSum) * expectation(diceSum));
    showV("E[X^2]",     expectationOf(diceSum, [](double x) { return x * x; }));
    std::cout << "\n      The gap between those last two is exactly "
              << variance(diceSum) << " -\n      which is the variance, by Part 6's identity.\n";

    note("The direction of the error is not random either. For");
    std::cout << "      any convex function - x^2, e^x, 1/x on positives -\n"
                 "      you always get E[g(X)] >= g(E[X]). That is JENSEN'S\n"
                 "      INEQUALITY, and the gap grows with the variance.\n";

    std::cout << "\n        g(x)          g(E[X])      E[g(X)]      gap\n"
              << "        " << repeat("-", 58) << "\n";
    const std::vector<std::pair<std::string, std::function<double(double)>>> gs = {
        {"x^2",     [](double x) { return x * x; }},
        {"sqrt(x)", [](double x) { return std::sqrt(x); }},
        {"1/x",     [](double x) { return 1.0 / x; }},
        {"log(x)",  [](double x) { return std::log(x); }},
    };
    for (const auto& g : gs) {
        const double ge = g.second(expectation(diceSum));
        const double eg = expectationOf(diceSum, g.second);
        std::cout << "        " << std::left << std::setw(12) << g.first << std::right
                  << std::setw(11) << ge << std::setw(13) << eg
                  << std::setw(12) << eg - ge << "\n";
    }

    answer("Convex g bends the average UP; concave g bends it DOWN.");

    note("Read the signs in that table. x^2 and 1/x are convex, so");
    std::cout << "      the gap is positive. sqrt and log are concave, so it is\n"
                 "      negative. Either way the gap is real and grows with the\n"
                 "      variance - only a STRAIGHT g gets you g(E[X]) exactly,\n"
                 "      which is why Part 7's linearity applied to aX + b and\n"
                 "      nothing else.\n";

    note("This is not a curiosity. A few places it bites:");
    std::cout << "\n"
        "         AVERAGE RETURNS. Compounding is multiplicative, so\n"
        "         the average of a growth path is not the growth of\n"
        "         the average path. +50% then -50% averages to 0% and\n"
        "         leaves you down 25%.\n"
        "\n"
        "         PROJECT PLANNING. The expected time to finish is not\n"
        "         the time computed from expected sub-task times, once\n"
        "         tasks run in parallel and you wait for the slowest.\n"
        "\n"
        "         COST CURVES. If cost grows faster than linearly with\n"
        "         demand, budgeting for average demand underestimates\n"
        "         average cost, every time.\n";
    {
        // The +50%/-50% claim, checked rather than asserted.
        const double up = 1.5, down = 0.5;
        std::cout << "\n        starting with 100, one up year and one down year:\n"
                  << "          arithmetic mean return   "
                  << ((up - 1) + (down - 1)) / 2 * 100 << "%\n"
                  << "          actual value after both  " << 100 * up * down << "\n";
        verifyClose("50% up then 50% down", 100 * up * down, 75.0, 1e-12, "leaves  ");
    }

    // ========================================================================
    part(9, "CONTINUOUS RANDOM VARIABLES", "where P(X = x) = 0");
    // ========================================================================
    question("What if X can be any real number, not just 2 through 12?");

    note("Then something strange happens: every individual value");
    std::cout << "      has probability ZERO. Not 'very small' - exactly zero.\n"
                 "      There are infinitely many possible values, and they all\n"
                 "      have to fit inside a total of 1.\n";

    note("Pick a random real number between 0 and 1. What is");
    std::cout << "      P(X = 0.5)? If it were any positive number e, then any\n"
                 "      1/e + 1 distinct values would already total more than 1.\n"
                 "      So it must be 0 - and yet SOME value occurs.\n";

    note("The fix is to stop asking about points and start asking");
    std::cout << "      about INTERVALS. The PMF is replaced by a PROBABILITY\n"
                 "      DENSITY FUNCTION f(x), and probability becomes AREA:\n"
        "\n"
        "         P(a <= X <= b)  =  the area under f between a and b\n"
        "\n"
        "      A density is not a probability. It can exceed 1 quite\n"
        "      happily - it is probability PER UNIT of x, and only\n"
        "      becomes a probability once you multiply by a width.\n";

    // Approximate a uniform density numerically to make the point concrete.
    {
        const int BINS = 10;
        std::cout << "\n      X uniform on [0,1], sliced into " << BINS
                  << " bins. Density is\n      " << BINS
                  << "x higher if you use " << BINS * 10 << " bins - but the AREA\n"
                     "      does not move:\n";
        std::cout << "\n        bins    density f(x)    width    area per bin    total\n"
                  << "        " << repeat("-", 62) << "\n";
        for (int b : {5, 10, 100, 1000}) {
            const double width = 1.0 / b;
            std::cout << "        " << std::setw(5) << b << std::setw(14) << 1.0
                      << std::setw(11) << width << std::setw(14) << width
                      << std::setw(12) << b * width << "\n";
        }
        note("Density stays 1 here because the interval is [0,1].");
        std::cout << "      Squash the same total probability into [0, 0.1] and the\n"
                     "      density becomes 10 - a number bigger than any\n"
                     "      probability is allowed to be, and perfectly legal.\n";
    }

    note("Everything else survives the transition unchanged. Sums");
    std::cout << "      become integrals and the meanings stay put:\n"
        "\n"
        "         discrete                    continuous\n"
        "         ------------------------    ------------------------\n"
        "         p(x) = P(X = x)             f(x), a density\n"
        "         sum of p(x) = 1             total area = 1\n"
        "         F(x) = sum p(t), t <= x     F(x) = area left of x\n"
        "         E[X] = sum x p(x)           E[X] = integral of x f(x)\n"
        "\n"
        "      And the CDF needs no translation at all, which is what\n"
        "      Part 4 meant by calling it the more fundamental object.\n";

    answer("Discrete has mass at points; continuous has density over intervals.");

    // ========================================================================
    part(10, "A REAL PROBLEM", "expected value decides, variance survives");
    // ========================================================================
    question("An insurer sells a 500-dollar policy. Claims cost 40,000");
    std::cout << "      and happen to 1 customer in 100. Good business?\n";

    {
        const double PREMIUM = 500.0, CLAIM = 40000.0, RATE = 0.01;
        Distribution profit{"profit per policy",
                            {PREMIUM, PREMIUM - CLAIM},
                            {1.0 - RATE, RATE}};
        std::cout << "\n        outcome                probability   profit\n"
                  << "        " << repeat("-", 58) << "\n"
                  << "        no claim               " << std::setw(11) << 1.0 - RATE
                  << std::setw(12) << PREMIUM << "\n"
                  << "        claim                  " << std::setw(11) << RATE
                  << std::setw(12) << PREMIUM - CLAIM << "\n";
        std::cout << "\n";
        showV("E[profit per policy]", expectation(profit));
        showV("sd(profit per policy)", stdDev(profit));

        answer("100 dollars expected profit, on a 3,980 dollar standard deviation.");

        note("The mean says sell it; the standard deviation says you");
        std::cout << "      cannot sell just one. A single policy is a coin flip\n"
                     "      with a small edge and an enormous swing - which is\n"
                     "      exactly 1.6's situation, and it has the same answer.\n";

        std::cout << "\n        policies   expected profit   sd of total   sd per policy\n"
                  << "        " << repeat("-", 62) << "\n";
        for (int n : {1, 10, 100, 1000, 10000, 100000}) {
            const double tot = n * expectation(profit);
            const double sdTot = std::sqrt(static_cast<double>(n)) * stdDev(profit);
            std::cout << "        " << std::setw(8) << commas(static_cast<unsigned long long>(n))
                      << std::setw(18) << tot << std::setw(14) << sdTot
                      << std::setw(15) << sdTot / n << "\n";
        }

        note("There it is again - total spread grows like sqrt(n)");
        std::cout << "      while total profit grows like n, so the risk PER\n"
                     "      POLICY collapses. That is not merely how insurance\n"
                     "      makes money; it is the only reason insurance can\n"
                     "      exist at all.\n";

        note("And the assumption underneath it, which 1.11 spent a");
        std::cout << "      whole lesson on: those sqrt(n) numbers require the\n"
                     "      claims to be INDEPENDENT. Sell 100,000 policies on\n"
                     "      houses in one flood plain and you have sold the same\n"
                     "      policy 100,000 times.\n";
    }

    // ========================================================================
    part(11, "DO NOT TRUST ME, SIMULATE IT", "500,000 rolls");
    // ========================================================================
    question("Roll two dice half a million times. Do the mean and the");
    std::cout << "      variance come out where Parts 5 and 6 said?\n";

    std::mt19937 rng(1729);
    std::uniform_int_distribution<int> die(1, 6);
    const int ROLLS = 500000;

    std::vector<int> seen(13, 0);
    double runningSum = 0.0, runningSq = 0.0;
    for (int t = 0; t < ROLLS; ++t) {
        const int s = die(rng) + die(rng);
        ++seen[static_cast<size_t>(s)];
        runningSum += s;
        runningSq  += static_cast<double>(s) * s;
    }
    const double simMean = runningSum / ROLLS;
    const double simVar  = runningSq / ROLLS - simMean * simMean;

    std::cout << "\n        x     P(X=x) formula   simulated     difference\n"
              << "        " << repeat("-", 60) << "\n";
    for (size_t i = 0; i < diceSum.values.size(); ++i) {
        const int s = static_cast<int>(diceSum.values[i]);
        const double sim = static_cast<double>(seen[static_cast<size_t>(s)]) / ROLLS;
        std::cout << "        " << std::setw(3) << s << std::setw(16) << diceSum.probs[i]
                  << std::setw(14) << sim << std::setw(14)
                  << std::fabs(diceSum.probs[i] - sim) << "\n";
    }

    std::cout << "\n";
    verifyClose("E[X]",   expectation(diceSum), simMean, 1e-2, "simulated");
    verifyClose("Var(X)", variance(diceSum),    simVar,  5e-2, "simulated");
    verifyClose("sd(X)",  stdDev(diceSum), std::sqrt(simVar), 1e-2, "simulated");

    answer("A machine that only knows how to roll dice reproduces all of it.");

    note("Note how the mean was computed in that loop: add up");
    std::cout << "      every roll and divide by how many. That is the ordinary\n"
                 "      average you already knew - and E[X] is what it converges\n"
                 "      to. The definition in Part 5 is not a new kind of\n"
                 "      average, it is the same one, computed in advance.\n";

    // ========================================================================
    part(12, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Build your own random variable.\n"
                 "\n"
                 "        1   n dice        - the sum of n dice, fully worked\n"
                 "        2   your own      - type values and probabilities\n"
                 "        3   a bet         - is this gamble worth taking?\n"
                 "        4   E[g(X)]       - see Jensen's gap for yourself\n"
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
                         "      1 n dice   2 your own   3 a bet   4 E[g(X)]   5 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 5, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            int n = 0, faces = 0;
            if (!askNumber("How many dice", 1, 8, n)) { keepGoing = false; break; }
            if (!askNumber("How many faces each", 2, 20, faces)) { keepGoing = false; break; }

            // Convolve the single-die distribution n times.
            std::vector<double> cur(static_cast<size_t>(faces) + 1, 0.0);
            for (int f = 1; f <= faces; ++f) cur[static_cast<size_t>(f)] = 1.0 / faces;
            for (int d = 1; d < n; ++d) {
                std::vector<double> next(cur.size() + static_cast<size_t>(faces), 0.0);
                for (size_t s = 0; s < cur.size(); ++s) {
                    if (cur[s] == 0.0) continue;
                    for (int f = 1; f <= faces; ++f)
                        next[s + static_cast<size_t>(f)] += cur[s] / faces;
                }
                cur = next;
            }
            Distribution d{"sum", {}, {}};
            for (size_t s = 0; s < cur.size(); ++s)
                if (cur[s] > 0.0) { d.values.push_back(static_cast<double>(s));
                                    d.probs.push_back(cur[s]); }

            std::cout << "\n      " << n << " dice of " << faces << " faces: sums "
                      << d.values.front() << " to " << d.values.back() << ".\n";
            showPmfAndCdf(d);
            std::cout << "\n";
            showV("E[X]",   expectation(d));
            showV("Var(X)", variance(d));
            showV("sd(X)",  stdDev(d));
            verifyClose("probabilities add to 1", probSum(d), 1.0, 1e-9, "must be");
            const double onePerDie = (faces + 1) / 2.0;
            verifyClose("E[X] = n x E[one die]", expectation(d), n * onePerDie,
                        1e-9, "n x mean");
            note("Linearity again - the mean of a sum is the sum of means.");
            break;
        }
        case 2: {
            int howMany = 0;
            if (!askNumber("How many possible values", 2, 8, howMany)) {
                keepGoing = false; break;
            }
            Distribution d{"yours", {}, {}};
            bool aborted = false;
            int weightTotal = 0;
            std::vector<int> weights;
            for (int i = 0; i < howMany; ++i) {
                int v = 0, w = 0;
                if (!askNumber("Value " + std::to_string(i + 1), -1000000, 1000000, v)) {
                    aborted = true; break;
                }
                if (!askNumber("  its weight (relative, any positive number)",
                               1, 1000000, w)) { aborted = true; break; }
                d.values.push_back(v);
                weights.push_back(w);
                weightTotal += w;
            }
            if (aborted) { keepGoing = false; break; }
            for (int w : weights) d.probs.push_back(static_cast<double>(w) / weightTotal);

            std::cout << "\n      Weights normalised by dividing by " << weightTotal
                      << ".\n";
            showPmfAndCdf(d);
            std::cout << "\n";
            showV("E[X]",       expectation(d));
            showV("E[X^2]",     expectationOf(d, [](double x) { return x * x; }));
            showV("(E[X])^2",   expectation(d) * expectation(d));
            showV("Var(X)",     variance(d));
            showV("sd(X)",      stdDev(d));
            verifyClose("probabilities add to 1", probSum(d), 1.0, 1e-9, "must be");
            verifyClose("both variance formulas", variance(d), varianceShortcut(d),
                        1e-6, "shortcut");
            break;
        }
        case 3: {
            int stake = 0, payout = 0, winPct = 0, plays = 0;
            if (!askNumber("Your stake per play", 1, 100000, stake)) { keepGoing = false; break; }
            if (!askNumber("Payout if you win (profit, not total)", 1, 10000000, payout)) {
                keepGoing = false; break;
            }
            if (!askNumber("Chance of winning, in %", 1, 99, winPct)) { keepGoing = false; break; }
            if (!askNumber("How many times will you play", 1, 1000000, plays)) {
                keepGoing = false; break;
            }
            const double pw = winPct / 100.0;
            Distribution bet{"profit", {static_cast<double>(payout),
                                        -static_cast<double>(stake)}, {pw, 1.0 - pw}};
            std::cout << "\n        outcome     probability      profit\n"
                      << "        " << repeat("-", 58) << "\n"
                      << "        win         " << std::setw(11) << pw
                      << std::setw(13) << payout << "\n"
                      << "        lose        " << std::setw(11) << 1.0 - pw
                      << std::setw(13) << -stake << "\n";
            std::cout << "\n";
            showV("E[profit per play]",  expectation(bet));
            showV("sd(profit per play)", stdDev(bet));
            std::cout << "\n";
            showV("expected total over " + std::to_string(plays) + " plays",
                  plays * expectation(bet));
            showV("sd of that total",
                  std::sqrt(static_cast<double>(plays)) * stdDev(bet));
            const double edgeRatio = plays * expectation(bet)
                / (std::sqrt(static_cast<double>(plays)) * stdDev(bet));
            std::cout << "\n      Expected total is " << std::fabs(edgeRatio)
                      << " standard deviations from zero.\n";
            std::cout << "\n      ";
            if (expectation(bet) > 0 && edgeRatio > 2)
                std::cout << "Positive edge, and enough plays for it to show.\n";
            else if (expectation(bet) > 0)
                std::cout << "Positive edge, but too few plays to rely on it.\n";
            else if (expectation(bet) == 0)
                std::cout << "A fair bet. Zero expected profit, all variance.\n";
            else
                std::cout << "Negative expected value. More plays makes it worse.\n";
            break;
        }
        case 4: {
            int lo = 0, hi = 0;
            if (!askNumber("X is uniform on a range - lowest value", 1, 1000, lo)) {
                keepGoing = false; break;
            }
            if (!askNumber("  highest value", lo, 1000, hi)) { keepGoing = false; break; }
            Distribution d{"uniform", {}, {}};
            const int count = hi - lo + 1;
            for (int v = lo; v <= hi; ++v) {
                d.values.push_back(v);
                d.probs.push_back(1.0 / count);
            }
            std::cout << "\n      X uniform on " << lo << " to " << hi
                      << ", E[X] = " << expectation(d) << ".\n";
            std::cout << "\n        g(x)          g(E[X])       E[g(X)]       gap\n"
                      << "        " << repeat("-", 60) << "\n";
            for (const auto& g : gs) {
                const double ge = g.second(expectation(d));
                const double eg = expectationOf(d, g.second);
                std::cout << "        " << std::left << std::setw(12) << g.first
                          << std::right << std::setw(12) << ge << std::setw(14) << eg
                          << std::setw(14) << eg - ge << "\n";
            }
            std::cout << "\n      Var(X) = " << variance(d)
                      << ", and the x^2 gap is exactly that.\n";
            verifyClose("x^2 gap equals Var(X)",
                        expectationOf(d, [](double x) { return x * x; })
                            - expectation(d) * expectation(d),
                        variance(d), 1e-6, "Var(X)  ");
            note("Wider range -> more variance -> bigger Jensen gap.");
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
    part(13, "WHAT YOU LEARNED", "1.12 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      A RANDOM VARIABLE IS A FUNCTION\n"
        "          X : sample space -> the numbers\n"
        "          not random, not a variable - the randomness is in\n"
        "          which OUTCOME occurs, never in what X does to it\n"
        "          'X = 7' is a NAME FOR AN EVENT, so P(X = 7) is 1.1\n"
        "\n"
        "      NOTATION\n"
        "          X  the mechanism      x  a particular value it takes\n"
        "\n"
        "      THE PMF - discrete\n"
        "          p(x) = P(X = x), and only two rules:\n"
        "          p(x) >= 0, and the p(x) add to exactly 1\n"
        "          it is DERIVED from the sample space, not chosen\n"
        "\n"
        "      THE CDF - and it is the more fundamental object\n"
        "          F(x) = P(X <= x), a running total\n"
        "          P(a < X <= b) = F(b) - F(a)\n"
        "          never decreases, runs 0 to 1\n"
        "          survives into the continuous case where the PMF dies\n"
        "\n"
        "      EXPECTED VALUE - the balance point\n"
        "          E[X] = sum of x P(X = x)\n"
        "          NOT what you expect: E[one die] = 3.5, impossible\n"
        "          it is the centre of mass, literally\n"
        "\n"
        "      VARIANCE - spread\n"
        "          Var(X) = E[(X-mu)^2] = E[X^2] - (E[X])^2\n"
        "          square because E[X - mu] is always exactly 0\n"
        "          sd(X) = sqrt(Var(X)), back in the original units\n"
        "\n"
        "      LINEARITY OF EXPECTATION - the best theorem here\n"
        "          E[X + Y] = E[X] + E[Y],  ALWAYS\n"
        "          no independence required - they can be perfectly\n"
        "          dependent and it still holds\n"
        "          it proves the binomial mean = np in one line\n"
        "\n"
        "      BUT E[g(X)] IS NOT g(E[X])\n"
        "          E[X^2] - (E[X])^2 = Var(X), never zero unless X is\n"
        "          convex g always bends the average UP - Jensen\n"
        "          +50% then -50% averages to 0% and leaves you at 75\n"
        "\n"
        "      CONTINUOUS - density, not mass\n"
        "          P(X = x) = 0 for every x, and yet X takes a value\n"
        "          f(x) is probability PER UNIT and may exceed 1\n"
        "          probability becomes AREA; sums become integrals\n"
        "\n"
        "      COMING NEXT\n"
        "          1.13 the two simplest random variables there are -\n"
        "               Bernoulli and binomial - and the proper proof\n"
        "               of the np and np(1-p) that 1.6 asserted.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
