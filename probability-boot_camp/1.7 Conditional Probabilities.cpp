// ============================================================================
//  1.7 - CONDITIONAL PROBABILITIES
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.7 Conditional Probabilities.cpp" -o p17
//      ./p17
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.1  P(A) = ways A can happen / things that can happen
//  1.2  counting those ways: n^r, nPr, nCr, with/without replacement
//  1.3  the language of sets
//  1.4  the complement trick, P(A) = 1 - P(A^c)
//  1.5  a crate on a dock: hypergeometric and the multinomial
//  1.6  the binomial, and the four boxes it has to tick
//  1.7  what P(A) becomes once somebody tells you B    <- you are here
//
//  This is the hinge of the whole course. Everything before it was about
//  probabilities standing alone. Everything after it - total probability,
//  Bayes' theorem, independence, every statistical model you will ever
//  fit - is about probabilities that MOVE when information arrives.
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA
//  ---------------------------------------------------------------------------
//  Conditioning does not modify A. It shrinks the WORLD.
//
//  In 1.1, P(A) was: ways A can happen, over everything that can happen.
//  The denominator was the whole sample space S. Now somebody tells you B
//  happened. Every outcome outside B is dead - it did not occur - so it has
//  no business sitting in your denominator any more.
//
//      P(A | B)  =  outcomes in BOTH A and B
//                   ------------------------
//                   outcomes in B
//
//  Divide top and bottom by |S| and the counts become probabilities:
//
//      P(A | B)  =  P(A and B)              defined whenever P(B) > 0
//                   ----------
//                     P(B)
//
//  That is the entire definition. It is 1.1's formula with a smaller
//  denominator, and every result in the next four lessons is bookkeeping
//  on top of it.
//
//  ---------------------------------------------------------------------------
//  THE TRAP, STATED UP FRONT
//  ---------------------------------------------------------------------------
//  P(A | B) and P(B | A) are DIFFERENT NUMBERS. They are not close, they
//  are not usually similar, and swapping them is the most expensive
//  mistake in applied probability - it has convicted people in court and
//  it terrifies patients about test results every day. Part 8 does nothing
//  but stare at this. Bayes' theorem, two lessons from now, is precisely
//  the machine for converting one into the other.
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
//  A SAMPLE SPACE YOU CAN HOLD IN YOUR HAND
//
//  Two fair dice: 36 equally likely outcomes. Small enough to print in
//  full, big enough to be interesting. Because every outcome is equally
//  likely, probability really is just counting - exactly 1.1 - so the
//  conditional formula can be checked by eye rather than believed.
// ============================================================================
using DiceEvent = std::function<bool(int, int)>;

// |E| - how many of the 36 outcomes satisfy the predicate.
int countDice(const DiceEvent& e) {
    int n = 0;
    for (int a = 1; a <= 6; ++a)
        for (int b = 1; b <= 6; ++b)
            if (e(a, b)) ++n;
    return n;
}

// P(E) = |E| / 36. Plain 1.1.
double probDice(const DiceEvent& e) { return countDice(e) / 36.0; }

// |A and B| - the intersection from 1.3, which is all "and" ever means.
int countBoth(const DiceEvent& a, const DiceEvent& b) {
    return countDice([&](int x, int y) { return a(x, y) && b(x, y); });
}

// P(A | B), written the way the definition reads: count the outcomes in
// both, divide by the outcomes in B. Note what is NOT here - no 36. The
// whole sample space has dropped out, because it is no longer the world.
double condDice(const DiceEvent& a, const DiceEvent& b) {
    const int nB = countBoth(b, b);
    if (nB == 0) return 0.0;                  // P(B) = 0, undefined; see Part 2
    return static_cast<double>(countBoth(a, b)) / nB;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.6)
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

// Conditional probability is a ratio of two COUNTS before it is a decimal,
// and the counts are where the understanding lives - so print both.
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

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // The events we will argue about. Written as predicates on (first, second)
    // so the code says the same thing the English does.
    const DiceEvent sumIs8    = [](int a, int b) { return a + b == 8; };
    const DiceEvent sumIs7    = [](int a, int b) { return a + b == 7; };
    const DiceEvent firstIs4  = [](int a, int  ) { return a == 4; };
    const DiceEvent firstIs1  = [](int a, int  ) { return a == 1; };
    const DiceEvent firstEven = [](int a, int  ) { return a % 2 == 0; };
    const DiceEvent secondIs6 = [](int  , int b) { return b == 6; };

    // ========================================================================
    part(1, "INFORMATION MOVES THE ANSWER", "the whole lesson, in one roll");
    // ========================================================================
    question("I roll two dice behind a screen. What is the chance the");
    std::cout << "      sum is 8?\n";

    note("1.1, nothing more. Count the ways, count everything:");
    std::cout << "\n      ";
    for (int a = 1; a <= 6; ++a)
        for (int b = 1; b <= 6; ++b)
            if (sumIs8(a, b)) std::cout << " (" << a << "," << b << ")";
    std::cout << "\n";
    std::cout << "\n";
    showFrac("P(sum is 8)", countDice(sumIs8), 36);

    question("Now I look, and I tell you the FIRST die is a 4. Same");
    std::cout << "      roll, same dice, nothing has been touched. What is the\n"
                 "      chance the sum is 8 now?\n";

    note("Something changed, and it was not the dice. Thirty of");
    std::cout << "      the thirty-six outcomes have just been ruled out - they\n"
                 "      did not happen. Only six worlds are still alive:\n";
    std::cout << "\n      ";
    for (int b = 1; b <= 6; ++b) std::cout << " (4," << b << ")";
    std::cout << "\n";

    note("Of those six survivors, exactly one has a sum of 8:");
    std::cout << "\n      ";
    for (int b = 1; b <= 6; ++b)
        if (sumIs8(4, b)) std::cout << " (4," << b << ")   <- this one\n";

    std::cout << "\n";
    showFrac("P(sum is 8)", countDice(sumIs8), 36);
    showFrac("P(sum is 8 | first die is 4)", countBoth(sumIs8, firstIs4),
             countDice(firstIs4));

    answer("The probability moved, because the world got smaller.");

    note("That is conditioning, and it is the only idea in this");
    std::cout << "      lesson. You did not change the event. You changed the\n"
                 "      DENOMINATOR - the set of things still considered\n"
                 "      possible. 1.1 said P(A) = ways A happens over ways\n"
                 "      anything happens; conditioning simply rewrites what\n"
                 "      'anything' means.\n";

    note("Sometimes the news is brutal. Suppose I had said the");
    std::cout << "      first die is a 1:\n";
    std::cout << "\n";
    showFrac("P(sum is 8 | first die is 1)", countBoth(sumIs8, firstIs1),
             countDice(firstIs1));
    note("Not unlikely - impossible. A 1 cannot reach 8 with one");
    std::cout << "      die. Information can drive a probability to zero, and\n"
                 "      that is not a special case, it is the same division.\n";

    // ========================================================================
    part(2, "THE FORMULA", "P(A|B) = P(A and B) / P(B)");
    // ========================================================================
    question("Write down what you just did, in symbols.");

    note("You counted the outcomes in BOTH events, and divided by");
    std::cout << "      the outcomes in the event you were told:\n"
        "\n"
        "                     outcomes in A and B           1\n"
        "         P(A | B) = --------------------  =  -----------  =  "
              << condDice(sumIs8, firstIs4) << "\n"
        "                     outcomes in B                6\n";

    note("Now divide top and bottom by 36. Nothing changes, but");
    std::cout << "      the counts turn into probabilities:\n"
        "\n"
        "                     |A and B| / 36        P(A and B)\n"
        "         P(A | B) = ----------------  =  --------------\n"
        "                       |B| / 36               P(B)\n";

    std::cout << "\n";
    showFrac("P(A and B)  - both", countBoth(sumIs8, firstIs4), 36);
    showFrac("P(B)        - the news", countDice(firstIs4), 36);
    showP  ("P(A|B) = P(A and B)/P(B)",
            probDice([&](int a, int b) { return sumIs8(a, b) && firstIs4(a, b); })
            / probDice(firstIs4));

    verifyClose("counts vs probabilities", condDice(sumIs8, firstIs4),
                probDice([&](int a, int b) { return sumIs8(a, b) && firstIs4(a, b); })
                / probDice(firstIs4), 1e-12, "ratio   ");

    note("Two forms, one idea. Use counts when the outcomes are");
    std::cout << "      equally likely and you can list them; use probabilities\n"
                 "      when they are not. Most real problems are the second\n"
                 "      kind, which is why the P(A and B)/P(B) version is the\n"
                 "      one printed in books.\n";

    note("One caveat, and it is not pedantry: P(B) must be > 0.");
    std::cout << "      'Given that an impossible thing happened' has no answer,\n"
                 "      because you would be dividing by an empty world. Every\n"
                 "      conditional statement quietly assumes its condition\n"
                 "      could actually occur.\n";

    // ========================================================================
    part(3, "DO NOT TRUST ME, LIST THEM", "all 36, on one page");
    // ========================================================================
    // The sample space is small enough to print entirely, so the formula
    // never has to be taken on faith.
    question("The formula gave 1/6. Does the actual sample space agree?");

    note("Here is all 36 of it. A marks 'sum is 8', B marks");
    std::cout << "      'first die is 4', and * marks both:\n";
    std::cout << "\n           second:    1     2     3     4     5     6\n"
              << "        " << repeat("-", 58) << "\n";
    for (int a = 1; a <= 6; ++a) {
        std::cout << "        first " << a << " ";
        for (int b = 1; b <= 6; ++b) {
            const bool inA = sumIs8(a, b), inB = firstIs4(a, b);
            std::string mark = inA && inB ? "*" : inA ? "A" : inB ? "B" : ".";
            std::cout << "   " << a << b << mark << " ";
        }
        std::cout << "\n";
    }

    note("Read the picture rather than the formula:");
    std::cout << "\n"
        "         everything            36 cells      the old world\n"
        "         row B (first = 4)      6 cells      the new world\n"
        "         cells marked *         1 cell       A, inside the new world\n"
        "\n"
        "      P(A|B) is that last line over the middle line. Conditioning\n"
        "      is literally 'look at one row instead of the grid'.\n";

    verifyCount("outcomes in B",       6, static_cast<unsigned long long>(countDice(firstIs4)));
    verifyCount("outcomes in A and B", 1, static_cast<unsigned long long>(countBoth(sumIs8, firstIs4)));

    note("Worth noticing before we move on: conditioning gives you");
    std::cout << "      a genuine probability distribution, not a fragment. The\n"
                 "      six sums still reachable from a first die of 4 must add\n"
                 "      to 1, because the new world is a world:\n";
    std::cout << "\n        sum   P(sum | first die is 4)\n"
              << "        " << repeat("-", 58) << "\n";
    double condTotal = 0.0;
    for (int s = 5; s <= 10; ++s) {
        const DiceEvent sumIsS = [s](int a, int b) { return a + b == s; };
        double p = condDice(sumIsS, firstIs4);
        condTotal += p;
        std::cout << "        " << std::setw(3) << s << std::setw(16) << p
                  << bar(p, 90) << "\n";
    }
    verifyClose("the conditioned world", condTotal, 1.0, 1e-12, "must be");

    // ========================================================================
    part(4, "THE CONTINGENCY TABLE", "where conditionals live in real life");
    // ========================================================================
    // Dice are a teaching device. This is the shape the data actually
    // arrives in, and it is 1.5's factory floor again.
    question("Nobody hands you dice at work. They hand you a table.");

    // Two machines, same part, different quality. Counts, not rates -
    // because counts are what a spreadsheet actually contains.
    const int A_DEF = 30,  A_OK = 570;   // machine A: 600 parts
    const int B_DEF = 60,  B_OK = 340;   // machine B: 400 parts
    const int A_TOT = A_DEF + A_OK, B_TOT = B_DEF + B_OK;
    const int DEF   = A_DEF + B_DEF, OK = A_OK + B_OK;
    const int TOTAL = A_TOT + B_TOT;

    note("One thousand parts, off two machines, inspected:");
    std::cout << "\n                     defective     good      total\n"
              << "        " << repeat("-", 58) << "\n"
              << "        machine A " << std::setw(11) << A_DEF << std::setw(10)
              << A_OK << std::setw(11) << A_TOT << "\n"
              << "        machine B " << std::setw(11) << B_DEF << std::setw(10)
              << B_OK << std::setw(11) << B_TOT << "\n"
              << "        " << repeat("-", 58) << "\n"
              << "        total     " << std::setw(11) << DEF << std::setw(10)
              << OK << std::setw(11) << TOTAL << "\n";

    note("Every probability you can ask about these parts is in");
    std::cout << "      that table. There are three kinds, and telling them\n"
                 "      apart is most of the skill:\n";

    std::cout << "\n      JOINT - both at once, over the grand total\n\n";
    showFrac("P(machine A and defective)", A_DEF, TOTAL);
    showFrac("P(machine B and defective)", B_DEF, TOTAL);

    std::cout << "\n      MARGINAL - one thing, ignoring the other. It is the\n"
                 "      row or column total, and 'marginal' is literal: the\n"
                 "      number written in the margin.\n\n";
    showFrac("P(machine A)", A_TOT, TOTAL);
    showFrac("P(defective)", DEF, TOTAL);

    std::cout << "\n      CONDITIONAL - divide by a ROW total instead of the\n"
                 "      grand total. That is the smaller world again.\n\n";
    showFrac("P(defective | machine A)", A_DEF, A_TOT);
    showFrac("P(defective | machine B)", B_DEF, B_TOT);

    answer("Machine B throws out defects three times as often as A.");

    note("Which is the number you would act on, and note that you");
    std::cout << "      cannot see it in the joint probabilities at all. B looks\n"
                 "      worse there too, but only because... well, look:\n";
    std::cout << "\n";
    showFrac("P(defective | machine A)", A_DEF, A_TOT);
    showFrac("P(defective | machine B)", B_DEF, B_TOT);
    showFrac("P(machine A | defective)", A_DEF, DEF);
    showFrac("P(machine B | defective)", B_DEF, DEF);

    note("Stare at those four lines. The last two condition on the");
    std::cout << "      COLUMN instead of the row, and they answer a completely\n"
                 "      different question - not 'how bad is this machine' but\n"
                 "      'given a defect in my hand, where did it come from'.\n"
                 "      Same table, same four cells, different denominators.\n";

    note("Both pairs are useful and they are not interchangeable.");
    std::cout << "      P(defective | B) = " << static_cast<double>(B_DEF) / B_TOT
              << " is a fact about the machine.\n"
                 "      P(B | defective) = " << static_cast<double>(B_DEF) / DEF
              << " is a fact about your defect pile,\n"
                 "      and it depends on how much each machine ran. Part 8\n"
                 "      is about what happens when people confuse them.\n";

    // The foreshadow: total probability, which is the next lesson.
    note("One more thing hiding in this table. The overall defect");
    std::cout << "      rate is a weighted blend of the two machine rates:\n"
        "\n"
        "         P(def) = P(def|A) P(A) + P(def|B) P(B)\n"
        "                = " << static_cast<double>(A_DEF) / A_TOT << " x "
              << static_cast<double>(A_TOT) / TOTAL << "  +  "
              << static_cast<double>(B_DEF) / B_TOT << " x "
              << static_cast<double>(B_TOT) / TOTAL << "\n"
        "                = " << static_cast<double>(DEF) / TOTAL << "\n";
    verifyClose("blended defect rate",
                (static_cast<double>(A_DEF) / A_TOT) * (static_cast<double>(A_TOT) / TOTAL)
              + (static_cast<double>(B_DEF) / B_TOT) * (static_cast<double>(B_TOT) / TOTAL),
                static_cast<double>(DEF) / TOTAL, 1e-12, "table   ");
    note("That is the LAW OF TOTAL PROBABILITY, and it is the");
    std::cout << "      whole of the next lesson. You just derived it by\n"
                 "      reading a table sideways.\n";

    // ========================================================================
    part(5, "THE MULTIPLICATION RULE", "the definition, rearranged");
    // ========================================================================
    // Same equation, solved for the other unknown - and in that form it is
    // more useful than the original.
    question("What if I know the conditional and want the joint?");

    note("Multiply both sides of the definition by P(B):");
    std::cout << "\n"
        "         P(A|B) = P(A and B) / P(B)          the definition\n"
        "\n"
        "         P(A and B) = P(A|B) x P(B)          the multiplication rule\n"
        "\n"
        "      No new content whatsoever - it is one line of algebra. But\n"
        "      it changes how you build problems, because it lets you walk\n"
        "      through a sequence one step at a time.\n";

    question("Deal two cards off a 52-card deck. What is P(both aces)?");

    note("Do it as a sequence. First card, then second card GIVEN");
    std::cout << "      the first - which is where the deck shrinking finally\n"
                 "      becomes easy to handle:\n";
    std::cout << "\n";
    showFrac("P(1st is an ace)",              4, 52);
    showFrac("P(2nd is an ace | 1st was)",    3, 51);
    note("Both numbers are honest counts: after an ace leaves, 3");
    std::cout << "      aces remain among 51 cards. Multiply:\n"
        "\n"
        "         4/52 x 3/51  =  12/2652  =  1/221  =  "
              << (4.0 / 52.0) * (3.0 / 51.0) << "\n";

    note("Cross-check it against 1.2, which would have counted");
    std::cout << "      instead: choose 2 aces from 4, over all 2-card hands.\n";
    std::cout << "\n";
    showFrac("C(4,2) / C(52,2)", 6, 1326);
    verifyClose("sequence vs counting", (4.0 / 52.0) * (3.0 / 51.0),
                6.0 / 1326.0, 1e-12, "C(4,2)/C(52,2)");

    answer("Identical. Sampling without replacement IS conditioning.");

    note("That is worth sitting with. Back in 1.2 'without");
    std::cout << "      replacement' was a rule about how to count. It was\n"
                 "      really a statement about conditional probability all\n"
                 "      along - each draw happens in a world the previous\n"
                 "      draw already shrank.\n";

    // ========================================================================
    part(6, "THE CHAIN RULE", "and 1.4 falls out of it");
    // ========================================================================
    // Two events extend to n events for free, and the payoff is that the
    // birthday problem stops being a trick and becomes an application.
    question("Two events chain. Do more than two?");

    note("Just keep going. Each new event is conditioned on");
    std::cout << "      everything already assumed:\n"
        "\n"
        "         P(A and B and C)\n"
        "              =  P(A) x P(B|A) x P(C | A and B)\n"
        "\n"
        "      and for n events, the same pattern all the way down. That\n"
        "      is the CHAIN RULE.\n";

    question("Now look at 1.4 again - the birthday problem.");

    note("In 1.4 we wrote P(all n birthdays different) as a");
    std::cout << "      product and moved on:\n"
        "\n"
        "         365/365 x 364/365 x 363/365 x ...\n"
        "\n"
        "      Every one of those fractions is a CONDITIONAL PROBABILITY,\n"
        "      and now we can say what each one means:\n"
        "\n"
        "         364/365   P(person 2 differs | person 1 placed)\n"
        "         363/365   P(person 3 differs | 1 and 2 differ)\n"
        "         362/365   P(person 4 differs | 1, 2, 3 all differ)\n"
        "\n"
        "      1.4's product was the chain rule the whole time. We used\n"
        "      it three lessons before we had a name for it.\n";

    // Rebuild 1.4's headline number as an explicit chain of conditionals.
    std::cout << "\n        people   P(next one differs)   P(all differ so far)\n"
              << "        " << repeat("-", 58) << "\n";
    double allDiffer = 1.0;
    for (int n = 1; n <= 23; ++n) {
        const double stepConditional = (365.0 - (n - 1)) / 365.0;
        allDiffer *= stepConditional;
        if (n <= 5 || n == 10 || n == 20 || n >= 22)
            std::cout << "        " << std::setw(5) << n << std::setw(19)
                      << stepConditional << std::setw(20) << allDiffer << "\n";
    }
    std::cout << "\n";
    showP("P(a shared birthday, n = 23)", 1.0 - allDiffer);
    verifyClose("1.4's answer, rebuilt", 1.0 - allDiffer, 0.5073, 1e-4, "1.4 said");

    answer("The same 50.7%, assembled out of 23 conditional statements.");

    // ========================================================================
    part(7, "INDEPENDENCE", "when the news tells you nothing");
    // ========================================================================
    // The special case worth naming, and the one people assume far too
    // freely - including in 1.6, out loud, as an assumption.
    question("Does conditioning ALWAYS move the answer?");

    note("No - and the case where it does not is important enough");
    std::cout << "      to have its own word. A and B are INDEPENDENT when:\n"
        "\n"
        "         P(A | B)  =  P(A)            knowing B changes nothing\n"
        "\n"
        "      Put that into the multiplication rule and it collapses to\n"
        "      the form you have seen since 1.2:\n"
        "\n"
        "         P(A and B)  =  P(A) x P(B)\n"
        "\n"
        "      Those two statements are the same statement. The second is\n"
        "      easier to check; the first is what it MEANS.\n";

    note("Test it on our dice. First die even, second die a six:");
    std::cout << "\n";
    showFrac("P(first die is even)", countDice(firstEven), 36);
    showFrac("P(first even | second is 6)", countBoth(firstEven, secondIs6),
             countDice(secondIs6));
    verifyClose("independent, as expected", condDice(firstEven, secondIs6),
                probDice(firstEven), 1e-12, "P(A)    ");
    note("Identical, and obviously so - the dice do not talk to");
    std::cout << "      each other. Separate physical objects, separate rolls.\n";

    question("Here is the one that catches people. Is 'the sum is 7'");
    std::cout << "      independent of the first die?\n";

    note("Intuition says no - the sum obviously depends on both");
    std::cout << "      dice. Intuition is wrong. Count it:\n";
    std::cout << "\n";
    showFrac("P(sum is 7)",              countDice(sumIs7), 36);
    showFrac("P(sum is 7 | first is 4)", countBoth(sumIs7, firstIs4),
             countDice(firstIs4));
    showFrac("P(sum is 7 | first is 1)", countBoth(sumIs7, firstIs1),
             countDice(firstIs1));
    verifyClose("sum 7 IS independent", condDice(sumIs7, firstIs4),
                probDice(sumIs7), 1e-12, "P(A)    ");

    note("Learning the first die tells you nothing about whether");
    std::cout << "      the sum is 7. Whatever it shows, exactly one face on\n"
                 "      the second die completes a 7 - so the answer is 1/6\n"
                 "      every single time.\n";

    note("Now the same question about the sum being 8:");
    std::cout << "\n";
    showFrac("P(sum is 8)",              countDice(sumIs8), 36);
    showFrac("P(sum is 8 | first is 4)", countBoth(sumIs8, firstIs4),
             countDice(firstIs4));
    showFrac("P(sum is 8 | first is 1)", countBoth(sumIs8, firstIs1),
             countDice(firstIs1));

    answer("Dependent - and 7 was the only sum that could have been otherwise.");

    note("Every other sum is unreachable from some first die, and");
    std::cout << "      that alone breaks independence. Run all eleven sums and\n"
                 "      watch 7 stand alone:\n";
    std::cout << "\n        sum   P(sum)   P(sum | first is 4)   independent?\n"
              << "        " << repeat("-", 58) << "\n";
    for (int s = 2; s <= 12; ++s) {
        const DiceEvent sumIsS = [s](int a, int b) { return a + b == s; };
        const double plain = probDice(sumIsS);
        const double given = condDice(sumIsS, firstIs4);
        const bool indep = std::fabs(plain - given) < 1e-12;
        std::cout << "        " << std::setw(3) << s << std::setw(10) << plain
                  << std::setw(18) << given << "      "
                  << (indep ? "yes  <-" : "no") << "\n";
    }

    note("The lesson is not about dice. It is that independence is");
    std::cout << "      a NUMERICAL fact, not a feeling. It has to be checked,\n"
                 "      and 'these seem unrelated' is not a check. In 1.6 we\n"
                 "      assumed trades were independent and said out loud that\n"
                 "      it was an assumption. This is why we said it.\n";

    note("One more distinction people collapse constantly:");
    std::cout << "\n"
        "         DISJOINT      A and B cannot both happen\n"
        "                       P(A and B) = 0                  from 1.3\n"
        "\n"
        "         INDEPENDENT   A and B do not inform each other\n"
        "                       P(A and B) = P(A) x P(B)\n"
        "\n"
        "      These are close to OPPOSITES. If A and B are disjoint and\n"
        "      both possible, then learning B happened tells you A did\n"
        "      NOT - which is about as far from 'tells you nothing' as\n"
        "      information gets.\n";
    std::cout << "\n";
    showFrac("P(sum is 7)",                    countDice(sumIs7), 36);
    showFrac("P(sum is 7 | sum is 8)", countBoth(sumIs7, sumIs8), countDice(sumIs8));
    note("Disjoint events are maximally DEPENDENT, not independent.");

    // ========================================================================
    part(8, "THE TRAP", "P(A|B) is not P(B|A)");
    // ========================================================================
    // The single most consequential error in applied probability, given a
    // whole part to itself because it deserves one.
    question("Does it matter which way round you write the bar?");

    note("It matters more than anything else in this lesson. Go");
    std::cout << "      back to the factory table and read both directions:\n";
    std::cout << "\n";
    showFrac("P(defective | machine B)", B_DEF, B_TOT);
    showFrac("P(machine B | defective)", B_DEF, DEF);

    note("Same two events, same table, same cell on top - and the");
    std::cout << "      answers differ by a factor of four. The only thing that\n"
                 "      changed is the denominator: 400 parts off machine B,\n"
                 "      versus 90 defects from anywhere.\n";

    note("Now a version with real stakes. A screening test that");
    std::cout << "      is right 99% of the time, for a disease 1 person in\n"
                 "      1,000 actually has. Take 100,000 people:\n";

    const double PREV = 0.001, SENS = 0.99, SPEC = 0.99;
    const long long POP = 100000;
    const long long sick = static_cast<long long>(POP * PREV);
    const long long well = POP - sick;
    const long long truePos  = static_cast<long long>(sick * SENS);
    const long long falsePos = static_cast<long long>(well * (1.0 - SPEC));

    std::cout << "\n                        tests +      tests -      total\n"
              << "        " << repeat("-", 58) << "\n"
              << "        has it   " << std::setw(12) << truePos << std::setw(13)
              << sick - truePos << std::setw(11) << sick << "\n"
              << "        has not  " << std::setw(12) << falsePos << std::setw(13)
              << well - falsePos << std::setw(11) << well << "\n"
              << "        " << repeat("-", 58) << "\n"
              << "        total    " << std::setw(12) << truePos + falsePos
              << std::setw(13) << POP - truePos - falsePos << std::setw(11)
              << POP << "\n";

    std::cout << "\n";
    showFrac("P(tests + | has it)",  truePos,  sick);
    showFrac("P(has it | tests +)",  truePos,  truePos + falsePos);

    answer("A 99% accurate test, and a positive still means under 1 in 10.");

    note("Nothing is wrong with the test. The trouble is that the");
    std::cout << "      well group is a thousand times larger, so its 1% of\n"
                 "      mistakes (" << falsePos << ") swamps the sick group's 99% of\n"
                 "      correct catches (" << truePos << "). The base rate did that,\n"
                 "      not the accuracy.\n";

    note("This exact swap is the PROSECUTOR'S FALLACY. 'The odds");
    std::cout << "      of this match if he were innocent are one in a million'\n"
                 "      is P(match | innocent). What a jury hears is P(innocent\n"
                 "      | match). Those are different numbers, and the gap\n"
                 "      between them is the size of the population searched.\n";

    note("Reversing the bar has a name and a formula, and it is");
    std::cout << "      two lessons away:\n"
        "\n"
        "                        P(B|A) x P(A)\n"
        "         P(A|B)  =  ---------------------      Bayes' theorem\n"
        "                            P(B)\n"
        "\n"
        "      which is just this lesson's definition written twice and\n"
        "      set equal - both P(A|B)P(B) and P(B|A)P(A) equal P(A and B).\n";
    verifyClose("Bayes, on the test table",
                (static_cast<double>(truePos) / sick) * PREV
                    / (static_cast<double>(truePos + falsePos) / POP),
                static_cast<double>(truePos) / (truePos + falsePos),
                1e-9, "table   ");
    note("It already works, and we have not even met it yet - it");
    std::cout << "      is nothing more than the multiplication rule from Part 5\n"
                 "      pointed backwards.\n";

    // ========================================================================
    part(9, "A REAL PROBLEM", "does your signal know anything?");
    // ========================================================================
    // 1.1's fourth world. Independence stops being an abstraction the
    // moment somebody tries to sell you a trading signal.
    question("A signal fires on some days. Is it worth anything?");

    note("Conditional probability answers this exactly, and the");
    std::cout << "      answer is uncomfortable for most signals. Take 1,000\n"
                 "      trading days, of which the market rose on 540:\n";

    const int DAYS = 1000;
    const int SIG_UP = 120, SIG_DOWN = 80;          // signal fired: 200 days
    const int NOS_UP = 420, NOS_DOWN = 380;         // quiet:        800 days
    const int SIG = SIG_UP + SIG_DOWN, NOSIG = NOS_UP + NOS_DOWN;
    const int UPS = SIG_UP + NOS_UP;

    std::cout << "\n                        market up   market down   total\n"
              << "        " << repeat("-", 58) << "\n"
              << "        signal fired " << std::setw(9) << SIG_UP << std::setw(14)
              << SIG_DOWN << std::setw(9) << SIG << "\n"
              << "        quiet        " << std::setw(9) << NOS_UP << std::setw(14)
              << NOS_DOWN << std::setw(9) << NOSIG << "\n"
              << "        " << repeat("-", 58) << "\n"
              << "        total        " << std::setw(9) << UPS << std::setw(14)
              << DAYS - UPS << std::setw(9) << DAYS << "\n";

    std::cout << "\n";
    showFrac("P(up)  - the base rate",   UPS,    DAYS);
    showFrac("P(up | signal fired)",     SIG_UP, SIG);
    showFrac("P(up | quiet)",            NOS_UP, NOSIG);

    const double base   = static_cast<double>(UPS) / DAYS;
    const double onSig  = static_cast<double>(SIG_UP) / SIG;
    std::cout << "\n      The signal moves the answer from " << base << " to "
              << onSig << ",\n"
                 "      an edge of " << std::showpos << (onSig - base) * 100.0
              << std::noshowpos << " percentage points.\n";

    note("Whether that is worth trading is a different question -");
    std::cout << "      but whether it contains INFORMATION is settled, and it\n"
                 "      is settled by Part 7's test. If P(up | signal) had come\n"
                 "      back equal to P(up), the signal would be independent of\n"
                 "      the outcome, which is the precise technical way of\n"
                 "      saying it knows nothing.\n";

    // A worthless signal, for contrast - built to fire proportionally.
    note("Here is what a worthless signal looks like. Same 1,000");
    std::cout << "      days, but this one fires without regard to the outcome:\n";
    const int W_UP = 108, W_DOWN = 92;              // 200 days, same 54/46 split
    std::cout << "\n";
    showFrac("P(up)",                    UPS,  DAYS);
    showFrac("P(up | worthless signal)", W_UP, W_UP + W_DOWN);
    verifyClose("worthless = independent",
                static_cast<double>(W_UP) / (W_UP + W_DOWN), base, 1e-12, "P(up)   ");
    note("Identical to the base rate. It fires, you learn nothing,");
    std::cout << "      and no amount of backtesting will change that - the\n"
                 "      table has already answered it.\n";

    note("And the trap from Part 8, in market clothing. These two");
    std::cout << "      are constantly swapped in strategy pitches:\n";
    std::cout << "\n";
    showFrac("P(up | signal fired)", SIG_UP, SIG);
    showFrac("P(signal fired | up)", SIG_UP, UPS);
    note("The second is not a claim about prediction at all. It is");
    std::cout << "      a claim about how often the signal shows up on good\n"
                 "      days, and it can be made large just by firing more.\n";

    // ========================================================================
    part(10, "DO NOT TRUST ME, SIMULATE IT", "500,000 rolls");
    // ========================================================================
    // Same discipline as 1.4 and 1.6: something mechanical has to agree.
    question("Roll two dice half a million times. Throw away every");
    std::cout << "      roll where the first die is not a 4, and see what is\n"
                 "      left. No formula anywhere in this loop.\n";

    std::mt19937 rng(1729);
    std::uniform_int_distribution<int> die(1, 6);
    const int ROLLS = 500000;

    int kept = 0, keptAnd8 = 0, keptAnd7 = 0, plain8 = 0, plain7 = 0;
    for (int t = 0; t < ROLLS; ++t) {
        const int a = die(rng), b = die(rng);
        if (a + b == 8) ++plain8;
        if (a + b == 7) ++plain7;
        if (a != 4) continue;                       // this line IS conditioning
        ++kept;
        if (a + b == 8) ++keptAnd8;
        if (a + b == 7) ++keptAnd7;
    }

    std::cout << "\n        rolls made                     " << commas(ROLLS) << "\n"
              << "        kept (first die was a 4)       " << commas(static_cast<unsigned long long>(kept)) << "\n"
              << "        of those, sum was 8            " << commas(static_cast<unsigned long long>(keptAnd8)) << "\n"
              << "        of those, sum was 7            " << commas(static_cast<unsigned long long>(keptAnd7)) << "\n";

    std::cout << "\n";
    verifyClose("P(sum 8)", probDice(sumIs8),
                static_cast<double>(plain8) / ROLLS, 5e-3, "simulated");
    verifyClose("P(sum 8 | first is 4)", condDice(sumIs8, firstIs4),
                static_cast<double>(keptAnd8) / kept, 5e-3, "simulated");
    verifyClose("P(sum 7 | first is 4)", condDice(sumIs7, firstIs4),
                static_cast<double>(keptAnd7) / kept, 5e-3, "simulated");

    answer("Conditioning is just throwing rolls away. That is all it is.");

    note("And Part 7's surprise survives the machine. Compare the");
    std::cout << "      two sums before and after the discarding:\n";
    std::cout << "\n        event              all rolls    kept rolls only\n"
              << "        " << repeat("-", 58) << "\n"
              << "        sum is 7  " << std::setw(17)
              << static_cast<double>(plain7) / ROLLS << std::setw(17)
              << static_cast<double>(keptAnd7) / kept << "\n"
              << "        sum is 8  " << std::setw(17)
              << static_cast<double>(plain8) / ROLLS << std::setw(17)
              << static_cast<double>(keptAnd8) / kept << "\n";
    verifyClose("sum 7 unmoved by the news", probDice(sumIs7),
                static_cast<double>(keptAnd7) / kept, 5e-3, "kept     ");
    note("Sum 7 barely budged - independent. Sum 8 jumped.");

    note("That discarding loop is the honest definition of a");
    std::cout << "      conditional probability, and it is worth keeping in your\n"
                 "      head. When somebody says 'given B', picture the rolls\n"
                 "      that got thrown out - and then ask how many were left,\n"
                 "      because that number is your new denominator.\n";

    // ========================================================================
    part(11, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Put your own conditions in. Every answer shows working.\n"
                 "\n"
                 "        1   two dice     - P(sum | something about a die)\n"
                 "        2   your table   - build a 2x2, get all six answers\n"
                 "        3   the test     - accuracy vs base rate\n"
                 "        4   independent? - check any two dice events\n"
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
                         "      1 dice   2 table   3 test   4 independent?   5 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 5, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            int wantSum = 0, known = 0;
            if (!askNumber("Which sum are you asking about", 2, 12, wantSum)) {
                keepGoing = false; break;
            }
            if (!askNumber("What did the first die show", 1, 6, known)) {
                keepGoing = false; break;
            }
            const DiceEvent A = [wantSum](int a, int b) { return a + b == wantSum; };
            const DiceEvent B = [known](int a, int) { return a == known; };

            std::cout << "\n      P(sum is " << wantSum << " | first die is " << known
                      << ")\n";
            std::cout << "\n        STEP 1  the new world - first die is " << known << "\n"
                         "                ";
            for (int b = 1; b <= 6; ++b) std::cout << " (" << known << "," << b << ")";
            std::cout << "\n                " << countDice(B) << " outcomes survive\n";

            std::cout << "\n        STEP 2  which of those give a sum of " << wantSum << "?\n"
                         "                ";
            const int hits = countBoth(A, B);
            if (hits == 0) std::cout << " none - impossible";
            for (int b = 1; b <= 6; ++b)
                if (known + b == wantSum) std::cout << " (" << known << "," << b << ")";
            std::cout << "\n                " << hits << " of them\n";

            std::cout << "\n        STEP 3  divide\n\n";
            showFrac("P(sum | first die)", hits, countDice(B));
            showFrac("P(sum), unconditioned", countDice(A), 36);
            const double moved = condDice(A, B) - probDice(A);
            std::cout << "\n      The news moved it by " << std::showpos << moved
                      << std::noshowpos << ".";
            std::cout << (std::fabs(moved) < 1e-12
                          ? "  Zero - these are INDEPENDENT.\n"
                          : "\n");
            break;
        }
        case 2: {
            int c11 = 0, c12 = 0, c21 = 0, c22 = 0;
            std::cout << "\n      Build a 2x2 table. Rows are the CONDITION\n"
                         "      (group 1 / group 2), columns are the OUTCOME\n"
                         "      (yes / no).\n";
            if (!askNumber("group 1, outcome yes", 0, 1000000, c11)) { keepGoing = false; break; }
            if (!askNumber("group 1, outcome no",  0, 1000000, c12)) { keepGoing = false; break; }
            if (!askNumber("group 2, outcome yes", 0, 1000000, c21)) { keepGoing = false; break; }
            if (!askNumber("group 2, outcome no",  0, 1000000, c22)) { keepGoing = false; break; }

            const long long r1 = c11 + c12, r2 = c21 + c22;
            const long long k1 = c11 + c21, k2 = c12 + c22;
            const long long tot = r1 + r2;
            if (tot == 0) { note("An empty table has nothing to say."); break; }

            std::cout << "\n                        yes          no        total\n"
                      << "        " << repeat("-", 58) << "\n"
                      << "        group 1  " << std::setw(12) << c11 << std::setw(12)
                      << c12 << std::setw(12) << r1 << "\n"
                      << "        group 2  " << std::setw(12) << c21 << std::setw(12)
                      << c22 << std::setw(12) << r2 << "\n"
                      << "        " << repeat("-", 58) << "\n"
                      << "        total    " << std::setw(12) << k1 << std::setw(12)
                      << k2 << std::setw(12) << tot << "\n";

            std::cout << "\n      MARGINAL - the totals in the margin\n\n";
            showFrac("P(group 1)", r1, tot);
            showFrac("P(yes)",     k1, tot);

            std::cout << "\n      JOINT - one cell over the grand total\n\n";
            showFrac("P(group 1 and yes)", c11, tot);

            std::cout << "\n      CONDITIONAL - divide by a row\n\n";
            if (r1) showFrac("P(yes | group 1)", c11, r1);
            if (r2) showFrac("P(yes | group 2)", c21, r2);

            std::cout << "\n      REVERSED - divide by a column instead\n\n";
            if (k1) showFrac("P(group 1 | yes)", c11, k1);

            if (r1 && r2 && k1) {
                const double p1 = static_cast<double>(c11) / r1;
                const double p2 = static_cast<double>(c21) / r2;
                std::cout << "\n      Group 1 says yes " << p1 << ", group 2 says "
                          << p2 << ".\n";
                if (std::fabs(p1 - p2) < 1e-12)
                    note("Equal - the group tells you NOTHING. Independent.");
                else
                    note("Different - so the group carries information.");
                std::cout << "\n      And note P(yes|group 1) = " << p1
                          << " is NOT\n      P(group 1|yes) = "
                          << static_cast<double>(c11) / k1 << ".  Part 8.\n";
            }
            break;
        }
        case 3: {
            int accuracy = 0, perThousand = 0;
            if (!askNumber("Test accuracy, in %", 50, 100, accuracy)) {
                keepGoing = false; break;
            }
            if (!askNumber("How many in 10,000 actually have it", 1, 10000,
                           perThousand)) { keepGoing = false; break; }

            const double acc = accuracy / 100.0;
            const long long pop = 1000000;
            const long long ill = pop / 10000 * perThousand;
            const long long healthy = pop - ill;
            const long long tp = static_cast<long long>(ill * acc);
            const long long fp = static_cast<long long>(healthy * (1.0 - acc));

            std::cout << "\n      A " << accuracy << "% accurate test, " << perThousand
                      << " in 10,000 affected,\n      one million people.\n";
            std::cout << "\n                        tests +      tests -      total\n"
                      << "        " << repeat("-", 58) << "\n"
                      << "        has it   " << std::setw(12) << tp << std::setw(13)
                      << ill - tp << std::setw(11) << ill << "\n"
                      << "        has not  " << std::setw(12) << fp << std::setw(13)
                      << healthy - fp << std::setw(11) << healthy << "\n"
                      << "        " << repeat("-", 58) << "\n"
                      << "        total    " << std::setw(12) << tp + fp
                      << std::setw(13) << pop - tp - fp << std::setw(11) << pop << "\n";

            std::cout << "\n";
            if (ill)     showFrac("P(tests + | has it)", tp, ill);
            if (tp + fp) showFrac("P(has it | tests +)", tp, tp + fp);
            if (tp + fp == 0) { note("Nobody tests positive at all."); break; }

            const double ppv = static_cast<double>(tp) / (tp + fp);
            std::cout << "\n      Out of every 100 positive results, about "
                      << static_cast<int>(ppv * 100 + 0.5) << " are real\n"
                         "      and " << 100 - static_cast<int>(ppv * 100 + 0.5)
                      << " are false alarms.\n";
            if (ppv < 0.5)
                note("Under half. The base rate is beating the accuracy.");
            else
                note("The condition is common enough for a positive to mean something.");
            break;
        }
        case 4: {
            int wantSum = 0, faceLow = 0;
            std::cout << "\n      Event A: the sum takes a value you pick.\n"
                         "      Event B: the first die is at least some value.\n";
            if (!askNumber("A - which sum", 2, 12, wantSum)) { keepGoing = false; break; }
            if (!askNumber("B - first die at least", 1, 6, faceLow)) {
                keepGoing = false; break;
            }
            const DiceEvent A = [wantSum](int a, int b) { return a + b == wantSum; };
            const DiceEvent B = [faceLow](int a, int) { return a >= faceLow; };

            const double pA = probDice(A), pB = probDice(B);
            const double pBoth = countBoth(A, B) / 36.0;
            const double pAgivenB = condDice(A, B);

            std::cout << "\n      A = sum is " << wantSum << ",  B = first die >= "
                      << faceLow << "\n\n";
            showFrac("P(A)",       countDice(A),     36);
            showFrac("P(B)",       countDice(B),     36);
            showFrac("P(A and B)", countBoth(A, B),  36);
            showFrac("P(A | B)",   countBoth(A, B),  countDice(B));
            std::cout << "\n        TEST 1   P(A|B) = P(A)?     " << pAgivenB
                      << "  vs  " << pA << "\n"
                         "        TEST 2   P(A and B) = P(A)P(B)?  " << pBoth
                      << "  vs  " << pA * pB << "\n";
            std::cout << "\n      ";
            if (std::fabs(pAgivenB - pA) < 1e-12)
                std::cout << "INDEPENDENT - B told you nothing about A.\n";
            else
                std::cout << "DEPENDENT - B moved A by " << std::showpos
                          << pAgivenB - pA << std::noshowpos << ".\n";
            note("Both tests always agree - they are the same equation.");
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
    part(12, "WHAT YOU LEARNED", "1.7 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE DEFINITION - conditioning shrinks the world\n"
        "                     P(A and B)          outcomes in A and B\n"
        "          P(A|B) = --------------  =  ----------------------\n"
        "                       P(B)              outcomes in B\n"
        "          requires P(B) > 0 - you cannot condition on nothing\n"
        "\n"
        "      WHAT ACTUALLY CHANGES\n"
        "          not the event, the DENOMINATOR. In simulation it is\n"
        "          one line: throw away every run where B did not happen\n"
        "\n"
        "      THE THREE PROBABILITIES IN ANY TABLE\n"
        "          joint        one cell / grand total      P(A and B)\n"
        "          marginal     a margin / grand total      P(A)\n"
        "          conditional  one cell / a row total      P(A|B)\n"
        "\n"
        "      THE MULTIPLICATION RULE - the definition rearranged\n"
        "          P(A and B) = P(A|B) x P(B)\n"
        "          sampling without replacement was this all along\n"
        "\n"
        "      THE CHAIN RULE - and 1.4 was already using it\n"
        "          P(A and B and C) = P(A) x P(B|A) x P(C|A and B)\n"
        "          the birthday product is 23 conditionals in a row\n"
        "\n"
        "      INDEPENDENCE - the case where the news is worthless\n"
        "          P(A|B) = P(A)      equivalently  P(A and B) = P(A)P(B)\n"
        "          it is a NUMBER you check, never a feeling\n"
        "          disjoint is nearly the OPPOSITE of independent\n"
        "\n"
        "      THE TRAP - P(A|B) is not P(B|A)\n"
        "          P(defective | machine B) = 0.15\n"
        "          P(machine B | defective) = 0.67\n"
        "          same cell, different denominators, different questions\n"
        "          a 99% test on a 1-in-1000 disease: a positive is\n"
        "          right less than 10% of the time\n"
        "\n"
        "      COMING NEXT\n"
        "          1.8 the law of total probability - the weighted blend\n"
        "              you already found by reading Part 4 sideways\n"
        "          1.9 Bayes' theorem - the machine that reverses the bar\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
