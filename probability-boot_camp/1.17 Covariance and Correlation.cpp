// ============================================================================
//  1.17 - COVARIANCE AND CORRELATION
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.17 Covariance and Correlation.cpp" -o p117
//      ./p117
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.11  independence - when one event tells you nothing about another
//  1.12  E[X] and Var(X) for a single random variable
//  1.13  to 1.16, a catalogue of single random variables
//  1.17  TWO at once                                     <- you are here
//
//  Every distribution so far has described one quantity on its own. But
//  almost nothing you care about lives alone:
//
//       a stock and the index it sits in
//       a machine's temperature and its failure rate
//       hours revised and marks scored
//
//  1.11 already gave you a yes/no answer to "are these related?" -
//  independent or not. That is a switch. What we need is a dial.
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA
//  ---------------------------------------------------------------------------
//  Variance asks how far X strays from its own mean:
//
//       Var(X)  =  E[ (X - muX)(X - muX) ]
//
//  Replace the second copy of X with Y and you have COVARIANCE - how far
//  the two stray from their means TOGETHER:
//
//       Cov(X,Y)  =  E[ (X - muX)(Y - muY) ]
//
//  That is the whole definition. Variance is just covariance of something
//  with itself: Cov(X,X) = Var(X).
//
//  Covariance has ugly units (it comes out in X-units times Y-units), so
//  we divide the units away and get CORRELATION:
//
//                    Cov(X,Y)
//       rho  =  ------------------        always between -1 and +1
//                sigmaX x sigmaY
//
//  One number, no units, bounded. It is the dial.
//
//  ---------------------------------------------------------------------------
//  AND THE WARNING, UP FRONT
//  ---------------------------------------------------------------------------
//  rho measures STRAIGHT-LINE association and nothing else. Part 6 builds
//  two variables where one completely determines the other and the
//  correlation is exactly zero. Independent implies rho = 0; rho = 0 does
//  NOT imply independent, and that gap is where real money is lost.
// ============================================================================

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// ============================================================================
//  A JOINT DISTRIBUTION
//
//  Everything in this lesson is computed from a table of joint
//  probabilities - P(X = x AND Y = y) for every pair - so nothing has to be
//  taken on trust. Small enough to print, exact enough to check by hand.
// ============================================================================
struct Joint {
    std::vector<double> xs;                  // values X can take
    std::vector<double> ys;                  // values Y can take
    std::vector<std::vector<double>> p;      // p[i][j] = P(X = xs[i], Y = ys[j])
};

// The MARGINAL of X: forget Y entirely and sum across its row. This is the
// law of total probability from 1.8, doing what it always does.
std::vector<double> marginalX(const Joint& j) {
    std::vector<double> m(j.xs.size(), 0.0);
    for (size_t i = 0; i < j.xs.size(); ++i)
        for (size_t k = 0; k < j.ys.size(); ++k) m[i] += j.p[i][k];
    return m;
}

std::vector<double> marginalY(const Joint& j) {
    std::vector<double> m(j.ys.size(), 0.0);
    for (size_t k = 0; k < j.ys.size(); ++k)
        for (size_t i = 0; i < j.xs.size(); ++i) m[k] += j.p[i][k];
    return m;
}

double totalProbability(const Joint& j) {
    double t = 0.0;
    for (const auto& row : j.p) for (double v : row) t += v;
    return t;
}

// E[g(X,Y)] for any g - every expectation below is this one function.
double expect(const Joint& j, const std::function<double(double, double)>& g) {
    double e = 0.0;
    for (size_t i = 0; i < j.xs.size(); ++i)
        for (size_t k = 0; k < j.ys.size(); ++k)
            e += g(j.xs[i], j.ys[k]) * j.p[i][k];
    return e;
}

double meanX(const Joint& j) { return expect(j, [](double x, double)   { return x; }); }
double meanY(const Joint& j) { return expect(j, [](double, double y)   { return y; }); }

double varX(const Joint& j) {
    const double m = meanX(j);
    return expect(j, [m](double x, double) { return (x - m) * (x - m); });
}
double varY(const Joint& j) {
    const double m = meanY(j);
    return expect(j, [m](double, double y) { return (y - m) * (y - m); });
}

// COVARIANCE, straight from the definition - the average product of the two
// deviations. Not the shortcut; the shortcut gets checked against this.
double covariance(const Joint& j) {
    const double mx = meanX(j), my = meanY(j);
    return expect(j, [mx, my](double x, double y) { return (x - mx) * (y - my); });
}

// The same thing by the computational shortcut, E[XY] - E[X]E[Y].
double covarianceShortcut(const Joint& j) {
    return expect(j, [](double x, double y) { return x * y; }) - meanX(j) * meanY(j);
}

// CORRELATION - covariance with the units divided out.
double correlation(const Joint& j) {
    return covariance(j) / (std::sqrt(varX(j)) * std::sqrt(varY(j)));
}

// Build the INDEPENDENT joint with the same two marginals, by multiplying
// them - which is exactly what 1.11 says independence means.
Joint independentVersion(const Joint& j) {
    Joint out;
    out.xs = j.xs;
    out.ys = j.ys;
    const std::vector<double> mx = marginalX(j), my = marginalY(j);
    out.p.assign(j.xs.size(), std::vector<double>(j.ys.size(), 0.0));
    for (size_t i = 0; i < j.xs.size(); ++i)
        for (size_t k = 0; k < j.ys.size(); ++k) out.p[i][k] = mx[i] * my[k];
    return out;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.16)
// ============================================================================
const int PAGE = 70;

std::string repeat(const std::string& unit, int times) {
    std::string s;
    for (int i = 0; i < times; ++i) s += unit;
    return s;
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

// The joint table, with its marginals down the side and along the bottom -
// which is where the word "marginal" comes from in the first place.
void showJoint(const Joint& j, const std::string& xName, const std::string& yName,
               const std::vector<std::string>& xLabels,
               const std::vector<std::string>& yLabels) {
    std::cout << "\n                        " << yName << "\n              ";
    for (const std::string& l : yLabels) std::cout << std::setw(9) << l;
    std::cout << std::setw(11) << "P(X)" << "\n"
              << "        " << repeat("-", 54) << "\n";
    const std::vector<double> mx = marginalX(j), my = marginalY(j);
    for (size_t i = 0; i < j.xs.size(); ++i) {
        std::cout << "  " << std::left << std::setw(6)
                  << (i == j.xs.size() / 2 ? xName : "")
                  << std::setw(6) << xLabels[i] << std::right;
        for (size_t k = 0; k < j.ys.size(); ++k)
            std::cout << std::setw(9) << std::fixed << std::setprecision(2) << j.p[i][k];
        std::cout << std::setw(11) << mx[i] << "\n";
    }
    std::cout << "        " << repeat("-", 54) << "\n"
              << "        " << std::left << std::setw(6) << "P(Y)" << std::right;
    for (double v : my) std::cout << std::setw(9) << v;
    std::cout << std::setw(11) << totalProbability(j) << "\n"
              << std::setprecision(4);
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // Two stocks. Each day each one is DOWN (-1), FLAT (0) or UP (+1).
    // The table is deliberately lumpy on the diagonal - they tend to move
    // together, which is the thing we are about to measure.
    Joint mkt;
    mkt.xs = {-1.0, 0.0, 1.0};
    mkt.ys = {-1.0, 0.0, 1.0};
    mkt.p  = {{0.20, 0.05, 0.05},
              {0.05, 0.20, 0.05},
              {0.05, 0.05, 0.30}};
    const std::vector<std::string> moves = {"down", "flat", "up"};

    // ========================================================================
    part(1, "ONE VARIABLE IS NOT ENOUGH", "1.12 only got half the job");
    // ========================================================================
    question("Two stocks. Each has its own mean and variance. Is that");
    std::cout << "      everything there is to know about the pair?\n";

    note("No, and here is the cheapest proof. Take two variables");
    std::cout << "      with identical means and variances, and add them:\n\n"
        "         same coin twice     X + X  =  2X, swings twice as wide\n"
        "         two separate coins  X + Y, the swings partly cancel\n"
        "\n"
        "      Both sums are built from parts with the same mean and the\n"
        "      same variance, and the sums behave completely differently.\n"
        "      So Var(X) and Var(Y) alone cannot tell you Var(X+Y).\n";

    answer("Something is missing, and it is about the PAIR, not the parts.");

    note("Here is the pair we will use. Two stocks, one day, nine");
    std::cout << "      possible outcomes - small enough to print in full:\n";
    showJoint(mkt, "stock", "stock B", moves, moves);
    verifyClose("the table sums to one", totalProbability(mkt), 1.0,
                1e-12, "must be");

    note("Look at the diagonal - 0.20, 0.20, 0.30 - against the");
    std::cout << "      corners. These two move together far more often than\n"
                 "      they move apart. That is what we are about to put a\n"
                 "      number on.\n";

    // ========================================================================
    part(2, "COVARIANCE", "variance, with a second variable");
    // ========================================================================
    question("How do you measure two things straying together?");

    note("Start from variance and change one symbol. Variance is");
    std::cout << "      the average of a deviation times ITSELF:\n\n"
        "         Var(X)    =  E[ (X - muX)(X - muX) ]\n"
        "\n"
        "      Replace the second copy with Y and you have covariance:\n\n"
        "         Cov(X,Y)  =  E[ (X - muX)(Y - muY) ]\n"
        "\n"
        "      Variance is the special case Cov(X,X). Nothing new was\n"
        "      invented here - the definition was already sitting in 1.12.\n";

    note("Why the SIGN of that product carries the whole meaning:");
    std::cout << "\n"
        "                        Y above its mean\n"
        "                              |\n"
        "             (-) x (+) = -    |    (+) x (+) = +\n"
        "                              |\n"
        "        X below  -------------+-------------  X above\n"
        "                              |\n"
        "             (-) x (-) = +    |    (+) x (-) = -\n"
        "                              |\n"
        "                        Y below its mean\n"
        "\n"
        "      Days where both are on the SAME side of their means give a\n"
        "      positive product. Days on opposite sides give a negative\n"
        "      one. Covariance is the average, so it asks: which kind of\n"
        "      day wins?\n";

    const double mx = meanX(mkt), my = meanY(mkt);
    std::cout << "\n";
    showV("E[X], stock A's mean day", mx);
    showV("E[Y], stock B's mean day", my);

    note("Every one of the nine outcomes, with its contribution:");
    std::cout << "\n        A       B      P      (x-mx)(y-my)   contribution\n"
              << "        " << repeat("-", 58) << "\n";
    double built = 0.0;
    for (size_t i = 0; i < mkt.xs.size(); ++i)
        for (size_t k = 0; k < mkt.ys.size(); ++k) {
            const double prod = (mkt.xs[i] - mx) * (mkt.ys[k] - my);
            const double c = prod * mkt.p[i][k];
            built += c;
            std::cout << "        " << std::left << std::setw(8) << moves[i]
                      << std::setw(7) << moves[k] << std::right
                      << std::setw(6) << std::setprecision(2) << mkt.p[i][k]
                      << std::setw(14) << std::setprecision(4) << prod
                      << std::setw(15) << c << "\n";
        }
    std::cout << std::setprecision(4);
    showV("Cov(X,Y) = the total", built);
    verifyClose("built outcome by outcome", covariance(mkt), built, 1e-12,
                "summed ");

    note("Positive, so the same-side days win. Read the sign first:");
    std::cout << "\n"
        "         Cov > 0    they tend to move the SAME way\n"
        "         Cov < 0    they tend to move OPPOSITE ways\n"
        "         Cov = 0    no straight-line tendency either way\n";

    // ========================================================================
    part(3, "THE SHORTCUT", "E[XY] - E[X]E[Y]");
    // ========================================================================
    question("Is there a version you can compute in one pass?");

    note("Expand the brackets and use linearity of expectation from");
    std::cout << "      1.12. The cross terms collapse:\n\n"
        "         E[(X-muX)(Y-muY)]\n"
        "           = E[XY] - muY E[X] - muX E[Y] + muX muY\n"
        "           = E[XY] - muX muY - muX muY + muX muY\n"
        "           = E[XY] - E[X] E[Y]\n";

    std::cout << "\n";
    showV("E[XY]", expect(mkt, [](double x, double y) { return x * y; }));
    showV("E[X] E[Y]", mx * my);
    showV("difference", covarianceShortcut(mkt));
    verifyClose("shortcut = definition", covarianceShortcut(mkt),
                covariance(mkt), 1e-12, "definition");

    note("It also says something on its own. If X and Y are");
    std::cout << "      independent then E[XY] = E[X]E[Y] - the expectation\n"
                 "      factorises - so the difference is zero:\n\n"
                 "         INDEPENDENT  =>  Cov = 0\n"
                 "\n"
                 "      Keep the arrow pointing that way. Part 6 is entirely\n"
                 "      about what happens if you try to reverse it.\n";

    const Joint indep = independentVersion(mkt);
    note("Same two marginals, multiplied together as 1.11 requires:");
    showJoint(indep, "stock", "stock B", moves, moves);
    verifyClose("independent => Cov = 0", covariance(indep), 0.0, 1e-12,
                "must be");

    // ========================================================================
    part(4, "THE UNITS PROBLEM", "why covariance alone is useless");
    // ========================================================================
    question("Cov = 0.39. Is that a big number?");

    note("There is no way to answer that, because covariance carries");
    std::cout << "      units - X-units multiplied by Y-units. Change how you\n"
                 "      write the same data down and the number changes:\n";

    // Identical information, expressed in basis points instead of units.
    Joint bp = mkt;
    for (double& v : bp.xs) v *= 100.0;
    for (double& v : bp.ys) v *= 100.0;

    std::cout << "\n";
    showV("Cov in the original units", covariance(mkt));
    showV("Cov with both in 'per 100'", covariance(bp));
    std::cout << "\n"
        "      Exactly the same two stocks, exactly the same days. The\n"
        "      covariance went up by a factor of 10,000 because we chose\n"
        "      different units. A quantity that moves when you change\n"
        "      nothing cannot be compared across problems.\n";

    answer("Divide the units out. That is all correlation is.");

    // ========================================================================
    part(5, "CORRELATION", "the dial, from -1 to +1");
    // ========================================================================
    question("How do you get a number that means the same everywhere?");

    note("Divide by the two standard deviations. Cov is in X-units");
    std::cout << "      times Y-units; sigmaX sigmaY is in exactly the same\n"
                 "      units, so the ratio has none at all:\n\n"
        "                        Cov(X,Y)\n"
        "         rho  =  ---------------------\n"
        "                  sigmaX  x  sigmaY\n";

    std::cout << "\n";
    showV("Cov(X,Y)", covariance(mkt));
    showV("sigmaX", std::sqrt(varX(mkt)));
    showV("sigmaY", std::sqrt(varY(mkt)));
    showV("rho", correlation(mkt));

    verifyClose("rho ignores units", correlation(bp),
                correlation(mkt), 1e-12, "original");

    note("And it is bounded. rho can never leave [-1, +1], because");
    std::cout << "      Cov is capped by the two spreads it is built from:\n\n"
        "         rho = +1    a perfect straight line, upward\n"
        "         rho =  0    no straight-line tendency\n"
        "         rho = -1    a perfect straight line, downward\n";

    note("The three extremes, built as joint tables and measured:");
    Joint perfectUp, perfectDown;
    perfectUp.xs = perfectDown.xs = {-1.0, 1.0};
    perfectUp.ys = perfectDown.ys = {-1.0, 1.0};
    perfectUp.p   = {{0.5, 0.0}, {0.0, 0.5}};      // Y always equals X
    perfectDown.p = {{0.0, 0.5}, {0.5, 0.0}};      // Y is always -X

    std::cout << "\n";
    showV("Y = X exactly", correlation(perfectUp));
    showV("Y = -X exactly", correlation(perfectDown));
    showV("our two stocks", correlation(mkt));
    showV("the independent version", correlation(indep));

    verifyClose("|rho| <= 1 for Y = X", std::fabs(correlation(perfectUp)), 1.0,
                1e-12, "the cap");

    // ========================================================================
    part(6, "WHAT CORRELATION IS NOT", "the expensive mistake");
    // ========================================================================
    // The single most important part of this lesson.
    question("rho = 0. Does that mean the two are independent?");

    note("No. And this is not a technicality - it is the difference");
    std::cout << "      between a risk model that works and one that does not.\n";

    note("Here are two variables where one COMPLETELY determines");
    std::cout << "      the other. X is -2, -1, +1 or +2, each equally likely,\n"
                 "      and Y is simply X squared. Knowing X tells you Y with\n"
                 "      certainty - there is no randomness left at all.\n";

    Joint parab;
    parab.xs = {-2.0, -1.0, 1.0, 2.0};
    parab.ys = {1.0, 4.0};
    // Y = X^2 exactly: each x puts all its 0.25 on the matching y.
    parab.p = {{0.00, 0.25},      // x = -2  ->  y = 4
               {0.25, 0.00},      // x = -1  ->  y = 1
               {0.25, 0.00},      // x = +1  ->  y = 1
               {0.00, 0.25}};     // x = +2  ->  y = 4

    std::cout << "\n        x      y = x^2    P\n"
              << "        " << repeat("-", 40) << "\n";
    for (size_t i = 0; i < parab.xs.size(); ++i)
        for (size_t k = 0; k < parab.ys.size(); ++k)
            if (parab.p[i][k] > 0.0)
                std::cout << "        " << std::setw(4) << std::setprecision(0)
                          << parab.xs[i] << std::setw(10) << parab.ys[k]
                          << std::setw(9) << std::setprecision(2) << parab.p[i][k]
                          << "\n";
    std::cout << std::setprecision(4);

    std::cout << "\n";
    showV("E[X]", meanX(parab));
    showV("E[XY] = E[X^3]", expect(parab, [](double x, double y) { return x * y; }));
    showV("Cov(X,Y)", covariance(parab));
    showV("rho", correlation(parab));
    verifyClose("perfect dependence", correlation(parab), 0.0, 1e-12,
                "measured");

    note("Zero. The negative x's contribute exactly what the");
    std::cout << "      positive x's contribute, with the opposite sign, and it\n"
                 "      all cancels. Y is a perfect function of X and the\n"
                 "      correlation cannot see it, because the relationship is\n"
                 "      a CURVE and rho only looks for a STRAIGHT LINE.\n";

    std::cout << "\n"
        "      So the arrow only runs one way:\n"
        "\n"
        "         independent      =>   rho = 0        always true\n"
        "         rho = 0          =>   independent    NOT true\n"
        "\n"
        "      A correlation of zero rules out a linear relationship.\n"
        "      It rules out nothing else.\n";

    note("Two more things rho does not mean:");
    std::cout << "\n"
        "         CAUSATION      ice cream sales and drownings correlate\n"
        "                        because both follow the weather. Neither\n"
        "                        causes the other.\n"
        "\n"
        "         STABILITY      rho is measured over some period. Part 8\n"
        "                        is about what happens when the period you\n"
        "                        measured is not the period you needed.\n";

    // ========================================================================
    part(7, "THE VARIANCE OF A SUM", "what Part 1 was missing");
    // ========================================================================
    question("So what IS Var(X + Y)?");

    note("Expand the definition and the covariance falls out - the");
    std::cout << "      middle term is the one 1.12 could not supply:\n\n"
        "         Var(X+Y)  =  Var(X) + Var(Y) + 2 Cov(X,Y)\n"
        "\n"
        "      and dividing by the sigmas, the same statement in rho:\n\n"
        "         Var(X+Y)  =  varX + varY + 2 rho sigmaX sigmaY\n";

    // Built directly: the distribution of the sum, from the joint table.
    double sumMean = expect(mkt, [](double x, double y) { return x + y; });
    double sumVar  = expect(mkt, [sumMean](double x, double y) {
        return (x + y - sumMean) * (x + y - sumMean);
    });

    std::cout << "\n";
    showV("Var(X)", varX(mkt));
    showV("Var(Y)", varY(mkt));
    showV("2 Cov(X,Y)", 2.0 * covariance(mkt));
    showV("sum of the three", varX(mkt) + varY(mkt) + 2.0 * covariance(mkt));
    showV("Var(X+Y) computed directly", sumVar);
    verifyClose("Var(X+Y) formula", varX(mkt) + varY(mkt) + 2.0 * covariance(mkt),
                sumVar, 1e-12, "direct ");

    note("Now Part 1's puzzle answers itself. Same parts, three");
    std::cout << "      different totals, and only the covariance changed:\n\n"
        "         Cov > 0   the sum swings MORE than the parts suggest\n"
        "         Cov = 0   variances simply add - the independent case\n"
        "         Cov < 0   the sum swings LESS; the two partly cancel\n";

    std::cout << "\n";
    showV("Var(X+Y), our correlated stocks", sumVar);
    double indepMean = expect(indep, [](double x, double y) { return x + y; });
    showV("Var(X+Y) if they were independent",
          expect(indep, [indepMean](double x, double y) {
              return (x + y - indepMean) * (x + y - indepMean); }));

    note("Same two stocks, same individual risks. Believing they");
    std::cout << "      were independent would understate the risk of holding\n"
                 "      both - and understating risk is the only kind of error\n"
                 "      that is expensive.\n";

    // ========================================================================
    part(8, "DIVERSIFICATION IS A CORRELATION", "1.1's promise, cashed");
    // ========================================================================
    // 1.1 claimed diversification works and that it fails in a crash. This
    // is the part where that claim becomes a formula.
    question("Why does splitting money across two stocks reduce risk -");
    std::cout << "      and why does it stop working exactly when you need it?\n";

    note("Put half your money in each of two stocks, both with the");
    std::cout << "      same volatility sigma. Apply Part 7 to the portfolio:\n\n"
        "         Var(half X + half Y)\n"
        "             = 0.25 varX + 0.25 varY + 2 (0.25) Cov(X,Y)\n"
        "             = 0.5 sigma^2 (1 + rho)\n"
        "\n"
        "      so the portfolio's volatility is\n"
        "\n"
        "         sigma_p  =  sigma  x  sqrt( (1 + rho) / 2 )\n"
        "\n"
        "      Everything depends on rho. Nothing else in that expression\n"
        "      can be changed by how you split the money.\n";

    const double vol = 0.20;                     // 20% each, annualised
    std::cout << "\n        rho     portfolio vol   risk removed\n"
              << "        " << repeat("-", 58) << "\n";
    for (double rho : {1.0, 0.8, 0.5, 0.2, 0.0, -0.5, -1.0}) {
        const double pv = vol * std::sqrt((1.0 + rho) / 2.0);
        std::cout << "        " << std::setw(5) << std::setprecision(2) << rho
                  << std::setw(15) << std::setprecision(4) << pv
                  << std::setw(13) << std::setprecision(1) << (1.0 - pv / vol) * 100.0
                  << "%" << bar(1.0 - pv / vol, 20) << "\n";
    }
    std::cout << std::setprecision(4);

    note("Read the top row and the bottom row. At rho = +1 you have");
    std::cout << "      not diversified at all - you own the same bet twice, and\n"
                 "      the portfolio is exactly as risky as one stock. At\n"
                 "      rho = -1 the two cancel perfectly and the risk is zero.\n"
                 "      Everything real lives in between.\n";

    answer("Diversification is not about owning MANY things.");
    note("It is about owning things with low rho. Thirty stocks");
    std::cout << "      all correlated at 0.9 is one bet wearing thirty hats.\n";

    // The failure mode, which is the part that actually matters.
    note("And now the sting. rho is not a constant - it is measured");
    std::cout << "      over some past window. In calm markets stocks drift on\n"
                 "      their own news and rho is modest. In a crash everyone\n"
                 "      sells everything at once and rho leaps toward 1:\n";

    std::cout << "\n        regime            rho    portfolio vol\n"
              << "        " << repeat("-", 54) << "\n";
    struct Regime { const char* name; double rho; };
    for (const Regime& r : std::vector<Regime>{{"calm market", 0.25},
                                               {"normal", 0.45},
                                               {"stress", 0.75},
                                               {"crash", 0.95}}) {
        std::cout << "        " << std::left << std::setw(16) << r.name << std::right
                  << std::setw(6) << std::setprecision(2) << r.rho
                  << std::setw(16) << std::setprecision(4)
                  << vol * std::sqrt((1.0 + r.rho) / 2.0) << "\n";
    }
    std::cout << std::setprecision(4);

    note("The protection you measured in calm markets is largest");
    std::cout << "      exactly when you do not need it, and smallest exactly\n"
                 "      when you do. That is not a flaw in the formula - the\n"
                 "      formula is telling the truth. It is a warning about\n"
                 "      which rho you fed it.\n";

    // ========================================================================
    part(9, "DO NOT TRUST ME, SIMULATE IT", "200,000 pairs");
    // ========================================================================
    question("Does a measured correlation actually recover rho?");

    note("Manufacture pairs with a known rho and measure them back.");
    std::cout << "      Take two independent standard normals from 1.15 and mix:\n\n"
        "         X = Z1\n"
        "         Y = rho Z1 + sqrt(1 - rho^2) Z2\n"
        "\n"
        "      Y is part X and part fresh noise, in exactly the\n"
        "      proportion that produces the correlation you asked for.\n";

    std::mt19937 rng(20260831);
    std::normal_distribution<double> gauss(0.0, 1.0);
    const int PAIRS = 200000;

    std::cout << "\n        asked for   measured    error\n"
              << "        " << repeat("-", 46) << "\n";
    double worstError = 0.0;
    for (double target : {-0.9, -0.4, 0.0, 0.3, 0.7, 0.95}) {
        double sx = 0, sy = 0, sxy = 0, sxx = 0, syy = 0;
        for (int i = 0; i < PAIRS; ++i) {
            const double z1 = gauss(rng), z2 = gauss(rng);
            const double x = z1;
            const double y = target * z1 + std::sqrt(1.0 - target * target) * z2;
            sx += x; sy += y; sxy += x * y; sxx += x * x; syy += y * y;
        }
        const double n = PAIRS;
        const double measured = (sxy / n - (sx / n) * (sy / n)) /
            (std::sqrt(sxx / n - (sx / n) * (sx / n)) *
             std::sqrt(syy / n - (sy / n) * (sy / n)));
        worstError = std::max(worstError, std::fabs(measured - target));
        std::cout << "        " << std::setw(9) << std::setprecision(2) << target
                  << std::setw(12) << std::setprecision(4) << measured
                  << std::setw(11) << std::fabs(measured - target) << "\n";
    }
    std::cout << std::setprecision(4);
    verifyClose("worst recovery error", worstError, 0.0, 0.01, "target ");

    note("Now the same machine, run on the Part 6 curve. Sample X,");
    std::cout << "      set Y = X squared, and measure the correlation of a\n"
                 "      relationship that is perfect but not straight:\n";
    {
        double sx = 0, sy = 0, sxy = 0, sxx = 0, syy = 0;
        for (int i = 0; i < PAIRS; ++i) {
            const double x = gauss(rng), y = x * x;
            sx += x; sy += y; sxy += x * y; sxx += x * x; syy += y * y;
        }
        const double n = PAIRS;
        const double measured = (sxy / n - (sx / n) * (sy / n)) /
            (std::sqrt(sxx / n - (sx / n) * (sx / n)) *
             std::sqrt(syy / n - (sy / n) * (sy / n)));
        showV("measured rho for Y = X^2", measured);
        verifyClose("a curve reads as rho = 0", measured, 0.0, 0.01, "target ");
    }
    note("Y is a perfect function of X and the correlation is zero.");
    std::cout << "      If you had only been shown that number, you would have\n"
                 "      concluded there was no relationship at all.\n";

    // ========================================================================
    part(10, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Put your own numbers in. Every answer shows its working.\n"
                 "\n"
                 "        1   two stocks    - portfolio risk for a given rho\n"
                 "        2   sum of two    - Var(X+Y) from the three pieces\n"
                 "        3   how many      - rho needed to hit a risk target\n"
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

    // rho and volatilities are read as whole percents, so every prompt stays
    // integer-only and the input handling cannot be confused.
    bool keepGoing = true;
    bool firstRound = true;
    while (keepGoing) {
        if (!firstRound)
            std::cout << "\n      " << repeat("-", 60) << "\n"
                         "      1 two stocks   2 sum of two   3 how many"
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
            int volA = 0, volB = 0, rhoPct = 0, weightA = 0;
            if (!askInt("stock A volatility, in %", 1, 100, volA)) { keepGoing = false; break; }
            if (!askInt("stock B volatility, in %", 1, 100, volB)) { keepGoing = false; break; }
            if (!askInt("correlation x 100 (-100 to 100)", -100, 100, rhoPct)) { keepGoing = false; break; }
            if (!askInt("percent of money in A", 0, 100, weightA)) { keepGoing = false; break; }

            const double sa = volA / 100.0, sb = volB / 100.0;
            const double rho = rhoPct / 100.0;
            const double wa = weightA / 100.0, wb = 1.0 - wa;
            const double cov = rho * sa * sb;
            const double var = wa * wa * sa * sa + wb * wb * sb * sb
                             + 2.0 * wa * wb * cov;

            std::cout << "\n      " << weightA << "% in A (vol " << volA
                      << "%), " << 100 - weightA << "% in B (vol " << volB
                      << "%), rho = " << std::setprecision(2) << rho << "\n"
                      << std::setprecision(4);

            std::cout << "\n        STEP 1  covariance from the correlation\n"
                         "                Cov = rho x sigmaA x sigmaB\n";
            showV("Cov(A,B)", cov);

            std::cout << "\n        STEP 2  Part 7's formula, with weights\n"
                         "                wA^2 varA + wB^2 varB + 2 wA wB Cov\n";
            showV("wA^2 varA", wa * wa * sa * sa);
            showV("wB^2 varB", wb * wb * sb * sb);
            showV("2 wA wB Cov", 2.0 * wa * wb * cov);
            showV("portfolio variance", var);

            std::cout << "\n        STEP 3  take the square root\n\n";
            showV("ANSWER  portfolio volatility", std::sqrt(var));

            const double naive = wa * sa + wb * sb;
            std::cout << "\n      Weighted average of the two vols would be "
                      << naive << ".\n";
            if (std::sqrt(var) < naive - 1e-9)
                std::cout << "      You are BELOW it - that gap is the diversification,\n"
                             "      and it exists only because rho < 1.\n";
            else
                std::cout << "      You are not below it - at rho = 1 there is no\n"
                             "      diversification to be had.\n";
            break;
        }
        case 2: {
            int varXi = 0, varYi = 0, covi = 0;
            if (!askInt("Var(X) x 100", 0, 10000, varXi)) { keepGoing = false; break; }
            if (!askInt("Var(Y) x 100", 0, 10000, varYi)) { keepGoing = false; break; }
            if (!askInt("Cov(X,Y) x 100 (may be negative)", -10000, 10000, covi)) {
                keepGoing = false;
                break;
            }
            const double vX = varXi / 100.0, vY = varYi / 100.0, c = covi / 100.0;
            const double cap = std::sqrt(vX * vY);

            std::cout << "\n      Var(X) = " << vX << ", Var(Y) = " << vY
                      << ", Cov = " << c << "\n";

            if (std::fabs(c) > cap + 1e-9) {
                note("Those numbers are impossible.");
                std::cout << "      |Cov| can never exceed sigmaX sigmaY = " << cap
                          << ",\n      because that would put rho outside [-1, 1].\n";
                break;
            }

            std::cout << "\n        STEP 1  the correlation they imply\n";
            showV("rho = Cov / (sigmaX sigmaY)", cap > 0 ? c / cap : 0.0);

            std::cout << "\n        STEP 2  Var(X+Y) = varX + varY + 2 Cov\n";
            showV("varX + varY", vX + vY);
            showV("2 Cov", 2.0 * c);
            std::cout << "\n";
            showV("ANSWER  Var(X+Y)", vX + vY + 2.0 * c);
            showV("        SD(X+Y)", std::sqrt(std::max(0.0, vX + vY + 2.0 * c)));

            std::cout << "\n      If they were independent it would be " << vX + vY
                      << ".\n";
            if (c > 0) std::cout << "      Positive covariance, so the real risk is HIGHER.\n";
            else if (c < 0) std::cout << "      Negative covariance, so the real risk is LOWER.\n";
            else std::cout << "      Cov is zero, so here they happen to agree.\n";
            break;
        }
        case 3: {
            int volPct = 0, targetPct = 0;
            if (!askInt("volatility of each stock, in %", 1, 100, volPct)) { keepGoing = false; break; }
            if (!askInt("target portfolio volatility, in %", 1, 100, targetPct)) { keepGoing = false; break; }

            const double s = volPct / 100.0, target = targetPct / 100.0;
            // sigma_p = sigma sqrt((1+rho)/2)  ->  rho = 2 (target/sigma)^2 - 1
            const double ratio = target / s;
            const double needed = 2.0 * ratio * ratio - 1.0;

            std::cout << "\n      Two stocks at " << volPct << "% each, 50/50, wanting "
                      << targetPct << "%.\n";
            std::cout << "\n        STEP 1  rearrange Part 8's formula\n"
                         "                sigma_p = sigma sqrt((1+rho)/2)\n"
                         "                rho = 2 (sigma_p / sigma)^2 - 1\n";
            showV("sigma_p / sigma", ratio);
            std::cout << "\n        STEP 2  solve\n\n";
            showV("ANSWER  rho you would need", needed);

            if (needed < -1.0)
                std::cout << "\n      Below -1, so it is IMPOSSIBLE with two stocks.\n"
                             "      Even perfect opposites only get you to zero, and\n"
                             "      you asked for less risk than that.\n";
            else if (needed > 1.0)
                std::cout << "\n      Above 1, so your target is easy - any two stocks\n"
                             "      at this volatility already beat it.\n";
            else
                std::cout << "\n      That is achievable in principle. Whether any two\n"
                             "      real stocks hold that rho when it matters is the\n"
                             "      question Part 8 ends on.\n";
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
    part(11, "WHAT YOU LEARNED", "1.17 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      COVARIANCE - variance with a second variable\n"
        "          Cov(X,Y) = E[(X - muX)(Y - muY)]\n"
        "          Cov(X,X) = Var(X), so nothing new was invented\n"
        "          shortcut:  Cov = E[XY] - E[X]E[Y]\n"
        "          the SIGN is the message: same side, or opposite sides\n"
        "\n"
        "      CORRELATION - covariance with the units divided out\n"
        "                    Cov(X,Y)\n"
        "          rho =  ---------------      always in [-1, +1]\n"
        "                 sigmaX sigmaY\n"
        "          unitless, so it compares across completely\n"
        "          different problems - which raw covariance cannot\n"
        "\n"
        "      THE ARROW ONLY POINTS ONE WAY\n"
        "          independent  =>  rho = 0        always\n"
        "          rho = 0      =>  independent    NEVER assume this\n"
        "          Y = X^2 is perfect dependence with rho exactly 0,\n"
        "          because rho only ever looks for a STRAIGHT LINE\n"
        "\n"
        "      THE VARIANCE OF A SUM - what 1.12 was missing\n"
        "          Var(X+Y) = Var(X) + Var(Y) + 2 Cov(X,Y)\n"
        "          variances only add when the covariance is zero\n"
        "\n"
        "      DIVERSIFICATION IS A STATEMENT ABOUT rho\n"
        "          sigma_p = sigma sqrt((1 + rho)/2) for a 50/50 pair\n"
        "          rho = +1  no benefit at all, one bet twice\n"
        "          rho =  0  risk falls by 29%\n"
        "          rho = -1  risk falls to zero\n"
        "          owning MANY things is not diversification;\n"
        "          owning things with low rho is\n"
        "\n"
        "      AND THE WARNING\n"
        "          rho is measured over a window, and it is not stable.\n"
        "          In a crash correlations run toward 1, so the\n"
        "          protection is thinnest exactly when it is needed.\n"
        "          The formula is honest - check which rho you fed it.\n"
        "\n"
        "      COMING NEXT\n"
        "          the central limit theorem - why the normal from 1.14\n"
        "          shows up for sums of almost ANY random variable, not\n"
        "          just the binomial.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
