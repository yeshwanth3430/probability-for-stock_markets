// ============================================================================
//  1.10 - BAYES' THEOREM EXAMPLE: DRUG TESTING
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.10 Bayes' Theorem Example - Drug Testing.cpp" -o p110
//      ./p110
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.7  conditioning: P(A|B) = P(A and B) / P(B)
//  1.8  the law of total probability: P(A) = sum P(A|Bi) P(Bi)
//  1.9  Bayes' theorem: P(H|E) = P(E|H) P(H) / P(E)
//  1.10 one problem, all the way down                <- you are here
//
//  1.9 gave you the machinery. This lesson does not add any. It takes a
//  single problem and refuses to leave until every part of it is answered,
//  because that is a different skill from knowing the formula.
//
//  ---------------------------------------------------------------------------
//  THE PROBLEM
//  ---------------------------------------------------------------------------
//  A company drug-tests its employees. The test is good. Somebody fails.
//
//  What should happen to them?
//
//  That is not a rhetorical question, and it is not primarily an ethical
//  one - it is a question with a number attached, and the number is
//  P(actually uses | tested positive). Most people, including most of the
//  people who write these policies, badly overestimate it.
//
//  ---------------------------------------------------------------------------
//  THE THREE NUMBERS, AND WHY PEOPLE MIX THEM UP
//  ---------------------------------------------------------------------------
//      SENSITIVITY   P(+ | user)        catches users
//      SPECIFICITY   P(- | non-user)    clears non-users
//      PREVALENCE    P(user)            how many people actually use
//
//  Sensitivity and specificity are properties of the TEST. Prevalence is a
//  property of the PEOPLE. A vendor can tell you the first two and has no
//  idea about the third - and the third is what decides the answer.
//
//  The phrase "99% accurate" is worse than useless because it blurs
//  sensitivity and specificity together, and Part 6 shows those two are
//  not remotely equally important.
// ============================================================================

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
//  THE TEST, AND THE POPULATION IT IS POINTED AT
// ============================================================================
struct Screen {
    double prevalence;    // P(user)
    double sensitivity;   // P(+ | user)
    double specificity;   // P(- | non-user)
};

// P(+) - anybody testing positive, user or not. 1.8's law of total probability.
double probPositive(const Screen& s) {
    return s.sensitivity * s.prevalence
         + (1.0 - s.specificity) * (1.0 - s.prevalence);
}

// PPV - POSITIVE PREDICTIVE VALUE = P(user | +). This is Bayes' theorem, and
// it is the number the whole lesson is about. Note it is NOT the sensitivity,
// however much the marketing material would like you to think it is.
double ppv(const Screen& s) {
    const double p = probPositive(s);
    return p > 0.0 ? s.sensitivity * s.prevalence / p : 0.0;
}

// NPV - NEGATIVE PREDICTIVE VALUE = P(non-user | -). The reassuring one.
double npv(const Screen& s) {
    const double pNeg = (1.0 - s.sensitivity) * s.prevalence
                      + s.specificity * (1.0 - s.prevalence);
    return pNeg > 0.0 ? s.specificity * (1.0 - s.prevalence) / pNeg : 0.0;
}

// The likelihood ratio of a positive result - 1.9's odds form. The entire
// evidential weight of the test in one number, independent of prevalence.
double positiveLR(const Screen& s) {
    const double fpr = 1.0 - s.specificity;
    return fpr > 0.0 ? s.sensitivity / fpr : INFINITY;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.9)
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

void showFrac(const std::string& label, long long top, long long bottom) {
    std::ostringstream frac;
    frac << top << "/" << bottom;
    const double p = bottom ? static_cast<double>(top) / static_cast<double>(bottom) : 0.0;
    std::cout << "        " << std::left << std::setw(30) << label
              << std::setw(12) << frac.str() << std::right
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

// THE 2x2 TABLE, in whole people. Everything in this lesson is read off it.
struct Counts { long long tp, fn, fp, tn, users, clean, pos, neg, total; };

Counts headcount(const Screen& s, long long population) {
    Counts c{};
    c.total = population;
    c.users = static_cast<long long>(population * s.prevalence + 0.5);
    c.clean = population - c.users;
    c.tp    = static_cast<long long>(c.users * s.sensitivity + 0.5);
    c.fn    = c.users - c.tp;
    c.tn    = static_cast<long long>(c.clean * s.specificity + 0.5);
    c.fp    = c.clean - c.tn;
    c.pos   = c.tp + c.fp;
    c.neg   = c.fn + c.tn;
    return c;
}

void showTable(const Counts& c) {
    std::cout << "\n                       tests +      tests -        total\n"
              << "        " << repeat("-", 60) << "\n"
              << "        uses      " << std::setw(12) << commas(static_cast<unsigned long long>(c.tp))
              << std::setw(13) << commas(static_cast<unsigned long long>(c.fn))
              << std::setw(13) << commas(static_cast<unsigned long long>(c.users)) << "\n"
              << "        does not  " << std::setw(12) << commas(static_cast<unsigned long long>(c.fp))
              << std::setw(13) << commas(static_cast<unsigned long long>(c.tn))
              << std::setw(13) << commas(static_cast<unsigned long long>(c.clean)) << "\n"
              << "        " << repeat("-", 60) << "\n"
              << "        total     " << std::setw(12) << commas(static_cast<unsigned long long>(c.pos))
              << std::setw(13) << commas(static_cast<unsigned long long>(c.neg))
              << std::setw(13) << commas(static_cast<unsigned long long>(c.total)) << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // The company, the test, and the workforce. Everything follows from these
    // three numbers - and only two of them are on the vendor's data sheet.
    const Screen JOB { 0.04, 0.98, 0.97 };   // 4% use, 98% caught, 97% cleared
    const long long STAFF = 10000;

    // ========================================================================
    part(1, "THE PROBLEM", "somebody just failed a drug test");
    // ========================================================================
    question("Your company drug-tests all 10,000 employees. The test is");
    std::cout << "      98% sensitive and 97% specific. Dana tests positive.\n"
                 "      Should Dana be fired?\n";

    note("Put the ethics aside for a moment - there is a factual");
    std::cout << "      question underneath it, and the policy cannot possibly\n"
                 "      be right if the factual answer is wrong:\n"
        "\n"
        "         P(Dana uses drugs | Dana tested positive)  =  ?\n"
        "\n"
        "      Most people reading '98% sensitive' will answer 98%. That\n"
        "      is P(+ | uses), which is 1.7's trap and 1.9's whole point.\n";

    note("Here are the three numbers, and notice where each one");
    std::cout << "      comes from - because two are the vendor's and one is\n"
                 "      yours:\n"
        "\n"
        "         SENSITIVITY  P(+ | uses)      = " << JOB.sensitivity
              << "   from the vendor\n"
        "                      catches users\n"
        "\n"
        "         SPECIFICITY  P(- | does not)  = " << JOB.specificity
              << "   from the vendor\n"
        "                      clears non-users\n"
        "\n"
        "         PREVALENCE   P(uses)          = " << JOB.prevalence
              << "   from YOUR workforce\n"
        "                      how many actually use\n";

    note("The vendor cannot supply that third number and it is the");
    std::cout << "      one that decides the answer. Nobody selling you a test\n"
                 "      knows who works at your company.\n";

    // ========================================================================
    part(2, "COUNT PEOPLE FIRST", "10,000 of them, one at a time");
    // ========================================================================
    // 1.9 Part 5's method: do not start with algebra, start with humans.
    question("Before any formula - walk all 10,000 employees through.");

    const Counts c = headcount(JOB, STAFF);

    std::cout << "\n"
        "         10,000 employees\n"
        "              |\n"
        "              +-- " << std::setw(5) << c.users << " actually use     (4% of staff)\n"
        "              |        +-- " << std::setw(4) << c.tp << " test POSITIVE   (98% caught)\n"
        "              |        +-- " << std::setw(4) << c.fn << " test negative   (2% missed)\n"
        "              |\n"
        "              +-- " << std::setw(5) << c.clean << " do not use\n"
        "                       +-- " << std::setw(4) << c.fp << " test POSITIVE   (3% wrongly)\n"
        "                       +-- " << std::setw(4) << c.tn << " test negative   (97% cleared)\n";

    showTable(c);

    note("Now find Dana. Dana is somewhere in the 'tests +'");
    std::cout << "      column, and that column has " << c.pos << " people in it:\n"
        "\n"
        "         " << c.tp << " of them genuinely use drugs\n"
        "         " << c.fp << " of them do not, and are about to be fired anyway\n";

    std::cout << "\n";
    showFrac("P(uses | tested positive)", c.tp, c.pos);

    answer("Nearly half the people this test accuses are innocent.");

    note("Read that again, because it is the entire lesson. The");
    std::cout << "      test is good. The test did its job. And still, "
              << static_cast<int>(100.0 * c.fp / c.pos + 0.5) << " of\n"
                 "      every 100 people it accuses are innocent - because\n"
                 "      there are " << c.clean / c.users
              << " times as many non-users to wrongly accuse\n"
                 "      as there are users to correctly catch.\n";

    note("Not a majority here - but nowhere near the 98% the word");
    std::cout << "      'sensitive' invites you to expect, and Part 5 shows how\n"
                 "      little has to change before the innocent DO become the\n"
                 "      majority of the accused.\n";

    std::cout << "\n      " << c.fp << " false positives against " << c.tp
              << " true ones - the innocent\n      outnumber the guilty in that column by "
              << static_cast<double>(c.fp) / c.tp << " to 1.\n";

    // ========================================================================
    part(3, "NOW THE ALGEBRA", "Bayes gives the same number");
    // ========================================================================
    question("Does the theorem agree with the headcount?");

    note("Set it up exactly as 1.9 did. H is 'uses', E is '+':");
    std::cout << "\n"
        "                        P(+|uses) x P(uses)\n"
        "         P(uses|+) = ------------------------------------\n"
        "                      P(+|uses)P(uses) + P(+|clean)P(clean)\n"
        "\n"
        "         numerator     " << JOB.sensitivity << " x " << JOB.prevalence
              << "  =  " << JOB.sensitivity * JOB.prevalence << "\n"
        "\n"
        "         denominator   " << JOB.sensitivity << " x " << JOB.prevalence
              << "  +  " << 1.0 - JOB.specificity << " x " << 1.0 - JOB.prevalence << "\n"
        "                     = " << JOB.sensitivity * JOB.prevalence << "  +  "
              << (1.0 - JOB.specificity) * (1.0 - JOB.prevalence)
              << "  =  " << probPositive(JOB) << "\n"
        "\n"
        "         divide        " << JOB.sensitivity * JOB.prevalence << " / "
              << probPositive(JOB) << "  =  " << ppv(JOB) << "\n";

    std::cout << "\n";
    showP("P(uses | tested +)   the PPV", ppv(JOB));
    showP("P(clean | tested +)  wrongly accused", 1.0 - ppv(JOB));
    verifyClose("Bayes vs headcount", ppv(JOB),
                static_cast<double>(c.tp) / c.pos, 2e-3, "counted ");

    note("The denominator is worth naming again: it is P(+), the");
    std::cout << "      chance ANYBODY tests positive, and it came from 1.8's\n"
                 "      law of total probability. Every Bayes problem spends\n"
                 "      most of its effort building that one number.\n";
    std::cout << "\n";
    showP("P(a random employee tests +)", probPositive(JOB));
    verifyClose("P(+) vs the column total", probPositive(JOB),
                static_cast<double>(c.pos) / c.total, 2e-3, "counted ");

    // ========================================================================
    part(4, "THE OTHER SIDE", "what a negative result means");
    // ========================================================================
    // The number nobody asks about, and it is the good news.
    question("What about the 9,000-odd people who tested negative?");

    std::cout << "\n";
    showFrac("P(clean | tested -)  the NPV", c.tn, c.neg);
    showFrac("P(uses | tested -)   missed", c.fn, c.neg);
    verifyClose("NPV, formula vs count", npv(JOB),
                static_cast<double>(c.tn) / c.neg, 2e-3, "counted ");

    answer("A negative result is extremely reliable. A positive one is not.");

    note("Same test, same day, and the two results are worth");
    std::cout << "      wildly different amounts. That asymmetry is not a flaw\n"
                 "      in the test - it is what happens when a rare condition\n"
                 "      meets an imperfect screen:\n"
        "\n"
        "         a NEGATIVE agrees with the prior (most people are\n"
        "         clean), so it is easy to believe\n"
        "\n"
        "         a POSITIVE contradicts the prior, so it has to fight\n"
        "         the base rate - and often loses\n";

    note("Which gives the practical rule for reading any screening");
    std::cout << "      test: it is good at RULING OUT and bad at RULING IN.\n"
                 "      That is what screening tests are for, and firing people\n"
                 "      on one is using a tool for the opposite of its purpose.\n";

    // ========================================================================
    part(5, "HOW COMMON IS IT, REALLY?", "the number you cannot buy");
    // ========================================================================
    question("How much does the answer depend on that 4%?");

    note("Almost entirely. Hold the test fixed at 98/97 and move");
    std::cout << "      only the fraction of the workforce that uses:\n";
    std::cout << "\n        prevalence   P(uses | +)   of 100 fired, this many innocent\n"
              << "        " << repeat("-", 60) << "\n";
    for (double prev : {0.001, 0.005, 0.01, 0.04, 0.10, 0.25, 0.50}) {
        const Screen s{prev, JOB.sensitivity, JOB.specificity};
        const int innocent = static_cast<int>((1.0 - ppv(s)) * 100 + 0.5);
        std::cout << "        " << std::setw(8) << prev << std::setw(14) << ppv(s)
                  << std::setw(18) << innocent << bar(ppv(s), 25) << "\n";
    }

    answer("The identical test is trustworthy at 50% use and useless at 0.1%.");

    note("So the honest position is this: unless you know how");
    std::cout << "      common drug use is in your specific workforce, you do\n"
                 "      not know what a positive result means, and you cannot\n"
                 "      find out from the test's data sheet.\n";

    note("And there is a nasty circularity. The usual way to");
    std::cout << "      estimate prevalence is... to test people. If you take\n"
                 "      the positive rate as the prevalence you get "
              << probPositive(JOB) << ",\n"
                 "      when the truth is " << JOB.prevalence
              << " - overstated by "
              << probPositive(JOB) / JOB.prevalence << "x, because the\n"
                 "      false positives got counted as users.\n";
    std::cout << "\n        what you would measure   P(+)        "
              << probPositive(JOB) << "\n"
                 "        what is actually true    P(uses)     "
              << JOB.prevalence << "\n"
                 "        overstated by                        "
              << probPositive(JOB) / JOB.prevalence << "x\n";
    note("Mistaking the first for the second is how a testing");
    std::cout << "      programme justifies its own expansion: it measures\n"
                 "      its own false positives and calls them a drug problem.\n";

    // ========================================================================
    part(6, "WHICH KNOB MATTERS?", "specificity, and it is not close");
    // ========================================================================
    // The most actionable result in the lesson: the two "accuracy" numbers
    // are not equally important, and the marketing one is the wrong one.
    question("You have budget to improve the test. Sensitivity or");
    std::cout << "      specificity - which do you buy?\n";

    note("Push sensitivity from 98% all the way to a perfect 100%,");
    std::cout << "      leaving specificity at 97%:\n";
    std::cout << "\n        sensitivity   P(uses | +)\n"
              << "        " << repeat("-", 58) << "\n";
    for (double sens : {0.80, 0.90, 0.98, 0.99, 1.00}) {
        const Screen s{JOB.prevalence, sens, JOB.specificity};
        std::cout << "        " << std::setw(9) << sens << std::setw(16) << ppv(s)
                  << bar(ppv(s), 40) << "\n";
    }
    note("Barely moves. A perfect catch rate buys you almost");
    std::cout << "      nothing, because the problem was never the users you\n"
                 "      missed.\n";

    note("Now hold sensitivity at 98% and improve specificity:");
    std::cout << "\n        specificity   false + rate   P(uses | +)\n"
              << "        " << repeat("-", 58) << "\n";
    for (double spec : {0.90, 0.95, 0.97, 0.99, 0.995, 0.999}) {
        const Screen s{JOB.prevalence, JOB.sensitivity, spec};
        std::cout << "        " << std::setw(9) << spec << std::setw(14)
                  << 1.0 - spec << std::setw(15) << ppv(s)
                  << bar(ppv(s), 40) << "\n";
    }

    answer("Specificity is the whole ballgame. Sensitivity is a rounding error.");

    note("The reason is arithmetic, not opinion. Sensitivity acts");
    std::cout << "      on " << c.users << " people; specificity acts on " << c.clean
              << ". A one\n      percentage point slip in specificity creates "
              << static_cast<int>(0.01 * c.clean + 0.5)
              << " false\n      positives - more than the entire population of\n"
                 "      genuine users. The big group dominates, always.\n";

    note("Which exposes what 'accurate' hides. Both of these are");
    std::cout << "      '98.5% accurate' if you average the two rates:\n";
    {
        const Screen catchy{JOB.prevalence, 1.00, 0.97};
        const Screen clean {JOB.prevalence, 0.97, 1.00};
        std::cout << "\n";
        showP("100% sens / 97% spec -> PPV", ppv(catchy));
        showP("97% sens / 100% spec -> PPV", ppv(clean));
        note("Same 'accuracy', and one of them is a perfect test.");
    }

    // ========================================================================
    part(7, "TEST TWICE", "and the assumption that decides whether it works");
    // ========================================================================
    question("Dana failed. Retest Dana. Does that fix it?");

    note("If the two tests are INDEPENDENT given Dana's true");
    std::cout << "      state, then yes - spectacularly. Use 1.9's odds form and\n"
                 "      just multiply the likelihood ratio again:\n";

    const double lr = positiveLR(JOB);
    const double priorOdds = JOB.prevalence / (1.0 - JOB.prevalence);
    std::cout << "\n"
        "         likelihood ratio   " << JOB.sensitivity << " / " << 1.0 - JOB.specificity
              << "  =  " << lr << "\n"
        "         prior odds         " << priorOdds << "\n";
    std::cout << "\n        positives in a row   posterior odds   P(uses | all +)\n"
              << "        " << repeat("-", 60) << "\n";
    for (int n = 0; n <= 3; ++n) {
        const double odds = priorOdds * std::pow(lr, n);
        std::cout << "        " << std::setw(12) << n << std::setw(19) << odds
                  << std::setw(17) << odds / (1.0 + odds)
                  << bar(odds / (1.0 + odds), 22) << "\n";
    }
    verifyClose("one test, odds vs Bayes", priorOdds * lr / (1.0 + priorOdds * lr),
                ppv(JOB), 1e-9, "PPV     ");

    answer("Two independent positives take Dana from 58% to 98%.");

    note("Now the caveat that makes this part worth writing. That");
    std::cout << "      table assumes the second test can fail in a way\n"
                 "      unrelated to the first. Often it cannot:\n"
        "\n"
        "         a cross-reacting medication Dana takes\n"
        "         a poppy seed bagel\n"
        "         a quirk of Dana's metabolism\n"
        "         a contaminated batch of test kits\n"
        "\n"
        "      Every one of those makes the SAME test fail the SAME way\n"
        "      twice. The second result then carries almost no new\n"
        "      information, and the 98% above is a fantasy.\n";

    note("Suppose 60% of false positives come from a persistent");
    std::cout << "      cause like those. Then a second positive on the same\n"
                 "      assay is far weaker evidence than it looks:\n";
    {
        // Of the false positives, a fraction repeat with certainty; the rest
        // behave independently. This is a crude model and it is enough to
        // show the direction and the size of the error.
        const double persistent = 0.60;
        const double fpr = 1.0 - JOB.specificity;
        const double fprSecond = persistent + (1.0 - persistent) * fpr;
        const double numer = JOB.sensitivity * JOB.sensitivity * JOB.prevalence;
        const double denom = numer + fpr * fprSecond * (1.0 - JOB.prevalence);
        std::cout << "\n";
        showP("2 positives, assuming independence",
              (priorOdds * lr * lr) / (1.0 + priorOdds * lr * lr));
        showP("2 positives, 60% persistent causes", numer / denom);
        note("The honest number is far lower, and the gap is entirely");
        std::cout << "      an independence assumption. This is why real protocols\n"
                     "      confirm with a DIFFERENT method - gas chromatography\n"
                     "      after an immunoassay - rather than running the same\n"
                     "      test again. It is a probability argument with a\n"
                     "      laboratory consequence.\n";
    }

    // ========================================================================
    part(8, "WHO GETS TESTED", "the policy hiding inside the prior");
    // ========================================================================
    // The most useful practical lever, and it is not a laboratory question.
    question("Same test, two policies. Test everybody, or test only");
    std::cout << "      people after a workplace accident?\n";

    const Screen everyone  {0.04, JOB.sensitivity, JOB.specificity};
    const Screen forCause  {0.30, JOB.sensitivity, JOB.specificity};

    note("Testing for cause changes the prevalence in the group");
    std::cout << "      you test - and prevalence is what drives the PPV:\n";
    std::cout << "\n";
    showP("random screen, 4% use -> PPV",   ppv(everyone));
    showP("for cause, 30% use -> PPV",      ppv(forCause));

    const Counts ce = headcount(everyone, STAFF);
    const Counts cf = headcount(forCause, 500);
    std::cout << "\n        policy            tested   accused   wrongly accused\n"
              << "        " << repeat("-", 60) << "\n"
              << "        everybody      " << std::setw(9) << commas(static_cast<unsigned long long>(ce.total))
              << std::setw(10) << commas(static_cast<unsigned long long>(ce.pos))
              << std::setw(15) << commas(static_cast<unsigned long long>(ce.fp)) << "\n"
              << "        for cause only " << std::setw(9) << commas(static_cast<unsigned long long>(cf.total))
              << std::setw(10) << commas(static_cast<unsigned long long>(cf.pos))
              << std::setw(15) << commas(static_cast<unsigned long long>(cf.fp)) << "\n";

    std::cout << "\n  A.  Same test, same laboratory. One policy wrongly accuses "
              << ce.fp << "\n      people; the other accuses " << cf.fp << ".\n";

    note("Nothing in the laboratory changed. The only difference");
    std::cout << "      is who walked through the door, and that is a policy\n"
                 "      choice rather than a scientific one. Universal screening\n"
                 "      for a rare condition manufactures false accusations at\n"
                 "      scale, and it does so by design, not by malfunction.\n";

    note("This generalises well past drug testing. It is the same");
    std::cout << "      argument for why mass surveillance flags mostly innocent\n"
                 "      people, why screening healthy populations for rare\n"
                 "      cancers is contentious, and why fraud filters annoy\n"
                 "      real customers. Rare thing, big population, imperfect\n"
                 "      test - the false positives always win.\n";

    // ========================================================================
    part(9, "WHAT SHOULD DANA'S EMPLOYER DO?", "the numbers have an opinion");
    // ========================================================================
    question("Put it together. What does the arithmetic actually say?");

    std::cout << "\n"
        "         P(Dana uses | one positive)       " << ppv(JOB) << "\n"
        "         P(Dana is innocent | one positive) " << 1.0 - ppv(JOB) << "\n"
        "\n"
        "      Firing on one positive means firing roughly "
              << static_cast<int>((1.0 - ppv(JOB)) * 100 + 0.5) << " innocent\n"
        "      people out of every 100 dismissals.\n";

    note("The arithmetic does not decide the policy - people do -");
    std::cout << "      but it does rule some policies out as factually\n"
                 "      confused. What survives contact with the numbers:\n"
        "\n"
        "         CONFIRM with a different method before acting.\n"
        "         Part 7's independence argument, applied.\n"
        "\n"
        "         BUY SPECIFICITY, not sensitivity. Part 6.\n"
        "\n"
        "         TEST FOR CAUSE, not universally. Part 8 - it raises\n"
        "         the prior, which is the only free improvement here.\n"
        "\n"
        "         QUOTE THE PPV, never the accuracy, in the policy\n"
        "         document. '98% accurate' is the sentence that causes\n"
        "         the whole problem.\n";

    answer("A single positive is a reason to look closer, not a verdict.");

    // ========================================================================
    part(10, "DO NOT TRUST ME, SIMULATE IT", "1,000,000 employees");
    // ========================================================================
    question("Hire a million people, test them all, then look only at");
    std::cout << "      the ones who failed. No Bayes anywhere in this loop.\n";

    std::mt19937 rng(1729);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    const int PEOPLE = 1000000;

    int uses = 0, flagged = 0, flaggedAndUses = 0, cleared = 0, clearedButUses = 0;
    for (int i = 0; i < PEOPLE; ++i) {
        const bool isUser = unit(rng) < JOB.prevalence;         // the truth
        if (isUser) ++uses;
        const bool positive = isUser ? (unit(rng) < JOB.sensitivity)
                                     : (unit(rng) < 1.0 - JOB.specificity);
        if (positive) { ++flagged; if (isUser) ++flaggedAndUses; }
        else          { ++cleared; if (isUser) ++clearedButUses; }
    }

    std::cout << "\n        employees                    " << commas(PEOPLE) << "\n"
              << "        actually use                 " << commas(static_cast<unsigned long long>(uses)) << "\n"
              << "        tested positive              " << commas(static_cast<unsigned long long>(flagged)) << "\n"
              << "        positive AND use             " << commas(static_cast<unsigned long long>(flaggedAndUses)) << "\n"
              << "        positive but clean           " << commas(static_cast<unsigned long long>(flagged - flaggedAndUses)) << "\n";

    std::cout << "\n";
    verifyClose("prevalence", JOB.prevalence,
                static_cast<double>(uses) / PEOPLE, 5e-3, "simulated");
    verifyClose("P(+), the evidence", probPositive(JOB),
                static_cast<double>(flagged) / PEOPLE, 5e-3, "simulated");
    verifyClose("PPV = P(uses | +)", ppv(JOB),
                static_cast<double>(flaggedAndUses) / flagged, 5e-3, "simulated");
    verifyClose("NPV = P(clean | -)", npv(JOB),
                static_cast<double>(cleared - clearedButUses) / cleared, 5e-3,
                "simulated");

    answer("Every number here falls out of counting. Bayes does the");
    note("counting without needing a million employees to do it to.");

    // ========================================================================
    part(11, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Put your own screening programme in.\n"
                 "\n"
                 "        1   one test      - the full working, PPV and NPV\n"
                 "        2   headcount     - the same thing in whole people\n"
                 "        3   which knob    - sweep sensitivity vs specificity\n"
                 "        4   confirm it    - retest, with and without independence\n"
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
                         "      1 one test   2 headcount   3 which knob   4 confirm   5 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 5, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }
        if (choice == 5) { note("Done. Run it again any time."); break; }

        int prevPerMille = 0, sens = 0, spec = 0;
        if (!askNumber("Users per 1,000 people", 1, 1000, prevPerMille)) break;
        if (!askNumber("Sensitivity P(+|user), in %", 1, 100, sens)) break;
        if (!askNumber("Specificity P(-|clean), in %", 1, 100, spec)) break;

        const Screen mine{prevPerMille / 1000.0, sens / 100.0, spec / 100.0};

        switch (choice) {
        case 1: {
            std::cout << "\n      " << prevPerMille << " in 1,000 use, test is "
                      << sens << "% sensitive, " << spec << "% specific.\n";
            std::cout << "\n        numerator     P(+|user) P(user)   = "
                      << mine.sensitivity << " x " << mine.prevalence << "  =  "
                      << mine.sensitivity * mine.prevalence << "\n"
                         "        denominator   P(+)                = "
                      << probPositive(mine) << "\n"
                         "        divide                            =  "
                      << ppv(mine) << "\n";
            std::cout << "\n";
            showP("P(+) - anybody tests positive", probPositive(mine));
            showP("PPV  P(user | tested +)", ppv(mine));
            showP("      P(innocent | tested +)", 1.0 - ppv(mine));
            showP("NPV  P(clean | tested -)", npv(mine));
            std::cout << "\n      Of every 100 people you accuse, about "
                      << static_cast<int>((1.0 - ppv(mine)) * 100 + 0.5)
                      << " are innocent.\n";
            std::cout << "      Likelihood ratio of a positive: " << positiveLR(mine)
                      << ".\n";
            if (ppv(mine) < 0.5)
                note("Under half. A positive is more likely wrong than right.");
            else
                note("Over half - but 'more likely than not' is a low bar.");
            break;
        }
        case 2: {
            const Counts h = headcount(mine, 100000);
            std::cout << "\n      100,000 people at " << prevPerMille
                      << " users per 1,000:\n";
            std::cout << "\n"
                "         100,000 people\n"
                "              |\n"
                "              +-- " << std::setw(6) << h.users << " use\n"
                "              |        +-- " << std::setw(6) << h.tp << " test POSITIVE\n"
                "              |        +-- " << std::setw(6) << h.fn << " test negative  <- missed\n"
                "              |\n"
                "              +-- " << std::setw(6) << h.clean << " do not\n"
                "                       +-- " << std::setw(6) << h.fp << " test POSITIVE  <- wrongly\n"
                "                       +-- " << std::setw(6) << h.tn << " test negative\n";
            showTable(h);
            std::cout << "\n";
            if (h.pos == 0) { note("Nobody tests positive at all."); break; }
            showFrac("PPV  P(user | +)", h.tp, h.pos);
            if (h.neg) showFrac("NPV  P(clean | -)", h.tn, h.neg);
            verifyClose("headcount vs formula", ppv(mine),
                        static_cast<double>(h.tp) / h.pos, 5e-3, "counted ");
            break;
        }
        case 3: {
            std::cout << "\n      Improving SENSITIVITY, specificity held at "
                      << spec << "%:\n";
            std::cout << "\n        sensitivity   PPV\n"
                      << "        " << repeat("-", 58) << "\n";
            for (double v : {0.70, 0.80, 0.90, 0.95, 0.99, 1.00}) {
                const Screen s{mine.prevalence, v, mine.specificity};
                std::cout << "        " << std::setw(9) << v << std::setw(14)
                          << ppv(s) << bar(ppv(s), 35) << "\n";
            }
            std::cout << "\n      Improving SPECIFICITY, sensitivity held at "
                      << sens << "%:\n";
            std::cout << "\n        specificity   false +   PPV\n"
                      << "        " << repeat("-", 58) << "\n";
            for (double v : {0.90, 0.95, 0.99, 0.995, 0.999, 1.00}) {
                const Screen s{mine.prevalence, mine.sensitivity, v};
                std::cout << "        " << std::setw(9) << v << std::setw(11)
                          << 1.0 - v << std::setw(11) << ppv(s)
                          << bar(ppv(s), 35) << "\n";
            }
            const Screen perfSens{mine.prevalence, 1.0, mine.specificity};
            const Screen perfSpec{mine.prevalence, mine.sensitivity, 1.0};
            std::cout << "\n";
            showP("PPV with PERFECT sensitivity", ppv(perfSens));
            showP("PPV with PERFECT specificity", ppv(perfSpec));
            note("One of those is worth having. The other barely moves.");
            break;
        }
        case 4: {
            int persist = 0;
            if (!askNumber("Of false positives, % from a persistent cause",
                           0, 100, persist)) { keepGoing = false; break; }
            const double fpr = 1.0 - mine.specificity;
            if (fpr <= 0.0) { note("With no false positives, one test is conclusive."); break; }
            const double pf = persist / 100.0;
            const double fprSecond = pf + (1.0 - pf) * fpr;

            const double o1 = mine.prevalence / (1.0 - mine.prevalence)
                            * positiveLR(mine);
            const double o2 = o1 * positiveLR(mine);
            const double numer = mine.sensitivity * mine.sensitivity * mine.prevalence;
            const double denom = numer + fpr * fprSecond * (1.0 - mine.prevalence);

            std::cout << "\n";
            showP("after 1 positive", o1 / (1.0 + o1));
            showP("after 2, assuming independence", o2 / (1.0 + o2));
            showP("after 2, with persistent causes", numer / denom);
            std::cout << "\n      The independence assumption is worth "
                      << (o2 / (1.0 + o2)) - (numer / denom)
                      << "\n      of confidence you may not actually have.\n";
            if (persist >= 50)
                note("At this level, a repeat of the SAME test proves little.");
            else
                note("Still, a different confirmatory method beats a repeat.");
            break;
        }
        default: break;
        }
    }

    // ========================================================================
    part(12, "WHAT YOU LEARNED", "1.10 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE THREE NUMBERS - and only two come from the vendor\n"
        "          SENSITIVITY  P(+|user)      catches users     TEST\n"
        "          SPECIFICITY  P(-|clean)     clears the clean  TEST\n"
        "          PREVALENCE   P(user)        how many use      PEOPLE\n"
        "          the third one decides the answer and cannot be bought\n"
        "\n"
        "      WHAT YOU ACTUALLY WANT\n"
        "          PPV = P(user | +)   -  Bayes' theorem\n"
        "          NPV = P(clean | -)\n"
        "          neither is the 'accuracy' printed on the box\n"
        "\n"
        "      THE HEADLINE RESULT\n"
        "          98% sensitive, 97% specific, 4% of staff use\n"
        "          -> a positive result is right only ~58% of the time\n"
        "          because the clean group is 24x larger and its 3%\n"
        "          error rate produces more false positives than there\n"
        "          are real users to find\n"
        "\n"
        "      SCREENS RULE OUT, THEY DO NOT RULE IN\n"
        "          a negative agrees with the prior -> easy to believe\n"
        "          a positive fights the prior      -> often loses\n"
        "\n"
        "      SPECIFICITY IS THE LEVER\n"
        "          sensitivity acts on the small group, specificity on\n"
        "          the huge one - so a point of specificity is worth\n"
        "          many points of sensitivity\n"
        "          'X% accurate' hides exactly this and should never\n"
        "          appear in a policy document\n"
        "\n"
        "      CONFIRMING - and the assumption underneath it\n"
        "          two INDEPENDENT positives multiply the likelihood\n"
        "          ratio and are very strong\n"
        "          repeating the SAME assay is not independent - the\n"
        "          medication, the metabolism, the bad batch all repeat\n"
        "          so confirm with a DIFFERENT method\n"
        "\n"
        "      WHO YOU TEST IS A STATISTICAL DECISION\n"
        "          testing for cause raises the prior, which is the\n"
        "          only free improvement available\n"
        "          universal screening for a rare thing manufactures\n"
        "          false accusations at scale, by design\n"
        "\n"
        "      COMING NEXT\n"
        "          1.11 independence - assumed by the binomial in 1.6,\n"
        "               by the retest in Part 7, and by almost every\n"
        "               model anyone ever builds. Time to take it apart.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
