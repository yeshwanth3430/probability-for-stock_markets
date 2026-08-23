// ============================================================================
//  1.11 - INDEPENDENCE IN PROBABILITY
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.11 Independence in Probability.cpp" -o p111
//      ./p111
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.6  the binomial - and box 4 was "the trials are INDEPENDENT"
//  1.7  conditioning, and independence met briefly as the case where
//       P(A|B) = P(A) and the news tells you nothing
//  1.8  the law of total probability
//  1.9  Bayes, whose repeated updates multiply only if the observations
//       are INDEPENDENT
//  1.10 a drug test, where retesting works only if the two tests are
//       INDEPENDENT given the truth
//  1.11 the assumption itself, taken apart          <- you are here
//
//  Independence has been quietly holding up the last five lessons. Every
//  time it appeared we used it and moved on, with a note saying we would
//  come back. This is coming back.
//
//  ---------------------------------------------------------------------------
//  WHY IT DESERVES ITS OWN LESSON
//  ---------------------------------------------------------------------------
//  Independence is the assumption that makes probability TRACTABLE. It is
//  what turns a joint distribution over n events - which needs 2^n numbers -
//  into n separate numbers you can multiply. Nothing else in the subject
//  buys that much.
//
//  It is also the assumption that is most often wrong, most often assumed
//  without checking, and most expensive when it fails. Correlated mortgage
//  defaults were priced as independent ones in 2007. That is the whole
//  story of what happened next.
//
//  ---------------------------------------------------------------------------
//  WHAT THIS LESSON ADDS TO 1.7
//  ---------------------------------------------------------------------------
//  1.7 gave the definition. This lesson gives the structure around it:
//
//      - the three equivalent definitions, and why they are the same
//      - independence survives complements (and why that matters)
//      - PAIRWISE independence is NOT MUTUAL independence - with a
//        concrete counterexample you can check by hand
//      - CONDITIONAL independence, which is a different thing entirely
//      - conditioning can CREATE dependence out of nothing
//      - what to do when the assumption is false
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
//  A SAMPLE SPACE YOU CAN CHECK BY HAND
//
//  Two fair dice again - 36 equally likely outcomes, as in 1.7. Everything
//  claimed about independence in this file is checked by counting, never
//  asserted, because independence is exactly the kind of claim that feels
//  obvious and turns out to be false.
// ============================================================================
using DiceEvent = std::function<bool(int, int)>;

int countDice(const DiceEvent& e) {
    int n = 0;
    for (int a = 1; a <= 6; ++a)
        for (int b = 1; b <= 6; ++b)
            if (e(a, b)) ++n;
    return n;
}

double probDice(const DiceEvent& e) { return countDice(e) / 36.0; }

DiceEvent bothOf(const DiceEvent& a, const DiceEvent& b) {
    return [a, b](int x, int y) { return a(x, y) && b(x, y); };
}

double condDice(const DiceEvent& a, const DiceEvent& b) {
    const int nB = countDice(b);
    if (nB == 0) return 0.0;
    return static_cast<double>(countDice(bothOf(a, b))) / nB;
}

// THE TEST. A and B are independent exactly when P(A and B) = P(A) P(B).
// Everything else in this lesson is a consequence of this one line.
bool independent(const DiceEvent& a, const DiceEvent& b, double tol = 1e-12) {
    return std::fabs(probDice(bothOf(a, b)) - probDice(a) * probDice(b)) < tol;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.10)
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

// The independence verdict, with the arithmetic that produced it on show.
void showIndependence(const std::string& aName, const DiceEvent& a,
                      const std::string& bName, const DiceEvent& b) {
    const double pA = probDice(a), pB = probDice(b);
    const double pBoth = probDice(bothOf(a, b));
    const bool indep = std::fabs(pBoth - pA * pB) < 1e-12;
    std::cout << "\n        A = " << aName << "\n"
              << "        B = " << bName << "\n\n"
              << "          P(A)            " << pA << "\n"
              << "          P(B)            " << pB << "\n"
              << "          P(A) x P(B)     " << pA * pB << "\n"
              << "          P(A and B)      " << pBoth << "\n"
              << "          P(A|B)          " << condDice(a, b)
              << (indep ? "      = P(A)\n" : "      but P(A) is different\n")
              << "\n          -> " << (indep ? "INDEPENDENT" : "DEPENDENT") << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // The events, written as predicates so the code reads like the English.
    const DiceEvent firstEven  = [](int a, int  ) { return a % 2 == 0; };
    const DiceEvent secondEven = [](int  , int b) { return b % 2 == 0; };
    const DiceEvent sumEven    = [](int a, int b) { return (a + b) % 2 == 0; };
    const DiceEvent sumIs7     = [](int a, int b) { return a + b == 7; };
    const DiceEvent sumIs8     = [](int a, int b) { return a + b == 8; };
    const DiceEvent firstIs4   = [](int a, int  ) { return a == 4; };
    const DiceEvent secondIs6  = [](int  , int b) { return b == 6; };

    // ========================================================================
    part(1, "THE DEFINITION, THREE WAYS", "and they are one way");
    // ========================================================================
    question("What does it actually mean for A and B to be independent?");

    note("Three statements, all standard, all equivalent:");
    std::cout << "\n"
        "         1   P(A | B)  =  P(A)          knowing B changes nothing\n"
        "                                        about A\n"
        "\n"
        "         2   P(B | A)  =  P(B)          and it works both ways\n"
        "\n"
        "         3   P(A and B) = P(A) x P(B)   the joint factorises\n";

    note("Form 3 is the one to define independence WITH, for two");
    std::cout << "      reasons. It is symmetric - it does not privilege A or B\n"
                 "      - and it stays meaningful when P(B) = 0, where forms 1\n"
                 "      and 2 divide by zero and stop being statements at all.\n";

    note("Forms 1 and 2 are the ones to THINK with. Form 3 is a");
    std::cout << "      fact about arithmetic; form 1 says what independence\n"
                 "      means: the news is worthless.\n";

    note("They are the same statement. Start from form 1 and apply");
    std::cout << "      1.7's definition of a conditional:\n"
        "\n"
        "         P(A|B) = P(A)                     form 1\n"
        "         P(A and B) / P(B) = P(A)          1.7's definition\n"
        "         P(A and B) = P(A) P(B)            multiply by P(B)\n"
        "\n"
        "      and that is form 3. Run it backwards for form 2. One idea,\n"
        "      three faces.\n";

    showIndependence("first die is even", firstEven,
                     "second die is even", secondEven);
    note("Two separate physical objects, so this had better come");
    std::cout << "      out independent - and it does. But 'separate objects' is\n"
                 "      not the test. Part 3 has two events on the SAME pair of\n"
                 "      dice that are also independent, and Part 5 has three\n"
                 "      that look independent and are not.\n";

    // ========================================================================
    part(2, "NOT THE SAME AS DISJOINT", "nearly the opposite, in fact");
    // ========================================================================
    // 1.7 raised this. It is worth doing properly because the confusion is
    // near-universal and the two ideas point in opposite directions.
    question("Disjoint and independent. Same thing?");

    note("They are close to opposites, and the confusion is worth");
    std::cout << "      killing outright:\n"
        "\n"
        "         DISJOINT      A and B cannot both happen\n"
        "                       P(A and B) = 0                     1.3\n"
        "\n"
        "         INDEPENDENT   A and B carry no news about each other\n"
        "                       P(A and B) = P(A) P(B)\n";

    note("Now put them together. If A and B are disjoint AND both");
    std::cout << "      possible, then P(A)P(B) > 0 while P(A and B) = 0, so\n"
                 "      they CANNOT be independent. Learning B happened tells\n"
                 "      you A definitely did not - which is the strongest news\n"
                 "      there is, not the absence of news.\n";

    showIndependence("the sum is 7", sumIs7, "the sum is 8", sumIs8);
    std::cout << "\n";
    showP("P(sum is 7)",             probDice(sumIs7));
    showP("P(sum is 7 | sum is 8)",  condDice(sumIs7, sumIs8));
    answer("Disjoint events are maximally DEPENDENT. They are not independent.");

    note("The only way to be both is if one of them is impossible,");
    std::cout << "      which is a degenerate case nobody cares about.\n";

    // ========================================================================
    part(3, "INDEPENDENCE IS NOT A FEELING", "the sum-of-7 surprise");
    // ========================================================================
    question("Can two events on the SAME dice be independent?");

    note("Yes, and this is where intuition fails badly. 1.7 found");
    std::cout << "      it; here is why it happens:\n";

    showIndependence("the sum is 7", sumIs7, "the first die is 4", firstIs4);

    note("The sum obviously depends on the first die - and yet");
    std::cout << "      'the sum is 7' does not. Whatever the first die shows,\n"
                 "      exactly one face on the second completes a 7, so the\n"
                 "      answer is 1/6 no matter what you are told.\n";

    showIndependence("the sum is 8", sumIs8, "the first die is 4", firstIs4);
    note("Sum 8 is dependent, because a first die of 1 cannot");
    std::cout << "      reach it at all. Seven is the only sum reachable from\n"
                 "      every face, which is exactly why it is the only one\n"
                 "      that is independent of the first die.\n";

    std::cout << "\n        sum   P(sum)   P(sum | first is 4)   verdict\n"
              << "        " << repeat("-", 58) << "\n";
    for (int s = 2; s <= 12; ++s) {
        const DiceEvent sumIsS = [s](int a, int b) { return a + b == s; };
        std::cout << "        " << std::setw(3) << s << std::setw(10) << probDice(sumIsS)
                  << std::setw(18) << condDice(sumIsS, firstIs4) << "      "
                  << (independent(sumIsS, firstIs4) ? "INDEPENDENT <-" : "dependent")
                  << "\n";
    }

    answer("Independence is a numerical accident, not a physical story.");

    note("That is the practical lesson. 'These two things seem");
    std::cout << "      unrelated' is not evidence of independence, and 'these\n"
                 "      two things are obviously connected' is not evidence\n"
                 "      against it. You multiply and you check.\n";

    // ========================================================================
    part(4, "COMPLEMENTS COME ALONG", "a small result you use constantly");
    // ========================================================================
    question("If A is independent of B, what about A and 'not B'?");

    note("Also independent - and so are (not A, B) and (not A,");
    std::cout << "      not B). The proof is two lines of 1.4 and 1.8:\n"
        "\n"
        "         P(A and B^c) = P(A) - P(A and B)      A splits in two\n"
        "                      = P(A) - P(A)P(B)        independence\n"
        "                      = P(A)(1 - P(B))\n"
        "                      = P(A) P(B^c)            which is the test\n";

    const DiceEvent notSecondEven = [](int, int b) { return b % 2 != 0; };
    const DiceEvent notFirstEven  = [](int a, int) { return a % 2 != 0; };
    std::cout << "\n        pair                              independent?\n"
              << "        " << repeat("-", 58) << "\n"
              << "        first even,  second even          "
              << (independent(firstEven, secondEven) ? "yes" : "no") << "\n"
              << "        first even,  second ODD           "
              << (independent(firstEven, notSecondEven) ? "yes" : "no") << "\n"
              << "        first ODD,   second even          "
              << (independent(notFirstEven, secondEven) ? "yes" : "no") << "\n"
              << "        first ODD,   second ODD           "
              << (independent(notFirstEven, notSecondEven) ? "yes" : "no") << "\n";

    answer("Independence is a property of the PAIR OF PARTITIONS, not the events.");

    note("Which is why 1.4 and 1.6 could keep flipping to");
    std::cout << "      complements without ever re-checking anything. When we\n"
                 "      wrote P(at least one) = 1 - (1-p)^n, that (1-p)^n\n"
                 "      needed the FAILURES to be independent - and this result\n"
                 "      is the reason we got that for free from the successes\n"
                 "      being independent.\n";

    // ========================================================================
    part(5, "PAIRWISE IS NOT MUTUAL", "the counterexample everyone should see");
    // ========================================================================
    // The genuinely surprising structural fact about independence, and it
    // is checkable by hand on our 36 outcomes.
    question("Three events, every PAIR independent. Are all three");
    std::cout << "      independent together?\n";

    note("No. And the counterexample is sitting in our dice:");
    std::cout << "\n"
        "         A   the first die is even\n"
        "         B   the second die is even\n"
        "         C   the SUM is even\n";

    note("Check the three pairs. All independent:");
    std::cout << "\n        pair      P(both)   P(x)P(y)   independent?\n"
              << "        " << repeat("-", 58) << "\n";
    const std::vector<std::pair<std::string, std::pair<DiceEvent, DiceEvent>>> pairs = {
        {"A and B", {firstEven, secondEven}},
        {"A and C", {firstEven, sumEven}},
        {"B and C", {secondEven, sumEven}},
    };
    for (const auto& pr : pairs) {
        const DiceEvent& x = pr.second.first;
        const DiceEvent& y = pr.second.second;
        std::cout << "        " << std::left << std::setw(10) << pr.first << std::right
                  << std::setw(9) << probDice(bothOf(x, y))
                  << std::setw(11) << probDice(x) * probDice(y) << "      "
                  << (independent(x, y) ? "yes" : "no") << "\n";
    }

    note("So far so good. Now all three at once. Mutual");
    std::cout << "      independence demands P(A and B and C) = P(A)P(B)P(C):\n";

    const DiceEvent allThree = bothOf(bothOf(firstEven, secondEven), sumEven);
    const double triple = probDice(allThree);
    const double product = probDice(firstEven) * probDice(secondEven) * probDice(sumEven);
    std::cout << "\n"
        "         P(A and B and C)      " << triple << "\n"
        "         P(A)P(B)P(C)          " << product << "\n";

    answer("They are not equal. Pairwise independent, mutually dependent.");

    note("And the reason is not subtle once you see it: if the");
    std::cout << "      first two dice are both even, the sum is even AUTOMATICALLY.\n"
                 "      C is not just influenced by A and B together - it is\n"
                 "      DETERMINED by them. Any two of these three events force\n"
                 "      the third.\n";
    std::cout << "\n";
    showP("P(sum even)",                 probDice(sumEven));
    showP("P(sum even | A and B)",       condDice(sumEven, bothOf(firstEven, secondEven)));
    verifyClose("C is forced by A and B",
                condDice(sumEven, bothOf(firstEven, secondEven)), 1.0, 1e-12,
                "certain ");

    note("So checking pairs is not enough. MUTUAL independence of");
    std::cout << "      n events requires the product rule to hold for EVERY\n"
                 "      subset - all the pairs, all the triples, and so on up.\n"
                 "      That is 2^n - n - 1 separate conditions, and pairwise\n"
                 "      checks only cover the first layer of them.\n";

    // Count exactly how many conditions mutual independence demands.
    std::cout << "\n        n events   pairs   conditions for MUTUAL independence\n"
              << "        " << repeat("-", 58) << "\n";
    for (int n = 2; n <= 10; ++n) {
        long long pairsN = static_cast<long long>(n) * (n - 1) / 2;
        long long conds = (1LL << n) - n - 1;
        std::cout << "        " << std::setw(6) << n << std::setw(10) << pairsN
                  << std::setw(16) << commas(static_cast<unsigned long long>(conds)) << "\n";
    }
    note("At 10 events there are 45 pairs and 1,013 conditions.");
    std::cout << "      Checking correlations pair by pair - which is what most\n"
                 "      risk reports actually do - inspects about 4% of what\n"
                 "      independence claims.\n";

    // ========================================================================
    part(6, "WHY MUTUAL INDEPENDENCE MATTERS", "1.6 was leaning on it");
    // ========================================================================
    question("Does that distinction ever cost anything real?");

    note("It costs the binomial. 1.6's formula multiplied p by");
    std::cout << "      itself n times to price one sequence:\n"
        "\n"
        "         P(HHTHH...)  =  p x p x (1-p) x p x p x ...\n"
        "\n"
        "      That is the MUTUAL independence of all n flips, not the\n"
        "      pairwise independence of some of them. Box 4 in 1.6 was\n"
        "      asking for the strong version all along.\n";

    note("Watch what pairwise-but-not-mutual does to a count. Take");
    std::cout << "      our three dice events A, B, C and ask how often all\n"
                 "      three happen. The binomial-style answer multiplies:\n";
    std::cout << "\n"
        "         if mutually independent   0.5 x 0.5 x 0.5  =  " << product << "\n"
        "         what actually happens                        =  " << triple << "\n"
        "         understated by a factor of                      "
              << triple / product << "\n";

    answer("The multiply-everything shortcut was off by 2x on three events.");

    note("Now scale that thought up. A portfolio of 100 loans");
    std::cout << "      priced as independent, where they are actually all\n"
                 "      exposed to one interest rate, is the same error with\n"
                 "      three more zeroes on it. Part 9 puts numbers on that.\n";

    // ========================================================================
    part(7, "CONDITIONAL INDEPENDENCE", "a different thing entirely");
    // ========================================================================
    // The idea 1.9 and 1.10 kept using without naming - and it neither
    // implies nor is implied by plain independence.
    question("1.10 said two drug tests are independent GIVEN the");
    std::cout << "      truth. What kind of independence is that?\n";

    note("A third kind, and it deserves its own notation:");
    std::cout << "\n"
        "         P(A and B | C)  =  P(A|C) x P(B|C)\n"
        "\n"
        "      'once you know C, A and B stop informing each other'.\n"
        "      This is CONDITIONAL independence, and it is the workhorse\n"
        "      assumption behind almost every statistical model - naive\n"
        "      Bayes classifiers, hidden Markov models, and the repeated\n"
        "      test in 1.10.\n";

    note("The crucial fact: conditional independence does NOT");
    std::cout << "      imply independence, and independence does NOT imply\n"
                 "      conditional independence. They are separate properties\n"
                 "      and neither one gets you the other.\n";

    note("Here is why the drug test case works that way. Two");
    std::cout << "      tests on the same person are NOT independent - the first\n"
                 "      result tells you a great deal about the second, because\n"
                 "      both are about the same person:\n";
    {
        // A person's true status drives both results, so unconditionally the
        // results are correlated. Given the status, they separate.
        const double prev = 0.04, sens = 0.98, spec = 0.97;
        const double pPos = sens * prev + (1 - spec) * (1 - prev);
        const double pBothPos = sens * sens * prev
                              + (1 - spec) * (1 - spec) * (1 - prev);
        std::cout << "\n"
            "         P(test 1 +)               " << pPos << "\n"
            "         P(test 2 +)               " << pPos << "\n"
            "         P(test 1 +) x P(test 2 +) " << pPos * pPos << "\n"
            "         P(both +)                 " << pBothPos << "\n";
        std::cout << "\n         P(both +) is " << pBothPos / (pPos * pPos)
                  << "x what independence predicts,\n"
                     "         so the two results are DEPENDENT.\n";

        note("But condition on the truth and they separate cleanly:");
        std::cout << "\n"
            "         P(both + | user)          " << sens * sens << "\n"
            "         P(+|user) x P(+|user)     " << sens * sens << "\n"
            "         P(both + | clean)         " << (1 - spec) * (1 - spec) << "\n"
            "         P(+|clean) x P(+|clean)   " << (1 - spec) * (1 - spec) << "\n";
        std::cout << "\n         Inside each row the joint IS the product - that is\n"
                     "         what conditional independence asserts, and it is an\n"
                     "         ASSUMPTION about the laboratory, not a theorem.\n";
    }

    answer("Dependent overall, independent once you know the cause.");

    note("That pattern - a shared hidden cause making effects");
    std::cout << "      look correlated - is everywhere. Two symptoms correlate\n"
                 "      because one disease causes both. Two stocks correlate\n"
                 "      because one economy moves both. Conditioning on the\n"
                 "      cause is what makes the effects separable, and it is\n"
                 "      why 1.9's repeated updating is written the way it is.\n";

    // ========================================================================
    part(8, "CONDITIONING CAN CREATE DEPENDENCE", "explaining away");
    // ========================================================================
    // The reverse of Part 7, and the more surprising direction.
    question("Part 7 had conditioning REMOVE dependence. Can it go");
    std::cout << "      the other way and create it?\n";

    note("Yes, and this one is genuinely counterintuitive. Two");
    std::cout << "      independent causes, one common effect:\n"
        "\n"
        "         A   the first die is a 6      independent of B\n"
        "         B   the second die is a 6     independent of A\n"
        "         C   at least one die is a 6   caused by either\n"
        "\n"
        "      A and B are independent - separate dice. But condition\n"
        "      on C and watch what happens.\n";

    const DiceEvent firstIs6  = [](int a, int) { return a == 6; };
    const DiceEvent secondIs6b = [](int, int b) { return b == 6; };
    const DiceEvent atLeastOne6 = [](int a, int b) { return a == 6 || b == 6; };

    showIndependence("first die is 6", firstIs6, "second die is 6", secondIs6b);

    note("Now restrict to the 11 outcomes where at least one die");
    std::cout << "      is a 6, and ask the same question inside that world:\n";
    std::cout << "\n";
    showP("P(first is 6 | at least one 6)", condDice(firstIs6, atLeastOne6));
    showP("P(first is 6 | second is 6)",     condDice(firstIs6, secondIs6b));
    showP("  ...and given at least one 6",
          condDice(firstIs6, bothOf(secondIs6b, atLeastOne6)));

    std::cout << "\n"
        "         inside the 'at least one 6' world:\n"
        "\n"
        "           P(A | C)         " << condDice(firstIs6, atLeastOne6) << "\n"
        "           P(A | B and C)   "
              << condDice(firstIs6, bothOf(secondIs6b, atLeastOne6)) << "\n"
        "\n"
        "         Those differ, so A and B are DEPENDENT given C.\n";

    answer("Two independent causes become dependent once you see the effect.");

    note("The intuition is called EXPLAINING AWAY. You know a six");
    std::cout << "      turned up somewhere. If I now tell you the second die is\n"
                 "      NOT a six, the first one must be - the second die's\n"
                 "      failure forces the first die's success. Learning about\n"
                 "      one cause changes what you believe about the other,\n"
                 "      purely because you already know the effect happened.\n";

    note("This is a real and common trap in data analysis. If you");
    std::cout << "      only ever see cases that were SELECTED on an outcome -\n"
                 "      patients admitted to hospital, startups that got funded,\n"
                 "      candidates who got hired - then independent causes will\n"
                 "      show up correlated in your data, and the correlation is\n"
                 "      manufactured entirely by the selection.\n";

    // ========================================================================
    part(9, "WHEN THE ASSUMPTION BREAKS", "what independence is worth");
    // ========================================================================
    question("Assume independence when it is false. What does it cost?");

    note("A portfolio of 100 loans, each with a 2% chance of");
    std::cout << "      defaulting. If they default independently, 1.6's binomial\n"
                 "      applies and the tail is astonishingly thin:\n";

    const int LOANS = 100;
    const double PD = 0.02;
    auto binomTail = [&](int k) {
        double tail = 0.0;
        for (int i = k; i <= LOANS; ++i)
            tail += std::exp(std::lgamma(LOANS + 1.0) - std::lgamma(i + 1.0)
                             - std::lgamma(LOANS - i + 1.0)
                             + i * std::log(PD) + (LOANS - i) * std::log1p(-PD));
        return tail;
    };
    std::cout << "\n        defaults    P(at least this many), if independent\n"
              << "        " << repeat("-", 58) << "\n";
    for (int k : {2, 5, 10, 20, 50}) {
        std::cout << "        " << std::setw(7) << k << std::setw(20) << binomTail(k);
        if (binomTail(k) < 1e-6) std::cout << "   effectively never";
        std::cout << "\n";
    }

    note("Twenty simultaneous defaults is a 1-in-80-trillion event");
    std::cout << "      - IF they are independent. Now add one shared cause. Say\n"
                 "      there is a 5% chance of a recession, and in a recession\n"
                 "      every loan's default probability jumps to 30%:\n";

    const double pRecession = 0.05, pdBad = 0.30, pdGood = 0.0053;
    // pdGood chosen so the unconditional default rate stays at 2%.
    auto mixedTail = [&](int k) {
        auto tailAt = [&](double pd) {
            double t = 0.0;
            for (int i = k; i <= LOANS; ++i)
                t += std::exp(std::lgamma(LOANS + 1.0) - std::lgamma(i + 1.0)
                              - std::lgamma(LOANS - i + 1.0)
                              + i * std::log(pd) + (LOANS - i) * std::log1p(-pd));
            return t;
        };
        return pRecession * tailAt(pdBad) + (1 - pRecession) * tailAt(pdGood);
    };

    std::cout << "\n      Each loan still defaults 2% of the time on average:\n"
        "\n"
        "         0.05 x 0.30  +  0.95 x " << pdGood << "  =  "
              << pRecession * pdBad + (1 - pRecession) * pdGood << "\n"
        "\n"
        "      Same expected losses. Completely different tail:\n";

    std::cout << "\n        defaults   independent      with a shared cause   ratio\n"
              << "        " << repeat("-", 62) << "\n";
    for (int k : {2, 5, 10, 20, 50}) {
        const double indep = binomTail(k), mixed = mixedTail(k);
        std::cout << "        " << std::setw(7) << k << std::setw(16)
                  << std::scientific << std::setprecision(2) << indep
                  << std::setw(20) << mixed << std::setw(12)
                  << (indep > 0 ? mixed / indep : 0.0) << std::fixed
                  << std::setprecision(4) << "\n";
    }

    answer("Same average loss. Twenty defaults goes from never to 1-in-20.");

    note("Nothing about the individual loans changed. Every loan");
    std::cout << "      still defaults 2% of the time; the AVERAGE outcome is\n"
                 "      identical. Only the dependence structure moved, and it\n"
                 "      moved the entire risk of the portfolio.\n";

    note("This is not a hypothetical. It is, in one paragraph,");
    std::cout << "      what happened to mortgage-backed securities in 2007.\n"
                 "      The models were not wrong about individual default\n"
                 "      rates. They were wrong about whether defaults were\n"
                 "      independent, and that single assumption was where all\n"
                 "      the risk had been hiding.\n";

    note("The general lesson: independence is a statement about");
    std::cout << "      the TAILS more than the average. Assuming it rarely\n"
                 "      changes your expected value much and can change your\n"
                 "      worst case beyond recognition.\n";

    // ========================================================================
    part(10, "TESTING IT ON DATA", "you never get to assume, only to check");
    // ========================================================================
    question("You have data, not a sample space. How do you check?");

    note("Build the 2x2 table and compare what you observed with");
    std::cout << "      what independence predicts. Independence says each cell\n"
                 "      should be (row total x column total) / grand total:\n";

    // A deliberately mild dependence - the kind that is easy to miss.
    const int n11 = 210, n12 = 290, n21 = 190, n22 = 310;
    const int r1 = n11 + n12, r2 = n21 + n22;
    const int k1 = n11 + n21, k2 = n12 + n22;
    const int N = r1 + r2;

    std::cout << "\n                       clicked    did not      total\n"
              << "        " << repeat("-", 58) << "\n"
              << "        saw ad A  " << std::setw(11) << n11 << std::setw(11)
              << n12 << std::setw(11) << r1 << "\n"
              << "        saw ad B  " << std::setw(11) << n21 << std::setw(11)
              << n22 << std::setw(11) << r2 << "\n"
              << "        " << repeat("-", 58) << "\n"
              << "        total     " << std::setw(11) << k1 << std::setw(11)
              << k2 << std::setw(11) << N << "\n";

    std::cout << "\n        cell        observed   expected if independent   gap\n"
              << "        " << repeat("-", 60) << "\n";
    const int obs[4] = {n11, n12, n21, n22};
    const double exp4[4] = {static_cast<double>(r1) * k1 / N,
                            static_cast<double>(r1) * k2 / N,
                            static_cast<double>(r2) * k1 / N,
                            static_cast<double>(r2) * k2 / N};
    const char* cellName[4] = {"A, clicked", "A, did not", "B, clicked", "B, did not"};
    double chiSq = 0.0;
    for (int i = 0; i < 4; ++i) {
        chiSq += (obs[i] - exp4[i]) * (obs[i] - exp4[i]) / exp4[i];
        std::cout << "        " << std::left << std::setw(12) << cellName[i]
                  << std::right << std::setw(9) << obs[i] << std::setw(20)
                  << exp4[i] << std::setw(12) << obs[i] - exp4[i] << "\n";
    }

    std::cout << "\n";
    showFrac("P(clicked | saw ad A)", n11, r1);
    showFrac("P(clicked | saw ad B)", n21, r2);
    showFrac("P(clicked), pooled",    k1,  N);

    note("The two rates differ, so the table is not independent -");
    std::cout << "      but that gap could easily be noise. The chi-squared\n"
                 "      statistic measures how big the gaps are relative to how\n"
                 "      big they would be by chance:\n";
    std::cout << "\n        chi-squared  =  sum of (obs - exp)^2 / exp  =  "
              << chiSq << "\n"
                 "        with 1 degree of freedom, about 3.84 is the 5% mark\n";
    std::cout << "\n      ";
    if (chiSq > 3.84)
        std::cout << "Above 3.84 - the dependence is unlikely to be luck.\n";
    else
        std::cout << "Below 3.84 - this much gap happens by luck often enough.\n";

    note("The formal machinery is a later chapter. The habit is");
    std::cout << "      the point now: independence is a claim about data, and\n"
                 "      claims about data get checked. Note also what the test\n"
                 "      can and cannot do - it can find dependence, but failing\n"
                 "      to find it is not proof of independence.\n";

    // ========================================================================
    part(11, "DO NOT TRUST ME, SIMULATE IT", "300,000 rolls");
    // ========================================================================
    question("Roll two dice a lot. Count the pairwise-but-not-mutual");
    std::cout << "      case from Part 5 and see if it really behaves that way.\n";

    std::mt19937 rng(1729);
    std::uniform_int_distribution<int> die(1, 6);
    const int ROLLS = 300000;

    int nA = 0, nB = 0, nC = 0, nAB = 0, nAC = 0, nBC = 0, nABC = 0;
    for (int t = 0; t < ROLLS; ++t) {
        const int a = die(rng), b = die(rng);
        const bool A = (a % 2 == 0), B = (b % 2 == 0), C = ((a + b) % 2 == 0);
        if (A) ++nA;  if (B) ++nB;  if (C) ++nC;
        if (A && B) ++nAB;
        if (A && C) ++nAC;
        if (B && C) ++nBC;
        if (A && B && C) ++nABC;
    }
    const double pA = static_cast<double>(nA) / ROLLS;
    const double pB = static_cast<double>(nB) / ROLLS;
    const double pC = static_cast<double>(nC) / ROLLS;

    std::cout << "\n        quantity              simulated    if independent\n"
              << "        " << repeat("-", 58) << "\n"
              << "        P(A and B)        " << std::setw(14)
              << static_cast<double>(nAB) / ROLLS << std::setw(16) << pA * pB << "\n"
              << "        P(A and C)        " << std::setw(14)
              << static_cast<double>(nAC) / ROLLS << std::setw(16) << pA * pC << "\n"
              << "        P(B and C)        " << std::setw(14)
              << static_cast<double>(nBC) / ROLLS << std::setw(16) << pB * pC << "\n"
              << "        P(A and B and C)  " << std::setw(14)
              << static_cast<double>(nABC) / ROLLS << std::setw(16)
              << pA * pB * pC << "\n";

    std::cout << "\n";
    verifyClose("A,B independent", static_cast<double>(nAB) / ROLLS, pA * pB,
                5e-3, "product ");
    verifyClose("A,C independent", static_cast<double>(nAC) / ROLLS, pA * pC,
                5e-3, "product ");
    verifyClose("B,C independent", static_cast<double>(nBC) / ROLLS, pB * pC,
                5e-3, "product ");
    verifyClose("the triple, vs 0.25", static_cast<double>(nABC) / ROLLS, 0.25,
                5e-3, "theory  ");

    answer("Every pair agrees with the product. The triple is 2x off.");

    note("A machine that knows no algebra reproduces the exact");
    std::cout << "      structure Part 5 derived: three events, pairwise\n"
                 "      independent, mutually dependent. It is not a technicality\n"
                 "      - it is a thing that happens.\n";

    // ========================================================================
    part(12, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Check independence for yourself.\n"
                 "\n"
                 "        1   two dice      - pick two events, get the verdict\n"
                 "        2   your table    - a 2x2 of counts, checked\n"
                 "        3   shared cause  - what dependence does to a tail\n"
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
                         "      1 dice   2 table   3 shared cause   4 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 4, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            std::cout << "\n      Event A: the sum equals a value you pick.\n"
                         "      Event B: the first die equals a value you pick.\n";
            int wantSum = 0, face = 0;
            if (!askNumber("A - which sum", 2, 12, wantSum)) { keepGoing = false; break; }
            if (!askNumber("B - which first die", 1, 6, face)) { keepGoing = false; break; }

            const DiceEvent A = [wantSum](int a, int b) { return a + b == wantSum; };
            const DiceEvent B = [face](int a, int) { return a == face; };
            showIndependence("the sum is " + std::to_string(wantSum), A,
                             "the first die is " + std::to_string(face), B);

            std::cout << "\n        TEST 1  P(A|B) = P(A)?        " << condDice(A, B)
                      << "  vs  " << probDice(A) << "\n"
                         "        TEST 2  P(A and B) = P(A)P(B)? "
                      << probDice(bothOf(A, B)) << "  vs  "
                      << probDice(A) * probDice(B) << "\n";
            note(independent(A, B)
                 ? "Both tests agree: INDEPENDENT."
                 : "Both tests agree: DEPENDENT.");
            if (wantSum != 7)
                std::cout << "\n      (7 is the only sum independent of the first die -\n"
                             "       it is the only one reachable from every face.)\n";
            break;
        }
        case 2: {
            int a = 0, b = 0, c = 0, d = 0;
            std::cout << "\n      A 2x2 table of counts. Rows are one variable,\n"
                         "      columns the other.\n";
            if (!askNumber("row 1, col 1", 0, 1000000, a)) { keepGoing = false; break; }
            if (!askNumber("row 1, col 2", 0, 1000000, b)) { keepGoing = false; break; }
            if (!askNumber("row 2, col 1", 0, 1000000, c)) { keepGoing = false; break; }
            if (!askNumber("row 2, col 2", 0, 1000000, d)) { keepGoing = false; break; }

            const long long R1 = a + b, R2 = c + d, K1 = a + c, K2 = b + d;
            const long long T = R1 + R2;
            if (T == 0 || R1 == 0 || R2 == 0 || K1 == 0 || K2 == 0) {
                note("Need a non-empty row and column in each direction.");
                break;
            }
            std::cout << "\n                       col 1      col 2      total\n"
                      << "        " << repeat("-", 58) << "\n"
                      << "        row 1  " << std::setw(12) << a << std::setw(11)
                      << b << std::setw(11) << R1 << "\n"
                      << "        row 2  " << std::setw(12) << c << std::setw(11)
                      << d << std::setw(11) << R2 << "\n"
                      << "        " << repeat("-", 58) << "\n"
                      << "        total  " << std::setw(12) << K1 << std::setw(11)
                      << K2 << std::setw(11) << T << "\n";

            std::cout << "\n";
            showFrac("P(col 1 | row 1)", a, R1);
            showFrac("P(col 1 | row 2)", c, R2);
            showFrac("P(col 1) pooled",  K1, T);

            const double e11 = static_cast<double>(R1) * K1 / T;
            const double e12 = static_cast<double>(R1) * K2 / T;
            const double e21 = static_cast<double>(R2) * K1 / T;
            const double e22 = static_cast<double>(R2) * K2 / T;
            const double chi = (a - e11) * (a - e11) / e11 + (b - e12) * (b - e12) / e12
                             + (c - e21) * (c - e21) / e21 + (d - e22) * (d - e22) / e22;
            std::cout << "\n        expected if independent:  " << e11 << "  " << e12
                      << "  " << e21 << "  " << e22 << "\n"
                         "        chi-squared            =  " << chi << "\n";
            std::cout << "\n      ";
            if (std::fabs(static_cast<double>(a) / R1 - static_cast<double>(c) / R2) < 1e-12)
                std::cout << "The two row rates are IDENTICAL - exactly independent.\n";
            else if (chi > 3.84)
                std::cout << "Dependent, and the gap is too big to be luck (chi > 3.84).\n";
            else
                std::cout << "The rates differ, but this much gap happens by luck.\n";
            break;
        }
        case 3: {
            int pdPct = 0, shockPct = 0, shockPd = 0, loans = 0;
            if (!askNumber("Loans in the portfolio", 10, 500, loans)) { keepGoing = false; break; }
            if (!askNumber("Default chance per loan, in %", 1, 50, pdPct)) { keepGoing = false; break; }
            if (!askNumber("Chance of a shared shock, in %", 1, 50, shockPct)) { keepGoing = false; break; }
            if (!askNumber("Default chance during the shock, in %", pdPct, 99, shockPd)) {
                keepGoing = false; break;
            }
            const double target = pdPct / 100.0;
            const double ps = shockPct / 100.0, pdS = shockPd / 100.0;
            const double pdCalm = (target - ps * pdS) / (1.0 - ps);
            if (pdCalm < 0.0) {
                note("Those numbers cannot average out to the overall rate.");
                std::cout << "      Lower the shock probability or its default rate.\n";
                break;
            }
            auto tailAt = [&](int k, double pd) {
                double t = 0.0;
                for (int i = k; i <= loans; ++i)
                    t += std::exp(std::lgamma(loans + 1.0) - std::lgamma(i + 1.0)
                                  - std::lgamma(loans - i + 1.0)
                                  + i * std::log(pd) + (loans - i) * std::log1p(-pd));
                return t;
            };
            std::cout << "\n      Calm-times default rate works out at " << pdCalm
                      << ",\n      so the overall average is still " << target << ".\n";
            std::cout << "\n        defaults   independent    shared cause    ratio\n"
                      << "        " << repeat("-", 60) << "\n";
            for (int k : {loans / 50 + 1, loans / 20, loans / 10, loans / 5, loans / 2}) {
                if (k < 1 || k > loans) continue;
                const double ind = tailAt(k, target);
                const double mix = ps * tailAt(k, pdS) + (1 - ps) * tailAt(k, pdCalm);
                std::cout << "        " << std::setw(7) << k << std::setw(16)
                          << std::scientific << std::setprecision(2) << ind
                          << std::setw(16) << mix << std::setw(12)
                          << (ind > 0 ? mix / ind : 0.0)
                          << std::fixed << std::setprecision(4) << "\n";
            }
            note("Same expected losses either way. Only the tail moved.");
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
    part(13, "WHAT YOU LEARNED", "1.11 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE DEFINITION, THREE EQUIVALENT WAYS\n"
        "          P(A|B) = P(A)          the news is worthless\n"
        "          P(B|A) = P(B)          and it works both ways\n"
        "          P(A and B) = P(A)P(B)  define it with this one -\n"
        "                                 symmetric, and safe when P(B)=0\n"
        "\n"
        "      NOT THE SAME AS DISJOINT - nearly the opposite\n"
        "          disjoint      P(A and B) = 0\n"
        "          independent   P(A and B) = P(A)P(B)\n"
        "          two possible disjoint events are maximally DEPENDENT\n"
        "\n"
        "      IT IS A NUMBER, NOT A FEELING\n"
        "          'sum is 7' is independent of the first die\n"
        "          'sum is 8' is not - because 1 cannot reach 8\n"
        "          you multiply and you check; stories do not settle it\n"
        "\n"
        "      COMPLEMENTS COME ALONG FOR FREE\n"
        "          A indep B  =>  A indep B^c, A^c indep B, A^c indep B^c\n"
        "          which is why 1.4's complement trick never needed\n"
        "          re-checking\n"
        "\n"
        "      PAIRWISE IS NOT MUTUAL\n"
        "          first even, second even, sum even:\n"
        "          every PAIR independent, all THREE not\n"
        "          mutual independence needs 2^n - n - 1 conditions,\n"
        "          and pairwise checks cover only the first layer\n"
        "          1.6's binomial needed the MUTUAL version\n"
        "\n"
        "      CONDITIONAL INDEPENDENCE - a separate property\n"
        "          P(A and B | C) = P(A|C) P(B|C)\n"
        "          neither implies nor is implied by independence\n"
        "          two drug tests: dependent overall, independent\n"
        "          once you condition on the truth\n"
        "\n"
        "      CONDITIONING CAN CREATE DEPENDENCE - explaining away\n"
        "          two independent causes of one effect become\n"
        "          dependent once you know the effect happened\n"
        "          selecting your data on an outcome manufactures\n"
        "          correlations that were never there\n"
        "\n"
        "      WHAT IT COSTS TO ASSUME IT WRONGLY\n"
        "          independence barely changes the AVERAGE and\n"
        "          transforms the TAIL\n"
        "          100 loans at 2%: 20 defaults is 1-in-a-billion if\n"
        "          independent, and routine with one shared cause\n"
        "          same expected loss, unrecognisable worst case\n"
        "\n"
        "      COMING NEXT\n"
        "          random variables and probability distributions -\n"
        "          where events become numbers, and everything from\n"
        "          1.6 gets a proper home.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
