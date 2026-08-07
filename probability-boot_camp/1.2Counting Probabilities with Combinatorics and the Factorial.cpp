// ============================================================================
//  1.2 - COUNTING PROBABILITIES WITH COMBINATORICS AND THE FACTORIAL
//
//  Build & run (quote the name, it has spaces):
//      g++ -std=c++17 -Wall -Wextra -O2 "1.2Counting Probabilities with Combinatorics and the Factorial.cpp" -o p12
//      ./p12
//
//  ---------------------------------------------------------------------------
//  WHY THIS LESSON EXISTS
//  ---------------------------------------------------------------------------
//  In 1.1 we learned the only formula we need:
//
//                  # ways A can happen
//       P(A) =  --------------------------
//                # of things that can happen
//
//  and we found both numbers by LISTING everything: 2 coins, 36 dice, 52 cards.
//
//  That stops working immediately. How many 5-card poker hands are there?
//  Two and a half million. You cannot list them. So this lesson answers:
//
//       HOW DO I COUNT OUTCOMES I CANNOT LOOK AT?
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA: SAMPLING
//  ---------------------------------------------------------------------------
//  Nearly every counting problem has the same shape:
//
//       I have n choices. I pick r of them. How many ways?
//
//  The answer depends on exactly TWO questions:
//
//       Q1: after I pick something, can I pick it AGAIN?   (replacement)
//       Q2: does the ORDER of my picks matter?
//
//  Two yes/no questions = four cases, three of which we build here:
//
//                          | order matters  | order ignored
//       -------------------+----------------+------------------
//       with replacement   |      n^r       |  (a later lesson)
//       without            |   n!/(n-r)!    |  n!/((n-r)! r!)
//
//  Every formula below is checked against brute-force listing, so you can see
//  it is not magic. The layout of this program is deliberate:
//
//       Q.  the question, in plain English
//           a picture of the slots
//       A.  the answer
//       ok  the formula checked against listing everything
// ============================================================================

#include <algorithm>  // std::next_permutation, std::sort, std::reverse
#include <iomanip>
#include <iostream>
#include <sstream>    // std::ostringstream, for formatting odds
#include <string>
#include <vector>

// ============================================================================
//  COUNTING HELPERS
//
//  A warning that matters: these numbers get huge FAST. 13! already blows past
//  a 32-bit int and 21! blows past a 64-bit one. So we use unsigned long long
//  everywhere, and we never build a big factorial just to cancel it again.
// ============================================================================

// n! = n * (n-1) * ... * 2 * 1          (and 0! = 1 by definition)
unsigned long long factorial(int n) {
    unsigned long long result = 1;
    for (int i = 2; i <= n; ++i) result *= static_cast<unsigned long long>(i);
    return result;
}

// PERMUTATIONS - pick r from n, WITHOUT replacement, ORDER MATTERS.
//     nPr = n! / (n-r)!  =  n * (n-1) * ... * (n-r+1)      <- exactly r terms
// We multiply those r terms directly rather than computing 52! and 47!.
// 52! overflows; 52*51*50*49*48 does not. Same answer, no explosion.
unsigned long long nPr(int n, int r) {
    if (r < 0 || r > n) return 0;
    unsigned long long result = 1;
    for (int i = 0; i < r; ++i) result *= static_cast<unsigned long long>(n - i);
    return result;
}

// COMBINATIONS - pick r from n, WITHOUT replacement, ORDER IGNORED.
//     nCr = n! / ((n-r)! r!)  =  nPr / r!
// Built one step at a time so the running value stays small. The division is
// exact at every step, so no fractions ever appear.
unsigned long long nCr(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r > n - r) r = n - r;             // C(52,47) == C(52,5): take the easy one
    unsigned long long result = 1;
    for (int i = 1; i <= r; ++i) {
        result *= static_cast<unsigned long long>(n - r + i);
        result /= static_cast<unsigned long long>(i);
    }
    return result;
}

// n^r, for the with-replacement case.
unsigned long long power(int n, int r) {
    unsigned long long result = 1;
    for (int i = 0; i < r; ++i) result *= static_cast<unsigned long long>(n);
    return result;
}

// ============================================================================
//  LAYOUT HELPERS
//
//  One place that decides how everything looks, so every section comes out
//  the same shape. Page width is 70 columns throughout.
// ============================================================================
const int PAGE = 70;

// Repeating a multi-byte character needs a string, not std::string(n, char).
std::string repeat(const std::string& unit, int times) {
    std::string s;
    for (int i = 0; i < times; ++i) s += unit;
    return s;
}

// 2598960 is unreadable. 2,598,960 is not. This is the single biggest
// readability win in the whole program.
std::string commas(unsigned long long value) {
    std::string digits = std::to_string(value);
    std::string out;
    int count = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        out += digits[static_cast<size_t>(i)];
        if (++count % 3 == 0 && i > 0) out += ',';
    }
    std::reverse(out.begin(), out.end());
    return out;
}

// Section header, with the formula for that section right-aligned.
void part(int number, const std::string& name, const std::string& formula) {
    std::string left = " PART " + std::to_string(number) + "   " + name;
    int pad = PAGE - static_cast<int>(left.size() + formula.size());
    if (pad < 1) pad = 1;
    std::cout << "\n" << repeat("=", PAGE) << "\n"
              << left << std::string(static_cast<size_t>(pad), ' ') << formula
              << "\n" << repeat("=", PAGE) << "\n";
}

void question(const std::string& text) { std::cout << "\n  Q.  " << text << "\n"; }
void answer  (const std::string& text) { std::cout << "\n  A.  " << text << "\n"; }
void note    (const std::string& text) { std::cout << "\n      " << text << "\n"; }

// A formula checked against brute force. Kept on one tidy line.
void verify(const std::string& what, unsigned long long formula,
            unsigned long long listed) {
    std::cout << "\n  " << (formula == listed ? "ok " : "XX ")
              << std::left << std::setw(30) << what << std::right
              << "formula " << std::setw(8) << commas(formula)
              << "   listed " << std::setw(8) << commas(listed) << "\n";
}

// "1 in N" - the form people actually think in. Short odds keep a decimal
// (1 in 3.2 is honest), long odds get rounded and comma'd (1 in 13,983,816).
std::string oneInForm(double p) {
    if (p <= 0.0) return "never";
    double inv = 1.0 / p;
    if (inv < 100.0) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << inv;
        return ss.str();
    }
    return commas(static_cast<unsigned long long>(inv + 0.5));
}

// P(A) on two lines so nothing ever runs past the 70-column page:
//      P(5 hearts)
//          1,287 / 2,598,960  =  0.00049520   =   1 in 2,020
void probability(const std::string& description,
                 unsigned long long favorable, unsigned long long total) {
    double p = static_cast<double>(favorable) / static_cast<double>(total);
    std::string fraction = commas(favorable) + " / " + commas(total);

    std::cout << "      P(" << description << ")\n"
              << "          " << std::right << std::setw(20) << fraction
              << "  =  " << std::fixed << std::setprecision(8) << p
              << "   =   1 in " << oneInForm(p) << "\n"
              << std::setprecision(4);
}

// Print a list of short strings in neat rows. The separator goes BEFORE each
// item, never after, so no line ever ends in trailing whitespace.
void printRow(const std::vector<std::string>& items, int perRow) {
    for (size_t i = 0; i < items.size(); ++i) {
        if (i % static_cast<size_t>(perRow) == 0) std::cout << "\n      ";
        else                                      std::cout << "  ";
        std::cout << items[i];
    }
    std::cout << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ========================================================================
    part(1, "THE SLOT METHOD", "everything starts here");
    // ========================================================================
    // Draw one empty slot per choice you must make. Write how many options
    // that slot has underneath. Multiply across. That is the whole method.
    question("How many licence plates exist, if a plate is");
    std::cout << "      4 letters followed by 2 digits?\n";

    std::cout << "\n"
        "         ___   ___   ___   ___     ___   ___\n"
        "          26    26    26    26      10    10\n"
        "         |_______________________|  |_________|\n"
        "              4 letter slots         2 digits\n";

    answer("26 x 26 x 26 x 26 x 10 x 10  =  " + commas(power(26, 4) * power(10, 2)));
    note("Every slot kept its full count, because a letter can be");
    std::cout << "      reused - 'AAAA' is a legal plate. That is WITH REPLACEMENT.\n";

    // ========================================================================
    part(2, "WHAT 'REPLACEMENT' MEANS", "the word itself");
    // ========================================================================
    // Two words drive every formula in this lesson, so they are worth one
    // section of their own. The idea is physical, not mathematical.
    question("After I pick something, do I put it BACK?");

    std::cout << "\n"
        "      WITH replacement            WITHOUT replacement\n"
        "      ------------------------    ------------------------\n"
        "      put it back each time       keep it, it is used up\n"
        "      the pool never shrinks      the pool shrinks by 1\n"
        "      you CAN repeat a pick       you can NEVER repeat\n";

    note("A bag holds 3 balls: A B C. Draw 2 of them.");
    std::cout << "\n"
        "      WITH replacement            WITHOUT replacement\n"
        "         draw 1:  3 choices          draw 1:  3 choices\n"
        "         draw 2:  3 choices          draw 2:  2 choices\n"
        "                  (A went back)               (A is gone)\n";

    // Small enough to list both ways in full - which is the whole point.
    const char bag[3] = {'A', 'B', 'C'};
    std::vector<std::string> withBack, withoutBack;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            std::string pick = std::string{bag[i], bag[j]};
            withBack.push_back(pick);
            if (i != j) withoutBack.push_back(pick);   // j == i needs a repeat
        }

    note("Every possible pair, WITH replacement (3 x 3):");
    printRow(withBack, 9);
    verify("2 from 3, with replacement", power(3, 2), withBack.size());

    note("Every possible pair, WITHOUT replacement (3 x 2):");
    printRow(withoutBack, 9);
    verify("2 from 3, without replacement", nPr(3, 2), withoutBack.size());

    std::cout << "\n"
        "      Compare the two lists. WITHOUT replacement is missing exactly\n"
        "      AA, BB and CC - the three picks that need a ball to show up\n"
        "      twice. That is the only difference, and it is why 9 becomes 6.\n";

    // ---- the practical test -------------------------------------------------
    answer("Look at the numbers under the slots. That is the whole test.");
    std::cout << "\n"
        "         ___  ___  ___  ___  ___      all the same\n"
        "          2    2    2    2    2       -> WITH replacement\n"
        "\n"
        "         ___  ___  ___  ___  ___      counting down\n"
        "          52   51   50   49   48      -> WITHOUT replacement\n";

    note("Which is which in real life:");
    std::cout << "\n"
        "         flipping a coin      the coin is not used up      WITH\n"
        "         rolling a die        you can roll 4 twice         WITH\n"
        "         licence plates       'AAAA' is a legal plate      WITH\n"
        "         dealing cards        that card has left the deck  WITHOUT\n"
        "         lottery balls        drawn balls stay out         WITHOUT\n"
        "         picking 3 stocks     you cannot buy one twice     WITHOUT\n";

    note("Rule of thumb: if picking uses the thing up, it is WITHOUT.");

    // ========================================================================
    part(3, "WITH REPLACEMENT, ORDER MATTERS", "n^r");
    // ========================================================================
    // If all r slots always offer all n options, the slot method gives
    // n * n * ... * n = n^r.
    question("Flip a coin 10 times and write down the sequence.");
    std::cout << "      How many different sequences are there?\n";

    std::cout << "\n"
        "         ___  ___  ___  ___  ___  ___  ___  ___  ___  ___\n"
        "          2    2    2    2    2    2    2    2    2    2\n";

    answer("2^10  =  " + commas(power(2, 10)) + " sequences");

    // ---- WHY is it 2^r? Grow the list one flip at a time and watch. --------
    //
    // This is the part that actually explains the exponent. Every sequence we
    // already have splits into TWO when one more flip is added: the old
    // sequence with H stuck on the end, and the same one with T. Nothing else
    // can happen. So the list cannot do anything except double.
    note("But WHY 2^10? Add one flip at a time and watch the list");
    std::cout << "      grow. Each sequence splits in two - itself with H on\n"
                 "      the end, and itself with T:\n\n";

    std::vector<std::string> sequences = {""};      // one empty sequence to start
    for (int flip = 1; flip <= 3; ++flip) {
        std::vector<std::string> grown;
        for (const std::string& s : sequences) {
            grown.push_back(s + "H");               // this sequence, plus heads
            grown.push_back(s + "T");               // the same one, plus tails
        }
        sequences = grown;

        std::cout << "        " << flip << (flip == 1 ? " flip " : " flips")
                  << "  ->  " << std::setw(2) << sequences.size() << ":   ";
        for (size_t i = 0; i < sequences.size(); ++i) {
            if (i > 0) std::cout << (i % 2 == 0 ? "   " : " ");   // gap per parent
            std::cout << sequences[i];
        }
        std::cout << "\n";
    }

    std::cout << "\n"
        "      Read the last row in pairs: HHH HHT came from HH, and\n"
        "      HTH HTT came from HT. Every parent makes exactly 2 children,\n"
        "      so the total can only ever double.\n";

    // The doubling, all the way out to 10 flips.
    note("Keep doubling and you get the answer, with no listing at all:");
    std::cout << "\n        flips        sequences\n";
    for (int r = 1; r <= 10; ++r) {
        std::cout << "         " << std::setw(2) << r
                  << std::setw(17) << commas(power(2, r));
        if (r > 1) std::cout << "   = " << commas(power(2, r - 1)) << " x 2";
        std::cout << "\n";
    }

    // Now the brute-force check, at a size we can still print.
    note("Check it at 5 flips - the formula says 2^5 = 32:");

    sequences.clear();
    for (int mask = 0; mask < 32; ++mask) {
        std::string seq;
        for (int bit = 4; bit >= 0; --bit)
            seq += ((mask >> bit) & 1) ? 'H' : 'T';    // read the bits as H/T
        sequences.push_back(seq);
    }
    printRow(sequences, 8);
    verify("5 coin flips, 2^5", power(2, 5), sequences.size());

    // ---- THE POINT ---------------------------------------------------------
    std::cout << "\n"
        "  SO WHAT?\n"
        "\n"
        "      1. The exponent counts SLOTS, not options. In n^r, the r is\n"
        "         how many times you multiply - one multiply per new flip.\n"
        "\n"
        "      2. Adding one slot does not add a little. It MULTIPLIES.\n"
        "         Ten flips is not twice as many as five, it is 32 times.\n"
        "\n"
        "      3. Straight back into 1.1: all " << power(2, 5) << " are equally likely,\n"
        "         so each exact sequence has probability 1/" << power(2, 5) << ".\n\n";

    probability("flipping exactly HHHHH", 1, power(2, 5));
    probability("flipping exactly HTHTH", 1, power(2, 5));

    std::cout << "\n"
        "      Those are the SAME. A run of five heads feels special and\n"
        "      HTHTH feels ordinary, but as exact sequences they are equally\n"
        "      rare. What makes streaks feel unusual is something else -\n"
        "      there is only 1 way to get five heads, and 10 ways to get\n"
        "      three heads. That gap is what PART 6 is about.\n";

    // ========================================================================
    part(4, "THE FACTORIAL", "n!");
    // ========================================================================
    // Take replacement away. Slot 1 has 3 choices; once used, slot 2 has only
    // 2 left, then 1. The count drops by one each slot because things get used
    // up, and that countdown is exactly what a factorial is.
    question("Arrange the letters A B C in a row, using each once.");

    std::cout << "\n"
        "         ___   ___   ___\n"
        "          3     2     1        3 x 2 x 1  =  3!\n";

    answer("3!  =  " + commas(factorial(3)) + " arrangements");

    // std::next_permutation walks a sorted sequence through every ordering.
    std::vector<char> letters = {'A', 'B', 'C'};
    std::sort(letters.begin(), letters.end());
    std::vector<std::string> arrangements;
    do {
        arrangements.push_back(std::string(letters.begin(), letters.end()));
    } while (std::next_permutation(letters.begin(), letters.end()));
    printRow(arrangements, 6);
    verify("arrangements of ABC", factorial(3), arrangements.size());

    // One more letter, to watch it grow.
    note("Add one letter and it jumps to 4! = " + commas(factorial(4)) + ":");
    std::vector<char> four = {'A', 'B', 'C', 'D'};
    arrangements.clear();
    do {
        arrangements.push_back(std::string(four.begin(), four.end()));
    } while (std::next_permutation(four.begin(), four.end()));
    printRow(arrangements, 8);
    verify("arrangements of ABCD", factorial(4), arrangements.size());

    // ---- why we will never brute force again --------------------------------
    note("n! grows faster than almost anything you have met:");
    std::cout << "\n"
              << "        n" << std::setw(30) << "n!" << "     can you list it?\n"
              << "      " << repeat("-", 58) << "\n";
    const int sizes[] = {3, 5, 10, 15, 20};
    for (int n : sizes) {
        std::cout << "      " << std::setw(3) << n
                  << std::setw(30) << commas(factorial(n)) << "     "
                  << (n <= 5  ? "yes, on this screen"
                     : n <= 10 ? "a long file, maybe"
                               : "no. not ever.") << "\n";
    }
    note("21! does not even fit in a 64-bit integer. That is exactly");
    std::cout << "      why we need formulas instead of loops.\n";

    // ========================================================================
    part(5, "WITHOUT REPLACEMENT, ORDER MATTERS", "n! / (n-r)!");
    // ========================================================================
    // "Order matters" sounds abstract until you give each slot a NAME.
    // Five named players is order made physical: the same five cards landing
    // on different people is a different deal, and everyone can see that.
    question("Deal one card each to five players, going round in");
    std::cout << "      this order. How many different DEALS are there?\n";

    std::cout << "\n"
        "          Ann   Ben   Cal   Dee   Eve\n"
        "          ___   ___   ___   ___   ___\n"
        "          52    51    50    49    48\n";

    note("Why the numbers fall: Ann picks from a full deck, 52 ways.");
    std::cout << "      Her card is now GONE, so Ben only has 51 left, then Cal\n"
                 "      has 50. That is WITHOUT REPLACEMENT - the deck shrinks.\n";

    note("Why ORDER MATTERS here: these two deals are different,");
    std::cout << "      even though the same five cards came out of the deck.\n\n"
        "          Ann   Ben   Cal   Dee   Eve\n"
        "           K     Q     2     3     7      <- Ann wins with the King\n"
        "           Q     K     2     3     7      <- Ben wins with the King\n\n"
        "      Ann cares a great deal which of those happened. So we count\n"
        "      them as two separate outcomes.\n";

    answer("52 x 51 x 50 x 49 x 48  =  " + commas(nPr(52, 5)) + " deals");

    // NOTE: we write "#" for "how many ways", never P. In this course P is
    // already taken - it means probability. Mixing them up is a real trap.
    note("As a formula, that countdown is a ratio of factorials.");
    std::cout << "      The 47! cancels everything below 48:\n\n"
        "                52!       52 x 51 x 50 x 49 x 48 x 47 x ... x 1\n"
        "           # = ----- =   ---------------------------------------\n"
        "                47!                47 x 46 x ... x 1\n"
        "\n"
        "             = 52 x 51 x 50 x 49 x 48\n"
        "\n"
        "      ('#' means 'how many ways'. Never P - here P means probability.)\n";

    // Prove the formula at a size we can actually see: 3 from 5 letters.
    note("Shrink it: pick 3 from A B C D E, order matters.");
    std::cout << "      Formula says 5 x 4 x 3 = " << nPr(5, 3)
              << ". Three nested loops,\n"
              << "      skipping any repeat, list them all:\n";

    const char pool[5] = {'A', 'B', 'C', 'D', 'E'};
    std::vector<std::string> ordered;
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j) {
            if (j == i) continue;                        // no replacement
            for (int k = 0; k < 5; ++k) {
                if (k == i || k == j) continue;          // no replacement
                ordered.push_back(std::string{pool[i], pool[j], pool[k]});
            }
        }
    printRow(ordered, 12);
    verify("pick 3 of 5, ordered", nPr(5, 3), ordered.size());

    // ========================================================================
    part(6, "ORDER IGNORED", "n! / ((n-r)! r!)  =  nCr");
    // ========================================================================
    // A poker hand is a SET of cards. The order they arrived in is irrelevant,
    // so the ordered count above counted every hand many times over. How many
    // times? Exactly r!, the number of ways to shuffle the r cards you hold.
    question("Same deal as PART 5, but now nobody cares WHO got what.");
    std::cout << "      You pick up your five cards and fan them out. How many\n"
                 "      different HANDS can you be holding?\n";

    note("A hand is a SET. Once the cards are in your hand, the");
    std::cout << "      order they arrived in is gone forever - you cannot tell\n"
                 "      by looking. Say you are holding  K Q 2 3 7.  Every one\n"
                 "      of these deals leaves you with exactly that hand:\n\n";

    // Do not assert the 120. Show it. These are real orderings of one hand,
    // generated the same way we generated arrangements back in PART 4.
    std::vector<std::string> hand = {"2", "3", "7", "K", "Q"};
    std::sort(hand.begin(), hand.end());
    std::vector<std::string> deals;
    do {
        std::string one;
        for (size_t i = 0; i < hand.size(); ++i) {
            if (i) one += " ";
            one += hand[i];
        }
        deals.push_back(one);
    } while (std::next_permutation(hand.begin(), hand.end()));

    for (size_t i = 0; i < 6; ++i)
        std::cout << "          " << deals[i] << "\n";
    std::cout << "          ... and " << commas(deals.size() - 6) << " more.\n";

    note("How many orderings is that? It is just PART 4 again -");
    std::cout << "      arranging 5 things you already hold:  5! = "
              << commas(factorial(5)) << "\n";
    verify("orderings of one poker hand", factorial(5), deals.size());

    note("So PART 5 counted every hand " + commas(factorial(5)) + " separate times,");
    std::cout << "      once per ordering. To count HANDS instead of DEALS,\n"
                 "      divide that duplication back out:\n";

    answer(commas(nPr(52, 5)) + " deals / " + commas(factorial(5))
           + " orderings  =  " + commas(nCr(52, 5)) + " hands");

    std::cout << "\n"
        "      The two questions side by side:\n\n"
        "          PART 5   how many DEALS?   " << std::setw(14)
              << commas(nPr(52, 5)) << "\n"
        "          PART 6   how many HANDS?   " << std::setw(14)
              << commas(nCr(52, 5)) << "\n\n"
        "      One question apart - 'does it matter who got it?' - and a\n"
        "      factor of " << commas(factorial(5)) << " between the answers.\n";

    // See the double counting happen at a size small enough to print in full.
    note("Same thing at a size you can check by eye. Pick 3 from");
    std::cout << "      A B C D E, order ignored. Same loops as PART 5, but force\n"
              << "      i < j < k so each group can appear only once:\n\n";

    unsigned long long groups = 0;
    for (int i = 0; i < 5; ++i)
        for (int j = i + 1; j < 5; ++j)                  // j starts AFTER i
            for (int k = j + 1; k < 5; ++k) {            // k starts AFTER j
                ++groups;
                std::vector<char> group = {pool[i], pool[j], pool[k]};
                std::cout << "         {" << group[0] << group[1] << group[2]
                          << "}   holds   ";
                std::sort(group.begin(), group.end());
                bool first = true;
                do {
                    if (!first) std::cout << " ";     // space BEFORE, never after
                    first = false;
                    std::cout << std::string(group.begin(), group.end());
                } while (std::next_permutation(group.begin(), group.end()));
                std::cout << "\n";
            }

    note(commas(groups) + " groups, each hiding 3! = " + commas(factorial(3))
         + " orderings.");
    std::cout << "      " << groups << " x " << factorial(3) << " = "
              << groups * factorial(3) << ", the ordered count from PART 5.\n";
    verify("pick 3 of 5, unordered", nCr(5, 3), groups);

    note("That i < j < k trick IS the formula. Forcing an order on");
    std::cout << "      the indices is the same thing as dividing by r!.\n";

    // ========================================================================
    part(7, "THE WHOLE LESSON IN ONE TABLE", "n things, pick r");
    // ========================================================================
    std::cout << "\n"
        "                            order matters      order ignored\n"
        "        --------------------+----------------+------------------\n"
        "         with replacement   |      n^r       |  a later lesson\n"
        "        --------------------+----------------+------------------\n"
        "         without            |   n!/(n-r)!    |  n!/((n-r)! r!)\n"
        "                            |                |  written nCr\n"
        "        --------------------+----------------+------------------\n";

    note("Same n, same r, three different answers. With n = 5, r = 3:");
    std::cout << "\n"
              << "         with replacement, ordered    5^3     = "
              << std::setw(6) << commas(power(5, 3)) << "\n"
              << "         without, ordered             5!/2!   = "
              << std::setw(6) << commas(nPr(5, 3)) << "\n"
              << "         without, unordered           5C3     = "
              << std::setw(6) << commas(nCr(5, 3)) << "\n";
    note("Reading the question carefully is most of the work.");

    // ========================================================================
    part(8, "BACK TO PROBABILITY", "P(A) = ways A / ways total");
    // ========================================================================
    // Counting was never the goal. It was the tool. Everything below is still
    // 1.1's formula, with combinatorics filling in the two numbers.

    // ---- coins ---------------------------------------------------------------
    // Total = 2^5 (with replacement, ordered - each flip is its own slot).
    // Favorable = choose WHICH 3 of the 5 slots are heads = 5C3. Which slots
    // we choose is a set, not a sequence, so it is a combination.
    std::cout << "\n  COINS   exactly 3 heads in 5 flips\n\n"
              << "      total sequences         2^5  = " << commas(power(2, 5)) << "\n"
              << "      which 3 slots are H     5C3  = " << commas(nCr(5, 3)) << "\n";

    unsigned long long threeHeads = 0;
    for (int mask = 0; mask < 32; ++mask) {
        int heads = 0;
        for (int bit = 0; bit < 5; ++bit) if ((mask >> bit) & 1) ++heads;
        if (heads == 3) ++threeHeads;
    }
    verify("5C3 vs all 32 sequences", nCr(5, 3), threeHeads);
    std::cout << "\n";
    probability("exactly 3 heads in 5 flips", nCr(5, 3), power(2, 5));

    // ---- cards ---------------------------------------------------------------
    std::cout << "\n  CARDS   all five cards are Hearts\n\n"
              << "      hands of hearts only   13C5  = " << commas(nCr(13, 5)) << "\n"
              << "      hands in total         52C5  = " << commas(nCr(52, 5)) << "\n\n";
    probability("all five cards are Hearts", nCr(13, 5), nCr(52, 5));

    // ---- lottery -------------------------------------------------------------
    std::cout << "\n  LOTTERY   pick 6 numbers from 49, order irrelevant\n\n"
              << "      tickets possible       49C6  = " << commas(nCr(49, 6)) << "\n\n";
    probability("jackpot with one ticket", 1, nCr(49, 6));

    // ---- market --------------------------------------------------------------
    // Keeping 1.1's fourth world. Same maths, different story.
    std::cout << "\n  MARKET   a 3-stock portfolio from a watchlist of 8\n\n"
              << "      if buying ORDER mattered  8P3  = " << commas(nPr(8, 3)) << "\n"
              << "      it does not, it is a set  8C3  = " << commas(nCr(8, 3)) << "\n\n";
    probability("guessing the best trio", 1, nCr(8, 3));
    note(commas(nPr(8, 3)) + " / 3! = " + commas(nCr(8, 3))
         + ", the divide-by-r! trick again.");

    // ========================================================================
    part(9, "WHAT YOU LEARNED", "1.2 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE SLOT METHOD\n"
        "          one slot per choice, options underneath, multiply across.\n"
        "          Every formula in this lesson comes from reading a diagram.\n"
        "\n"
        "      THE FACTORIAL\n"
        "          n! = n x (n-1) x ... x 2 x 1, the ways to arrange all n.\n"
        "\n"
        "      ASK TWO QUESTIONS\n"
        "          can I reuse a pick?    ->  with / without replacement\n"
        "          does order matter?     ->  sequence / set\n"
        "\n"
        "                            order matters      order ignored\n"
        "          with replacement       n^r           a later lesson\n"
        "          without              n!/(n-r)!       n!/((n-r)! r!) = nCr\n"
        "\n"
        "      THE KEY MOVE\n"
        "          ordered count / r! = unordered count, because every group\n"
        "          hides r! shuffles of itself.\n"
        "\n"
        "      AND ALWAYS\n"
        "          P(A) = ways A can happen / things that can happen.\n"
        "          Counting is only the tool. 1.1 was the goal.\n"
        "\n"
        "      COMING LATER\n"
        "          the axioms, P(not A) = 1 - P(A), conditional probability,\n"
        "          Bayes' theorem.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
