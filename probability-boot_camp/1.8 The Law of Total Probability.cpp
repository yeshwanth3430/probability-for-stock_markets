// ============================================================================
//  1.8 - THE LAW OF TOTAL PROBABILITY
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.8 The Law of Total Probability.cpp" -o p18
//      ./p18
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.1  P(A) = ways A can happen / things that can happen
//  1.2  counting those ways: n^r, nPr, nCr, with/without replacement
//  1.3  the language of sets - and the word PARTITION
//  1.4  the complement trick, P(A) = 1 - P(A^c)
//  1.5  a crate on a dock: hypergeometric and the multinomial
//  1.6  the binomial, and the four boxes it has to tick
//  1.7  conditioning: P(A|B) = P(A and B) / P(B)
//  1.8  putting the conditionals back together       <- you are here
//
//  1.7 took P(A) apart. This lesson puts it back together, and the joins
//  are the conditional probabilities you just learned to compute.
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA
//  ---------------------------------------------------------------------------
//  You almost never measure P(A) directly. You measure it in PIECES,
//  because the pieces are what the world hands you.
//
//  Nobody knows "the defect rate". Somebody knows machine A's defect rate,
//  somebody else knows machine B's, and a third person knows how much each
//  machine ran. The law of total probability is how those three facts
//  become the one number you were asked for:
//
//      P(A)  =  P(A|B1) P(B1)  +  P(A|B2) P(B2)  +  ...  +  P(A|Bn) P(Bn)
//
//  In words: split the world into cases, work out A's chance inside each
//  case, and average them - WEIGHTED by how likely each case is.
//
//  ---------------------------------------------------------------------------
//  THE PRICE OF ADMISSION
//  ---------------------------------------------------------------------------
//  The cases B1...Bn have to form a PARTITION, which is 1.3's word and
//  which means exactly two things:
//
//      DISJOINT     no two cases can both happen    (no double counting)
//      EXHAUSTIVE   at least one case must happen   (nothing left out)
//
//  Break either and the law quietly returns a wrong number rather than an
//  error. Part 8 breaks it on purpose so you know what that looks like.
//
//  ---------------------------------------------------------------------------
//  WHERE IT IS GOING
//  ---------------------------------------------------------------------------
//  Bayes' theorem is next, and this law is its denominator. Every Bayes
//  calculation you will ever do spends most of its effort right here.
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
//  A BRANCH OF THE PARTITION
//
//  One case, and the two numbers that case contributes:
//      weight  P(Bi)      how likely this case is at all
//      rate    P(A|Bi)    how likely A is once you are inside it
//
//  Almost every problem in this lesson is a list of these. Writing it as a
//  struct is not decoration - it is the shape of the law.
// ============================================================================
struct Branch {
    std::string name;
    double weight;   // P(Bi)
    double rate;     // P(A | Bi)
};

// THE LAW ITSELF. Multiply along each branch, add across all of them.
// Two lines of code, and the rest of this file is about deserving them.
double totalProbability(const std::vector<Branch>& branches) {
    double p = 0.0;
    for (const auto& b : branches) p += b.rate * b.weight;
    return p;
}

// The partition check. If these weights do not add to 1 you do not have a
// partition, and the number the law hands back means nothing.
double weightSum(const std::vector<Branch>& branches) {
    double s = 0.0;
    for (const auto& b : branches) s += b.weight;
    return s;
}

// Running the law BACKWARDS: given that A happened, which branch was it?
//
//      P(Bi | A)  =  P(A|Bi) P(Bi)  /  P(A)
//
// That is Bayes' theorem, and the denominator is the function above. It
// appears here because it costs three lines and it shows why this lesson
// exists - but 1.9 is where it gets its own name and its own argument.
double reverseBranch(const std::vector<Branch>& branches, size_t i) {
    const double total = totalProbability(branches);
    if (total <= 0.0) return 0.0;
    return branches[i].rate * branches[i].weight / total;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.7)
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

// ============================================================================
//  THE WORKING TABLE - one row per branch, and the total underneath.
//
//  This layout IS the law: the last column is P(A|Bi) x P(Bi), and the
//  number at the bottom is their sum. Print it and the algebra is done.
// ============================================================================
void showBranches(const std::vector<Branch>& branches, const std::string& caseLabel,
                  const std::string& rateLabel) {
    std::cout << "\n        " << std::left << std::setw(16) << caseLabel
              << std::setw(12) << "P(case)" << std::setw(14) << rateLabel
              << "product\n" << std::right
              << "        " << repeat("-", 58) << "\n";
    for (const auto& b : branches) {
        std::cout << "        " << std::left << std::setw(16) << b.name << std::right
                  << std::setw(8) << b.weight << std::setw(14) << b.rate
                  << std::setw(15) << b.rate * b.weight << "\n";
    }
    std::cout << "        " << repeat("-", 58) << "\n"
              << "        " << std::left << std::setw(16) << "TOTAL" << std::right
              << std::setw(8) << weightSum(branches) << std::setw(29)
              << totalProbability(branches) << "\n";
}

// The same thing as a picture. Multiply going across, add going down.
void showTree(const std::vector<Branch>& branches, const std::string& eventName) {
    std::cout << "\n        start\n";
    for (size_t i = 0; i < branches.size(); ++i) {
        const Branch& b = branches[i];
        const bool last = (i + 1 == branches.size());
        std::cout << "          |\n"
                  << "          +-- " << std::left << std::setw(14) << b.name
                  << std::right << "P = " << b.weight << "\n"
                  << (last ? "        " : "          |     ")
                  << (last ? "        |\n" : "|\n");
        const int leafWidth = static_cast<int>(eventName.size()) + 6;
        std::cout << (last ? "                " : "          |     ")
                  << "+-- " << std::left << std::setw(leafWidth) << eventName
                  << std::right << b.weight << " x " << b.rate << " = "
                  << b.weight * b.rate << "   <-- add\n";
        std::cout << (last ? "                " : "          |     ")
                  << "+-- " << std::left << std::setw(leafWidth) << ("not " + eventName)
                  << std::right << b.weight << " x " << 1.0 - b.rate << " = "
                  << b.weight * (1.0 - b.rate) << "\n";
    }
    std::cout << "\n        adding the marked leaves:  "
              << totalProbability(branches) << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // 1.7's factory, exactly as we left it. Continuity is the point: this
    // lesson does not need a new example, it needs a new way to read one.
    const int A_DEF = 30,  A_OK = 570;   // machine A: 600 parts, 5% bad
    const int B_DEF = 60,  B_OK = 340;   // machine B: 400 parts, 15% bad
    const int A_TOT = A_DEF + A_OK, B_TOT = B_DEF + B_OK;
    const int DEF   = A_DEF + B_DEF;
    const int TOTAL = A_TOT + B_TOT;

    const std::vector<Branch> factory = {
        {"machine A", static_cast<double>(A_TOT) / TOTAL,
                      static_cast<double>(A_DEF) / A_TOT},
        {"machine B", static_cast<double>(B_TOT) / TOTAL,
                      static_cast<double>(B_DEF) / B_TOT},
    };

    // ========================================================================
    part(1, "THE QUESTION", "you have the pieces, not the whole");
    // ========================================================================
    question("What is this factory's defect rate?");

    note("Notice that nobody in the building knows. Ask around and");
    std::cout << "      you get three answers, none of which is the one you\n"
                 "      wanted:\n"
        "\n"
        "         the engineer on machine A   'we run about 5% defective'\n"
        "         the engineer on machine B   'we run about 15%'\n"
        "         the scheduler               'A does 60% of the volume'\n"
        "\n"
        "      Three facts. None of them is the defect rate. Together\n"
        "      they determine it completely.\n";

    note("Say those three facts in the notation of 1.7 and the");
    std::cout << "      shape of the answer becomes obvious:\n"
        "\n"
        "         P(defective | A) = 0.05      a CONDITIONAL\n"
        "         P(defective | B) = 0.15      a CONDITIONAL\n"
        "         P(A) = 0.60, P(B) = 0.40     the WEIGHTS\n"
        "\n"
        "      You are being handed conditionals and asked for the\n"
        "      unconditional. That is this entire lesson.\n";

    note("This is not a contrived shortage of information - it is");
    std::cout << "      the normal state of affairs. Measurements come from\n"
                 "      inside one machine, one region, one age group, one\n"
                 "      market regime. The overall number is almost always\n"
                 "      something you have to ASSEMBLE.\n";

    // ========================================================================
    part(2, "THE PARTITION", "1.3's word, finally doing a job");
    // ========================================================================
    // The law's preconditions come first, because they are the part people
    // skip and the part that bites.
    question("Before any arithmetic - what makes 'machine A or B' a");
    std::cout << "      legitimate way to slice this problem?\n";

    note("Two properties, both from 1.3, and both required:");
    std::cout << "\n"
        "         DISJOINT     no part came off both machines\n"
        "                      P(A and B) = 0\n"
        "                      -> nothing gets counted twice\n"
        "\n"
        "         EXHAUSTIVE   every part came off one of them\n"
        "                      P(A) + P(B) = 1\n"
        "                      -> nothing gets left out\n"
        "\n"
        "      A family of cases with both properties is a PARTITION.\n"
        "      It cuts the sample space into pieces that fit together\n"
        "      with no overlap and no gaps.\n";

    std::cout << "\n"
        "         +---------------------------------------------+\n"
        "         |                     |                       |\n"
        "         |     machine A       |      machine B        |\n"
        "         |       0.60          |        0.40           |\n"
        "         |                     |                       |\n"
        "         |   +-------------+   |    +--------------+   |\n"
        "         |   | defective   |   |    | defective    |   |\n"
        "         |   |   0.03      |   |    |    0.06      |   |\n"
        "         |   +-------------+   |    +--------------+   |\n"
        "         |                     |                       |\n"
        "         +---------------------------------------------+\n"
        "                    the whole sample space = 1\n";

    note("The event 'defective' is the shaded piece, and it is cut");
    std::cout << "      in two by the partition. That picture is the proof of\n"
                 "      the law - the shaded area is just its parts added up.\n";

    verifyClose("weights form a partition", weightSum(factory), 1.0, 1e-12,
                "must be");

    // ========================================================================
    part(3, "THE LAW", "three lines, and you already own all of them");
    // ========================================================================
    question("Now derive it. No new ideas are allowed.");

    note("STEP 1 - split the event along the partition. A defective");
    std::cout << "      part came from A or from B, and it cannot be both, so\n"
                 "      by 1.3's addition rule for disjoint sets:\n"
        "\n"
        "         P(def)  =  P(def and A)  +  P(def and B)\n";

    note("STEP 2 - rewrite each piece with 1.7's multiplication");
    std::cout << "      rule, P(X and Y) = P(X|Y) P(Y):\n"
        "\n"
        "         P(def and A)  =  P(def|A) x P(A)\n"
        "         P(def and B)  =  P(def|B) x P(B)\n";

    note("STEP 3 - put them back. That is the whole law:");
    std::cout << "\n"
        "         P(def)  =  P(def|A) P(A)  +  P(def|B) P(B)\n"
        "\n"
        "      and for n cases instead of 2, the same line with more\n"
        "      terms:\n"
        "\n"
        "         P(A)  =  sum over i of  P(A | Bi) x P(Bi)\n";

    answer("A weighted average of the conditional rates. Nothing more.");

    note("Put the factory's numbers in:");
    std::cout << "\n"
        "         P(def)  =  0.05 x 0.60  +  0.15 x 0.40\n"
        "                 =  " << 0.05 * 0.60 << "      +  " << 0.15 * 0.40 << "\n"
        "                 =  " << totalProbability(factory) << "\n";

    showBranches(factory, "case", "P(def|case)");

    note("Read that table as the law. Column 2 times column 3 is");
    std::cout << "      column 4; the sum of column 4 is the answer. Every\n"
                 "      problem in this lesson is that table with different\n"
                 "      numbers in it.\n";

    std::cout << "\n";
    showP("P(defective)", totalProbability(factory));
    verifyClose("against the raw count", totalProbability(factory),
                static_cast<double>(DEF) / TOTAL, 1e-12, "90/1000 ");

    // ========================================================================
    part(4, "WHY NOT JUST AVERAGE THE RATES?", "because the weights matter");
    // ========================================================================
    // The single most common way this goes wrong in practice, and it is
    // worth its own part because the wrong answer looks so reasonable.
    question("5% and 15%. Why is the answer not 10%?");

    note("Because the machines did not run the same amount. The");
    std::cout << "      naive average silently assumes they did:\n";
    std::cout << "\n"
        "         plain average    (0.05 + 0.15) / 2      =  "
              << (0.05 + 0.15) / 2 << "\n"
        "         weighted (law)   0.05(0.60) + 0.15(0.40) =  "
              << totalProbability(factory) << "\n";

    note("The gap is small here because 60/40 is nearly even. Watch");
    std::cout << "      what happens when the volumes are lopsided - same two\n"
                 "      machines, same two rates, only the schedule changes:\n";

    std::cout << "\n        A's share   P(defective)   plain average   error\n"
              << "        " << repeat("-", 58) << "\n";
    for (double share : {0.05, 0.25, 0.50, 0.60, 0.75, 0.95}) {
        const std::vector<Branch> what = {{"A", share, 0.05},
                                          {"B", 1.0 - share, 0.15}};
        const double truth = totalProbability(what);
        std::cout << "        " << std::setw(7) << share << std::setw(15) << truth
                  << std::setw(15) << 0.10 << std::setw(11)
                  << std::fabs(truth - 0.10) << bar(std::fabs(truth - 0.10), 200)
                  << "\n";
    }

    answer("The plain average is right only when the branches are equal.");

    note("Which is a special case, not the normal one. The law is");
    std::cout << "      what an average becomes once you admit the cases have\n"
                 "      different sizes - and 'different sizes' describes very\n"
                 "      nearly every real partition you will meet.\n";

    // ========================================================================
    part(5, "DO NOT TRUST ME, COUNT THEM", "all 1,000 parts");
    // ========================================================================
    // The law is short enough to be suspicious of. The factory is small
    // enough to check part by part.
    question("The law says 0.09. What do the actual parts say?");

    note("1.7's table, which we never left:");
    std::cout << "\n                     defective     good      total\n"
              << "        " << repeat("-", 58) << "\n"
              << "        machine A " << std::setw(11) << A_DEF << std::setw(10)
              << A_OK << std::setw(11) << A_TOT << "\n"
              << "        machine B " << std::setw(11) << B_DEF << std::setw(10)
              << B_OK << std::setw(11) << B_TOT << "\n"
              << "        " << repeat("-", 58) << "\n"
              << "        total     " << std::setw(11) << DEF << std::setw(10)
              << A_OK + B_OK << std::setw(11) << TOTAL << "\n";

    note("The law, in counts rather than probabilities:");
    std::cout << "\n"
        "         defects from A   0.05 x 600  =  " << A_DEF << "\n"
        "         defects from B   0.15 x 400  =  " << B_DEF << "\n"
        "         all defects                     " << DEF << "\n";
    std::cout << "\n";
    showFrac("P(defective), counted", DEF, TOTAL);
    verifyCount("defects, law vs table",
                static_cast<unsigned long long>(0.05 * A_TOT + 0.15 * B_TOT),
                static_cast<unsigned long long>(DEF));

    note("So the law is doing something completely mundane: it is");
    std::cout << "      adding up a column. The weights convert each machine's\n"
                 "      RATE back into a COUNT, and counts can simply be added.\n"
                 "      That is why the weights cannot be skipped - without\n"
                 "      them you are adding rates, which is not a thing.\n";

    // The column-total reading, which is the honest intuition.
    note("Same point, said once more, because it is the whole");
    std::cout << "      lesson: a rate times a size is a count.\n";
    std::cout << "\n        machine   rate    x  size   =  defects\n"
              << "        " << repeat("-", 58) << "\n"
              << "        A        " << 0.05 << "     x   " << A_TOT
              << "   =   " << std::setw(6) << A_DEF << "\n"
              << "        B        " << 0.15 << "     x   " << B_TOT
              << "   =   " << std::setw(6) << B_DEF << "\n"
              << "        " << repeat("-", 58) << "\n"
              << "        total                 " << TOTAL << "   =   "
              << std::setw(6) << DEF << "\n";

    // ========================================================================
    part(6, "THE TREE DIAGRAM", "multiply across, add down");
    // ========================================================================
    // The picture most people actually reason with, and it makes the
    // partition conditions visible rather than stated.
    question("Is there a way to see this instead of computing it?");

    note("Draw it as a tree. Branch on the case first, then on the");
    std::cout << "      event. Multiply along a path, add the paths you want:\n";

    showTree(factory, "defective");

    note("Two rules, and they are 1.7 and 1.3 wearing hats:");
    std::cout << "\n"
        "         ALONG a path you MULTIPLY    P(A and B) = P(A|B)P(B)\n"
        "                                      the multiplication rule\n"
        "\n"
        "         ACROSS paths you ADD         disjoint paths, so the\n"
        "                                      addition rule applies\n";

    note("The tree also makes the partition conditions visible");
    std::cout << "      rather than merely stated. DISJOINT means no two paths\n"
                 "      lead to the same leaf. EXHAUSTIVE means the first level\n"
                 "      of branches accounts for everything - which shows up\n"
                 "      as those P values summing to 1.\n";

    // Every leaf, not just the marked ones - because the tree is a
    // distribution and distributions add to 1.
    double leafTotal = 0.0;
    for (const auto& b : factory) leafTotal += b.weight * b.rate
                                             + b.weight * (1.0 - b.rate);
    verifyClose("all four leaves", leafTotal, 1.0, 1e-12, "must be");

    note("All four leaves add to 1, because the leaves are");
    std::cout << "      themselves a partition - a finer one. The law picks out\n"
                 "      the leaves you care about and ignores the rest.\n";

    // ========================================================================
    part(7, "MORE THAN TWO CASES", "the law does not care how many");
    // ========================================================================
    question("Three suppliers instead of two machines. Does anything");
    std::cout << "      change?\n";

    note("Only the number of rows. You buy the same component from");
    std::cout << "      three suppliers, at different volumes and different\n"
                 "      quality:\n";

    const std::vector<Branch> suppliers = {
        {"supplier X", 0.50, 0.02},
        {"supplier Y", 0.30, 0.05},
        {"supplier Z", 0.20, 0.11},
    };

    showBranches(suppliers, "supplier", "P(bad|sup)");
    verifyClose("the shares partition", weightSum(suppliers), 1.0, 1e-12,
                "must be");

    std::cout << "\n";
    showP("P(a random component is bad)", totalProbability(suppliers));

    note("Now the question that makes this worth doing. Z is your");
    std::cout << "      worst supplier by a mile - five times X's defect rate.\n"
                 "      But which supplier causes the most ACTUAL defects?\n";

    std::cout << "\n        supplier   share   rate    defects per 10,000   share of\n"
              << "                                        components       all defects\n"
              << "        " << repeat("-", 58) << "\n";
    for (size_t i = 0; i < suppliers.size(); ++i) {
        const Branch& s = suppliers[i];
        const double per10k = s.weight * s.rate * 10000;
        std::cout << "        " << std::left << std::setw(11) << s.name << std::right
                  << std::setw(7) << s.weight << std::setw(8) << s.rate
                  << std::setw(15) << static_cast<int>(per10k + 0.5)
                  << std::setw(17) << reverseBranch(suppliers, i) << "\n";
    }

    answer("Z is the worst supplier and still not the biggest problem.");

    note("X ships half your volume at a low rate, and that volume");
    std::cout << "      buys it a share of the defect pile comparable to Z's.\n"
                 "      A rate alone never tells you where the damage is - you\n"
                 "      need the rate AND the weight, which is the law's point.\n";

    note("That last column is worth a second look. It answers");
    std::cout << "      'given a defect, who sent it?' - the REVERSED question\n"
                 "      from 1.7 Part 8, and it is Bayes' theorem computed\n"
                 "      quietly in the loop above. Two lessons of foreshadowing\n"
                 "      end in the next video.\n";
    double reverseTotal = 0.0;
    for (size_t i = 0; i < suppliers.size(); ++i) reverseTotal += reverseBranch(suppliers, i);
    verifyClose("blame adds to 1", reverseTotal, 1.0, 1e-12, "must be");

    // ========================================================================
    part(8, "WHEN IT BREAKS", "not a partition, not an answer");
    // ========================================================================
    // The law fails silently. That is exactly why it is worth breaking on
    // purpose, in front of you, with the numbers visible.
    question("What happens if the cases are NOT a partition?");

    note("You get a wrong number and no warning. Nothing throws,");
    std::cout << "      nothing complains - the arithmetic is happy to run on\n"
                 "      cases that overlap or leave gaps.\n";

    note("FAILURE 1 - NOT EXHAUSTIVE. You forgot a supplier. Drop");
    std::cout << "      Z from the list and just... do not mention it:\n";
    const std::vector<Branch> forgotZ = {
        {"supplier X", 0.50, 0.02},
        {"supplier Y", 0.30, 0.05},
    };
    showBranches(forgotZ, "supplier", "P(bad|sup)");
    std::cout << "\n"
        "         weights add to " << weightSum(forgotZ)
              << ", not 1 - that is the tell\n"
        "         the law returns " << totalProbability(forgotZ)
              << " instead of " << totalProbability(suppliers) << "\n"
        "         you have understated your defect rate by "
              << (1.0 - totalProbability(forgotZ) / totalProbability(suppliers)) * 100
              << "%\n";
    note("And notice WHY the error goes the way it does: the");
    std::cout << "      supplier you dropped was the worst one. Missing cases do\n"
                 "      not average out, they bias in whatever direction the\n"
                 "      missing case pointed.\n";

    note("FAILURE 2 - NOT DISJOINT. A component can be dual-sourced,");
    std::cout << "      so it appears under two suppliers. Now the shares add to\n"
                 "      more than 1 and the same parts get counted twice:\n";
    const std::vector<Branch> overlapping = {
        {"supplier X", 0.50, 0.02},
        {"supplier Y", 0.30, 0.05},
        {"supplier Z", 0.20, 0.11},
        {"X and Z both", 0.15, 0.06},
    };
    std::cout << "\n         weights add to " << weightSum(overlapping)
              << ", not 1 - the same tell\n"
                 "         the law returns " << totalProbability(overlapping)
              << ", which is not a probability of anything\n";

    answer("Check the weights add to 1. It is the cheapest test you own.");

    note("That check catches both failures at once, and it costs");
    std::cout << "      one line. It is the same instinct as 1.6's 'does the\n"
                 "      distribution sum to 1' and 1.3's partition rule - and\n"
                 "      in every case it is checking that you have described a\n"
                 "      whole world rather than a piece of one.\n";

    note("One caveat: weights summing to 1 is NECESSARY, not");
    std::cout << "      sufficient. Two overlapping cases can still sum to 1 by\n"
                 "      coincidence. The check is a smoke alarm, not a proof -\n"
                 "      but a smoke alarm you decline to install is worse.\n";

    // ========================================================================
    part(9, "TWO BOXES AND A BALL", "the law in sequence");
    // ========================================================================
    // The textbook shape of the problem, and the one where the partition is
    // a genuine random step rather than a category.
    question("Two boxes on a table. I pick one at random, then draw");
    std::cout << "      one ball from it. What is P(red)?\n";

    const int B1_RED = 3, B1_BLUE = 7;    // box 1: 10 balls, 30% red
    const int B2_RED = 6, B2_BLUE = 4;    // box 2: 10 balls, 60% red

    std::cout << "\n"
        "         box 1     " << B1_RED << " red, " << B1_BLUE << " blue\n"
        "         box 2     " << B2_RED << " red, " << B2_BLUE << " blue\n"
        "         the box is chosen by a fair coin\n";

    const std::vector<Branch> boxes = {
        {"box 1", 0.50, static_cast<double>(B1_RED) / (B1_RED + B1_BLUE)},
        {"box 2", 0.50, static_cast<double>(B2_RED) / (B2_RED + B2_BLUE)},
    };

    note("There is no single 'chance of red' until you say which");
    std::cout << "      box - so condition on the box, which is the partition:\n";

    showBranches(boxes, "which box", "P(red|box)");
    showTree(boxes, "red");

    std::cout << "\n";
    showP("P(red)", totalProbability(boxes));

    note("Here the partition is not a category you imposed - it is");
    std::cout << "      an actual random step in the experiment. That is the\n"
                 "      commonest use of this law: any two-stage process where\n"
                 "      the first stage decides the odds for the second.\n";

    note("Careful with a tempting shortcut. There are 9 red balls");
    std::cout << "      out of 20 on the table, so is P(red) = 9/20 = 0.45?\n"
                 "      It agrees here - but only because both boxes hold 10\n"
                 "      balls. Change that and the shortcut dies:\n";

    const int C1_RED = 3, C1_BLUE = 7;      // box 1: 10 balls
    const int C2_RED = 60, C2_BLUE = 40;    // box 2: 100 balls
    const std::vector<Branch> unevenBoxes = {
        {"box 1", 0.50, static_cast<double>(C1_RED) / (C1_RED + C1_BLUE)},
        {"box 2", 0.50, static_cast<double>(C2_RED) / (C2_RED + C2_BLUE)},
    };
    std::cout << "\n"
        "         box 1     " << C1_RED << " red, " << C1_BLUE << " blue   (10 balls)\n"
        "         box 2     " << C2_RED << " red, " << C2_BLUE << " blue   (100 balls)\n";
    std::cout << "\n";
    showFrac("all balls pooled (WRONG)", C1_RED + C2_RED,
             C1_RED + C1_BLUE + C2_RED + C2_BLUE);
    showP  ("the law (RIGHT)", totalProbability(unevenBoxes));

    answer("Pooling assumes every ball was equally reachable. It was not.");

    note("A ball in box 1 has a much better chance of being drawn");
    std::cout << "      than a ball in box 2, because the boxes were equally\n"
                 "      likely but the box sizes were not. The law knows this;\n"
                 "      pooling does not. This is the same mistake as Part 4's\n"
                 "      plain average, arriving from the opposite direction.\n";

    // The reverse question, which is where this is all heading.
    note("And now the question you actually want to ask. The ball");
    std::cout << "      in my hand is red. Which box did it come from?\n";
    std::cout << "\n";
    showP("P(box 1 | red)", reverseBranch(boxes, 0));
    showP("P(box 2 | red)", reverseBranch(boxes, 1));
    verifyClose("box 1 or box 2, given red",
                reverseBranch(boxes, 0) + reverseBranch(boxes, 1), 1.0, 1e-12,
                "must be");
    note("It started as a 50/50 box and the red ball made it 1/3");
    std::cout << "      against 2/3. That is Bayes' theorem, and the denominator\n"
                 "      it divides by is the P(red) this lesson just built.\n";

    // ========================================================================
    part(10, "A REAL PROBLEM", "the market has regimes");
    // ========================================================================
    // 1.1's fourth world. The law is the honest way to talk about a number
    // everybody quotes as if it were a constant.
    question("What is the chance the market rises tomorrow?");

    note("There is no such number, and quoting one is the mistake.");
    std::cout << "      Markets behave differently in different regimes, and\n"
                 "      'the' probability is a blend of the regime you are in:\n";

    const std::vector<Branch> regimes = {
        {"calm bull",   0.55, 0.62},
        {"choppy",      0.30, 0.51},
        {"stressed",    0.15, 0.38},
    };

    showBranches(regimes, "regime", "P(up|regime)");
    verifyClose("regimes partition", weightSum(regimes), 1.0, 1e-12, "must be");

    std::cout << "\n";
    showP("P(market up tomorrow)", totalProbability(regimes));

    note("That single number is what gets quoted, and on its own it");
    std::cout << "      is nearly useless - it describes an average day in a\n"
                 "      market that is never having an average day. The rows are\n"
                 "      the information; the total is the summary that throws\n"
                 "      the information away.\n";

    note("The law also tells you what your forecast is worth when");
    std::cout << "      you are unsure which regime you are in. Suppose you\n"
                 "      think stress is more likely than usual:\n";

    const std::vector<Branch> worried = {
        {"calm bull",   0.20, 0.62},
        {"choppy",      0.35, 0.51},
        {"stressed",    0.45, 0.38},
    };
    std::cout << "\n";
    showP("P(up), normal regime weights",  totalProbability(regimes));
    showP("P(up), worried regime weights", totalProbability(worried));

    answer("Same conditionals, different weights, a different forecast.");

    note("Nothing about the market changed between those two lines");
    std::cout << "      - only your belief about which world you are in. That\n"
                 "      is the law's real content: your forecast is a weighted\n"
                 "      average of what you know, and the weights are as much\n"
                 "      of the answer as the rates are.\n";

    note("And the reverse question is the one a risk desk asks:");
    std::cout << "      the market fell today - has the regime changed?\n";
    std::cout << "\n        regime        P(regime)   P(regime | market fell)\n"
              << "        " << repeat("-", 58) << "\n";
    std::vector<Branch> fell;
    for (const auto& r : regimes) fell.push_back({r.name, r.weight, 1.0 - r.rate});
    for (size_t i = 0; i < fell.size(); ++i)
        std::cout << "        " << std::left << std::setw(14) << fell[i].name
                  << std::right << std::setw(9) << fell[i].weight
                  << std::setw(19) << reverseBranch(fell, i) << "\n";
    note("Every weight moved, and 'stressed' moved the most. One");
    std::cout << "      down day is weak evidence - but it is evidence, and\n"
                 "      1.9 is the machine that prices it properly.\n";

    // ========================================================================
    part(11, "WHEN THE WEIGHTS FLIP THE ANSWER", "Simpson's paradox");
    // ========================================================================
    // The most spectacular consequence of the law, and it is nothing more
    // than Part 4's lesson taken to its logical end.
    question("Can one treatment beat another in every single group,");
    std::cout << "      and still lose overall?\n";

    note("It sounds impossible. It is not, and it happens in real");
    std::cout << "      data often enough to have a name. Two treatments, two\n"
                 "      kinds of case:\n";

    // Treatment A does better in both subgroups but is given the hard cases.
    const int A_MILD_OK = 81,  A_MILD_N = 87;    // 93%
    const int A_SEV_OK  = 192, A_SEV_N  = 263;   // 73%
    const int B_MILD_OK = 234, B_MILD_N = 270;   // 87%
    const int B_SEV_OK  = 55,  B_SEV_N  = 80;    // 69%

    std::cout << "\n                      mild cases        severe cases\n"
              << "        " << repeat("-", 58) << "\n"
              << "        treatment A   " << std::setw(4) << A_MILD_OK << "/"
              << std::left << std::setw(6) << A_MILD_N << std::right
              << std::setw(7) << static_cast<double>(A_MILD_OK) / A_MILD_N
              << "   " << std::setw(4) << A_SEV_OK << "/" << std::left
              << std::setw(6) << A_SEV_N << std::right << std::setw(7)
              << static_cast<double>(A_SEV_OK) / A_SEV_N << "\n"
              << "        treatment B   " << std::setw(4) << B_MILD_OK << "/"
              << std::left << std::setw(6) << B_MILD_N << std::right
              << std::setw(7) << static_cast<double>(B_MILD_OK) / B_MILD_N
              << "   " << std::setw(4) << B_SEV_OK << "/" << std::left
              << std::setw(6) << B_SEV_N << std::right << std::setw(7)
              << static_cast<double>(B_SEV_OK) / B_SEV_N << "\n";

    note("Read the rows: A wins on mild cases, and A wins on severe");
    std::cout << "      cases. Now pool them and read the overall rates:\n";

    const double aOverall = static_cast<double>(A_MILD_OK + A_SEV_OK)
                          / (A_MILD_N + A_SEV_N);
    const double bOverall = static_cast<double>(B_MILD_OK + B_SEV_OK)
                          / (B_MILD_N + B_SEV_N);
    std::cout << "\n";
    showFrac("treatment A, overall", A_MILD_OK + A_SEV_OK, A_MILD_N + A_SEV_N);
    showFrac("treatment B, overall", B_MILD_OK + B_SEV_OK, B_MILD_N + B_SEV_N);

    std::cout << "\n      A wins mild by "
              << static_cast<double>(A_MILD_OK) / A_MILD_N
                 - static_cast<double>(B_MILD_OK) / B_MILD_N
              << " and severe by "
              << static_cast<double>(A_SEV_OK) / A_SEV_N
                 - static_cast<double>(B_SEV_OK) / B_SEV_N
              << ",\n      and then loses overall by " << bOverall - aOverall
              << ".\n";

    answer("A wins both groups and loses overall. Nothing here is a typo.");

    note("The law explains it in one line: those overall rates are");
    std::cout << "      weighted averages, and the two treatments carry\n"
                 "      DIFFERENT WEIGHTS:\n";
    std::cout << "\n        treatment   share mild   share severe\n"
              << "        " << repeat("-", 58) << "\n"
              << "        A          " << std::setw(11)
              << static_cast<double>(A_MILD_N) / (A_MILD_N + A_SEV_N)
              << std::setw(15)
              << static_cast<double>(A_SEV_N) / (A_MILD_N + A_SEV_N) << "\n"
              << "        B          " << std::setw(11)
              << static_cast<double>(B_MILD_N) / (B_MILD_N + B_SEV_N)
              << std::setw(15)
              << static_cast<double>(B_SEV_N) / (B_MILD_N + B_SEV_N) << "\n";

    note("A was given the severe cases - the ones with the worse");
    std::cout << "      outcomes for anybody. Its overall number is dragged\n"
                 "      down by its patient mix, not by its performance. B\n"
                 "      looks better because it treated easier people.\n";

    // Show the fix: give both treatments the same weights.
    note("The fix is to stop letting the weights differ. Score both");
    std::cout << "      treatments on the SAME case mix - say 50/50 - and the\n"
                 "      contradiction evaporates:\n";
    const std::vector<Branch> fairA = {
        {"mild",   0.50, static_cast<double>(A_MILD_OK) / A_MILD_N},
        {"severe", 0.50, static_cast<double>(A_SEV_OK)  / A_SEV_N},
    };
    const std::vector<Branch> fairB = {
        {"mild",   0.50, static_cast<double>(B_MILD_OK) / B_MILD_N},
        {"severe", 0.50, static_cast<double>(B_SEV_OK)  / B_SEV_N},
    };
    std::cout << "\n";
    showP("treatment A on a 50/50 mix", totalProbability(fairA));
    showP("treatment B on a 50/50 mix", totalProbability(fairB));
    note("A wins, as the subgroup rows said it should all along.");

    note("The moral is Part 4's, sharpened: a rate that pools");
    std::cout << "      across a partition is a weighted average, and comparing\n"
                 "      two such rates compares their weights as much as their\n"
                 "      performance. Whenever somebody shows you one overall\n"
                 "      number, ask what it is averaging over.\n";

    // ========================================================================
    part(12, "DO NOT TRUST ME, SIMULATE IT", "400,000 parts");
    // ========================================================================
    question("Build the factory. No law anywhere in this loop - just");
    std::cout << "      pick a machine, then make a part, and count.\n";

    std::mt19937 rng(1729);
    std::uniform_real_distribution<double> coin(0.0, 1.0);
    const int RUNS = 400000;

    int madeA = 0, defA = 0, defB = 0, defects = 0;
    for (int t = 0; t < RUNS; ++t) {
        const bool fromA = coin(rng) < factory[0].weight;   // pick the machine
        const double rate = fromA ? factory[0].rate : factory[1].rate;
        const bool bad = coin(rng) < rate;                  // then make the part
        if (fromA) ++madeA;
        if (bad) {
            ++defects;
            if (fromA) ++defA; else ++defB;
        }
    }

    std::cout << "\n        parts made                   " << commas(RUNS) << "\n"
              << "        came off machine A           " << commas(static_cast<unsigned long long>(madeA)) << "\n"
              << "        came off machine B           " << commas(static_cast<unsigned long long>(RUNS - madeA)) << "\n"
              << "        defective, from A            " << commas(static_cast<unsigned long long>(defA)) << "\n"
              << "        defective, from B            " << commas(static_cast<unsigned long long>(defB)) << "\n"
              << "        defective, total             " << commas(static_cast<unsigned long long>(defects)) << "\n";

    std::cout << "\n";
    verifyClose("P(machine A)", factory[0].weight,
                static_cast<double>(madeA) / RUNS, 5e-3, "simulated");
    verifyClose("P(def | A)", factory[0].rate,
                static_cast<double>(defA) / madeA, 5e-3, "simulated");
    verifyClose("P(def | B)", factory[1].rate,
                static_cast<double>(defB) / (RUNS - madeA), 5e-3, "simulated");
    verifyClose("P(defective), the law", totalProbability(factory),
                static_cast<double>(defects) / RUNS, 5e-3, "simulated");

    answer("The law's 0.09 comes out of a machine that never heard of it.");

    // The reverse direction, counted rather than derived.
    note("And the reversed question, counted straight off the pile");
    std::cout << "      of defects the loop produced:\n";
    std::cout << "\n";
    verifyClose("P(A | defective)", reverseBranch(factory, 0),
                static_cast<double>(defA) / defects, 5e-3, "simulated");
    verifyClose("P(B | defective)", reverseBranch(factory, 1),
                static_cast<double>(defB) / defects, 5e-3, "simulated");
    note("Bayes' theorem, arrived at by tallying. That is the whole");
    std::cout << "      of the next lesson, and you have now seen it work three\n"
                 "      separate times without being told its name.\n";

    // ========================================================================
    part(13, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Put your own partition in. Every answer shows working.\n"
                 "\n"
                 "        1   two cases     - the classic P(A|B)P(B) blend\n"
                 "        2   three cases   - same law, one more row\n"
                 "        3   two boxes     - pick a box, draw a ball\n"
                 "        4   weights sweep - watch the weights move the answer\n"
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
                         "      1 two cases   2 three cases   3 boxes   4 sweep   5 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 5, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            int w1 = 0, r1 = 0, r2 = 0;
            if (!askNumber("Case 1's share of everything, in %", 0, 100, w1)) {
                keepGoing = false; break;
            }
            if (!askNumber("P(event | case 1), in %", 0, 100, r1)) { keepGoing = false; break; }
            if (!askNumber("P(event | case 2), in %", 0, 100, r2)) { keepGoing = false; break; }

            const std::vector<Branch> mine = {
                {"case 1", w1 / 100.0,         r1 / 100.0},
                {"case 2", (100 - w1) / 100.0, r2 / 100.0},
            };
            std::cout << "\n      Case 2 takes the remaining " << 100 - w1 << "%.\n";
            showBranches(mine, "case", "P(evt|case)");
            verifyClose("weights add to 1", weightSum(mine), 1.0, 1e-9, "must be");
            showTree(mine, "event");
            std::cout << "\n";
            showP("P(event)", totalProbability(mine));
            std::cout << "\n      Plain average of the two rates would say "
                      << (r1 + r2) / 200.0 << ",\n      which is "
                      << (std::fabs(totalProbability(mine) - (r1 + r2) / 200.0) < 1e-9
                          ? "right here only because the cases are even.\n"
                          : "wrong, because the cases are different sizes.\n");
            if (totalProbability(mine) > 0) {
                note("Reversed - given the event happened, which case?");
                std::cout << "\n";
                showP("P(case 1 | event)", reverseBranch(mine, 0));
                showP("P(case 2 | event)", reverseBranch(mine, 1));
            }
            break;
        }
        case 2: {
            int w1 = 0, w2 = 0, r1 = 0, r2 = 0, r3 = 0;
            if (!askNumber("Case 1's share, in %", 0, 100, w1)) { keepGoing = false; break; }
            if (!askNumber("Case 2's share, in %", 0, 100 - w1, w2)) { keepGoing = false; break; }
            if (!askNumber("P(event | case 1), in %", 0, 100, r1)) { keepGoing = false; break; }
            if (!askNumber("P(event | case 2), in %", 0, 100, r2)) { keepGoing = false; break; }
            if (!askNumber("P(event | case 3), in %", 0, 100, r3)) { keepGoing = false; break; }

            const std::vector<Branch> mine = {
                {"case 1", w1 / 100.0,                r1 / 100.0},
                {"case 2", w2 / 100.0,                r2 / 100.0},
                {"case 3", (100 - w1 - w2) / 100.0,   r3 / 100.0},
            };
            std::cout << "\n      Case 3 takes the remaining " << 100 - w1 - w2 << "%.\n";
            showBranches(mine, "case", "P(evt|case)");
            verifyClose("weights add to 1", weightSum(mine), 1.0, 1e-9, "must be");
            std::cout << "\n";
            showP("P(event)", totalProbability(mine));

            if (totalProbability(mine) > 0) {
                note("Given the event happened, where did it come from?");
                std::cout << "\n        case      P(case)   P(case | event)   shifted by\n"
                          << "        " << repeat("-", 58) << "\n";
                for (size_t i = 0; i < mine.size(); ++i) {
                    const double after = reverseBranch(mine, i);
                    std::cout << "        " << std::left << std::setw(10) << mine[i].name
                              << std::right << std::setw(8) << mine[i].weight
                              << std::setw(16) << after << std::setw(14)
                              << std::showpos << after - mine[i].weight
                              << std::noshowpos << "\n";
                }
                note("Cases with a high rate gain; low-rate cases lose.");
            }
            break;
        }
        case 3: {
            int red1 = 0, blue1 = 0, red2 = 0, blue2 = 0, pick1 = 0;
            if (!askNumber("Box 1 - red balls",  0, 100, red1))  { keepGoing = false; break; }
            if (!askNumber("Box 1 - blue balls", 0, 100, blue1)) { keepGoing = false; break; }
            if (!askNumber("Box 2 - red balls",  0, 100, red2))  { keepGoing = false; break; }
            if (!askNumber("Box 2 - blue balls", 0, 100, blue2)) { keepGoing = false; break; }
            if (red1 + blue1 == 0 || red2 + blue2 == 0) {
                note("Both boxes need at least one ball.");
                break;
            }
            if (!askNumber("Chance of picking box 1, in %", 0, 100, pick1)) {
                keepGoing = false; break;
            }

            const std::vector<Branch> mine = {
                {"box 1", pick1 / 100.0,
                          static_cast<double>(red1) / (red1 + blue1)},
                {"box 2", (100 - pick1) / 100.0,
                          static_cast<double>(red2) / (red2 + blue2)},
            };
            std::cout << "\n      box 1  " << red1 << " red, " << blue1
                      << " blue    chosen " << pick1 << "% of the time\n"
                         "      box 2  " << red2 << " red, " << blue2
                      << " blue    chosen " << 100 - pick1 << "% of the time\n";
            showBranches(mine, "which box", "P(red|box)");
            showTree(mine, "red");
            std::cout << "\n";
            showP("P(red)", totalProbability(mine));
            showFrac("all balls pooled (WRONG)", red1 + red2,
                     red1 + blue1 + red2 + blue2);
            const double pooled = static_cast<double>(red1 + red2)
                                / (red1 + blue1 + red2 + blue2);
            if (std::fabs(pooled - totalProbability(mine)) < 1e-9)
                note("They agree - equal boxes, equally likely. A coincidence.");
            else
                note("They disagree, and the law is the one to believe.");

            if (totalProbability(mine) > 0) {
                std::cout << "\n";
                showP("P(box 1 | the ball is red)", reverseBranch(mine, 0));
                showP("P(box 2 | the ball is red)", reverseBranch(mine, 1));
            }
            break;
        }
        case 4: {
            int r1 = 0, r2 = 0;
            if (!askNumber("P(event | case 1), in %", 0, 100, r1)) { keepGoing = false; break; }
            if (!askNumber("P(event | case 2), in %", 0, 100, r2)) { keepGoing = false; break; }

            std::cout << "\n      Holding both rates fixed, sweep case 1's share\n"
                         "      from 0 to 100% and watch P(event) travel:\n";
            std::cout << "\n        case 1 share   P(event)\n"
                      << "        " << repeat("-", 58) << "\n";
            for (int w = 0; w <= 100; w += 10) {
                const std::vector<Branch> mine = {
                    {"case 1", w / 100.0,         r1 / 100.0},
                    {"case 2", (100 - w) / 100.0, r2 / 100.0},
                };
                const double p = totalProbability(mine);
                std::cout << "        " << std::setw(9) << w << "%" << std::setw(14)
                          << p << bar(p, 45) << "\n";
            }
            std::cout << "\n      It runs in a straight line from " << r2 / 100.0
                      << " to " << r1 / 100.0 << ",\n"
                         "      because the law is linear in the weights. The\n"
                         "      answer can never escape the range of the rates you\n"
                         "      put in - a weighted average always lands between\n"
                         "      its smallest and largest ingredient.\n";
            note("Which is a useful sanity check: if your blended number");
            std::cout << "      sits outside every conditional rate you started\n"
                         "      with, you have made an arithmetic mistake.\n";
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
    part(14, "WHAT YOU LEARNED", "1.8 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE LAW - assemble P(A) out of the pieces you can measure\n"
        "          P(A)  =  sum over i of  P(A | Bi) x P(Bi)\n"
        "                   -----------    ---------   ------\n"
        "                   the answer     the rate    the weight\n"
        "                                  inside a    of that\n"
        "                                  case        case\n"
        "\n"
        "      THE PRICE OF ADMISSION - B1...Bn must be a PARTITION\n"
        "          DISJOINT     no overlap, nothing counted twice\n"
        "          EXHAUSTIVE   no gaps, nothing left out\n"
        "          the check: the weights must add to exactly 1\n"
        "\n"
        "      HOW IT IS DERIVED - three lines, no new ideas\n"
        "          split A along the partition          1.3\n"
        "          P(A and Bi) = P(A|Bi) P(Bi)          1.7\n"
        "          add the disjoint pieces              1.3\n"
        "\n"
        "      THE TREE\n"
        "          multiply ALONG a path, add ACROSS paths\n"
        "          every leaf together adds to 1\n"
        "\n"
        "      WHY NOT JUST AVERAGE THE RATES\n"
        "          a plain average assumes equal cases\n"
        "          a rate times a size is a COUNT, and counts add\n"
        "          the blend always lands between the smallest and\n"
        "          largest rate you put in\n"
        "\n"
        "      WHEN IT BREAKS - and it breaks silently\n"
        "          missing a case understates, and biases toward\n"
        "          whatever the missing case was unlike\n"
        "          overlapping cases double-count\n"
        "          weights summing to 1 is necessary, not sufficient\n"
        "\n"
        "      SIMPSON'S PARADOX - the weights can flip a conclusion\n"
        "          a treatment can win every subgroup and lose overall\n"
        "          if it was handed the harder cases\n"
        "          always ask what an overall rate is averaging over\n"
        "\n"
        "      RUNNING IT BACKWARDS\n"
        "                          P(A|Bi) P(Bi)\n"
        "          P(Bi | A)  =  -----------------\n"
        "                         sum of all such\n"
        "          the denominator IS this lesson - which is why this\n"
        "          lesson comes first\n"
        "\n"
        "      COMING NEXT\n"
        "          1.9 Bayes' theorem - the line above, with a name,\n"
        "              an argument, and the reason it matters.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
