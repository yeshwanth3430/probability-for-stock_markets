// ============================================================================
//  1.15 - THE STANDARD UNIT NORMAL AND PROBABILITY COMPUTATIONS
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.15 The Standard Unit Normal and Probability Computations.cpp" -o p115
//      ./p115
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.12  random variables, E[X], Var(X), and linearity of expectation
//  1.13  Bernoulli and binomial, mean np and variance np(1-p)
//  1.14  the normal distribution, and the binomial's limit
//  1.15  reducing every normal to ONE normal      <- you are here
//
//  1.14 left a practical problem unsolved. The normal density cannot be
//  integrated in closed form - there is no elementary antiderivative for
//  exp(-x^2/2), and this is a theorem, not a gap in anyone's cleverness.
//
//  So every normal probability has to come from a numerical table or a
//  numerical routine. And there are infinitely many normals, one for each
//  pair (mu, sigma). You cannot tabulate infinitely many curves.
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA
//  ---------------------------------------------------------------------------
//  You do not have to. Every normal is the SAME CURVE, shifted and
//  stretched. Undo the shift and the stretch and you are back to one
//  canonical curve - the STANDARD NORMAL, with mean 0 and variance 1:
//
//                X - mu
//      Z  =  ------------
//                sigma
//
//  In words: how many standard deviations above the mean is X? That
//  number is the Z-SCORE, it has no units, and it is comparable across
//  completely different quantities - test scores, heights, response
//  times, portfolio returns.
//
//  One table. Every normal problem. That is the whole lesson.
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
//  THE STANDARD NORMAL
// ============================================================================
const double TWO_PI = 6.283185307179586;

// phi(z) - the standard normal DENSITY. Note it is symmetric: phi(-z) = phi(z).
double phi(double z) { return std::exp(-0.5 * z * z) / std::sqrt(TWO_PI); }

// Phi(z) = P(Z <= z) - the standard normal CDF, the function every z-table
// is a printout of. erfc keeps its accuracy deep in the left tail, where
// erf would lose every digit to cancellation.
double Phi(double z) { return 0.5 * std::erfc(-z / std::sqrt(2.0)); }

// The general normal, expressed through the standard one - which IS the
// point of the lesson, so it is worth writing the code this way round.
double normalCdf(double x, double mu, double sigma) { return Phi((x - mu) / sigma); }
double normalPdf(double x, double mu, double sigma) {
    return phi((x - mu) / sigma) / sigma;      // the 1/sigma is the stretch
}
double normalBetween(double a, double b, double mu, double sigma) {
    return normalCdf(b, mu, sigma) - normalCdf(a, mu, sigma);
}

// THE INVERSE - given a probability, which z has it to the left?
//
// Phi has no closed form and neither does its inverse, so this is Acklam's
// rational approximation followed by one Halley step, which lands within
// about 1e-15. Part 7 is what it is for.
double PhiInverse(double p) {
    if (p <= 0.0) return -INFINITY;
    if (p >= 1.0) return  INFINITY;
    static const double a[] = {-3.969683028665376e+01,  2.209460984245205e+02,
                               -2.759285104469687e+02,  1.383577518672690e+02,
                               -3.066479806614716e+01,  2.506628277459239e+00};
    static const double b[] = {-5.447609879822406e+01,  1.615858368580409e+02,
                               -1.556989798598866e+02,  6.680131188771972e+01,
                               -1.328068155288572e+01};
    static const double c[] = {-7.784894002430293e-03, -3.223964580411365e-01,
                               -2.400758277161838e+00, -2.549732539343734e+00,
                                4.374664141464968e+00,  2.938163982698783e+00};
    static const double d[] = { 7.784695709041462e-03,  3.224671290700398e-01,
                                2.445134137142996e+00,  3.754408661907416e+00};
    const double low = 0.02425;
    double x;
    if (p < low) {
        const double q = std::sqrt(-2.0 * std::log(p));
        x = (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5])
          / ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1.0);
    } else if (p <= 1.0 - low) {
        const double q = p - 0.5, r = q * q;
        x = (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5]) * q
          / (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1.0);
    } else {
        const double q = std::sqrt(-2.0 * std::log(1.0 - p));
        x = -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5])
           / ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1.0);
    }
    const double e = Phi(x) - p;                       // one Halley refinement
    const double u = e * std::sqrt(TWO_PI) * std::exp(0.5 * x * x);
    return x - u / (1.0 + 0.5 * x * u);
}

// Un-standardising: given a z, which x does it correspond to?
double fromZ(double z, double mu, double sigma) { return mu + z * sigma; }

// The exact binomial, for Part 9's round trip back to 1.14.
double binomialPmf(int n, int k, double p) {
    if (k < 0 || k > n) return 0.0;
    return std::exp(std::lgamma(n + 1.0) - std::lgamma(k + 1.0)
                    - std::lgamma(n - k + 1.0)
                    + k * std::log(p) + (n - k) * std::log1p(-p));
}
double binomialBetween(int n, int lo, int hi, double p) {
    double t = 0.0;
    for (int k = std::max(0, lo); k <= std::min(n, hi); ++k) t += binomialPmf(n, k, p);
    return t;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.14)
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

// The z-table, printed the way the back of a textbook prints it:
// row = z to one decimal, column = the second decimal, cell = Phi(z).
void showZTable(double from, double to) {
    // Cells are printed as ".9713" rather than "0.9713" - the leading zero
    // carries no information and costs 10 columns across the row, which is
    // the difference between fitting the 70-column page and not.
    auto cell = [](double v) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(4) << v;
        std::string t = ss.str();
        if (t.rfind("0.", 0) == 0) t = t.substr(1);
        return t;
    };
    std::cout << "\n       z";
    for (int c = 0; c < 10; ++c) {
        std::ostringstream h;
        h << "." << std::setw(2) << std::setfill('0') << c;
        std::cout << std::setw(6) << std::setfill(' ') << h.str();
    }
    std::cout << "\n    " << repeat("-", 64) << "\n";
    for (double z = from; z < to + 1e-9; z += 0.1) {
        std::cout << "    " << std::setw(4) << std::fixed << std::setprecision(1) << z;
        for (int c = 0; c < 10; ++c) std::cout << std::setw(6) << cell(Phi(z + c / 100.0));
        std::cout << "\n";
    }
    std::cout << std::setprecision(4);
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ========================================================================
    part(1, "THE PROBLEM", "infinitely many curves, one book");
    // ========================================================================
    question("1.14 gave us the normal. How do you actually compute");
    std::cout << "      a probability from it?\n";

    note("Here is the obstacle, and it is permanent. Probability");
    std::cout << "      is AREA under the density (1.12 Part 9), so you need\n"
                 "      the integral of\n"
        "\n"
        "         exp( -x^2 / 2 )\n"
        "\n"
        "      and that function has NO elementary antiderivative. Not\n"
        "      'nobody has found one' - it was proved in the 1830s that\n"
        "      none exists. Every normal probability is a numerical\n"
        "      answer, always.\n";

    note("So you need a table. But there is one normal for every");
    std::cout << "      pair (mu, sigma), which is infinitely many curves, and\n"
                 "      no book has infinitely many pages:\n";
    std::cout << "\n        a quantity              mu        sigma\n"
              << "        " << repeat("-", 58) << "\n"
              << "        adult heights (cm)     " << std::setw(8) << 170.0
              << std::setw(11) << 10.0 << "\n"
              << "        IQ scores              " << std::setw(8) << 100.0
              << std::setw(11) << 15.0 << "\n"
              << "        daily returns (%)      " << std::setw(8) << 0.03
              << std::setw(11) << 1.20 << "\n"
              << "        part diameter (mm)     " << std::setw(8) << 25.0
              << std::setw(11) << 0.05 << "\n";

    note("Four problems, four different curves, and no shared");
    std::cout << "      table between them. Unless the four curves are secretly\n"
                 "      the same curve - which is exactly what Part 2 shows.\n";

    // ========================================================================
    part(2, "THE Z-SCORE", "shift, stretch, done");
    // ========================================================================
    question("How different ARE two normals, really?");

    note("Barely. Every normal is the same shape, moved and");
    std::cout << "      rescaled. mu slides it left or right; sigma stretches or\n"
                 "      squashes it. Undo both and there is only one curve left:\n"
        "\n"
        "                X - mu\n"
        "         Z  =  --------\n"
        "                 sigma\n"
        "\n"
        "         subtract mu    -> now centred at 0\n"
        "         divide by sigma-> now measured in sigmas, not inches\n";

    note("Read the result out loud: Z is HOW MANY STANDARD");
    std::cout << "      DEVIATIONS ABOVE THE MEAN X is. Negative means below.\n"
                 "      That number has no units at all, which is what makes it\n"
                 "      comparable across quantities that share nothing:\n";

    {
        struct Case { std::string what; double x, mu, sigma; };
        const std::vector<Case> cases = {
            {"a 190cm adult",        190.0, 170.0, 10.0},
            {"an IQ of 130",         130.0, 100.0, 15.0},
            {"a 3.5% up day",          3.5,   0.03,  1.20},
            {"a 25.08mm part",        25.08, 25.0,  0.05},
        };
        std::cout << "\n        case                    x        mu     sigma       z\n"
                  << "        " << repeat("-", 62) << "\n";
        for (const auto& c : cases)
            std::cout << "        " << std::left << std::setw(20) << c.what << std::right
                      << std::setw(9) << c.x << std::setw(9) << c.mu
                      << std::setw(9) << c.sigma << std::setw(9)
                      << (c.x - c.mu) / c.sigma << "\n";
        note("Now they sit on one scale and can be ranked. The most");
        std::cout << "      extreme entry is the 3.5% market day at 2.89 sigma; the\n"
                     "      least is the out-of-spec part at 1.60. Nothing in the\n"
                     "      raw numbers tells you that - 3.5 and 25.08 are not\n"
                     "      comparable quantities - and the z-score does, in one\n"
                     "      column.\n";

        note("The tall adult and the IQ of 130 both land on exactly");
        std::cout << "      2.00, which is the idea working: 190cm is as unusual\n"
                     "      among heights as 130 is among IQs, and now you can say\n"
                     "      so precisely rather than vaguely.\n";
    }

    // ========================================================================
    part(3, "WHY Z HAS MEAN 0 AND SD 1", "1.12's linearity, cashed in");
    // ========================================================================
    question("Is Z really standard, or is that just a name?");

    note("It is a theorem, and 1.12 already gave you both halves.");
    std::cout << "      For the mean, expectation is linear:\n"
        "\n"
        "         E[Z]  =  E[ (X - mu)/sigma ]\n"
        "               =  ( E[X] - mu ) / sigma\n"
        "               =  ( mu - mu ) / sigma\n"
        "               =  0\n"
        "\n"
        "      For the variance, constants come out SQUARED - which is\n"
        "      1.12 Part 8's warning that E[g(X)] is not g(E[X]), in its\n"
        "      most useful form:\n"
        "\n"
        "         Var(Z)  =  Var( (X - mu)/sigma )\n"
        "                 =  Var(X) / sigma^2        shift changes nothing\n"
        "                 =  sigma^2 / sigma^2\n"
        "                 =  1\n";

    note("Note the asymmetry there. Subtracting mu moved the mean");
    std::cout << "      and left the variance alone; dividing by sigma changed\n"
                 "      both. A shift cannot change a spread - every point moves\n"
                 "      together - and that is worth carrying around.\n";

    // Check it numerically on an arbitrary normal.
    {
        const double mu = 170.0, sigma = 10.0, step = 0.002;
        double mean = 0.0, meanSq = 0.0;
        for (double x = mu - 8 * sigma; x <= mu + 8 * sigma; x += step) {
            const double w = normalPdf(x, mu, sigma) * step;
            const double z = (x - mu) / sigma;
            mean += z * w;
            meanSq += z * z * w;
        }
        verifyClose("E[Z] = 0",   mean, 0.0, 1e-6, "integr. ");
        verifyClose("Var(Z) = 1", meanSq - mean * mean, 1.0, 1e-6, "integr. ");
    }

    answer("Any normal, standardised, becomes THE standard normal. One curve.");

    // ========================================================================
    part(4, "THE Z-TABLE", "what is actually printed in the back of the book");
    // ========================================================================
    question("So what does the table contain?");

    note("Phi(z) = P(Z <= z), the area to the LEFT of z under the");
    std::cout << "      standard curve. Row gives z to one decimal, column gives\n"
                 "      the second decimal, cell gives the area:\n";

    showZTable(0.0, 1.9);

    note("Read Phi(1.96) - row 1.9, column 0.06 - and you get");
    std::cout << "      " << Phi(1.96) << ", the number every statistics course\n"
                 "      quotes. Here is the rest of the table:\n";

    showZTable(2.0, 3.4);

    verifyClose("Phi(0) = 0.5",     Phi(0.0),  0.5,    1e-12, "by symm.");
    verifyClose("Phi(1.96)",        Phi(1.96), 0.9750, 1e-4,  "the book");
    verifyClose("Phi(1.645)",       Phi(1.645), 0.9500, 1e-4, "the book");
    verifyClose("Phi(2.576)",       Phi(2.576), 0.9950, 1e-4, "the book");

    note("This program computes Phi directly, so it never needs");
    std::cout << "      the table - but the table is what the formulas in every\n"
                 "      textbook assume you are holding, and being able to read\n"
                 "      one is still worth ten minutes.\n";

    // ========================================================================
    part(5, "COMPUTING PROBABILITIES", "the four questions, all the same one");
    // ========================================================================
    question("Heights are Normal(170, 10). What fraction of people");
    std::cout << "      are between 165cm and 185cm?\n";

    {
        const double mu = 170.0, sigma = 10.0;
        note("Three steps, every time. Standardise, look up, subtract:");
        std::cout << "\n"
            "         STEP 1  turn both ends into z-scores\n"
            "                 z1 = (165 - 170)/10 = " << (165.0 - mu) / sigma << "\n"
            "                 z2 = (185 - 170)/10 = " << (185.0 - mu) / sigma << "\n"
            "\n"
            "         STEP 2  look each one up\n"
            "                 Phi(" << (165.0 - mu) / sigma << ") = "
                  << Phi((165.0 - mu) / sigma) << "\n"
            "                 Phi(" << (185.0 - mu) / sigma << ") = "
                  << Phi((185.0 - mu) / sigma) << "\n"
            "\n"
            "         STEP 3  subtract - it is 1.12's F(b) - F(a)\n"
            "                 " << Phi(1.5) << " - " << Phi(-0.5) << " = "
                  << Phi(1.5) - Phi(-0.5) << "\n";
        std::cout << "\n";
        showP("P(165 <= height <= 185)", normalBetween(165, 185, mu, sigma));

        note("Every other question is the same three steps with the");
        std::cout << "      last one adjusted:\n";
        std::cout << "\n";
        showP("P(height <= 165)",      normalCdf(165, mu, sigma));
        showP("P(height >= 185)",      1.0 - normalCdf(185, mu, sigma));
        showP("P(165 <= h <= 185)",    normalBetween(165, 185, mu, sigma));
        showP("P(height >= 190)",      1.0 - normalCdf(190, mu, sigma));
        showP("P(height = 175)",       0.0);

        note("That last line is not a bug. X is continuous, so every");
        std::cout << "      single value has probability exactly zero (1.12 Part 9)\n"
                     "      - which also means <= and < give identical answers here,\n"
                     "      and you can stop worrying about which one you wrote.\n";

        verifyClose("the three pieces total 1",
                    normalCdf(165, mu, sigma) + normalBetween(165, 185, mu, sigma)
                        + (1.0 - normalCdf(185, mu, sigma)),
                    1.0, 1e-12, "must be");
    }

    // ========================================================================
    part(6, "SYMMETRY", "why the table only lists positive z");
    // ========================================================================
    question("Old tables stop at z = 0. How do you look up z = -1.5?");

    note("You use the symmetry, and it is the one identity worth");
    std::cout << "      memorising in this lesson:\n"
        "\n"
        "         Phi(-z)  =  1 - Phi(z)\n"
        "\n"
        "      The curve is a mirror image about 0, so the area left of\n"
        "      -z equals the area right of +z.\n";

    std::cout << "\n        z       Phi(z)     Phi(-z)   1 - Phi(z)\n"
              << "        " << repeat("-", 58) << "\n";
    double worst = 0.0;
    for (double z : {0.5, 1.0, 1.5, 1.96, 2.5, 3.0}) {
        worst = std::max(worst, std::fabs(Phi(-z) - (1.0 - Phi(z))));
        std::cout << "        " << std::setw(5) << z << std::setw(11) << Phi(z)
                  << std::setw(11) << Phi(-z) << std::setw(13) << 1.0 - Phi(z) << "\n";
    }
    verifyClose("Phi(-z) = 1 - Phi(z)", worst, 0.0, 1e-12, "gap of  ");

    note("Three consequences worth having at your fingertips:");
    std::cout << "\n"
        "         Phi(0) = 0.5                    half the area each side\n"
        "         P(|Z| <= z) = 2 Phi(z) - 1      the two-sided answer\n"
        "         P(|Z| >  z) = 2 (1 - Phi(z))    both tails together\n";
    std::cout << "\n";
    showP("P(|Z| <= 1.96), two-sided", 2 * Phi(1.96) - 1);
    showP("P(|Z| >  1.96), both tails", 2 * (1 - Phi(1.96)));
    verifyClose("two-sided at 1.96", 2 * Phi(1.96) - 1, 0.95, 1e-3, "the 95% ");

    note("That last check is where 1.96 comes from, and it is why");
    std::cout << "      you see it everywhere. It is simply the z that leaves\n"
                 "      2.5% in each tail.\n";

    // ========================================================================
    part(7, "GOING BACKWARDS", "from a probability to a cutoff");
    // ========================================================================
    // The inverse direction, which is what almost every practical question
    // actually needs.
    question("Turn it round. What height is the 90th percentile?");

    note("Now you know the AREA and want the z - the table read");
    std::cout << "      right to left. That is the inverse, written Phi^-1(p) or\n"
                 "      z_p, and it is the quantile function:\n"
        "\n"
        "         Phi(z) = 0.90   ->   z = " << PhiInverse(0.90) << "\n"
        "\n"
        "      then un-standardise to get back to centimetres:\n"
        "\n"
        "         x  =  mu + z sigma  =  170 + " << PhiInverse(0.90)
              << " x 10  =  " << fromZ(PhiInverse(0.90), 170, 10) << "\n";
    std::cout << "\n";
    showV("90th percentile height (cm)", fromZ(PhiInverse(0.90), 170, 10));
    verifyClose("inverse then forward", Phi(PhiInverse(0.90)), 0.90, 1e-12, "round-trip");

    note("The z values you will meet again and again, because they");
    std::cout << "      are the ones confidence intervals and hypothesis tests\n"
                 "      are built from:\n";
    std::cout << "\n        confidence   one-tailed z   two-tailed z\n"
              << "        " << repeat("-", 58) << "\n";
    for (double conf : {0.80, 0.90, 0.95, 0.99, 0.999}) {
        std::cout << "        " << std::setw(9) << conf
                  << std::setw(14) << PhiInverse(conf)
                  << std::setw(15) << PhiInverse(1.0 - (1.0 - conf) / 2) << "\n";
    }
    verifyClose("two-tailed 95% is 1.96", PhiInverse(0.975), 1.96, 1e-3, "the book");
    verifyClose("one-tailed 95% is 1.645", PhiInverse(0.95), 1.645, 1e-3, "the book");
    verifyClose("two-tailed 99% is 2.576", PhiInverse(0.995), 2.576, 1e-3, "the book");

    answer("1.645, 1.96 and 2.576 are worth knowing without looking them up.");

    note("Watch the one-tailed versus two-tailed column carefully -");
    std::cout << "      it is the commonest slip in applied work. For 95%:\n"
        "\n"
        "         ONE-TAILED   5% all in one tail       z = 1.645\n"
        "                      'is it BIGGER than...'\n"
        "\n"
        "         TWO-TAILED   2.5% in each tail        z = 1.960\n"
        "                      'is it DIFFERENT from...'\n"
        "\n"
        "      Using 1.645 where you needed 1.96 makes your interval\n"
        "      too narrow and your result too significant. It is the\n"
        "      same 'which question am I answering' error as 1.7's\n"
        "      P(A|B) versus P(B|A), in a different costume.\n";

    // ========================================================================
    part(8, "A REAL PROBLEM", "a machine that has to hit a tolerance");
    // ========================================================================
    question("A lathe turns parts to a mean 25.00mm with sigma");
    std::cout << "      0.05mm. Spec is 25.00 +/- 0.10mm. What is the scrap rate?\n";

    {
        const double mu = 25.00, sigma = 0.05;
        const double lo = 24.90, hi = 25.10;
        const double zLo = (lo - mu) / sigma, zHi = (hi - mu) / sigma;

        std::cout << "\n"
            "         z at the lower limit   (24.90 - 25.00)/0.05  =  " << zLo << "\n"
            "         z at the upper limit   (25.10 - 25.00)/0.05  =  " << zHi << "\n";
        std::cout << "\n";
        showP("P(within spec)",  normalBetween(lo, hi, mu, sigma));
        showP("P(scrap)",        1.0 - normalBetween(lo, hi, mu, sigma));
        std::cout << "\n      That is " << (1.0 - normalBetween(lo, hi, mu, sigma)) * 1e6
                  << " defective parts per million.\n";

        note("Now the two ways to improve it, and they are not");
        std::cout << "      equally easy. First: tighten the machine, reducing\n"
                     "      sigma while the spec stays put:\n";
        std::cout << "\n        sigma     spec in sigma   P(scrap)      per million\n"
                  << "        " << repeat("-", 62) << "\n";
        for (double s2 : {0.05, 0.04, 0.033, 0.025, 0.02, 0.0167}) {
            const double scrap = 1.0 - normalBetween(lo, hi, mu, s2);
            std::cout << "        " << std::setw(6) << s2 << std::setw(14) << 0.10 / s2
                      << std::setw(13) << std::scientific << std::setprecision(2) << scrap
                      << std::setw(15) << scrap * 1e6
                      << std::fixed << std::setprecision(4) << "\n";
        }
        note("The last row is 6 sigma of tolerance - the origin of");
        std::cout << "      the manufacturing slogan, and it means about 2 parts\n"
                     "      per billion rather than any kind of metaphor.\n";

        note("Second, and much worse: let the machine DRIFT off");
        std::cout << "      centre while sigma stays at 0.05. This is what actually\n"
                     "      happens as a tool wears:\n";
        std::cout << "\n        mean      offset in sigma   P(scrap)     per million\n"
                  << "        " << repeat("-", 62) << "\n";
        for (double m2 : {25.000, 25.010, 25.025, 25.040, 25.050}) {
            const double scrap = 1.0 - normalBetween(lo, hi, m2, sigma);
            std::cout << "        " << std::setw(7) << m2 << std::setw(15)
                      << (m2 - mu) / sigma << std::setw(14)
                      << std::scientific << std::setprecision(2) << scrap
                      << std::setw(14) << scrap * 1e6
                      << std::fixed << std::setprecision(4) << "\n";
        }

        answer("A one-sigma drift multiplies the scrap rate by about 30.");

        note("Which is why process control charts watch the MEAN as");
        std::cout << "      closely as the spread. A drift you could barely see on\n"
                     "      a histogram is the difference between 46 defects per\n"
                     "      million and 1,350.\n";

        // The inverse direction, which is the design question.
        note("And the design question runs backwards. If you are");
        std::cout << "      allowed 100 defects per million, how tight must sigma\n"
                     "      be? Solve Phi(0.10/sigma) - Phi(-0.10/sigma) = 0.9999:\n";
        const double zNeeded = PhiInverse(1.0 - 0.0001 / 2);
        const double sigmaNeeded = 0.10 / zNeeded;
        std::cout << "\n"
            "         z needed      " << zNeeded << "\n"
            "         sigma needed  0.10 / " << zNeeded << "  =  " << sigmaNeeded << "\n";
        verifyClose("that sigma hits 100 ppm",
                    (1.0 - normalBetween(lo, hi, mu, sigmaNeeded)) * 1e6, 100.0,
                    1.0, "target  ");
    }

    // ========================================================================
    part(9, "BACK TO THE BINOMIAL", "1.14 and 1.15, working together");
    // ========================================================================
    question("Put the whole chapter together. 1,000 coin flips - what");
    std::cout << "      is P(at least 530 heads)?\n";

    {
        const int n = 1000; const double p = 0.5;
        const double mu = n * p, sigma = std::sqrt(n * p * (1 - p));

        note("Four steps, and each one is a different lesson:");
        std::cout << "\n"
            "         1  the mean and variance          1.13\n"
            "            mu = np = " << mu << ",  sigma = sqrt(np(1-p)) = " << sigma << "\n"
            "\n"
            "         2  approximate by a normal        1.14\n"
            "            np = " << mu << " and n(1-p) = " << n * (1 - p)
                  << ", both well over 10\n"
            "\n"
            "         3  apply the continuity correction 1.14\n"
            "            P(X >= 530) uses the boundary at 529.5\n"
            "\n"
            "         4  standardise and look up         1.15\n"
            "            z = (529.5 - " << mu << ") / " << sigma << " = "
                  << (529.5 - mu) / sigma << "\n";

        const double approx = 1.0 - Phi((529.5 - mu) / sigma);
        const double exact = binomialBetween(n, 530, n, p);
        std::cout << "\n";
        showP("P(X >= 530), normal via z", approx);
        showP("P(X >= 530), exact binomial", exact);
        verifyClose("the whole chapter", approx, exact, 1e-3, "exact   ");

        answer("Agreement to three decimals, using a table and a subtraction.");

        note("And the question a sceptic actually asks: is this coin");
        std::cout << "      fair? 530 heads is " << (530 - mu) / sigma
                  << " sigma above what a fair\n      coin gives. The two-sided p-value is:\n";
        std::cout << "\n";
        showP("P(|X - 500| >= 30) if fair", 2 * (1.0 - Phi((529.5 - mu) / sigma)));
        note("Under 5%, so at the conventional threshold you would");
        std::cout << "      call the coin suspect. That sentence is the whole of\n"
                     "      hypothesis testing in miniature, and every piece of it\n"
                     "      came from this chapter.\n";
    }

    // ========================================================================
    part(10, "DO NOT TRUST ME, SIMULATE IT", "400,000 draws");
    // ========================================================================
    question("Draw 400,000 heights from Normal(170, 10), standardise");
    std::cout << "      every one, and see whether the z's behave.\n";

    {
        std::mt19937 rng(1729);
        std::normal_distribution<double> heights(170.0, 10.0);
        const int DRAWS = 400000;

        double zSum = 0.0, zSq = 0.0;
        int in1 = 0, in2 = 0, in3 = 0, below165 = 0, between = 0;
        for (int t = 0; t < DRAWS; ++t) {
            const double x = heights(rng);
            const double z = (x - 170.0) / 10.0;
            zSum += z; zSq += z * z;
            if (std::fabs(z) <= 1.0) ++in1;
            if (std::fabs(z) <= 2.0) ++in2;
            if (std::fabs(z) <= 3.0) ++in3;
            if (x <= 165.0) ++below165;
            if (x >= 165.0 && x <= 185.0) ++between;
        }
        const double zMean = zSum / DRAWS;
        const double zVar = zSq / DRAWS - zMean * zMean;

        std::cout << "\n";
        verifyClose("E[Z] = 0",   0.0, zMean, 1e-2, "simulated");
        verifyClose("Var(Z) = 1", 1.0, zVar,  1e-2, "simulated");
        verifyClose("within 1 sigma", 2 * Phi(1) - 1,
                    static_cast<double>(in1) / DRAWS, 5e-3, "simulated");
        verifyClose("within 2 sigma", 2 * Phi(2) - 1,
                    static_cast<double>(in2) / DRAWS, 5e-3, "simulated");
        verifyClose("within 3 sigma", 2 * Phi(3) - 1,
                    static_cast<double>(in3) / DRAWS, 5e-3, "simulated");
        verifyClose("P(height <= 165)", Phi(-0.5),
                    static_cast<double>(below165) / DRAWS, 5e-3, "simulated");
        verifyClose("P(165 <= h <= 185)", Phi(1.5) - Phi(-0.5),
                    static_cast<double>(between) / DRAWS, 5e-3, "simulated");

        answer("Standardising really does produce a mean of 0 and a variance of 1.");

        note("Unlike 1.14's binomial, nothing here needed a");
        std::cout << "      continuity correction - heights are already continuous,\n"
                     "      so the smooth answers are the right ones and they land\n"
                     "      on the nose.\n";
    }

    // ========================================================================
    part(11, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Compute your own normal probabilities.\n"
                 "\n"
                 "        1   a probability - mu, sigma, and a range\n"
                 "        2   a percentile  - go backwards, area to cutoff\n"
                 "        3   a z-table     - print any slice of it\n"
                 "        4   binomial      - 1.14 and 1.15 end to end\n"
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
                         "      1 probability   2 percentile   3 z-table   4 binomial   5 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 5, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            int mu = 0, sigma = 0, lo = 0, hi = 0;
            if (!askNumber("Mean mu", -100000, 100000, mu)) { keepGoing = false; break; }
            if (!askNumber("Standard deviation sigma", 1, 100000, sigma)) { keepGoing = false; break; }
            if (!askNumber("Lower end of the range", mu - 6 * sigma, mu + 6 * sigma, lo)) {
                keepGoing = false; break;
            }
            if (!askNumber("Upper end of the range", lo, mu + 6 * sigma, hi)) {
                keepGoing = false; break;
            }
            const double zLo = (lo - static_cast<double>(mu)) / sigma;
            const double zHi = (hi - static_cast<double>(mu)) / sigma;
            std::cout << "\n        STEP 1  standardise both ends\n"
                         "                z1 = (" << lo << " - " << mu << ")/" << sigma
                      << " = " << zLo << "\n"
                         "                z2 = (" << hi << " - " << mu << ")/" << sigma
                      << " = " << zHi << "\n";
            std::cout << "\n        STEP 2  look them up\n"
                         "                Phi(" << zLo << ") = " << Phi(zLo) << "\n"
                         "                Phi(" << zHi << ") = " << Phi(zHi) << "\n";
            std::cout << "\n        STEP 3  subtract\n\n";
            showP("P(lo <= X <= hi)", Phi(zHi) - Phi(zLo));
            showP("P(X <= lo)",       Phi(zLo));
            showP("P(X >= hi)",       1.0 - Phi(zHi));
            verifyClose("the three total 1", Phi(zLo) + (Phi(zHi) - Phi(zLo))
                        + (1.0 - Phi(zHi)), 1.0, 1e-12, "must be");
            break;
        }
        case 2: {
            int mu = 0, sigma = 0, pctile = 0;
            if (!askNumber("Mean mu", -100000, 100000, mu)) { keepGoing = false; break; }
            if (!askNumber("Standard deviation sigma", 1, 100000, sigma)) { keepGoing = false; break; }
            if (!askNumber("Which percentile", 1, 99, pctile)) { keepGoing = false; break; }
            const double p = pctile / 100.0;
            const double z = PhiInverse(p);
            std::cout << "\n        STEP 1  find the z with that much area to its left\n"
                         "                Phi^-1(" << p << ") = " << z << "\n";
            std::cout << "\n        STEP 2  un-standardise:  x = mu + z sigma\n"
                         "                " << mu << " + " << z << " x " << sigma
                      << " = " << fromZ(z, mu, sigma) << "\n";
            std::cout << "\n";
            showV("the " + std::to_string(pctile) + "th percentile", fromZ(z, mu, sigma));
            showV("its z-score", z);
            verifyClose("check by going forward", Phi(z), p, 1e-9, "asked for");
            std::cout << "\n      A symmetric interval holding " << pctile << "% of the values:\n";
            const double zTwo = PhiInverse(1.0 - (1.0 - p) / 2);
            std::cout << "        " << fromZ(-zTwo, mu, sigma) << "  to  "
                      << fromZ(zTwo, mu, sigma) << "   (z = +/-" << zTwo << ")\n";
            break;
        }
        case 3: {
            int from = 0, to = 0;
            if (!askNumber("Table from z = (tenths, so 0 means 0.0)", -34, 34, from)) {
                keepGoing = false; break;
            }
            if (!askNumber("            to z = (tenths)", from, std::min(34, from + 20), to)) {
                keepGoing = false; break;
            }
            showZTable(from / 10.0, to / 10.0);
            note("Row is z to one decimal, column is the second decimal.");
            std::cout << "      Each cell is Phi(z) = P(Z <= z), the area to the LEFT.\n"
                         "      For negative z you can also use Phi(-z) = 1 - Phi(z).\n";
            break;
        }
        case 4: {
            int n = 0, pct = 0, k = 0;
            if (!askNumber("Number of trials n", 20, 100000, n)) { keepGoing = false; break; }
            if (!askNumber("Success chance p, in %", 1, 99, pct)) { keepGoing = false; break; }
            if (!askNumber("Threshold k, for P(X >= k)", 0, n, k)) { keepGoing = false; break; }
            const double p = pct / 100.0;
            const double mu = n * p, sigma = std::sqrt(n * p * (1 - p));
            std::cout << "\n        STEP 1  mean and sd                    1.13\n"
                         "                mu = " << mu << ", sigma = " << sigma << "\n";
            std::cout << "\n        STEP 2  is the normal safe here?       1.14\n"
                         "                np = " << mu << ", n(1-p) = " << n * (1 - p)
                      << "  -> "
                      << ((n * p >= 10 && n * (1 - p) >= 10) ? "yes" : "NO, be careful")
                      << "\n";
            std::cout << "\n        STEP 3  continuity correction          1.14\n"
                         "                P(X >= " << k << ") uses the boundary at "
                      << k - 0.5 << "\n";
            const double z = (k - 0.5 - mu) / sigma;
            std::cout << "\n        STEP 4  standardise and look up        1.15\n"
                         "                z = (" << k - 0.5 << " - " << mu << ")/"
                      << sigma << " = " << z << "\n";
            std::cout << "\n";
            showP("P(X >= k), via the z-table", 1.0 - Phi(z));
            if (n <= 20000) {
                const double exact = binomialBetween(n, k, n, p);
                showP("P(X >= k), exact binomial", exact);
                verifyClose("normal vs exact", 1.0 - Phi(z), exact, 5e-3, "exact   ");
            } else {
                note("(n too large to sum the exact binomial here.)");
            }
            std::cout << "\n      A 95% range for X:  " << mu - 1.96 * sigma
                      << "  to  " << mu + 1.96 * sigma << "\n";
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
    part(12, "WHAT YOU LEARNED", "1.15 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      WHY THIS LESSON EXISTS\n"
        "          exp(-x^2/2) has NO elementary antiderivative - proved,\n"
        "          not merely undiscovered - so every normal probability\n"
        "          is a numerical answer\n"
        "          and there are infinitely many normals to tabulate\n"
        "\n"
        "      THE Z-SCORE - the fix\n"
        "                X - mu\n"
        "          Z = ----------    how many sigma above the mean X is\n"
        "               sigma\n"
        "          unitless, so it compares heights with IQs with returns\n"
        "          190cm and an IQ of 130 are both exactly z = 2.00\n"
        "\n"
        "      AND Z IS STANDARD - both halves are 1.12\n"
        "          E[Z] = 0        by linearity of expectation\n"
        "          Var(Z) = 1      constants come out SQUARED\n"
        "          a shift moves the mean and cannot touch the spread\n"
        "\n"
        "      THE TABLE\n"
        "          Phi(z) = P(Z <= z), the area to the LEFT\n"
        "          Phi(0) = 0.5     Phi(1.96) = 0.975\n"
        "\n"
        "      THE THREE STEPS, EVERY TIME\n"
        "          1  standardise both ends of the range\n"
        "          2  look up Phi at each\n"
        "          3  subtract - it is 1.12's F(b) - F(a)\n"
        "          P(X = x) = 0, so <= and < give the same answer\n"
        "\n"
        "      SYMMETRY - why tables stop at zero\n"
        "          Phi(-z) = 1 - Phi(z)\n"
        "          P(|Z| <= z) = 2 Phi(z) - 1\n"
        "          P(|Z| >  z) = 2 (1 - Phi(z))\n"
        "\n"
        "      BACKWARDS - area to cutoff\n"
        "          z = Phi^-1(p), then x = mu + z sigma\n"
        "          1.645  one-tailed 95%    'is it BIGGER than'\n"
        "          1.960  two-tailed 95%    'is it DIFFERENT from'\n"
        "          2.576  two-tailed 99%\n"
        "          mixing up one- and two-tailed is 1.7's P(A|B) vs\n"
        "          P(B|A) error in a different costume\n"
        "\n"
        "      THE WHOLE CHAIN, ON ONE PROBLEM\n"
        "          1.13 gives mu = np and sigma = sqrt(np(1-p))\n"
        "          1.14 replaces the binomial with a normal, +/- 0.5\n"
        "          1.15 standardises and reads off the answer\n"
        "          1,000 flips, P(X >= 530): three decimals, no factorials\n"
        "\n"
        "      COMING NEXT\n"
        "          the Poisson distribution - the OTHER limit of the\n"
        "          binomial, for large n and tiny p, exactly the regime\n"
        "          where 1.14 told you the normal gives up.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
