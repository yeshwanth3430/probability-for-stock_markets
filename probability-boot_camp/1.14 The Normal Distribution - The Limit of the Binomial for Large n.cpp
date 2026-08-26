// ============================================================================
//  1.14 - THE NORMAL DISTRIBUTION
//         The limit of the binomial distribution for large n
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.14 The Normal Distribution - The Limit of the Binomial for Large n.cpp" -o p114
//      ./p114
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.6   the binomial - and Part 7 drew a bell without naming it
//  1.11  independence, and what it costs when it fails
//  1.12  random variables, E[X], Var(X), and density vs mass
//  1.13  binomial = sum of Bernoullis, mean np, variance np(1-p)
//  1.14  the bell, named                             <- you are here
//
//  In 1.6 we plotted Binomial(4, 0.5), then Binomial(10, 0.5), then
//  Binomial(30, 0.5), and watched a ragged little histogram turn into a
//  smooth symmetric mound. The file said "that bell is the normal
//  distribution" and moved on.
//
//  This lesson earns that sentence. It says which bell, why that one, and
//  how close the approximation actually gets.
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA
//  ---------------------------------------------------------------------------
//  A binomial with large n is very nearly a normal with the SAME mean and
//  the SAME variance:
//
//      Binomial(n, p)   ~   Normal( np,  np(1-p) )
//
//  Both of those numbers came from 1.13, and they were derived there from
//  linearity and independence. Nothing new is being smuggled in - the
//  approximation reuses the two facts you already proved.
//
//  This is the DE MOIVRE-LAPLACE theorem, from 1733, and it is the oldest
//  special case of the central limit theorem.
//
//  ---------------------------------------------------------------------------
//  WHAT MAKES IT WORTH DOING
//  ---------------------------------------------------------------------------
//  The exact binomial needs C(n,k), and for n = 1,000,000 that is a number
//  with hundreds of thousands of digits. Answering "what is P(X > 502,000)"
//  exactly means summing 498,000 such terms.
//
//  The normal answers it with two numbers and one function call, to about
//  four decimal places. That trade - exactness for tractability - is why
//  this distribution runs applied statistics.
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
//  THE NORMAL DISTRIBUTION
//
//                       1              (x - mu)^2
//      f(x)  =  --------------- exp( - ---------- )
//                sigma sqrt(2pi)         2 sigma^2
//
//  Every piece has a job, and Part 2 takes them one at a time.
// ============================================================================
const double TWO_PI = 6.283185307179586;

double normalPdf(double x, double mu, double sigma) {
    const double z = (x - mu) / sigma;
    return std::exp(-0.5 * z * z) / (sigma * std::sqrt(TWO_PI));
}

// P(X <= x).  Built on erfc rather than erf: erfc keeps its precision far
// out in the left tail, where erf would be computing 1 minus something
// very close to 1 and losing every significant digit to cancellation.
double normalCdf(double x, double mu, double sigma) {
    return 0.5 * std::erfc(-(x - mu) / (sigma * std::sqrt(2.0)));
}

// P(a <= X <= b), the only question anyone actually asks of a continuous
// distribution - because P(X = a) is zero, as 1.12 Part 9 established.
double normalBetween(double a, double b, double mu, double sigma) {
    return normalCdf(b, mu, sigma) - normalCdf(a, mu, sigma);
}

// ============================================================================
//  THE BINOMIAL, exactly - for comparison
//
//  Everything goes through logs (1.5's discipline) so that n in the
//  thousands does not overflow on the way to a perfectly ordinary answer.
// ============================================================================
double binomialPmf(int n, int k, double p) {
    if (k < 0 || k > n) return 0.0;
    if (p <= 0.0) return (k == 0) ? 1.0 : 0.0;
    if (p >= 1.0) return (k == n) ? 1.0 : 0.0;
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
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.13)
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

// A histogram of the exact binomial with the normal curve overlaid, so the
// approximation can be looked at rather than described.
void showOverlay(int n, double p, int maxBar) {
    const double mu = n * p, sigma = std::sqrt(n * p * (1 - p));
    const int lo = std::max(0, static_cast<int>(mu - 4 * sigma));
    const int hi = std::min(n, static_cast<int>(mu + 4 * sigma + 1));

    // Scale to the tallest bar rather than to a fixed constant: a skewed p
    // concentrates probability into few values, and a fixed scale would run
    // the bar clean off the 70-column page.
    double peak = 0.0;
    for (int k = lo; k <= hi; ++k)
        peak = std::max(peak, std::max(binomialPmf(n, k, p), normalPdf(k, mu, sigma)));
    const double scale = peak > 0.0 ? maxBar / peak : 0.0;

    std::cout << "\n        k      binomial     normal      picture\n"
              << "        " << repeat("-", 60) << "\n";
    for (int k = lo; k <= hi; ++k) {
        const double b = binomialPmf(n, k, p);
        const double f = normalPdf(k, mu, sigma);
        if (b < 0.0008 && f < 0.0008) continue;
        const int nb = static_cast<int>(b * scale + 0.5);
        const int nf = static_cast<int>(f * scale + 0.5);
        std::string pic(static_cast<size_t>(std::max(nb, nf)), ' ');
        for (int i = 0; i < nb && i < static_cast<int>(pic.size()); ++i)
            pic[static_cast<size_t>(i)] = '#';
        if (nf > 0 && nf <= static_cast<int>(pic.size()))
            pic[static_cast<size_t>(nf - 1)] = (nf <= nb) ? '|' : '*';
        std::cout << "        " << std::setw(4) << k << std::setw(12) << b
                  << std::setw(12) << f << "   " << pic << "\n";
    }
    std::cout << "\n        # = exact binomial      | or * = the normal curve\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ========================================================================
    part(1, "THE BELL WE ALREADY DREW", "1.6 Part 7, revisited");
    // ========================================================================
    question("What happens to a binomial as n gets large?");

    note("1.6 answered this with a picture and left the answer");
    std::cout << "      unnamed. Here is that picture again, fair coin, three\n"
                 "      values of n - but this time watch the WIDTH as well as\n"
                 "      the shape:\n";

    for (int n : {4, 16, 64}) {
        const double mu = n * 0.5, sd = std::sqrt(n * 0.25);
        std::cout << "\n        n = " << std::setw(3) << n << "   mean " << mu
                  << "   sd " << sd << "\n";
        for (int k = 0; k <= n; ++k) {
            const double b = binomialPmf(n, k, 0.5);
            if (b < 0.004) continue;
            std::cout << "        " << std::setw(4) << k << std::setw(11) << b
                      << bar(b, 110) << "\n";
        }
    }

    answer("Two things happen at once, and they pull in opposite directions.");

    note("The distribution SPREADS - the sd grows like sqrt(n) - and");
    std::cout << "      it also SMOOTHS, because each individual bar gets shorter\n"
                 "      as the probability spreads over more values. The shape\n"
                 "      that survives both is the bell.\n";

    note("That shape has a name, a formula, and a reason. This");
    std::cout << "      lesson supplies all three.\n";

    // ========================================================================
    part(2, "THE FORMULA", "every piece has a job");
    // ========================================================================
    question("What IS the normal distribution?");

    std::cout << "\n"
        "                        1               (x - mu)^2\n"
        "         f(x)  =  ---------------  exp( - ---------- )\n"
        "                   sigma sqrt(2pi)         2 sigma^2\n";

    note("It looks forbidding and it is four ideas stacked up.");
    std::cout << "      Take them from the inside out:\n"
        "\n"
        "         (x - mu)          how far from the centre\n"
        "\n"
        "         / sigma           measured in standard deviations,\n"
        "                           not in inches or dollars\n"
        "\n"
        "         squared, negated  symmetric, and falling away\n"
        "         and exponentiated fast - the tails die like e^(-x^2),\n"
        "                           which is extraordinarily quick\n"
        "\n"
        "         1/(sigma sqrt(2pi))  the normaliser: whatever it takes\n"
        "                              to make the area exactly 1\n";

    note("The exponent is the whole distribution. Everything in");
    std::cout << "      front of it is bookkeeping to satisfy 1.12's rule that\n"
                 "      the total area must come to 1.\n";

    // Verify the normaliser numerically rather than asserting it.
    {
        const double mu = 0.0, sigma = 1.0, step = 0.0005;
        double area = 0.0;
        for (double x = -8.0; x <= 8.0; x += step) area += normalPdf(x, mu, sigma) * step;
        verifyClose("the area really is 1", area, 1.0, 1e-6, "integr. ");
    }

    note("And 1.12 Part 9's warning applies in full: f(x) is a");
    std::cout << "      DENSITY, not a probability. It can exceed 1 whenever\n"
                 "      sigma is small, and P(X = x) is exactly zero for every\n"
                 "      single x. Only areas mean anything.\n";
    std::cout << "\n";
    showV("f(0) with sigma = 1",   normalPdf(0, 0, 1));
    showV("f(0) with sigma = 0.1", normalPdf(0, 0, 0.1));
    note("The second is 3.99 - a perfectly legal density, and a");
    std::cout << "      number no probability is ever allowed to be.\n";

    // ========================================================================
    part(3, "WHICH NORMAL?", "1.13's two numbers, doing the work");
    // ========================================================================
    question("There are infinitely many normals. Which one matches a");
    std::cout << "      given binomial?\n";

    note("The one with the same mean and the same variance - and");
    std::cout << "      you proved both of those in 1.13:\n"
        "\n"
        "         mu     =  np              linearity of expectation\n"
        "         sigma^2 = np(1-p)         plus independence\n"
        "         sigma  =  sqrt(np(1-p))\n"
        "\n"
        "      So the approximation is:\n"
        "\n"
        "         Binomial(n, p)  ~  Normal( np, np(1-p) )\n"
        "\n"
        "      That is DE MOIVRE-LAPLACE, and it needs no new inputs\n"
        "      whatsoever. Two facts you already own pick the curve.\n";

    std::cout << "\n        n       p      mu = np    sigma = sqrt(np(1-p))\n"
              << "        " << repeat("-", 60) << "\n";
    for (const auto& c : std::vector<std::pair<int, double>>{
             {10, 0.5}, {100, 0.5}, {100, 0.1}, {1000, 0.02}, {10000, 0.5}}) {
        const int n = c.first; const double p = c.second;
        std::cout << "        " << std::setw(6) << commas(static_cast<unsigned long long>(n))
                  << std::setw(8) << p << std::setw(11) << n * p
                  << std::setw(17) << std::sqrt(n * p * (1 - p)) << "\n";
    }

    // ========================================================================
    part(4, "THE APPROXIMATION IN ACTION", "look at it");
    // ========================================================================
    question("How good is it, really?");

    note("Binomial(20, 0.5) with its matching normal drawn on top:");
    showOverlay(20, 0.5, 32);

    note("Now a smaller n, where the mismatch is visible:");
    showOverlay(8, 0.5, 32);

    note("And a case with a skewed p, where it is worse still:");
    showOverlay(20, 0.1, 32);

    answer("Good at the centre, symmetric by construction, and it cannot");
    note("handle skew - because the normal has none to give.");

    // ========================================================================
    part(5, "THE CONTINUITY CORRECTION", "a half-unit that matters");
    // ========================================================================
    // The detail that separates a decent approximation from a bad one, and
    // it follows straight from 1.12's mass-versus-density distinction.
    question("The binomial is discrete and the normal is continuous.");
    std::cout << "      How do you even compare them?\n";

    note("Carefully - and this is where most people get it wrong.");
    std::cout << "      The binomial puts a lump of probability AT k. The normal\n"
                 "      spreads density over a continuum and gives any single\n"
                 "      point zero. So P(X = 10) has no direct normal answer.\n";

    note("The fix: treat the bar at k as covering the interval");
    std::cout << "      from k - 0.5 to k + 0.5. That is the CONTINUITY\n"
                 "      CORRECTION, and it is just taking the histogram\n"
                 "      literally - each bar has width 1, centred on k.\n";

    {
        const int n = 20; const double p = 0.5;
        const double mu = n * p, sd = std::sqrt(n * p * (1 - p));
        std::cout << "\n        k    exact      normal, no fix   normal, corrected\n"
                  << "        " << repeat("-", 62) << "\n";
        double worstRaw = 0.0, worstFix = 0.0;
        for (int k = 6; k <= 14; ++k) {
            const double exact = binomialPmf(n, k, p);
            const double raw = normalPdf(k, mu, sd);
            const double fix = normalBetween(k - 0.5, k + 0.5, mu, sd);
            worstRaw = std::max(worstRaw, std::fabs(exact - raw));
            worstFix = std::max(worstFix, std::fabs(exact - fix));
            std::cout << "        " << std::setw(3) << k << std::setw(11) << exact
                      << std::setw(16) << raw << std::setw(19) << fix << "\n";
        }
        verifyClose("corrected beats raw", worstFix < worstRaw ? 1.0 : 0.0, 1.0,
                    1e-9, "better  ");
        std::cout << "\n        worst error, density only   " << worstRaw << "\n"
                  << "        worst error, corrected      " << worstFix << "\n";
    }

    note("For RANGES the same rule applies, and the direction of");
    std::cout << "      the half-unit depends on whether the endpoint is\n"
                 "      included:\n"
        "\n"
        "         P(X <= k)   ->  normal up to  k + 0.5\n"
        "         P(X <  k)   ->  normal up to  k - 0.5\n"
        "         P(X >= k)   ->  normal above  k - 0.5\n"
        "         P(X >  k)   ->  normal above  k + 0.5\n"
        "         P(X = k)    ->  normal between k-0.5 and k+0.5\n";

    {
        const int n = 100; const double p = 0.5;
        const double mu = n * p, sd = std::sqrt(n * p * (1 - p));
        std::cout << "\n        question        exact      no fix    corrected\n"
                  << "        " << repeat("-", 60) << "\n";
        const double e1 = binomialBetween(n, 0, 45, p);
        std::cout << "        P(X <= 45)  " << std::setw(11) << e1
                  << std::setw(11) << normalCdf(45, mu, sd)
                  << std::setw(12) << normalCdf(45.5, mu, sd) << "\n";
        const double e2 = binomialBetween(n, 55, n, p);
        std::cout << "        P(X >= 55)  " << std::setw(11) << e2
                  << std::setw(11) << 1 - normalCdf(55, mu, sd)
                  << std::setw(12) << 1 - normalCdf(54.5, mu, sd) << "\n";
        const double e3 = binomialBetween(n, 45, 55, p);
        std::cout << "        P(45<=X<=55)" << std::setw(11) << e3
                  << std::setw(11) << normalBetween(45, 55, mu, sd)
                  << std::setw(12) << normalBetween(44.5, 55.5, mu, sd) << "\n";
        verifyClose("corrected P(X<=45)", normalCdf(45.5, mu, sd), e1, 3e-3, "exact   ");
        verifyClose("corrected P(45..55)", normalBetween(44.5, 55.5, mu, sd), e3,
                    3e-3, "exact   ");
    }

    answer("Half a unit, and the error drops by roughly an order of magnitude.");

    // ========================================================================
    part(6, "WHEN IT WORKS", "and when it quietly does not");
    // ========================================================================
    question("Can I always do this?");

    note("No. The normal is perfectly symmetric; a binomial is only");
    std::cout << "      symmetric when p = 0.5. Push p towards 0 or 1 and the\n"
                 "      binomial develops a skew the normal cannot represent -\n"
                 "      it is bounded at 0 on one side and has room to run on\n"
                 "      the other.\n";

    std::cout << "\n        n      p       np    n(1-p)   worst error   usable?\n"
              << "        " << repeat("-", 62) << "\n";
    for (const auto& c : std::vector<std::pair<int, double>>{
             {10, 0.5}, {10, 0.1}, {50, 0.1}, {100, 0.05}, {100, 0.5},
             {1000, 0.01}, {1000, 0.5}}) {
        const int n = c.first; const double p = c.second;
        const double mu = n * p, sd = std::sqrt(n * p * (1 - p));
        double worst = 0.0;
        for (int k = 0; k <= n; ++k)
            worst = std::max(worst, std::fabs(binomialPmf(n, k, p)
                             - normalBetween(k - 0.5, k + 0.5, mu, sd)));
        const bool ok = (n * p >= 10) && (n * (1 - p) >= 10);
        std::cout << "        " << std::setw(5) << n << std::setw(7) << p
                  << std::setw(9) << mu << std::setw(9) << n * (1 - p)
                  << std::setw(14) << worst << "      " << (ok ? "yes" : "NO") << "\n";
    }

    answer("The rule of thumb: np >= 10 AND n(1-p) >= 10. Both, not either.");

    note("The two conditions guard the two tails. np >= 10 keeps");
    std::cout << "      the distribution clear of the wall at 0; n(1-p) >= 10\n"
                 "      keeps it clear of the wall at n. Fail either and the\n"
                 "      binomial is pressed against a boundary that the normal\n"
                 "      does not know exists.\n";

    note("And notice the case that fails hardest - large n with");
    std::cout << "      tiny p. Binomial(1000, 0.01) has n = 1000, which sounds\n"
                 "      plenty, but np = 10 sits right on the line. That regime\n"
                 "      has its own limit and its own distribution: the POISSON,\n"
                 "      which is the next video but one.\n";

    // ========================================================================
    part(7, "68 - 95 - 99.7", "the only three numbers you must memorise");
    // ========================================================================
    question("How much of a normal sits near the middle?");

    note("Because the shape is always the same once you measure in");
    std::cout << "      standard deviations, these fractions never change - for\n"
                 "      ANY normal, with any mu and any sigma:\n";

    std::cout << "\n        within        fraction inside     outside\n"
              << "        " << repeat("-", 60) << "\n";
    for (int k : {1, 2, 3, 4, 5}) {
        const double inside = normalBetween(-k, k, 0, 1);
        std::cout << "        " << std::setw(3) << k << " sigma" << std::setw(17)
                  << inside << std::setw(14) << 1 - inside
                  << bar(inside, 20) << "\n";
    }
    verifyClose("1 sigma is about 68%", normalBetween(-1, 1, 0, 1), 0.6827, 1e-3, "the rule");
    verifyClose("2 sigma is about 95%", normalBetween(-2, 2, 0, 1), 0.9545, 1e-3, "the rule");
    verifyClose("3 sigma is about 99.7%", normalBetween(-3, 3, 0, 1), 0.9973, 1e-3, "the rule");

    answer("68%, 95%, 99.7% - and they are worth knowing cold.");

    note("Look at how fast the tail collapses. Each extra sigma");
    std::cout << "      does not shave the outside probability, it demolishes it:\n";
    std::cout << "\n        beyond      P(one tail)        roughly\n"
              << "        " << repeat("-", 60) << "\n";
    for (int k : {1, 2, 3, 4, 5, 6}) {
        const double tail = 1.0 - normalCdf(k, 0, 1);
        std::cout << "        " << std::setw(3) << k << " sigma" << std::setw(16)
                  << std::scientific << std::setprecision(2) << tail
                  << std::fixed << std::setprecision(4)
                  << "     1 in " << commas(static_cast<unsigned long long>(1.0 / tail))
                  << "\n";
    }

    note("Six sigma is one in a billion, which is where the");
    std::cout << "      manufacturing slogan comes from. It is also why real\n"
                 "      data that produces 'six sigma' events every few years is\n"
                 "      telling you the distribution is not normal - not that\n"
                 "      you got unlucky.\n";

    // ========================================================================
    part(8, "WHY THIS SHAPE?", "and not some other one");
    // ========================================================================
    question("Why a bell? Why not a triangle, or a bump with corners?");

    note("Because it is what ADDING does. 1.13 showed a binomial");
    std::cout << "      is a sum of n independent Bernoullis. Now watch what\n"
                 "      happens when you add up something that is not remotely\n"
                 "      bell-shaped - a fair six-sided die, which is FLAT:\n";

    {
        // Convolve the uniform die distribution with itself, repeatedly.
        std::vector<double> cur(7, 0.0);
        for (int f = 1; f <= 6; ++f) cur[static_cast<size_t>(f)] = 1.0 / 6;
        for (int dice = 1; dice <= 5; ++dice) {
            if (dice > 1) {
                std::vector<double> next(cur.size() + 6, 0.0);
                for (size_t s = 0; s < cur.size(); ++s) {
                    if (cur[s] == 0.0) continue;
                    for (int f = 1; f <= 6; ++f)
                        next[s + static_cast<size_t>(f)] += cur[s] / 6;
                }
                cur = next;
            }
            std::cout << "\n        " << dice << (dice > 1 ? " dice" : " die")
                      << "\n";
            for (size_t s = 0; s < cur.size(); ++s) {
                if (cur[s] < 0.002) continue;
                std::cout << "        " << std::setw(4) << s << std::setw(11) << cur[s]
                          << bar(cur[s], 200) << "\n";
            }
        }
    }

    answer("One die is flat. Five dice are already a bell. Nothing was bell-shaped.");

    note("That is the CENTRAL LIMIT THEOREM showing its hand: the");
    std::cout << "      sum of many independent contributions tends to a normal\n"
                 "      REGARDLESS of what the individual pieces look like. The\n"
                 "      binomial-to-normal result in this lesson is one special\n"
                 "      case of it, and dice are another.\n";

    note("Which explains why the normal turns up so relentlessly");
    std::cout << "      in real data. Heights, measurement errors, and daily\n"
                 "      returns are all sums of many small independent effects,\n"
                 "      and sums of many small independent effects have very\n"
                 "      little choice about their shape.\n";

    note("The word INDEPENDENT in that sentence is 1.11's, and it");
    std::cout << "      is doing real work. When the contributions are correlated\n"
                 "      the sum keeps its mean and grows much fatter tails - which\n"
                 "      is precisely why financial returns are famously NOT normal\n"
                 "      in the tails, however normal they look in the middle.\n";

    // ========================================================================
    part(9, "A REAL PROBLEM", "an exact answer nobody can compute");
    // ========================================================================
    question("A site gets 1,000,000 visitors. Each converts with");
    std::cout << "      probability 0.02. What is P(more than 20,300 conversions)?\n";

    {
        const int n = 1000000;
        const double p = 0.02;
        const double mu = n * p, sd = std::sqrt(n * p * (1 - p));

        note("The exact answer is a sum of about 980,000 binomial");
        std::cout << "      terms, each involving C(1000000, k) - a number with\n"
                     "      hundreds of thousands of digits. It exists, and no\n"
                     "      spreadsheet on earth will give it to you.\n";

        std::cout << "\n";
        showV("mu = np",               mu);
        showV("sigma = sqrt(np(1-p))", sd);
        std::cout << "\n"
            "         how far out is 20,300?\n"
            "\n"
            "            (20300 - " << mu << ") / " << sd << "  =  "
                  << (20300 - mu) / sd << " sigma\n";
        std::cout << "\n";
        showP("P(more than 20,300 conversions)", 1.0 - normalCdf(20300.5, mu, sd));

        answer("Two numbers and one function call, accurate to four decimals.");

        note("Sanity-check the approximation on a version small");
        std::cout << "      enough to compute exactly - same p, n = 10,000:\n";
        {
            const int m = 10000;
            const double mu2 = m * p, sd2 = std::sqrt(m * p * (1 - p));
            const double exact = binomialBetween(m, 220, m, p);
            const double approx = 1.0 - normalCdf(219.5, mu2, sd2);
            std::cout << "\n";
            showP("P(X >= 220), exact binomial",  exact);
            showP("P(X >= 220), normal + correction", approx);
            verifyClose("normal vs exact", approx, exact, 3e-3, "exact   ");
        }

        note("It holds to three decimals at n = 10,000, and the");
        std::cout << "      approximation only gets BETTER as n grows - so trusting\n"
                     "      it at a million is not a leap, it is the safe direction.\n";

        // The planning question that actually matters.
        note("The question a business actually asks is the reverse:");
        std::cout << "      how bad is a normal bad week? Here is the range that\n"
                     "      holds 95% of outcomes:\n";
        std::cout << "\n"
            "         mu +/- 1.96 sigma  =  " << mu << " +/- " << 1.96 * sd << "\n"
            "                            =  " << mu - 1.96 * sd << "  to  "
                  << mu + 1.96 * sd << "\n";
        std::cout << "\n      So a drop to 19,700 conversions is an ordinary bad\n"
                     "      week, not a broken site. Knowing that number is the\n"
                     "      difference between a calm morning and a panic.\n";
    }

    // ========================================================================
    part(10, "DO NOT TRUST ME, SIMULATE IT", "300,000 experiments");
    // ========================================================================
    question("Run Binomial(100, 0.5) three hundred thousand times and");
    std::cout << "      histogram it. Does the normal curve land on top?\n";

    std::mt19937 rng(1729);
    std::bernoulli_distribution coin(0.5);
    const int RUNS = 300000, NN = 100;
    const double MU = NN * 0.5, SD = std::sqrt(NN * 0.25);

    std::vector<int> tally(static_cast<size_t>(NN) + 1, 0);
    for (int t = 0; t < RUNS; ++t) {
        int heads = 0;
        for (int i = 0; i < NN; ++i) if (coin(rng)) ++heads;
        ++tally[static_cast<size_t>(heads)];
    }

    std::cout << "\n        k     simulated    normal      exact binomial\n"
              << "        " << repeat("-", 60) << "\n";
    for (int k = 35; k <= 65; ++k) {
        const double sim = static_cast<double>(tally[static_cast<size_t>(k)]) / RUNS;
        if (sim < 0.004) continue;
        std::cout << "        " << std::setw(3) << k << std::setw(12) << sim
                  << std::setw(12) << normalBetween(k - 0.5, k + 0.5, MU, SD)
                  << std::setw(16) << binomialPmf(NN, k, 0.5)
                  << bar(sim, 250) << "\n";
    }

    // The three headline claims, checked against the tally.
    {
        int within1 = 0, within2 = 0, within3 = 0;
        double sum = 0.0, sumSq = 0.0;
        for (int k = 0; k <= NN; ++k) {
            const int c = tally[static_cast<size_t>(k)];
            sum += static_cast<double>(k) * c;
            sumSq += static_cast<double>(k) * k * c;
            const double z = std::fabs(k - MU) / SD;
            if (z <= 1.0) within1 += c;
            if (z <= 2.0) within2 += c;
            if (z <= 3.0) within3 += c;
        }
        const double simMean = sum / RUNS;
        const double simSd = std::sqrt(sumSq / RUNS - simMean * simMean);
        std::cout << "\n";
        verifyClose("mean = np",  MU, simMean, 5e-2, "simulated");
        verifyClose("sd = sqrt(np(1-p))", SD, simSd, 5e-2, "simulated");

        // "Within k sigma" of a DISCRETE variable is a whole number of bars.
        // Here sigma is exactly 5, so it is k in [45,55], [40,60], [35,65] -
        // and those are wider than the smooth 68/95/99.7 slices, by exactly
        // the half-bar at each end. Compare against the exact binomial.
        note("Careful here - and it is Part 5 all over again. X is");
        std::cout << "      DISCRETE, so 'within 1 sigma' means k from 45 to 55,\n"
                     "      which is 11 whole bars. That is wider than the smooth\n"
                     "      68.3% slice by half a bar at each end:\n";
        std::cout << "\n        within     integer range   exact binomial   smooth normal\n"
                  << "        " << repeat("-", 62) << "\n";
        for (int k = 1; k <= 3; ++k) {
            const int lo = static_cast<int>(MU - k * SD), hi = static_cast<int>(MU + k * SD);
            std::cout << "        " << std::setw(3) << k << " sigma" << std::setw(11)
                      << lo << " to " << std::left << std::setw(6) << hi << std::right
                      << std::setw(12) << binomialBetween(NN, lo, hi, 0.5)
                      << std::setw(16) << normalBetween(-k, k, 0, 1) << "\n";
        }
        std::cout << "\n";
        verifyClose("within 1 sigma", binomialBetween(NN, 45, 55, 0.5),
                    static_cast<double>(within1) / RUNS, 5e-3, "simulated");
        verifyClose("within 2 sigma", binomialBetween(NN, 40, 60, 0.5),
                    static_cast<double>(within2) / RUNS, 5e-3, "simulated");
        verifyClose("within 3 sigma", binomialBetween(NN, 35, 65, 0.5),
                    static_cast<double>(within3) / RUNS, 5e-3, "simulated");

        note("And the continuity correction reconciles the two columns");
        std::cout << "      exactly - widen the smooth slice by half a bar each\n"
                     "      side and it lands on the discrete answer:\n";
        std::cout << "\n";
        verifyClose("45..55 with correction",
                    normalBetween(44.5, 55.5, MU, SD),
                    binomialBetween(NN, 45, 55, 0.5), 3e-3, "exact   ");
    }

    answer("The bell, the mean, the sd - all recovered by flipping coins.");

    note("Nothing in that loop knows what a normal distribution");
    std::cout << "      is. It flips a hundred coins and counts. The bell is not\n"
                 "      something we imposed on the data - it is what the data\n"
                 "      does.\n";

    // ========================================================================
    part(11, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Try the approximation yourself.\n"
                 "\n"
                 "        1   compare       - binomial vs normal, side by side\n"
                 "        2   a question    - P(X >= k) exact and approximate\n"
                 "        3   sigma rule    - how much lies within k sigma\n"
                 "        4   big n         - where exact is impossible\n"
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
                         "      1 compare   2 question   3 sigma rule   4 big n   5 quit\n";
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
            if (!askNumber("Number of trials n", 2, 200, n)) { keepGoing = false; break; }
            if (!askNumber("Success chance p, in %", 1, 99, pct)) { keepGoing = false; break; }
            const double p = pct / 100.0;
            const double mu = n * p, sd = std::sqrt(n * p * (1 - p));
            std::cout << "\n      Binomial(" << n << ", " << p << ")  ~  Normal("
                      << mu << ", " << sd * sd << ")\n";
            std::cout << "\n";
            showV("mu = np",               mu);
            showV("sigma = sqrt(np(1-p))", sd);
            showV("np",                    n * p);
            showV("n(1-p)",                n * (1 - p));
            const bool usable = (n * p >= 10) && (n * (1 - p) >= 10);
            std::cout << "\n      ";
            if (usable) std::cout << "Both are 10 or more - the approximation is safe.\n";
            else        std::cout << "One of them is under 10 - expect visible error.\n";
            showOverlay(n, p, 32);
            double worst = 0.0;
            for (int k = 0; k <= n; ++k)
                worst = std::max(worst, std::fabs(binomialPmf(n, k, p)
                                 - normalBetween(k - 0.5, k + 0.5, mu, sd)));
            std::cout << "\n      Worst single-value error: " << worst << "\n";
            break;
        }
        case 2: {
            int n = 0, pct = 0, k = 0;
            if (!askNumber("Number of trials n", 2, 2000, n)) { keepGoing = false; break; }
            if (!askNumber("Success chance p, in %", 1, 99, pct)) { keepGoing = false; break; }
            if (!askNumber("Threshold k", 0, n, k)) { keepGoing = false; break; }
            const double p = pct / 100.0;
            const double mu = n * p, sd = std::sqrt(n * p * (1 - p));

            std::cout << "\n      " << k << " is " << (k - mu) / sd
                      << " sigma from the mean of " << mu << ".\n";
            std::cout << "\n        question       exact       no fix     corrected\n"
                      << "        " << repeat("-", 60) << "\n";
            const double eEq = binomialPmf(n, k, p);
            std::cout << "        P(X = k)  " << std::setw(12) << eEq
                      << std::setw(11) << normalPdf(k, mu, sd)
                      << std::setw(13) << normalBetween(k - 0.5, k + 0.5, mu, sd) << "\n";
            const double eLe = binomialBetween(n, 0, k, p);
            std::cout << "        P(X <= k) " << std::setw(12) << eLe
                      << std::setw(11) << normalCdf(k, mu, sd)
                      << std::setw(13) << normalCdf(k + 0.5, mu, sd) << "\n";
            const double eGe = binomialBetween(n, k, n, p);
            std::cout << "        P(X >= k) " << std::setw(12) << eGe
                      << std::setw(11) << 1 - normalCdf(k, mu, sd)
                      << std::setw(13) << 1 - normalCdf(k - 0.5, mu, sd) << "\n";
            std::cout << "\n";
            verifyClose("corrected P(X <= k)", normalCdf(k + 0.5, mu, sd), eLe,
                        1e-2, "exact   ");
            if ((n * p < 10) || (n * (1 - p) < 10))
                note("np or n(1-p) is under 10 - treat the approximation warily.");
            break;
        }
        case 3: {
            int sigmas = 0;
            if (!askNumber("How many sigma, as a whole number", 1, 8, sigmas)) {
                keepGoing = false; break;
            }
            std::cout << "\n        k sigma   inside        outside      one tail\n"
                      << "        " << repeat("-", 62) << "\n";
            for (int k = 1; k <= sigmas; ++k) {
                const double inside = normalBetween(-k, k, 0, 1);
                std::cout << "        " << std::setw(5) << k << std::setw(13) << inside
                          << std::setw(15) << std::scientific << std::setprecision(3)
                          << 1 - inside << std::setw(14) << (1 - inside) / 2
                          << std::fixed << std::setprecision(4) << "\n";
            }
            const double tail = (1 - normalBetween(-sigmas, sigmas, 0, 1)) / 2;
            if (tail > 0)
                std::cout << "\n      Beyond " << sigmas << " sigma on one side: about 1 in "
                          << commas(static_cast<unsigned long long>(1.0 / tail)) << ".\n";
            note("These fractions are the same for EVERY normal, whatever");
            std::cout << "      its mu and sigma. That is what the next lesson is\n"
                         "      built on.\n";
            break;
        }
        case 4: {
            int n = 0, perMille = 0, thresholdPct = 0;
            if (!askNumber("Number of trials n", 10000, 100000000, n)) { keepGoing = false; break; }
            if (!askNumber("Success rate per 1,000", 1, 999, perMille)) { keepGoing = false; break; }
            if (!askNumber("Threshold as % of the mean", 50, 200, thresholdPct)) {
                keepGoing = false; break;
            }
            const double p = perMille / 1000.0;
            const double mu = n * p, sd = std::sqrt(n * p * (1 - p));
            const double k = mu * thresholdPct / 100.0;
            std::cout << "\n      n = " << commas(static_cast<unsigned long long>(n))
                      << ", p = " << p << "\n";
            std::cout << "\n";
            showV("mu = np",   mu);
            showV("sigma",     sd);
            showV("threshold", k);
            showV("distance in sigma", (k - mu) / sd);
            std::cout << "\n";
            showP("P(X >= threshold)", 1.0 - normalCdf(k - 0.5, mu, sd));
            showP("P(X <= threshold)", normalCdf(k + 0.5, mu, sd));
            std::cout << "\n      A 95% range for X:  " << mu - 1.96 * sd
                      << "  to  " << mu + 1.96 * sd << "\n";
            std::cout << "\n      The exact binomial here would need C(n,k) for n = "
                      << commas(static_cast<unsigned long long>(n)) << ",\n"
                         "      which has more digits than this program has memory.\n";
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
    part(12, "WHAT YOU LEARNED", "1.14 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE DISTRIBUTION\n"
        "                         1              (x - mu)^2\n"
        "          f(x)  =  --------------- exp(- ----------)\n"
        "                    sigma sqrt(2pi)       2 sigma^2\n"
        "          the exponent IS the distribution; the fraction in\n"
        "          front is whatever makes the area come to 1\n"
        "          f(x) is a DENSITY - it may exceed 1, and P(X=x) = 0\n"
        "\n"
        "      DE MOIVRE-LAPLACE - the binomial's limit\n"
        "          Binomial(n,p)  ~  Normal( np, np(1-p) )\n"
        "          both numbers come straight from 1.13, so nothing\n"
        "          new is required to pick the curve\n"
        "\n"
        "      THE CONTINUITY CORRECTION - do not skip it\n"
        "          the bar at k covers k-0.5 to k+0.5\n"
        "          P(X <= k) -> up to k+0.5      P(X < k)  -> up to k-0.5\n"
        "          P(X >= k) -> above k-0.5      P(X > k)  -> above k+0.5\n"
        "          worth roughly an order of magnitude of accuracy\n"
        "\n"
        "      WHEN IT WORKS\n"
        "          np >= 10 AND n(1-p) >= 10 - both, not either\n"
        "          the two conditions keep the distribution clear of\n"
        "          the walls at 0 and at n, which the normal cannot see\n"
        "          large n with tiny p is the POISSON's territory instead\n"
        "\n"
        "      68 - 95 - 99.7\n"
        "          within 1 sigma  68.3%      2 sigma  95.4%\n"
        "          within 3 sigma  99.7%      and the tails then vanish\n"
        "          6 sigma is 1 in a billion - so real data producing\n"
        "          6-sigma events regularly is data that is NOT normal\n"
        "\n"
        "      WHY A BELL AND NOT SOMETHING ELSE\n"
        "          it is what ADDING does: one flat die is flat, five\n"
        "          flat dice are already a bell\n"
        "          sums of many small independent effects have very\n"
        "          little choice about their shape - the central limit\n"
        "          theorem, of which this lesson is one special case\n"
        "          'independent' is load-bearing: correlated pieces keep\n"
        "          the mean and grow much fatter tails\n"
        "\n"
        "      COMING NEXT\n"
        "          1.15 the standard normal - one curve, one table, and\n"
        "               the z-score that reduces every normal to it.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
