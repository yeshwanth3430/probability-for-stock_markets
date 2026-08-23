// ============================================================================
//  1.9 - BAYES' THEOREM
//
//  Build & run:
//      g++ -std=c++17 -Wall -Wextra -O2 "1.9 Bayes' Theorem (with Example).cpp" -o p19
//      ./p19
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.1  P(A) = ways A can happen / things that can happen
//  1.2  counting those ways
//  1.3  the language of sets, and the word PARTITION
//  1.4  the complement trick, P(A) = 1 - P(A^c)
//  1.5  hypergeometric and multinomial, on a factory floor
//  1.6  the binomial, and the four boxes it has to tick
//  1.7  conditioning: P(A|B) = P(A and B) / P(B)
//  1.8  putting it back: P(A) = sum P(A|Bi) P(Bi)
//  1.9  reversing the bar                            <- you are here
//
//  1.7 ended with a warning: P(A|B) is not P(B|A). 1.8 kept computing the
//  reversal anyway and refusing to name it. This is the lesson where it
//  gets its name, and the name is Bayes' theorem.
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA
//  ---------------------------------------------------------------------------
//  You know how the world produces evidence. You want to know what the
//  evidence says about the world. Those are opposite directions, and
//  Bayes' theorem is the exchange rate between them.
//
//      you KNOW        P(positive test | sick)     forwards - the mechanism
//      you WANT        P(sick | positive test)     backwards - the diagnosis
//
//      you KNOW        P(this evidence | guilty)
//      you WANT        P(guilty | this evidence)
//
//      you KNOW        P(these returns | the strategy works)
//      you WANT        P(the strategy works | these returns)
//
//  Every one of those pairs is a forwards fact and a backwards question.
//  Science, medicine, law and finance all run on the second column and
//  can only ever measure the first.
//
//  ---------------------------------------------------------------------------
//  THE THEOREM
//  ---------------------------------------------------------------------------
//                    P(B|A) x P(A)
//      P(A|B)  =  --------------------
//                        P(B)
//
//  and the denominator is almost always expanded with 1.8's law:
//
//      P(B)  =  P(B|A) P(A)  +  P(B|A^c) P(A^c)
//
//  The derivation is one line of algebra, done in Part 2. The hard part
//  of Bayes has never been the maths - it is believing the answer.
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
//  A HYPOTHESIS, AND WHAT THE EVIDENCE DOES TO IT
//
//      prior       P(Hi)         what you believed before
//      likelihood  P(E | Hi)     how well this hypothesis explains E
//
//  Bayes turns those two columns into a third: the posterior, P(Hi | E).
//  The struct is 1.8's Branch with new job titles, because it is literally
//  the same two numbers - that is the point of putting 1.8 first.
// ============================================================================
struct Hypothesis {
    std::string name;
    double prior;        // P(Hi)
    double likelihood;   // P(E | Hi)
};

double priorSum(const std::vector<Hypothesis>& hs) {
    double s = 0.0;
    for (const auto& h : hs) s += h.prior;
    return s;
}

// P(E) - the EVIDENCE, sometimes called the marginal likelihood.
// This is exactly 1.8's law of total probability. It is the only piece of
// Bayes that takes any work, and it is work you already did.
double evidence(const std::vector<Hypothesis>& hs) {
    double p = 0.0;
    for (const auto& h : hs) p += h.likelihood * h.prior;
    return p;
}

// P(Hi | E) - the POSTERIOR. Prior times likelihood, normalised.
double posterior(const std::vector<Hypothesis>& hs, size_t i) {
    const double e = evidence(hs);
    if (e <= 0.0) return 0.0;                  // the evidence was impossible
    return hs[i].likelihood * hs[i].prior / e;
}

// THE ODDS FORM.  posterior odds = prior odds x likelihood ratio.
// Same theorem with the denominator cancelled away - see Part 8. Odds of
// p are p/(1-p); the conversions back and forth are one line each.
double toOdds(double p)      { return p >= 1.0 ? INFINITY : p / (1.0 - p); }
double fromOdds(double odds) { return odds / (1.0 + odds); }

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 through 1.8)
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

// THE UPDATE TABLE - prior, likelihood, product, posterior, in one place.
// Reading left to right across a row is the whole of Bayes' theorem.
void showUpdate(const std::vector<Hypothesis>& hs, const std::string& evidenceName) {
    std::cout << "\n        " << std::left << std::setw(16) << "hypothesis"
              << std::setw(10) << "prior" << std::setw(14)
              << ("P(" + evidenceName + "|H)") << std::setw(13) << "product"
              << "posterior\n" << std::right
              << "        " << repeat("-", 62) << "\n";
    for (size_t i = 0; i < hs.size(); ++i) {
        std::cout << "        " << std::left << std::setw(16) << hs[i].name
                  << std::right << std::setw(7) << hs[i].prior
                  << std::setw(13) << hs[i].likelihood
                  << std::setw(13) << hs[i].prior * hs[i].likelihood
                  << std::setw(13) << posterior(hs, i) << "\n";
    }
    std::cout << "        " << repeat("-", 62) << "\n"
              << "        " << std::left << std::setw(16) << "TOTAL" << std::right
              << std::setw(7) << priorSum(hs) << std::setw(26)
              << evidence(hs) << std::setw(13) << 1.0 << "\n"
              << "        " << std::string(43, ' ') << "^ P(" << evidenceName
              << "), from 1.8\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ========================================================================
    part(1, "THE BACKWARDS QUESTION", "1.7 left this on the table");
    // ========================================================================
    question("A test for a disease is 99% accurate. You test positive.");
    std::cout << "      Do you have the disease?\n";

    note("Almost everyone answers 99%, including - in study after");
    std::cout << "      study - most doctors. The right answer is that the\n"
                 "      question CANNOT BE ANSWERED as asked, because it is\n"
                 "      missing a fact. Supply that fact and the answer lands\n"
                 "      anywhere from 99% to under 1%.\n";

    note("Which fact is missing? How common the disease is. Part 7");
    std::cout << "      holds the test at 99% and moves only that number, and\n"
                 "      the answer swings across the entire range.\n";

    note("The mistake is not arithmetic. It is answering a");
    std::cout << "      different question from the one asked:\n"
        "\n"
        "         '99% accurate' means   P(test + | you are sick)\n"
        "         you want to know       P(you are sick | test +)\n"
        "\n"
        "      1.7 Part 8 showed those are different numbers. It did not\n"
        "      say how to get from one to the other. That is today.\n";

    note("The shape of the problem is always the same. There is a");
    std::cout << "      direction the world runs in, and it is not the direction\n"
                 "      you need:\n"
        "\n"
        "         FORWARDS  cause -> evidence      what you can measure\n"
        "         BACKWARDS evidence -> cause      what you actually want\n"
        "\n"
        "      A lab can measure how often sick people test positive. It\n"
        "      cannot measure how often positive people are sick, because\n"
        "      that depends on who walked through the door.\n";

    // ========================================================================
    part(2, "THE DERIVATION", "one line, and you have seen both halves");
    // ========================================================================
    question("Where does the theorem come from?");

    note("1.7's multiplication rule, written twice. P(A and B) is");
    std::cout << "      one number, and there are two ways to build it:\n"
        "\n"
        "         P(A and B)  =  P(A|B) x P(B)          condition on B\n"
        "         P(A and B)  =  P(B|A) x P(A)          condition on A\n"
        "\n"
        "      Both equal the same thing, so they equal each other:\n"
        "\n"
        "         P(A|B) x P(B)  =  P(B|A) x P(A)\n"
        "\n"
        "      Divide by P(B):\n"
        "\n"
        "                     P(B|A) x P(A)\n"
        "         P(A|B)  =  ---------------          BAYES' THEOREM\n"
        "                         P(B)\n";

    answer("That is the entire derivation. It is one division.");

    note("Which should be unsettling rather than reassuring. A");
    std::cout << "      result this cheap has no business being this hard to\n"
                 "      believe - and yet Part 6 will produce a number that\n"
                 "      almost nobody accepts on first reading.\n";

    // ========================================================================
    part(3, "THE FOUR NAMES", "prior, likelihood, evidence, posterior");
    // ========================================================================
    question("Every term in that fraction has a name. Learn them -");
    std::cout << "      they are how people talk about this.\n";

    std::cout << "\n"
        "                     P(E|H)  x  P(H)\n"
        "         P(H|E)  =  -----------------\n"
        "                          P(E)\n"
        "\n"
        "         P(H)     PRIOR        what you believed before the\n"
        "                               evidence arrived\n"
        "\n"
        "         P(E|H)   LIKELIHOOD   how well this hypothesis explains\n"
        "                               the evidence you got\n"
        "\n"
        "         P(E)     EVIDENCE     how likely that evidence was at\n"
        "                               all, across every hypothesis\n"
        "\n"
        "         P(H|E)   POSTERIOR    what you believe now\n";

    note("Read the whole thing as a sentence and it stops being a");
    std::cout << "      formula:\n"
        "\n"
        "         new belief  =  old belief  x  how well it explains\n"
        "                                       what you just saw\n"
        "\n"
        "         ...divided by however well EVERYTHING explains it.\n"
        "\n"
        "      A hypothesis gains ground when it predicts the evidence\n"
        "      better than its rivals do. That is all Bayes says.\n";

    note("One warning about the word LIKELIHOOD. In ordinary");
    std::cout << "      English it is a synonym for probability. Here it is a\n"
                 "      specific one - P(evidence | hypothesis), never the other\n"
                 "      way round. Mixing the two up recreates exactly the error\n"
                 "      this lesson exists to fix.\n";

    // ========================================================================
    part(4, "THE DENOMINATOR IS 1.8", "which is why 1.8 came first");
    // ========================================================================
    question("P(E) - how do you ever get that number?");

    note("You do not measure it. You build it, with the law of");
    std::cout << "      total probability, out of the same two columns you\n"
                 "      already have:\n"
        "\n"
        "         P(E)  =  P(E|H) P(H)  +  P(E|H^c) P(H^c)\n"
        "\n"
        "      That is 1.8, unchanged. Which makes the full theorem:\n"
        "\n"
        "                          P(E|H) P(H)\n"
        "         P(H|E)  =  ------------------------------\n"
        "                     P(E|H) P(H) + P(E|H^c) P(H^c)\n";

    note("Look at what that fraction is doing. The numerator is");
    std::cout << "      one branch of 1.8's sum; the denominator is the whole\n"
                 "      sum. Bayes' theorem asks: OF ALL THE WAYS this evidence\n"
                 "      could have arisen, what fraction came through H?\n";

    note("Which means the posterior is a SHARE OF THE BLAME. 1.8");
    std::cout << "      Part 7 already computed exactly this for three suppliers\n"
                 "      and called it 'share of all defects'. Same arithmetic,\n"
                 "      formal name.\n";

    // Show the identity between the two framings.
    const std::vector<Hypothesis> suppliers = {
        {"supplier X", 0.50, 0.02},
        {"supplier Y", 0.30, 0.05},
        {"supplier Z", 0.20, 0.11},
    };
    note("1.8's suppliers, re-labelled with today's vocabulary:");
    showUpdate(suppliers, "bad");
    verifyClose("priors partition", priorSum(suppliers), 1.0, 1e-12, "must be");
    double post = 0.0;
    for (size_t i = 0; i < suppliers.size(); ++i) post += posterior(suppliers, i);
    verifyClose("posteriors partition", post, 1.0, 1e-12, "must be");

    note("The posteriors add to 1 as well, and for the same");
    std::cout << "      reason: given a defect, it came from SOMEWHERE, and the\n"
                 "      suppliers are a partition. Every Bayes calculation ends\n"
                 "      with a distribution, not a lone number.\n";

    // ========================================================================
    part(5, "COUNT PEOPLE, NOT PROBABILITIES", "the trick that makes it obvious");
    // ========================================================================
    // The single most effective way to teach this, and it is not algebra.
    question("Before the formula - can we make the answer obvious?");

    const double PREV = 0.01;    // 1 in 100 have it
    const double SENS = 0.99;    // P(+ | sick)
    const double SPEC = 0.99;    // P(- | well)
    const long long POP = 10000;

    const long long sick  = static_cast<long long>(POP * PREV);
    const long long well  = POP - sick;
    const long long tp    = static_cast<long long>(sick * SENS);
    const long long fn    = sick - tp;
    const long long fp    = static_cast<long long>(well * (1.0 - SPEC));
    const long long tn    = well - fp;

    note("Yes, and the trick is to stop using percentages. Take");
    std::cout << "      10,000 actual people and walk them through:\n";

    std::cout << "\n"
        "         10,000 people\n"
        "              |\n"
        "              +-- " << std::setw(5) << sick << " are sick        (1 in 100)\n"
        "              |        +-- " << std::setw(4) << tp << " test POSITIVE   (99% of them)\n"
        "              |        +-- " << std::setw(4) << fn << " test negative\n"
        "              |\n"
        "              +-- " << std::setw(5) << well << " are well\n"
        "                       +-- " << std::setw(4) << fp << " test POSITIVE   (1% of them)\n"
        "                       +-- " << std::setw(4) << tn << " test negative\n";

    note("Now just look at the positives. There are " + commas(static_cast<unsigned long long>(tp + fp))
         + " of them,");
    std::cout << "      and only " << tp << " are actually sick:\n";
    std::cout << "\n";
    showFrac("P(sick | tested positive)", tp, tp + fp);

    answer("Under 1 in 2 of the positives are real - and it is only 50%");
    note("here because I was generous. Part 6 uses a rarer disease");
    std::cout << "      and the number collapses further.\n";

    note("Nothing was hidden. The false positives come from a");
    std::cout << "      group " << well / sick << " times larger, so even a 1% error rate\n"
                 "      on that group produces " << fp << " of them - as many as the\n"
                 "      " << tp << " true positives the test correctly caught.\n";

    note("This is worth doing every single time before you touch");
    std::cout << "      the algebra. Natural frequencies turn a problem people\n"
                 "      reliably get wrong into one they reliably get right,\n"
                 "      and the arithmetic is multiplication and one division.\n";

    // ========================================================================
    part(6, "NOW THE ALGEBRA", "same answer, general machinery");
    // ========================================================================
    question("Do the counting and the theorem agree?");

    const std::vector<Hypothesis> test = {
        {"sick",  PREV,       SENS},
        {"well",  1.0 - PREV, 1.0 - SPEC},
    };

    note("Set it up. H is 'sick', E is 'tested positive':");
    std::cout << "\n"
        "         prior       P(sick)          =  " << PREV << "\n"
        "         likelihood  P(+ | sick)      =  " << SENS << "\n"
        "         the rival   P(+ | well)      =  " << 1.0 - SPEC
              << "     <- the false positive rate\n";

    showUpdate(test, "+");

    note("Step by step, so nothing is hidden:");
    std::cout << "\n"
        "         numerator    P(+|sick) P(sick)  =  " << SENS << " x " << PREV
              << "  =  " << SENS * PREV << "\n"
        "\n"
        "         denominator  P(+|sick)P(sick) + P(+|well)P(well)\n"
        "                      = " << SENS * PREV << " + " << (1.0 - SPEC) * (1.0 - PREV)
              << "  =  " << evidence(test) << "\n"
        "\n"
        "         divide       " << SENS * PREV << " / " << evidence(test)
              << "  =  " << posterior(test, 0) << "\n";

    std::cout << "\n";
    showP("P(sick | tested +)", posterior(test, 0));
    showP("P(well | tested +)", posterior(test, 1));
    verifyClose("algebra vs headcount", posterior(test, 0),
                static_cast<double>(tp) / (tp + fp), 1e-9, "counted ");

    answer("Identical. The formula is the headcount, written compactly.");

    note("And the update, stated plainly: you walked in with a 1%");
    std::cout << "      chance of being sick and walked out with "
              << posterior(test, 0) * 100 << "%.\n"
                 "      The test moved you by a factor of " << posterior(test, 0) / PREV
              << " - which is\n"
                 "      real, substantial evidence. It is just not certainty,\n"
                 "      and it never claimed to be.\n";

    // ========================================================================
    part(7, "WHY IT FEELS WRONG", "the prior is doing the work");
    // ========================================================================
    // The intuition, which is more valuable than the formula.
    question("Why does a 99% accurate test give a 50% answer?");

    note("Because 'accurate' describes the test, and the question");
    std::cout << "      is about the PEOPLE. Two forces pull on the answer:\n"
        "\n"
        "         the PRIOR       how common the disease is\n"
        "                         - pulls the answer down\n"
        "\n"
        "         the LIKELIHOOD  how sharply the test separates\n"
        "                         sick from well - pulls it up\n"
        "\n"
        "      When a disease is rare, the prior is a very strong force,\n"
        "      and a merely-good test cannot overcome it.\n";

    note("Hold the test at 99% and vary only how common the");
    std::cout << "      disease is:\n";
    std::cout << "\n        how common        P(sick)   P(sick | tested +)\n"
              << "        " << repeat("-", 58) << "\n";
    for (double prev : {0.00001, 0.0001, 0.001, 0.01, 0.05, 0.20, 0.50}) {
        const std::vector<Hypothesis> h = {{"sick", prev, SENS},
                                           {"well", 1.0 - prev, 1.0 - SPEC}};
        std::cout << "        1 in " << std::left << std::setw(12)
                  << commas(static_cast<unsigned long long>(1.0 / prev + 0.5))
                  << std::right << std::setw(8) << prev << std::setw(15)
                  << posterior(h, 0) << bar(posterior(h, 0), 40) << "\n";
    }

    answer("Same test, answers from 0.1% to 99%. The test never changed.");

    note("This is BASE RATE NEGLECT, and it is not a maths error -");
    std::cout << "      it is forgetting that a question about people needs a\n"
                 "      fact about people. The test's accuracy is genuinely\n"
                 "      irrelevant on its own.\n";

    note("Now hold the disease at 1 in 1,000 and improve the test");
    std::cout << "      instead. Watch which knob actually matters:\n";
    std::cout << "\n        false + rate    P(sick | tested +)\n"
              << "        " << repeat("-", 58) << "\n";
    for (double fpr : {0.10, 0.05, 0.01, 0.005, 0.001, 0.0001}) {
        const std::vector<Hypothesis> h = {{"sick", 0.001, SENS},
                                           {"well", 0.999, fpr}};
        std::cout << "        " << std::setw(10) << fpr << std::setw(16)
                  << posterior(h, 0) << bar(posterior(h, 0), 40) << "\n";
    }

    note("The false positive rate is the lever, not the catch");
    std::cout << "      rate. Missing sick people is bad for those people;\n"
                 "      wrongly flagging well people is what destroys the\n"
                 "      meaning of a positive result, because there are so\n"
                 "      many well people to flag. 1.10 is entirely about this.\n";

    // ========================================================================
    part(8, "THE ODDS FORM", "the denominator cancels");
    // ========================================================================
    // Genuinely the most useful version for thinking, and it is three lines
    // from what we already have.
    question("Is there a version you can do in your head?");

    note("Yes, and it comes from writing Bayes twice - once for H");
    std::cout << "      and once for H^c - then dividing. P(E) is in both, so\n"
                 "      it cancels and never has to be computed:\n"
        "\n"
        "         P(H|E)      P(E|H)      P(H)\n"
        "         -------  =  -------  x  ------\n"
        "         P(H^c|E)    P(E|H^c)    P(H^c)\n"
        "\n"
        "         posterior     likelihood     prior\n"
        "           odds     =    ratio     x   odds\n";

    answer("Belief updating is multiplication, once you work in odds.");

    note("Run our test through it. Prior odds of being sick are");
    std::cout << "      1 to 99, and the test's likelihood ratio is:\n"
        "\n"
        "         LR  =  P(+|sick) / P(+|well)  =  " << SENS << " / " << 1.0 - SPEC
              << "  =  " << SENS / (1.0 - SPEC) << "\n";

    const double priorOdds = toOdds(PREV);
    const double lr = SENS / (1.0 - SPEC);
    std::cout << "\n"
        "         prior odds       " << priorOdds << "   (about 1 to 99)\n"
        "         x LR             " << lr << "\n"
        "         = posterior odds " << priorOdds * lr << "   (about 1 to 1)\n"
        "         as a probability " << fromOdds(priorOdds * lr) << "\n";
    verifyClose("odds form vs Bayes", fromOdds(priorOdds * lr),
                posterior(test, 0), 1e-9, "Bayes   ");

    note("The likelihood ratio is the whole evidential value of a");
    std::cout << "      test, in one number. LR = 99 means the positive result\n"
                 "      is 99 times more likely from a sick person than a well\n"
                 "      one - and that is a property of the TEST alone, with no\n"
                 "      reference to how common the disease is.\n";

    std::cout << "\n        likelihood ratio   what it means\n"
              << "        " << repeat("-", 58) << "\n"
              << "        LR > 10            strong evidence for H\n"
              << "        LR near 1          the evidence is nearly useless\n"
              << "        LR < 0.1           strong evidence against H\n"
              << "        LR = 1             independent - tells you nothing\n";

    note("That last row is 1.7's independence, arriving from a new");
    std::cout << "      direction: if the evidence is equally likely either way,\n"
                 "      the posterior odds equal the prior odds and nothing\n"
                 "      moves. 1.11 takes that idea apart properly.\n";

    // ========================================================================
    part(9, "UPDATING TWICE", "today's posterior is tomorrow's prior");
    // ========================================================================
    question("You test positive. They run it again. It is positive");
    std::cout << "      again. Now what?\n";

    note("Feed the answer back in. Your posterior after the first");
    std::cout << "      test becomes your prior for the second - which is the\n"
                 "      property that makes Bayes a LEARNING rule rather than\n"
                 "      a one-off calculation:\n";

    std::cout << "\n        test #   prior going in   posterior coming out\n"
              << "        " << repeat("-", 58) << "\n";
    double belief = PREV;
    for (int t = 1; t <= 4; ++t) {
        const std::vector<Hypothesis> step = {{"sick", belief, SENS},
                                              {"well", 1.0 - belief, 1.0 - SPEC}};
        const double after = posterior(step, 0);
        std::cout << "        " << std::setw(4) << t << std::setw(16) << belief
                  << std::setw(20) << after << bar(after, 30) << "\n";
        belief = after;
    }

    answer("Two positives take you from 1% to 99%. Evidence compounds.");

    note("In odds form this is obvious - you are just multiplying");
    std::cout << "      by the likelihood ratio again:\n"
        "\n"
        "         after 1 test    " << toOdds(PREV) << " x " << lr << "  =  "
              << toOdds(PREV) * lr << "\n"
        "         after 2 tests   " << toOdds(PREV) << " x " << lr << "^2  =  "
              << toOdds(PREV) * lr * lr << "\n"
        "         after 3 tests   " << toOdds(PREV) << " x " << lr << "^3  =  "
              << toOdds(PREV) * lr * lr * lr << "\n";
    {
        const std::vector<Hypothesis> once = {{"sick", PREV, SENS},
                                              {"well", 1.0 - PREV, 1.0 - SPEC}};
        const double after1 = posterior(once, 0);
        const std::vector<Hypothesis> twice = {{"sick", after1, SENS},
                                               {"well", 1.0 - after1, 1.0 - SPEC}};
        verifyClose("two updates = LR^2", posterior(twice, 0),
                    fromOdds(toOdds(PREV) * lr * lr), 1e-9, "odds    ");
    }

    note("A serious caveat, and it is the reason 1.11 exists. That");
    std::cout << "      compounding assumes the two tests are INDEPENDENT given\n"
                 "      your true state. If the test failed because of something\n"
                 "      about your blood chemistry, it will fail the same way\n"
                 "      twice, the second result carries almost no new\n"
                 "      information, and this table is far too confident.\n";

    note("Which is why real protocols retest with a DIFFERENT");
    std::cout << "      method rather than the same one. It is a direct,\n"
                 "      practical consequence of a probability assumption.\n";

    // ========================================================================
    part(10, "A REAL PROBLEM", "is the strategy any good?");
    // ========================================================================
    question("A manager shows you 3 straight winning years. Are they");
    std::cout << "      skilled, or lucky?\n";

    note("This is Bayes with the labels changed. Say 10% of");
    std::cout << "      managers genuinely have skill. A skilled one beats the\n"
                 "      market 70% of years; an unskilled one, 50% - a coin.\n";

    const double SKILL_PRIOR = 0.10;
    const double P_WIN_SKILLED = 0.70, P_WIN_LUCKY = 0.50;

    std::cout << "\n        years won   P(skilled | that record)\n"
              << "        " << repeat("-", 58) << "\n";
    for (int years = 0; years <= 6; ++years) {
        const std::vector<Hypothesis> mgr = {
            {"skilled",   SKILL_PRIOR,       std::pow(P_WIN_SKILLED, years)},
            {"unskilled", 1.0 - SKILL_PRIOR, std::pow(P_WIN_LUCKY, years)},
        };
        std::cout << "        " << std::setw(6) << years << std::setw(18)
                  << posterior(mgr, 0) << bar(posterior(mgr, 0), 45) << "\n";
    }

    answer("Three winning years moves 10% to about 25%. Not a verdict.");

    note("The likelihood ratio per year is small - a winning");
    std::cout << "      year is only " << P_WIN_SKILLED / P_WIN_LUCKY
              << " times more likely under skill,\n"
                 "      You need a long record before a track record means\n"
                 "      much - which is 1.6's sqrt(n) lesson again, in the\n"
                 "      language of belief instead of the language of spread.\n";

    note("And the survivorship trap the industry runs on: this");
    std::cout << "      calculation is for a manager chosen BEFORE the record.\n"
                 "      If you picked them precisely because they won 3 years,\n"
                 "      you conditioned on the evidence when you selected, and\n"
                 "      the prior you should use is not 10% at all.\n";

    // How many years to be convinced?
    int yearsNeeded = 0;
    while (yearsNeeded < 100) {
        const std::vector<Hypothesis> mgr = {
            {"skilled",   SKILL_PRIOR,       std::pow(P_WIN_SKILLED, yearsNeeded)},
            {"unskilled", 1.0 - SKILL_PRIOR, std::pow(P_WIN_LUCKY, yearsNeeded)},
        };
        if (posterior(mgr, 0) >= 0.95) break;
        ++yearsNeeded;
    }
    note("To reach 95% confidence in skill you would need about");
    std::cout << "      " << yearsNeeded << " consecutive winning years. Almost nobody has\n"
                 "      a career that long, which is a real and uncomfortable\n"
                 "      result rather than a rhetorical flourish.\n";

    // ========================================================================
    part(11, "DO NOT TRUST ME, SIMULATE IT", "1,000,000 people");
    // ========================================================================
    question("Build a million people, test them all, then look only");
    std::cout << "      at the ones who tested positive. No Bayes in the loop.\n";

    std::mt19937 rng(1729);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    const int PEOPLE = 1000000;

    int positives = 0, positiveAndSick = 0, sickCount = 0;
    int negatives = 0, negativeAndSick = 0;
    for (int i = 0; i < PEOPLE; ++i) {
        const bool isSick = unit(rng) < PREV;                  // nature decides
        if (isSick) ++sickCount;
        const bool testsPos = isSick ? (unit(rng) < SENS)      // then the test
                                     : (unit(rng) < 1.0 - SPEC);
        if (testsPos) { ++positives; if (isSick) ++positiveAndSick; }
        else          { ++negatives; if (isSick) ++negativeAndSick; }
    }

    std::cout << "\n        people                       " << commas(PEOPLE) << "\n"
              << "        actually sick                " << commas(static_cast<unsigned long long>(sickCount)) << "\n"
              << "        tested positive              " << commas(static_cast<unsigned long long>(positives)) << "\n"
              << "        positive AND sick            " << commas(static_cast<unsigned long long>(positiveAndSick)) << "\n"
              << "        positive but well            " << commas(static_cast<unsigned long long>(positives - positiveAndSick)) << "\n";

    std::cout << "\n";
    verifyClose("P(sick), the prior", PREV,
                static_cast<double>(sickCount) / PEOPLE, 5e-3, "simulated");
    verifyClose("P(+), the evidence", evidence(test),
                static_cast<double>(positives) / PEOPLE, 5e-3, "simulated");
    verifyClose("P(sick | +), posterior", posterior(test, 0),
                static_cast<double>(positiveAndSick) / positives, 5e-3, "simulated");

    answer("Bayes' theorem is what tallying the survivors would have told you.");

    note("The negative side, which nobody worries about and");
    std::cout << "      probably should worry about less:\n";
    std::cout << "\n";
    showFrac("P(sick | tested NEGATIVE)", negativeAndSick, negatives);
    note("A negative result IS strongly reassuring here - because");
    std::cout << "      now the rare disease and the accurate test pull the SAME\n"
                 "      way instead of against each other.\n";

    // ========================================================================
    part(12, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    std::cout << "\n      Put your own beliefs in. Every answer shows working.\n"
                 "\n"
                 "        1   the test      - prior, sensitivity, false + rate\n"
                 "        2   headcount     - the same thing in whole people\n"
                 "        3   odds form     - prior odds x likelihood ratio\n"
                 "        4   update again  - feed the posterior back in\n"
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
                         "      1 test   2 headcount   3 odds   4 update   5 quit\n";
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 5, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        if (choice == 5) { note("Done. Run it again any time."); break; }

        // Every mode wants the same three numbers, so ask once.
        int perX = 0, sens = 0, fpr = 0;
        if (!askNumber("Prior: 1 in how many have it", 1, 1000000, perX)) break;
        if (!askNumber("Sensitivity P(+|has it), in %", 1, 100, sens))    break;
        if (!askNumber("False positive rate P(+|has not), in %", 0, 99, fpr)) break;

        const double prior = 1.0 / perX;
        const double sensP = sens / 100.0;
        const double fprP  = fpr / 100.0;
        const std::vector<Hypothesis> mine = {
            {"has it",  prior,       sensP},
            {"has not", 1.0 - prior, fprP},
        };

        if (fprP <= 0.0 && choice != 2) {
            note("A zero false positive rate makes a positive conclusive.");
            std::cout << "\n";
            showP("P(has it | tested +)", posterior(mine, 0));
            continue;
        }

        switch (choice) {
        case 1: {
            showUpdate(mine, "+");
            std::cout << "\n"
                "         numerator    " << sensP << " x " << prior << "  =  "
                      << sensP * prior << "\n"
                "         denominator  " << sensP << " x " << prior << "  +  "
                      << fprP << " x " << 1.0 - prior << "  =  " << evidence(mine) << "\n"
                "         divide       " << posterior(mine, 0) << "\n";
            std::cout << "\n";
            showP("P(has it | tested +)", posterior(mine, 0));
            showP("P(has not | tested +)", posterior(mine, 1));
            std::cout << "\n      You went in at " << prior * 100 << "% and came out at "
                      << posterior(mine, 0) * 100 << "%,\n      a factor of "
                      << posterior(mine, 0) / prior << ".\n";
            break;
        }
        case 2: {
            const long long pop = 1000000;
            const long long has = static_cast<long long>(pop * prior);
            const long long hasNot = pop - has;
            const long long TP = static_cast<long long>(has * sensP);
            const long long FP = static_cast<long long>(hasNot * fprP);
            std::cout << "\n"
                "         " << commas(pop) << " people\n"
                "              |\n"
                "              +-- " << std::setw(7) << has << " have it\n"
                "              |        +-- " << std::setw(7) << TP << " test POSITIVE\n"
                "              |        +-- " << std::setw(7) << has - TP << " test negative\n"
                "              |\n"
                "              +-- " << std::setw(7) << hasNot << " do not\n"
                "                       +-- " << std::setw(7) << FP << " test POSITIVE\n"
                "                       +-- " << std::setw(7) << hasNot - FP << " test negative\n";
            std::cout << "\n";
            if (TP + FP == 0) { note("Nobody tests positive at all."); break; }
            showFrac("P(has it | tested +)", TP, TP + FP);
            std::cout << "\n      Of every 100 positives, about "
                      << static_cast<int>(100.0 * TP / (TP + FP) + 0.5)
                      << " are real.\n";
            verifyClose("headcount vs formula", posterior(mine, 0),
                        static_cast<double>(TP) / (TP + FP), 5e-3, "counted ");
            break;
        }
        case 3: {
            const double po = toOdds(prior), ratio = sensP / fprP;
            std::cout << "\n"
                "         prior odds        " << po << "   (1 to " << perX - 1 << ")\n"
                "         likelihood ratio  " << sensP << " / " << fprP << "  =  "
                      << ratio << "\n"
                "         posterior odds    " << po << " x " << ratio << "  =  "
                      << po * ratio << "\n";
            std::cout << "\n";
            showP("as a probability", fromOdds(po * ratio));
            verifyClose("odds vs Bayes", fromOdds(po * ratio), posterior(mine, 0),
                        1e-9, "Bayes   ");
            std::cout << "\n      ";
            if (ratio > 10)      std::cout << "LR above 10 - strong evidence.\n";
            else if (ratio > 2)  std::cout << "LR between 2 and 10 - moderate evidence.\n";
            else if (ratio > 1)  std::cout << "LR just above 1 - weak evidence.\n";
            else                 std::cout << "LR at or below 1 - a positive means nothing.\n";
            break;
        }
        case 4: {
            int rounds = 0;
            if (!askNumber("How many positive tests in a row", 1, 10, rounds)) {
                keepGoing = false; break;
            }
            std::cout << "\n        test #   prior going in   posterior out\n"
                      << "        " << repeat("-", 58) << "\n";
            double belief = prior;
            for (int t = 1; t <= rounds; ++t) {
                const std::vector<Hypothesis> step = {{"has it", belief, sensP},
                                                      {"has not", 1.0 - belief, fprP}};
                const double after = posterior(step, 0);
                std::cout << "        " << std::setw(4) << t << std::setw(16) << belief
                          << std::setw(17) << after << bar(after, 30) << "\n";
                belief = after;
            }
            std::cout << "\n";
            showP("belief after " + std::to_string(rounds) + " positives", belief);
            verifyClose("same as LR^n", belief,
                        fromOdds(toOdds(prior) * std::pow(sensP / fprP, rounds)),
                        1e-6, "odds    ");
            note("This assumes the tests are independent given the truth.");
            std::cout << "      Repeating the SAME test rarely satisfies that - see\n"
                         "      Part 9, and 1.11 next.\n";
            break;
        }
        default: break;
        }
    }

    // ========================================================================
    part(13, "WHAT YOU LEARNED", "1.9 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE THEOREM - one line of algebra from 1.7\n"
        "                     P(E|H) x P(H)\n"
        "          P(H|E) = -----------------\n"
        "                          P(E)\n"
        "          because P(H|E)P(E) and P(E|H)P(H) are both P(H and E)\n"
        "\n"
        "      THE FOUR NAMES\n"
        "          P(H)     PRIOR        what you believed before\n"
        "          P(E|H)   LIKELIHOOD   how well H explains E\n"
        "          P(E)     EVIDENCE     how likely E was at all\n"
        "          P(H|E)   POSTERIOR    what you believe now\n"
        "\n"
        "      THE DENOMINATOR IS 1.8\n"
        "          P(E) = P(E|H)P(H) + P(E|H^c)P(H^c)\n"
        "          so the posterior is a SHARE OF THE BLAME: of all the\n"
        "          ways E could have happened, what fraction went via H\n"
        "\n"
        "      COUNT PEOPLE, NOT PERCENTAGES\n"
        "          take 10,000 of them and walk the tree\n"
        "          a problem people reliably get wrong becomes one\n"
        "          they reliably get right, with no algebra at all\n"
        "\n"
        "      WHY A 99% TEST GIVES A 50% ANSWER\n"
        "          'accurate' is about the test, the answer is about\n"
        "          the people - and the well group is huge\n"
        "          ignoring the prior is BASE RATE NEGLECT\n"
        "          the FALSE POSITIVE rate is the lever, not the catch rate\n"
        "\n"
        "      THE ODDS FORM - Bayes you can do in your head\n"
        "          posterior odds = prior odds x likelihood ratio\n"
        "          LR = P(E|H) / P(E|H^c), a property of the test alone\n"
        "          LR = 1 means the evidence is independent of H\n"
        "\n"
        "      UPDATING IS ITERATIVE\n"
        "          today's posterior is tomorrow's prior\n"
        "          n independent observations multiply n likelihood ratios\n"
        "          'independent' is load-bearing, and often false\n"
        "\n"
        "      COMING NEXT\n"
        "          1.10 the drug testing example, worked end to end -\n"
        "               the same theorem where the stakes are somebody's job\n"
        "          1.11 independence, which every result above quietly\n"
        "               assumed at least once\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
