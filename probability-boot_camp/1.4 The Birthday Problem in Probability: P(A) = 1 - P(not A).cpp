// ============================================================================
//  1.4 - THE BIRTHDAY PROBLEM:  P(A) = 1 - P(not A)
//
//  Build & run (quote the name, it has spaces and punctuation):
//      g++ -std=c++17 -Wall -Wextra -O2 "1.4 The Birthday Problem in Probability: P(A) = 1 - P(not A).cpp" -o p14
//      ./p14
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.1  P(A) = ways A can happen / things that can happen
//  1.2  how to COUNT those ways: n^r, nPr, nCr
//  1.3  the language of sets: union, intersection, COMPLEMENT
//  1.4  all three at once, on one famous problem            <- you are here
//
//  This lesson is not really about birthdays. It is about the single most
//  useful move in all of probability:
//
//       when a question says "AT LEAST ONE", stop.
//       Do not count it. Count the opposite and subtract.
//
//                    P(A)  =  1 - P(A^c)
//
//  That is just 1.3's complement rule. The birthday problem is simply the
//  most spectacular demonstration of why it matters - the direct route is
//  a nightmare and the flipped route is four lines of arithmetic.
//
//  ---------------------------------------------------------------------------
//  THE ASSUMPTION WE ARE MAKING
//  ---------------------------------------------------------------------------
//  We assume 365 equally likely birthdays: no 29 February, and no seasonal
//  clustering. Both are false in the real world. Real birthdays bunch up in
//  certain months, and bunching makes matches MORE likely - so the famous
//  answer below is, if anything, slightly conservative.
// ============================================================================

#include <cmath>      // std::pow
#include <iomanip>
#include <iostream>
#include <random>     // mt19937, uniform_int_distribution
#include <string>
#include <vector>

// ============================================================================
//  THE MATHS
// ============================================================================
const int DAYS = 365;

// P(everybody in a room of n has a DIFFERENT birthday).
//
// THE IMPORTANT ENGINEERING POINT: the honest formula is 365Pn / 365^n, but
// 365^23 is about 8.5e58 - it overflows a 64-bit integer many times over.
// So we never build either giant number. We multiply the ratios one person
// at a time, and every partial result stays between 0 and 1.
// Same lesson as 1.2: never build a huge number just to divide it away.
double probabilityAllDifferent(int people) {
    double p = 1.0;
    for (int i = 0; i < people; ++i) {
        // Person i arrives. i birthdays are already taken, so (365 - i) of
        // the 365 days still keep everybody distinct.
        p *= static_cast<double>(DAYS - i) / static_cast<double>(DAYS);
        if (p == 0.0) break;                  // past 365 people it is truly 0
    }
    return p;
}

// The headline. "At least one shared birthday" is the complement of
// "all birthdays different", so one subtraction finishes the job.
double probabilityShared(int people) {
    return 1.0 - probabilityAllDifferent(people);
}

// The OTHER question, the one people confuse this with:
// does anybody share MY birthday specifically? Each of the other n-1 people
// misses my day with probability 364/365, independently.
double probabilitySharesWithMe(int people) {
    if (people < 2) return 0.0;
    return 1.0 - std::pow(364.0 / 365.0, people - 1);
}

// C(n,r), lifted straight from 1.2, for the "at least one ace" example.
unsigned long long nCr(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r > n - r) r = n - r;
    unsigned long long result = 1;
    for (int i = 1; i <= r; ++i) {
        result *= static_cast<unsigned long long>(n - r + i);
        result /= static_cast<unsigned long long>(i);
    }
    return result;
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2 and 1.3)
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
    std::string reversed(out.rbegin(), out.rend());
    return reversed;
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

// A probability, as a decimal and as a percentage people can feel.
void showP(const std::string& label, double p) {
    std::cout << "        " << std::left << std::setw(34) << label << std::right
              << std::fixed << std::setprecision(4) << p
              << std::setw(9) << std::setprecision(1) << p * 100.0 << "%\n"
              << std::setprecision(4);
}

// Formula against simulation. Sampling noise is real, so we allow a small
// tolerance and say so rather than pretending the match is exact.
void verifyClose(const std::string& claim, double formula, double simulated,
                 double tolerance) {
    bool ok = std::fabs(formula - simulated) < tolerance;
    std::cout << "\n  " << (ok ? "ok " : "XX ") << std::left << std::setw(28)
              << claim << std::right << "formula " << std::fixed
              << std::setprecision(4) << formula
              << "   simulated " << simulated << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ========================================================================
    part(1, "THE QUESTION", "and why the obvious route dies");
    // ========================================================================
    question("In a room of 23 people, what is the probability that");
    std::cout << "      AT LEAST TWO of them share a birthday?\n";

    note("Most people guess something tiny - a few percent. There");
    std::cout << "      are 365 days and only 23 people, so it feels hopeless.\n"
                 "      Hold on to your guess. We will come back for it.\n";

    note("First, why we cannot just count it directly. 'At least");
    std::cout << "      two share' hides an enormous pile of separate cases:\n"
        "\n"
        "         exactly one pair shares, everyone else distinct\n"
        "         two separate pairs share\n"
        "         three separate pairs share\n"
        "         three people share one day\n"
        "         three share one day AND two share another\n"
        "         four share one day\n"
        "         ... and so on, all the way to all 23 on one day\n"
        "\n"
        "      Every one of those is a different counting problem, and they\n"
        "      all have to be added up without overlapping. Nobody sane\n"
        "      does this.\n";

    // ========================================================================
    part(2, "THE TRICK", "P(A) = 1 - P(A^c)");
    // ========================================================================
    // Straight out of 1.3: A and A^c are disjoint and together they fill S,
    // so their probabilities must add to 1.
    question("What is the OPPOSITE of 'at least two share a birthday'?");

    note("Careful - the opposite is not 'exactly one pair shares'.");
    std::cout << "      The opposite of 'at least one match' is 'NO match at\n"
                 "      all', which means every single birthday is different.\n";

    std::cout << "\n"
        "         A     =  at least two people share      <- messy\n"
        "         A^c   =  all 23 birthdays are different <- one clean case\n"
        "\n"
        "      From 1.3: A and A^c never overlap and together they cover\n"
        "      everything that can happen. So their probabilities add to 1:\n"
        "\n"
        "                    P(A)  =  1 - P(A^c)\n";

    answer("A horrible pile of cases became ONE easy case.");
    note("Whenever a question says 'at least one', reach for this");
    std::cout << "      first. It is the most reliable move in the subject.\n";

    // ========================================================================
    part(3, "COUNTING THE EASY CASE", "1.2 does the work");
    // ========================================================================
    // Both counts are straight from 1.2's sampling table - and they sit in
    // the two cells that differ only by "can we repeat?".
    question("So how likely is it that all 23 birthdays are different?");

    note("Everything that can happen - each person gets any day,");
    std::cout << "      and of course birthdays CAN repeat. WITH replacement:\n"
        "\n"
        "         person1  person2  person3   ...   person 23\n"
        "         ___      ___      ___             ___\n"
        "         365      365      365             365      ->  365^23\n";

    note("The good case - every birthday different. Once Ann takes");
    std::cout << "      a day, Ben cannot have it. The pool shrinks. WITHOUT\n"
                 "      replacement:\n"
        "\n"
        "         person1  person2  person3   ...   person 23\n"
        "         ___      ___      ___             ___\n"
        "         365      364      363             343      ->  365 P 23\n";

    note("Those are the SAME two cells of 1.2's table - order");
    std::cout << "      matters in both, and the only difference is whether a\n"
                 "      day can be reused. Divide one by the other:\n"
        "\n"
        "                        365 P 23        365   364   363         343\n"
        "         P(A^c)  =   -------------  =   --- x --- x --- x ... x ---\n"
        "                        365 ^ 23        365   365   365         365\n";

    note("Never actually build those two numbers - 365^23 is about");
    std::cout << "      8.5e58 and overflows a 64-bit integer many times over.\n"
                 "      Multiply the fractions one at a time instead; each stays\n"
                 "      between 0 and 1. Same lesson as 1.2's factorials.\n";

    note("Watch it fall, person by person:");
    std::cout << "\n        person   their day must miss     P(all different)\n"
              << "        " << repeat("-", 54) << "\n";
    double running = 1.0;
    for (int i = 0; i < 23; ++i) {
        running *= static_cast<double>(DAYS - i) / DAYS;
        if (i < 5 || i >= 20) {
            std::cout << "        " << std::setw(4) << i + 1
                      << std::setw(15) << i << " taken"
                      << std::setw(22) << std::setprecision(6) << running << "\n";
        } else if (i == 5) {
            std::cout << "         ...\n";
        }
    }
    std::cout << std::setprecision(4);

    // ========================================================================
    part(4, "THE ANSWER", "and it is 23");
    // ========================================================================
    const int ROOM = 23;
    answer("In a room of 23 people:");
    std::cout << "\n";
    showP("P(all birthdays different)", probabilityAllDifferent(ROOM));
    showP("P(at least two share)", probabilityShared(ROOM));

    note("Just over half. With 23 people. That is the famous");
    std::cout << "      result, and it is why this problem is called a paradox -\n"
                 "      not because it is wrong, but because it feels wrong.\n";

    // The full curve. This is where the shape of the answer lives.
    note("The whole curve, so you can see how fast it climbs:");
    std::cout << "\n        people   P(shared)                              \n"
              << "        " << repeat("-", 58) << "\n";
    const int sizes[] = {1, 5, 10, 15, 20, 22, 23, 25, 30, 40, 50, 57, 70, 100};
    for (int n : sizes) {
        double p = probabilityShared(n);
        std::cout << "        " << std::setw(5) << n << std::setw(10) << p << "   "
                  << repeat("#", static_cast<int>(p * 40.0 + 0.5))
                  << (n == 23 ? "  <- half way" : "") << "\n";
    }

    // Let the program find the famous number rather than asserting it.
    int firstHalf = 1;
    while (probabilityShared(firstHalf) < 0.5) ++firstHalf;
    int firstNinetyNine = 1;
    while (probabilityShared(firstNinetyNine) < 0.99) ++firstNinetyNine;

    note("Two landmarks, found by the program rather than quoted:");
    std::cout << "\n        smallest room that beats 50%      " << firstHalf
              << " people\n"
              << "        smallest room that beats 99%      " << firstNinetyNine
              << " people\n";
    std::cout << "\n"
        "      57 people gets you to 99%, but you need all 366 before\n"
        "      it becomes a certainty. The curve rushes up and then\n"
        "      crawls the last stretch.\n";

    // ========================================================================
    part(5, "DO NOT TRUST ME, SIMULATE IT", "100,000 rooms");
    // ========================================================================
    // The formula is short enough to be suspicious. So fill a room with
    // random birthdays, check it, and do that a hundred thousand times.
    question("Is that actually true? Fill rooms at random and count.");

    std::mt19937 rng(1729);
    std::uniform_int_distribution<int> anyDay(0, DAYS - 1);
    const int TRIALS = 100000;

    note("One room of 23, repeated 100,000 times:");
    int roomsWithMatch = 0;
    for (int t = 0; t < TRIALS; ++t) {
        std::vector<bool> used(DAYS, false);
        bool matched = false;
        for (int person = 0; person < ROOM; ++person) {
            int day = anyDay(rng);
            if (used[static_cast<size_t>(day)]) { matched = true; break; }
            used[static_cast<size_t>(day)] = true;
        }
        if (matched) ++roomsWithMatch;
    }
    double simulated = static_cast<double>(roomsWithMatch) / TRIALS;

    std::cout << "\n        rooms simulated                   " << commas(TRIALS) << "\n"
              << "        rooms with a shared birthday      " << commas(static_cast<unsigned long long>(roomsWithMatch)) << "\n";
    verifyClose("23 people", probabilityShared(ROOM), simulated, 0.01);

    note("Same experiment at a few other room sizes:");
    std::cout << "\n        people      formula   simulated    difference\n"
              << "        " << repeat("-", 52) << "\n";
    for (int n : {10, 23, 40, 60}) {
        int hits = 0;
        for (int t = 0; t < 20000; ++t) {
            std::vector<bool> used(DAYS, false);
            bool matched = false;
            for (int person = 0; person < n; ++person) {
                int day = anyDay(rng);
                if (used[static_cast<size_t>(day)]) { matched = true; break; }
                used[static_cast<size_t>(day)] = true;
            }
            if (matched) ++hits;
        }
        double sim = static_cast<double>(hits) / 20000.0;
        double formula = probabilityShared(n);
        std::cout << "        " << std::setw(6) << n << std::setw(13) << formula
                  << std::setw(12) << sim << std::setw(14)
                  << std::fabs(formula - sim) << "\n";
    }
    note("The formula survives contact with reality.");

    // ========================================================================
    part(6, "WHY IT FEELS SO WRONG", "you asked the wrong question");
    // ========================================================================
    // The intuition failure is not really a failure of arithmetic. People
    // silently answer a DIFFERENT question, and that question has a small
    // answer. Putting the two side by side fixes the intuition permanently.
    question("Why does 50% feel far too high?");

    note("Because your gut answered a different question. It asked");
    std::cout << "      'does anyone share MY birthday?' - and that answer\n"
                 "      really is tiny:\n\n";
    showP("P(someone shares YOUR birthday)", probabilitySharesWithMe(ROOM));
    showP("P(SOME two people share)", probabilityShared(ROOM));

    note("The difference is what you are counting. You are not in");
    std::cout << "      23 comparisons. The room is full of pairs, and every\n"
                 "      pair is another chance to match:\n";

    std::cout << "\n        room size    pairs = C(n,2)    P(shared)\n"
              << "        " << repeat("-", 50) << "\n";
    for (int n : {5, 10, 23, 40, 60}) {
        std::cout << "        " << std::setw(6) << n
                  << std::setw(15) << commas(nCr(n, 2))
                  << std::setw(16) << probabilityShared(n) << "\n";
    }

    std::cout << "\n"
        "      23 people is only 23 people - but it is "
              << commas(nCr(ROOM, 2)) << " PAIRS.\n"
        "      Pairs grow like n^2 while people grow like n, and that\n"
        "      gap is the entire paradox.\n";

    int meHalf = 2;
    while (probabilitySharesWithMe(meHalf) < 0.5) ++meHalf;
    note("For a coin flip on YOUR birthday specifically, you need");
    std::cout << "      a room of " << meHalf << " people. That is the number your\n"
                 "      intuition was reaching for.\n";

    // ========================================================================
    part(7, "THE SAME TRICK EVERYWHERE", "at least one -> flip it");
    // ========================================================================
    // Birthdays were the excuse. THIS is the lesson: the same two lines work
    // on every "at least one" question you will ever meet.
    question("Where else does 'at least one' show up? Everywhere.");

    note("DICE - at least one six in four rolls.");
    std::cout << "\n"
        "         direct   count one six, two sixes, three, four,\n"
        "                  then add them all up without overlap\n"
        "         flipped  P(no six at all) = (5/6)^4, then subtract\n\n";
    showP("P(at least one six in 4 rolls)", 1.0 - std::pow(5.0 / 6.0, 4));

    note("COINS - at least one head in five flips.");
    std::cout << "      Only ONE of the 32 sequences has no head at all: TTTTT.\n\n";
    showP("P(at least one head in 5)", 1.0 - std::pow(0.5, 5));

    note("CARDS - at least one ace in a 5-card hand. Straight from");
    std::cout << "      1.2: count the hands built only from the 48 non-aces.\n\n";
    double noAce = static_cast<double>(nCr(48, 5)) / static_cast<double>(nCr(52, 5));
    std::cout << "        hands with no ace   48C5 = " << commas(nCr(48, 5)) << "\n"
              << "        hands in total      52C5 = " << commas(nCr(52, 5)) << "\n\n";
    showP("P(no ace at all)", noAce);
    showP("P(at least one ace)", 1.0 - noAce);

    note("MARKET - a stock whose daily move is one of six equally");
    std::cout << "      likely outcomes, one of which is a 5% crash. What is\n"
                 "      the chance of at least one crash in a trading month?\n\n";
    for (int days : {1, 5, 21, 252}) {
        std::string label = "P(a crash within " + std::to_string(days) +
                            (days == 1 ? " day)" : " days)");
        showP(label, 1.0 - std::pow(5.0 / 6.0, days));
    }
    note("Same shape as the birthday curve: rare events stop being");
    std::cout << "      rare the moment you give them enough chances. That is\n"
                 "      the real reason 23 people is enough.\n";

    // ========================================================================
    part(8, "WHAT YOU LEARNED", "1.4 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE MOVE\n"
        "          P(A) = 1 - P(A^c)\n"
        "          When a question says AT LEAST ONE, count the opposite.\n"
        "          'At least one match' has dozens of cases.\n"
        "          'No match at all' has exactly one.\n"
        "\n"
        "      THE BIRTHDAY ANSWER\n"
        "          P(all different) = 365/365 x 364/365 x ... x 343/365\n"
        "          P(shared)        = 1 - that\n"
        "          23 people  ->  50.7%        57 people  ->  99%\n"
        "\n"
        "      WHY IT SURPRISES YOU\n"
        "          you think about 23 PEOPLE, the maths counts "
              << commas(nCr(ROOM, 2)) << " PAIRS\n"
        "          'shares MY birthday' is a different question, and its\n"
        "          answer really is small - about 6% for 23 people\n"
        "\n"
        "      THE ENGINEERING\n"
        "          never build 365^23; multiply the ratios one at a time\n"
        "          so every partial answer stays between 0 and 1\n"
        "\n"
        "      IT USED ALL THREE EARLIER LESSONS\n"
        "          1.1  P(A) = ways A / ways total\n"
        "          1.2  365^n with replacement, 365Pn without\n"
        "          1.3  the complement rule that made it easy\n"
        "\n"
        "      COMING LATER\n"
        "          conditional probability P(A given B), independence,\n"
        "          and Bayes' theorem.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
