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
// Written for ANY number of slots, not just 365, because PART 8 needs the
// identical maths with 60 earnings dates instead of 365 birthdays. The
// problem was never really about birthdays.
double probabilityNoCollision(int slots, int items) {
    if (items > slots) return 0.0;            // pigeonhole: a clash is certain
    double p = 1.0;
    for (int i = 0; i < items; ++i) {
        // Item i arrives. i slots are already taken, so (slots - i) of the
        // slots still keep everything distinct.
        p *= static_cast<double>(slots - i) / static_cast<double>(slots);
    }
    return p;
}

// "At least two collide" is the complement of "all distinct".
double probabilityCollision(int slots, int items) {
    return 1.0 - probabilityNoCollision(slots, items);
}

double probabilityAllDifferent(int people) {
    return probabilityNoCollision(DAYS, people);
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

// P(NO day holds three or more people).
//
// This is the k = 3 version, and it is the whole point of PART 7: it is not
// a single product any more. We have to split by j = how many days hold
// exactly a pair, and add up every case:
//
//     sum over j of   C(days, j)                pick which days hold a pair
//                   x n! / ((n-2j)! 2^j)        choose and pair the people
//                   x P(days-j, n-2j)           place the loners on free days
//                   / days^n
//
// Every one of those terms is astronomically large, so we work in logs with
// lgamma (the log of the factorial) and only exponentiate at the very end.
// The lgamma(days-j+1) from C(days,j) cancels the one from P(days-j, n-2j),
// which is why it does not appear below.
double probabilityNoTriple(int days, int people) {
    if (people < 3) return 1.0;
    double total = 0.0;
    for (int j = 0; 2 * j <= people; ++j) {
        if (days + j - people < 0) continue;      // not enough days to spread out
        const double logTerm =
              std::lgamma(days + 1.0)   - std::lgamma(j + 1.0)
            + std::lgamma(people + 1.0) - std::lgamma(people - 2.0 * j + 1.0)
            - j * std::log(2.0)
            - std::lgamma(days + j - people + 1.0)
            - people * std::log(static_cast<double>(days));
        total += std::exp(logTerm);
    }
    return total > 1.0 ? 1.0 : total;             // guard tiny rounding overshoot
}

// "At least k on some day", by simulation. For k of 4 or more the exact sum
// gets worse again (you would split by pairs AND triples at once), so this is
// what people actually reach for.
double simulateAtLeastK(int days, int people, int k, int trials,
                        std::mt19937& rng) {
    std::uniform_int_distribution<int> anyDay(0, days - 1);
    int hits = 0;
    for (int t = 0; t < trials; ++t) {
        std::vector<int> count(static_cast<size_t>(days), 0);
        bool found = false;
        for (int i = 0; i < people; ++i)
            if (++count[static_cast<size_t>(anyDay(rng))] >= k) found = true;
        if (found) ++hits;
    }
    return static_cast<double>(hits) / trials;
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
    // 38 fits the longest label in the file ("ANSWER  P(it happens at
    // least once)") and still leaves the line inside 70 columns.
    std::cout << "        " << std::left << std::setw(38) << label << std::right
              << std::fixed << std::setprecision(4) << p
              << std::setw(9) << std::setprecision(1) << p * 100.0 << "%\n"
              << std::setprecision(4);
}

// Formula against simulation. Sampling noise is real, so we allow a small
// tolerance and say so rather than pretending the match is exact.
void verifyClose(const std::string& claim, double formula, double other,
                 double tolerance, const std::string& otherLabel = "simulated") {
    bool ok = std::fabs(formula - other) < tolerance;
    std::cout << "\n  " << (ok ? "ok " : "XX ") << std::left << std::setw(26)
              << claim << std::right << "formula " << std::fixed
              << std::setprecision(4) << formula
              << "   " << otherLabel << " " << other << "\n";
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
    std::cout << "\n        people   P(shared)\n"
              << "        " << repeat("-", 58) << "\n";
    const int sizes[] = {1, 5, 10, 15, 20, 22, 23, 25, 30, 40, 50, 57, 70, 100};
    for (int n : sizes) {
        double p = probabilityShared(n);
        const std::string bar = repeat("#", static_cast<int>(p * 40.0 + 0.5));
        const std::string tail = (n == 23 ? "  <- half way" : "");
        std::cout << "        " << std::setw(5) << n << std::setw(10) << p;
        // Skip the separator entirely when there is nothing after it, or the
        // line ends in trailing spaces.
        if (!bar.empty() || !tail.empty()) std::cout << "   " << bar << tail;
        std::cout << "\n";
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
    part(7, "WHAT ABOUT THREE PEOPLE?", "where the trick runs out");
    // ========================================================================
    question("Same room. What about at least THREE sharing a day?");
    std::cout << "      Or four? Or any k?\n";

    note("Fair question, and the honest answer is that the trick");
    std::cout << "      still helps - but it stops being a one-liner.\n";

    note("For TWO, the opposite was a single clean case:");
    std::cout << "\n"
        "         at least 2 share    ->   opposite: ALL DIFFERENT\n"
        "                                  one case, one product, done\n"
        "\n"
        "      For THREE, it is not:\n"
        "\n"
        "         at least 3 share    ->   opposite: NO DAY HAS 3+\n"
        "                                  = every day holds 0, 1 or 2\n"
        "\n"
        "      And 'every day holds 0, 1 or 2' is not one case. Maybe no\n"
        "      day holds a pair. Maybe one day does. Maybe seven days do.\n"
        "      Every one of those is a different count, and they all have\n"
        "      to be added together.\n";

    note("So you split by j = how many days hold exactly a pair:");
    std::cout << "\n"
        "         P(no triple)  =  sum over j of\n"
        "\n"
        "               C(365, j)             which days hold a pair\n"
        "            x  n! / ((n-2j)! 2^j)    choose and pair the people\n"
        "            x  365-j P n-2j          place the loners on free days\n"
        "            /  365^n                 out of everything possible\n"
        "\n"
        "      That is exactly the pile of cases PART 1 ran away from.\n"
        "      The complement bought us ONE clean case at k = 2, and it\n"
        "      stops buying it at k = 3. The trick did not fail - the\n"
        "      question just got harder.\n";

    // Trust it where it can be checked by hand before trusting it at 365.
    note("Check that on a case small enough to count by hand: 2");
    std::cout << "      days, 3 people. There are 2^3 = 8 equally likely lists,\n"
                 "      and exactly two of them are triples (all on day 1, all\n"
                 "      on day 2). So P(no triple) has to be 6/8 = 0.75.\n";
    verifyClose("2 days, 3 people", probabilityNoTriple(2, 3), 0.75,
                1e-9, "by hand  ");

    note("Now run it on 365 days, next to the k = 2 answer:");
    std::cout << "\n        people   P(>=2 share)   P(>=3 share)   P(>=4 share)\n"
              << "        " << repeat("-", 58) << "\n";
    for (int n : {10, 23, 50, 88, 120, 187, 250}) {
        std::cout << "        " << std::setw(6) << n
                  << std::setw(13) << probabilityShared(n)
                  << std::setw(15) << 1.0 - probabilityNoTriple(DAYS, n)
                  << std::setw(15) << simulateAtLeastK(DAYS, n, 4, 20000, rng)
                  << "\n";
    }
    note("The k >= 4 column is simulated, not exact - see below.");

    // The exact k = 3 answer had better agree with brute reality.
    verifyClose("88 people, a triple", 1.0 - probabilityNoTriple(DAYS, 88),
                simulateAtLeastK(DAYS, 88, 3, 100000, rng), 0.01);

    int tripleHalf = 1;
    while (1.0 - probabilityNoTriple(DAYS, tripleHalf) < 0.5) ++tripleHalf;

    note("The landmarks, and they are much further apart than you");
    std::cout << "      would guess:\n"
        "\n"
        "         at least 2 share a day      " << firstHalf << " people\n"
        "         at least 3 share a day      " << tripleHalf << " people\n"
        "         at least 4 share a day      about 187 people\n"
        "\n"
        "      Going from pairs to triples costs you nearly four times\n"
        "      the room. The reason is NOT that triples are scarce -\n"
        "      there are far more triples than pairs:\n";

    std::cout << "\n        " << std::left << std::setw(16) << "room of 23"
              << "pairs " << std::setw(12) << commas(nCr(23, 2))
              << "triples " << commas(nCr(23, 3)) << "\n"
              << "        " << std::setw(16) << "room of 88"
              << "pairs " << std::setw(12) << commas(nCr(88, 2))
              << "triples " << commas(nCr(88, 3)) << "\n" << std::right;

    std::cout << "\n"
        "      The reason is that each triple is far harder to fire:\n"
        "\n"
        "         a given PAIR matches       1 in 365\n"
        "         a given TRIPLE matches     1 in 365^2 = 1 in 133,225\n"
        "\n"
        "      So what matters is how many x how likely - the expected\n"
        "      number of matches:\n"
        "\n"
        "         23 people   " << commas(nCr(23, 2)) << " pairs / 365"
              << "            = " << std::setprecision(2)
              << static_cast<double>(nCr(23, 2)) / 365.0 << " expected\n"
        "         88 people   " << commas(nCr(88, 3)) << " triples / 133,225 = "
              << static_cast<double>(nCr(88, 3)) / (365.0 * 365.0)
              << " expected\n" << std::setprecision(4) <<
        "\n"
        "      Both land near 1 - and 'expect about one' is roughly where\n"
        "      'at least one' crosses a coin flip. That is the real reason\n"
        "      the two landmarks sit where they do.\n";

    note("And for k = 4 or more? The sum gets worse again - now you");
    std::cout << "      split by how many days hold pairs AND how many hold\n"
                 "      triples, at the same time. There is still a formula, but\n"
                 "      nobody enjoys it.\n";
    answer("So for general k: simulate. It is five lines and never wrong.");
    std::cout << "\n"
        "         fill n random days, count how many landed on each,\n"
        "         ask whether any count reached k, repeat 100,000 times\n"
        "\n"
        "      That is what the k >= 4 column above actually is. Knowing\n"
        "      when to stop deriving and start simulating is a real skill,\n"
        "      not a cop-out.\n";

    // ========================================================================
    part(8, "THE SAME TRICK EVERYWHERE", "at least one -> flip it");
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
    part(9, "A REAL MARKET PROBLEM", "the birthday problem in disguise");
    // ========================================================================
    // Not an analogy - literally the same computation with 60 slots instead
    // of 365. This is a risk that real portfolio managers actually watch.
    const int EARNINGS_DAYS = 60;      // trading days a reporting season covers

    question("You hold 12 stocks. Earnings season packs almost every");
    std::cout << "      report into about 60 trading days. What is the chance\n"
                 "      that at least TWO of your holdings report on the SAME\n"
                 "      day?\n";

    note("Why you would care: two of your positions moving on one");
    std::cout << "      announcement day means double the event risk, all of it\n"
                 "      landing at once. Diversification does not help you on a\n"
                 "      day when two of your names both gap on news.\n";

    note("The gut answer is 'unlikely, there are 60 days'. That is");
    std::cout << "      the same instinct that said 23 people was hopeless.\n";

    const int HOLDINGS = 12;

    // ---- STEP 1: the bottom of the fraction --------------------------------
    note("STEP 1 - everything that can happen. Each stock picks a");
    std::cout << "      reporting day, and of course two of them ARE allowed to\n"
                 "      pick the same day. So every slot keeps all 60 options -\n"
                 "      that is WITH replacement, from 1.2:\n"
        "\n"
        "         stock1   stock2   stock3    ...   stock12\n"
        "         ___      ___      ___             ___\n"
        "         60       60       60              60      ->  60^12\n";

    // ---- STEP 2: the top of the fraction -----------------------------------
    note("STEP 2 - the clean opposite: all 12 land on different");
    std::cout << "      days. Now each stock must dodge every day already taken,\n"
                 "      so the options count down - WITHOUT replacement:\n"
        "\n"
        "         stock1   stock2   stock3    ...   stock12\n"
        "         ___      ___      ___             ___\n"
        "         60       59       58              49      ->  60 P 12\n"
        "\n"
        "      Where does that 49 come from? The twelfth stock has 11 days\n"
        "      already spoken for, so 60 - 11 = 49 days still keep everyone\n"
        "      apart. In general the last slot is 60 - 12 + 1 = 49.\n";

    // ---- STEP 3: divide, and watch the giant numbers cancel ----------------
    note("STEP 3 - divide, using 1.1's formula. P(A) is ways A can");
    std::cout << "      happen over ways ANYTHING can happen:\n"
        "\n"
        "                          ways all 12 differ      60 P 12\n"
        "         P(no clash)  =  --------------------  =  -------\n"
        "                          ways anything           60 ^ 12\n"
        "\n"
        "      Write both out in full and they line up perfectly - 12\n"
        "      numbers on top, 12 numbers underneath:\n"
        "\n"
        "               60 x 59 x 58 x ... x 49      60   59   58        49\n"
        "         =  ----------------------------  = -- x -- x -- ... x --\n"
        "               60 x 60 x 60 x ... x 60      60   60   60        60\n"
        "\n"
        "      So the whole thing is just 12 fractions multiplied. The\n"
        "      first is 60/60 = 1, because stock 1 cannot clash with\n"
        "      anything - there is nothing there yet. Every fraction\n"
        "      after that is slightly less than 1, so the running total\n"
        "      can only fall.\n";

    // ---- STEP 4: do it one stock at a time ---------------------------------
    // Exactly the table PART 3 draws for birthdays. Seeing the number decay
    // is the difference between believing the result and understanding it.
    note("STEP 4 - multiply them one stock at a time and watch:");
    std::cout << "\n        stock   taken   free days   P(still no clash)\n"
              << "        " << repeat("-", 52) << "\n";
    double noClash = 1.0;
    for (int i = 0; i < HOLDINGS; ++i) {
        noClash *= static_cast<double>(EARNINGS_DAYS - i) / EARNINGS_DAYS;
        std::cout << "        " << std::setw(5) << i + 1
                  << std::setw(8) << i
                  << std::setw(12) << (std::to_string(EARNINGS_DAYS - i) + "/60")
                  << std::setw(16) << std::setprecision(4) << noClash << "\n";
    }

    std::cout << "\n"
        "      Look at the middle of that table. Stock 6 only has to\n"
        "      dodge 5 days, but stock 12 has to dodge 11 - so the later\n"
        "      arrivals do most of the damage. The drop accelerates.\n";

    std::cout << "\n";
    showP("P(all 12 report on different days)",
          probabilityNoCollision(EARNINGS_DAYS, HOLDINGS));
    showP("P(at least two clash)",
          probabilityCollision(EARNINGS_DAYS, HOLDINGS));

    answer("A 12-stock portfolio is about 70% likely to have a clash.");
    note("Read that again. Not 12%, not 20% - roughly seven times");
    std::cout << "      in ten, a 12-stock book has two names reporting on the\n"
                 "      same day, somewhere in the season.\n";

    // Where the tipping point sits, found rather than quoted.
    int portfolioHalf = 1;
    while (probabilityCollision(EARNINGS_DAYS, portfolioHalf) < 0.5) ++portfolioHalf;

    note("The whole curve for a 60-day season, with the pair count");
    std::cout << "      alongside it - because the pairs are what drive it:\n";
    std::cout << "\n        holdings   pairs   P(a clash)\n"
              << "        " << repeat("-", 58) << "\n";
    for (int n : {2, 5, 8, 10, 12, 15, 20, 30, 40}) {
        double p = probabilityCollision(EARNINGS_DAYS, n);
        const std::string bar = repeat("#", static_cast<int>(p * 26.0 + 0.5));
        const std::string tail = (n == portfolioHalf ? " <- half way" : "");
        std::cout << "        " << std::setw(8) << n << std::setw(8) << nCr(n, 2)
                  << std::setw(11) << p;
        if (!bar.empty() || !tail.empty()) std::cout << "  " << bar << tail;
        std::cout << "\n";
    }

    note("Why the curve bends like that: adding one more holding");
    std::cout << "      does not add one more chance to clash. It adds one\n"
                 "      chance against EVERY name you already own. Going from\n"
                 "      11 stocks to 12 does not add 1 pair - it adds 11.\n";

    note("It takes only " + std::to_string(portfolioHalf) + " holdings to make a clash "
         "more likely");
    std::cout << "      than not. Most people run far more names than that and\n"
                 "      have never once thought about it.\n";

    // The same intuition trap as PART 6, in market clothes.
    note("And the same trap as PART 6 is waiting here. Asking");
    std::cout << "      'does anything clash with my BIGGEST position?' is a\n"
                 "      different, much smaller question:\n\n";
    showP("P(something clashes with ONE name)",
          1.0 - std::pow(static_cast<double>(EARNINGS_DAYS - 1) / EARNINGS_DAYS,
                         HOLDINGS - 1));
    showP("P(SOME two of the 12 clash)",
          probabilityCollision(EARNINGS_DAYS, HOLDINGS));
    std::cout << "\n        12 holdings is " << commas(nCr(HOLDINGS, 2))
              << " pairs - that is where the risk hides.\n";

    // Simulate it, same as PART 5, because a claim about money deserves it.
    std::uniform_int_distribution<int> anyReportDay(0, EARNINGS_DAYS - 1);
    int clashes = 0;
    for (int t = 0; t < TRIALS; ++t) {
        std::vector<bool> taken(static_cast<size_t>(EARNINGS_DAYS), false);
        bool clash = false;
        for (int stock = 0; stock < HOLDINGS; ++stock) {
            int day = anyReportDay(rng);
            if (taken[static_cast<size_t>(day)]) { clash = true; break; }
            taken[static_cast<size_t>(day)] = true;
        }
        if (clash) ++clashes;
    }
    verifyClose("12 holdings, 60 days",
                probabilityCollision(EARNINGS_DAYS, HOLDINGS),
                static_cast<double>(clashes) / TRIALS, 0.01);

    // ========================================================================
    part(10, "YOUR TURN", "type a number, get an answer");
    // ========================================================================
    // Reading about it is one thing. Poke it with your own numbers and the
    // shape of the curve sticks.
    std::cout << "\n      Pick a question and give it a number. Every answer\n"
                 "      comes with its working, not just a result.\n";

    // A number read safely. Returns false when the input ends (Ctrl-D, or the
    // program being run from a pipe), so the loop can finish cleanly instead
    // of spinning forever on a dead stream.
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
            if (std::cin.eof()) return false;      // stream finished, give up
            std::cin.clear();                       // it was not a number at all
            std::string junk;
            std::getline(std::cin, junk);
            std::cout << "      That is not a number.\n";
        }
    };

    // The running product, one arrival at a time - the same table PART 3 and
    // PART 8 draw. Long runs are elided in the middle so a room of 400 does
    // not print 400 rows; the first three and last three carry the story.
    auto showRunning = [](int slots, int items, const std::string& noun) {
        std::cout << "                " << std::left << std::setw(9) << noun
                  << std::right << std::setw(10) << "free days"
                  << std::setw(20) << "P(all different)" << "\n";
        double p = 1.0;
        for (int i = 0; i < items; ++i) {
            p *= static_cast<double>(slots - i) / static_cast<double>(slots);
            if (i < 3 || i >= items - 3) {
                std::cout << "                " << std::left << std::setw(9) << i + 1
                          << std::right << std::setw(10)
                          << (std::to_string(slots - i) + "/" + std::to_string(slots))
                          << std::setw(20) << std::setprecision(4) << p << "\n";
            } else if (i == 3) {
                std::cout << "                   ...\n";
            }
        }
    };

    // The full menu goes up ONCE. Redrawing it between every answer buries
    // the working you actually came to read.
    std::cout << "\n"
                 "        1   birthdays     - a room of N people\n"
                 "        2   earnings      - a portfolio of N stocks\n"
                 "        3   tail risk     - a 1-in-K event, N tries\n"
                 "        4   quit\n";

    bool keepGoing = true;
    bool firstRound = true;
    while (keepGoing) {
        if (!firstRound) {
            // A rule and a one-line reminder, for when the menu above has
            // scrolled off the screen.
            std::cout << "\n      " << repeat("-", 60) << "\n"
                         "      1 birthdays   2 earnings   3 tail risk   4 quit\n";
        }
        firstRound = false;

        int choice = 0;
        if (!askNumber("Your choice", 1, 4, choice)) {
            std::cout << "\n";
            note("No more input - stopping here.");
            break;
        }

        switch (choice) {
        case 1: {
            int people = 0;
            if (!askNumber("How many people in the room", 1, 400, people)) {
                keepGoing = false;
                break;
            }
            const double none = probabilityAllDifferent(people);

            std::cout << "\n      " << people << " people, " << DAYS << " days.\n";

            std::cout << "\n        STEP 1  the two counts, straight from 1.2\n"
                         "\n"
                         "                everything that can happen - repeats\n"
                         "                ARE allowed, so WITH replacement:\n"
                         "                    365 x 365 x ...  =  365^" << people
                      << "\n"
                         "\n"
                         "                the good case - nobody repeats a day, so\n"
                         "                the days get used up, WITHOUT replacement:\n";
            if (people <= DAYS) {
                std::cout << "                    365 x 364 x ... x "
                          << DAYS - people + 1 << "  =  365 P " << people << "\n";
            } else {
                std::cout << "                    there are NONE - you run out of\n"
                             "                    days after the 365th person\n";
            }

            if (people > DAYS) {
                // The honest answer here is not arithmetic at all. Printing a
                // countdown past 365 would show a negative day count, which is
                // meaningless - so we say why instead.
                std::cout << "\n        STEP 2  " << people << " people but only "
                          << DAYS << " days.\n"
                             "                Two of them MUST land together - there\n"
                             "                is nowhere else to put them. That is the\n"
                             "                PIGEONHOLE PRINCIPLE, and it settles the\n"
                             "                answer before any arithmetic happens.\n"
                             "\n"
                             "                P(all different) = 0, exactly.\n";
            } else {
                std::cout << "\n        STEP 2  divide one by the other. Written out,\n"
                             "                the terms line up and it becomes "
                          << people << "\n                fractions multiplied together:\n"
                             "\n"
                             "                365/365 x 364/365 x ... x "
                          << DAYS - people + 1 << "/365\n"
                             "\n"
                             "                Multiply them one person at a time so\n"
                             "                nothing ever overflows:\n\n";
                showRunning(DAYS, people, "person");
            }

            std::cout << "\n        STEP 3  flip it - the whole point of 1.4:\n"
                         "                1 - " << std::setprecision(4) << none
                      << "  =  " << 1.0 - none << "\n\n";
            showP("ANSWER  P(at least two share)", 1.0 - none);

            std::cout << "\n      WHY: " << people << " people is not " << people
                      << " chances to match.\n      It is "
                      << commas(nCr(people, 2))
                      << " PAIRS, and every pair is its own chance.\n";
            break;
        }
        case 2: {
            int holdings = 0;
            if (!askNumber("How many stocks in the portfolio", 1, 200, holdings)) {
                keepGoing = false;
                break;
            }
            int season = 0;
            if (!askNumber("How many trading days in the season", 1, 252, season)) {
                keepGoing = false;
                break;
            }
            const double none = probabilityNoCollision(season, holdings);

            std::cout << "\n      " << holdings << " stocks sharing " << season
                      << " possible reporting days.\n";

            std::cout << "\n        STEP 1  the two counts\n"
                         "\n"
                         "                every way the dates could fall, clashes\n"
                         "                allowed:   " << season << "^" << holdings
                      << "\n\n"
                         "                every way they could all miss each other:\n";
            if (holdings <= season) {
                std::cout << "                    " << season << " x " << season - 1
                          << " x ... x " << season - holdings + 1
                          << "  =  " << season << " P " << holdings << "\n";
            } else {
                std::cout << "                    there are NONE - more stocks than\n"
                             "                    days to spread them over\n";
            }

            if (holdings > season) {
                std::cout << "\n        STEP 2  " << holdings << " stocks but only "
                          << season << " days.\n"
                             "                A clash is guaranteed by the PIGEONHOLE\n"
                             "                PRINCIPLE - there is nowhere else to put\n"
                             "                them.\n"
                             "\n"
                             "                P(no clash) = 0, exactly.\n";
            } else {
                std::cout << "\n        STEP 2  divide, then multiply the fractions\n"
                             "                one stock at a time:\n\n";
                showRunning(season, holdings, "stock");
            }

            std::cout << "\n        STEP 3  flip it:\n"
                         "                1 - " << std::setprecision(4) << none
                      << "  =  " << 1.0 - none << "\n\n";
            showP("ANSWER  P(at least two clash)", 1.0 - none);

            if (holdings <= season) {
                std::cout << "\n      WHY: " << commas(nCr(holdings, 2))
                          << " pairs of holdings, and each pair is a\n"
                             "      separate chance for two earnings dates to collide.\n";
            }
            break;
        }
        case 3: {
            int oneIn = 0, tries = 0;
            if (!askNumber("The event is 1 in K - give K", 2, 1000000, oneIn)) {
                keepGoing = false;
                break;
            }
            if (!askNumber("How many tries", 1, 100000, tries)) {
                keepGoing = false;
                break;
            }
            double miss = 1.0 - 1.0 / oneIn;
            double never = std::pow(miss, tries);
            std::cout << "\n      A 1 in " << commas(static_cast<unsigned long long>(oneIn))
                      << " event, tried " << commas(static_cast<unsigned long long>(tries))
                      << " times.\n\n"
                "        STEP 1  one try MISSES with probability\n"
                "                1 - 1/" << oneIn << " = " << std::setprecision(6)
                      << miss << "\n\n"
                "        STEP 2  all " << tries << " tries miss:\n"
                "                " << miss << "^" << tries << " = " << never
                      << "\n\n" << std::setprecision(4);
            showP("ANSWER  P(it happens at least once)", 1.0 - never);
            std::cout << "\n      WHY: each try is another chance. Rare events\n"
                         "      stop being rare once you repeat them enough.\n";
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
    part(11, "WHAT YOU LEARNED", "1.4 summary");
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
