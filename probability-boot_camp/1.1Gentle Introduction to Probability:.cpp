// ============================================================================
// 1.1 - A GENTLE INTRODUCTION TO PROBABILITY
// Probability Bootcamp (Steve Brunton) - https://youtu.be/4T3aOIfNdTY
//
// Build & run (quote the name, it has spaces):
//     g++ -std=c++17 -Wall -Wextra -O2 "1.1Gentle Introduction to Probability:.cpp" -o gentle
//     ./gentle
//
// The whole idea of this file: probability is not magic. It is *set theory*
// plus one counting rule. So we literally build the sets in C++ and count.
//
//   Experiment  -> something random we do            (roll two dice)
//   Outcome     -> one possible result               (die1 = 2, die2 = 5)
//   Sample space S -> the set of ALL outcomes        (all 36 pairs)
//   Event A     -> ANY subset of S                   ("the sum is 7")
//   P(A)        -> a number in [0, 1] measuring A
//
// For a *finite* experiment where every outcome is equally likely, the whole
// theory collapses to one formula (the "classical" / Laplace definition):
//
//                     |A|      number of outcomes in A
//              P(A) = -----  = --------------------------
//                     |S|      number of outcomes total
//
// That is why we use std::set below: |A| is just A.size().
// ============================================================================

#include <algorithm>   // std::set_union, std::set_intersection, std::set_difference
#include <iomanip>     // std::setprecision, std::setw
#include <iostream>
#include <iterator>    // std::inserter
#include <random>      // std::mt19937, std::uniform_int_distribution
#include <set>
#include <string>
#include <tuple>       // std::tie  (easy lexicographic comparison)
#include <vector>

// ----------------------------------------------------------------------------
// 1. THE OUTCOME
// ----------------------------------------------------------------------------
// One elementary outcome of the experiment "roll a red die and a blue die".
// The dice are distinguishable, so (2,5) and (5,2) are DIFFERENT outcomes.
// That is exactly why the sample space has 36 elements and not 21.
struct Outcome {
    int d1;  // the red die,  1..6
    int d2;  // the blue die, 1..6

    // std::set is an ordered container: it needs to know how to sort its
    // elements, and it also uses this same operator to detect duplicates
    // (a and b are "equal" when !(a<b) && !(b<a)).
    // std::tie makes a tuple of references, and tuples already compare
    // lexicographically - so this sorts by d1 first, then by d2.
    bool operator<(const Outcome& other) const {
        return std::tie(d1, d2) < std::tie(other.d1, other.d2);
    }

    int sum() const { return d1 + d2; }
};

// Let std::cout know how to print an Outcome, so we can write `cout << o`.
std::ostream& operator<<(std::ostream& os, const Outcome& o) {
    return os << "(" << o.d1 << "," << o.d2 << ")";
}

// Two names for the same C++ type, but they mean different things to a human:
//   - SampleSpace is the universe S
//   - Event is any subset A of that universe
// This is the key insight of the lecture: an event *is* a set of outcomes.
using SampleSpace = std::set<Outcome>;
using Event       = std::set<Outcome>;

// ----------------------------------------------------------------------------
// 2. BUILDING THE SAMPLE SPACE S
// ----------------------------------------------------------------------------
// S = { (1,1), (1,2), ..., (6,6) }.  |S| = 6 * 6 = 36.
// This "6 * 6" is the multiplication rule of counting: if step 1 has m ways and
// step 2 has n ways, the pair has m*n ways.
SampleSpace makeTwoDiceSampleSpace() {
    SampleSpace S;
    for (int a = 1; a <= 6; ++a)
        for (int b = 1; b <= 6; ++b)
            S.insert(Outcome{a, b});
    return S;
}

// ----------------------------------------------------------------------------
// 3. BUILDING EVENTS FROM A DESCRIPTION
// ----------------------------------------------------------------------------
// In words we say "the event that the sum is 7". In math that is
//     A = { s in S : sum(s) == 7 }
// which is set-builder notation: "keep the outcomes that satisfy a condition".
// In C++ the "condition" is a predicate - any callable returning bool. The
// template lets us pass a lambda without naming its type.
template <typename Predicate>
Event eventWhere(const SampleSpace& S, Predicate holds) {
    Event A;
    for (const Outcome& s : S)
        if (holds(s))
            A.insert(s);
    return A;
}

// ----------------------------------------------------------------------------
// 4. SET ALGEBRA = THE LANGUAGE OF EVENTS
// ----------------------------------------------------------------------------
// Every English word about events is really a set operation:
//
//     "A or B"       -> A union B          (A | B)   at least one happens
//     "A and B"      -> A intersect B      (A & B)   both happen
//     "not A"        -> S \ A              (complement)  A does not happen
//     "A but not B"  -> A \ B              (difference)
//
// The std::set_* algorithms need sorted ranges - std::set is always sorted, so
// they work directly. std::inserter is the "output iterator" that puts each
// produced element into the result set.

Event setUnion(const Event& A, const Event& B) {
    Event out;
    std::set_union(A.begin(), A.end(), B.begin(), B.end(),
                   std::inserter(out, out.end()));
    return out;
}

Event setIntersection(const Event& A, const Event& B) {
    Event out;
    std::set_intersection(A.begin(), A.end(), B.begin(), B.end(),
                          std::inserter(out, out.end()));
    return out;
}

Event setDifference(const Event& A, const Event& B) {  // A \ B
    Event out;
    std::set_difference(A.begin(), A.end(), B.begin(), B.end(),
                        std::inserter(out, out.end()));
    return out;
}

Event complement(const SampleSpace& S, const Event& A) {  // A^c = S \ A
    return setDifference(S, A);
}

// Two events are mutually exclusive (disjoint) when they cannot both happen:
// their intersection is the empty set. This matters because the third axiom
// only applies to disjoint events.
bool areDisjoint(const Event& A, const Event& B) {
    return setIntersection(A, B).empty();
}

// ----------------------------------------------------------------------------
// 5. THE PROBABILITY MEASURE
// ----------------------------------------------------------------------------
// P is a *function from events to numbers*: P : subsets of S -> [0,1].
// Here we implement the equally-likely case, |A| / |S|.
//
// static_cast<double> is essential: 6 / 36 in integer arithmetic is 0.
double P(const Event& A, const SampleSpace& S) {
    return static_cast<double>(A.size()) / static_cast<double>(S.size());
}

// Small helpers just for pretty output.
void printEvent(const std::string& name, const Event& A, const SampleSpace& S) {
    std::cout << std::left << std::setw(34) << name
              << " |A| = " << std::setw(3) << A.size()
              << "  P = " << std::fixed << std::setprecision(4) << P(A, S) << "\n";
}

void rule(const std::string& title) {
    std::cout << "\n=== " << title << " "
              << std::string(title.size() < 60 ? 60 - title.size() : 0, '=') << "\n";
}

// A tiny assertion helper: doubles are never *exactly* equal after arithmetic,
// so we compare within a tolerance instead of using ==.
void check(const std::string& claim, double lhs, double rhs) {
    bool ok = std::abs(lhs - rhs) < 1e-12;
    std::cout << (ok ? "  [OK]   " : "  [FAIL] ") << std::left << std::setw(46) << claim
              << lhs << "  vs  " << rhs << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ------------------------------------------------------------------------
    rule("STEP 1: the sample space S");
    // ------------------------------------------------------------------------
    const SampleSpace S = makeTwoDiceSampleSpace();
    std::cout << "Experiment : roll a red die and a blue die.\n";
    std::cout << "|S|        : " << S.size() << " equally likely outcomes\n";
    std::cout << "first few  : ";
    int shown = 0;
    for (const Outcome& s : S) {
        if (shown++ == 8) { std::cout << "..."; break; }
        std::cout << s << " ";
    }
    std::cout << "\n";
    // Each single outcome is itself an event with probability 1/36.
    std::cout << "P(one specific outcome) = 1/36 = "
              << 1.0 / static_cast<double>(S.size()) << "\n";

    // ------------------------------------------------------------------------
    rule("STEP 2: events are subsets of S");
    // ------------------------------------------------------------------------
    // Each lambda below is the condition inside the set-builder braces.
    Event A = eventWhere(S, [](const Outcome& s) { return s.sum() == 7; });
    Event B = eventWhere(S, [](const Outcome& s) { return s.d1 == s.d2; });
    Event C = eventWhere(S, [](const Outcome& s) { return s.sum() % 2 == 0; });
    Event D = eventWhere(S, [](const Outcome& s) { return s.d1 >= 5; });

    printEvent("A = sum is 7",            A, S);
    printEvent("B = doubles (d1 == d2)",  B, S);
    printEvent("C = sum is even",         C, S);
    printEvent("D = red die is 5 or 6",   D, S);

    std::cout << "\nA in full: ";
    for (const Outcome& s : A) std::cout << s << " ";
    std::cout << "\n(6 outcomes out of 36 -> P(A) = 6/36 = 1/6)\n";

    // ------------------------------------------------------------------------
    rule("STEP 3: the three axioms of probability (Kolmogorov)");
    // ------------------------------------------------------------------------
    // Everything else in probability is *derived* from exactly these three.
    //
    //   Axiom 1 (non-negativity): P(A) >= 0 for every event A
    //   Axiom 2 (normalization) : P(S) = 1
    //   Axiom 3 (additivity)    : if A and B are disjoint, P(A or B) = P(A)+P(B)
    std::cout << "Axiom 1  P(A) >= 0        : ";
    bool allNonNegative = true;
    for (const Event& E : {A, B, C, D})
        if (P(E, S) < 0.0) allNonNegative = false;
    std::cout << (allNonNegative ? "holds for all events tested\n" : "VIOLATED\n");

    check("Axiom 2  P(S) = 1", P(S, S), 1.0);

    // A (sum 7) and B (doubles) really are disjoint: a sum of 7 is odd, but
    // doubles always give an even sum, so no outcome can be in both.
    std::cout << "A and B disjoint?          : "
              << (areDisjoint(A, B) ? "yes" : "no") << "\n";
    check("Axiom 3  P(A u B) = P(A) + P(B)",
          P(setUnion(A, B), S), P(A, S) + P(B, S));

    // ------------------------------------------------------------------------
    rule("STEP 4: results derived from the axioms");
    // ------------------------------------------------------------------------
    // (a) Complement rule: A and A^c are disjoint and together they are S,
    //     so P(A) + P(A^c) = P(S) = 1  =>  P(A^c) = 1 - P(A).
    //     This is the workhorse trick: "at least one" is usually computed as
    //     1 - P(none).
    Event notA = complement(S, A);
    printEvent("A^c = sum is NOT 7", notA, S);
    check("P(A^c) = 1 - P(A)", P(notA, S), 1.0 - P(A, S));

    // (b) The empty set (the impossible event) has probability 0.
    check("P(empty set) = 0", P(Event{}, S), 0.0);

    // (c) Inclusion-exclusion (the "addition rule"). Axiom 3 needs DISJOINT
    //     events. When A and B overlap, the naive sum counts the overlap twice,
    //     so we subtract it once:
    //         P(A u B) = P(A) + P(B) - P(A n B)
    //     C (even sum) and D (red die >= 5) DO overlap, e.g. (5,1) and (6,2).
    Event CuD = setUnion(C, D);
    Event CnD = setIntersection(C, D);
    std::cout << "\nC and D disjoint?          : "
              << (areDisjoint(C, D) ? "yes" : "no  (they overlap)") << "\n";
    printEvent("C n D  (even sum AND red >= 5)", CnD, S);
    printEvent("C u D  (even sum OR  red >= 5)", CuD, S);
    check("P(C u D) = P(C)+P(D)-P(C n D)",
          P(CuD, S), P(C, S) + P(D, S) - P(CnD, S));
    std::cout << "  (naive P(C)+P(D) would give "
              << P(C, S) + P(D, S) << " - too big, the overlap was double counted)\n";

    // (d) Monotonicity: if A is a subset of B then P(A) <= P(B).
    //     Doubles with an even sum is *all* doubles, so B is inside C.
    std::cout << "\nB subset of C?             : "
              << (setDifference(B, C).empty() ? "yes" : "no") << "\n";
    std::cout << "  so P(B) = " << P(B, S) << " <= P(C) = " << P(C, S) << "\n";

    // ------------------------------------------------------------------------
    rule("STEP 5: partitioning S (a complete set of alternatives)");
    // ------------------------------------------------------------------------
    // A partition splits S into disjoint events that cover everything.
    // By repeated use of Axiom 3, their probabilities must add to exactly 1.
    // Here: the 11 possible sums 2..12.
    std::cout << "sum   count   P(sum)   histogram\n";
    double total = 0.0;
    for (int k = 2; k <= 12; ++k) {
        Event Ek = eventWhere(S, [k](const Outcome& s) { return s.sum() == k; });
        double p = P(Ek, S);
        total += p;
        std::cout << std::setw(3) << k
                  << std::setw(8) << Ek.size()
                  << "   " << p << "   "
                  << std::string(Ek.size(), '#') << "\n";
    }
    check("sum of all P(sum = k) = 1", total, 1.0);
    std::cout << "  Note: the outcomes are uniform, the SUM is not. 7 is the\n"
                 "  most likely sum because it has the most outcomes behind it.\n";

    // ------------------------------------------------------------------------
    rule("STEP 6: the frequentist view - probability by simulation");
    // ------------------------------------------------------------------------
    // The classical formula above is theory. The other way to read P(A) is
    // "the long-run fraction of trials in which A happens". If the theory is
    // right, simulating should converge to the same number (law of large
    // numbers). This is the Monte Carlo method in its simplest form.
    //
    // <random> in C++ separates two ideas:
    //   - an ENGINE (mt19937) produces raw random bits, and
    //   - a DISTRIBUTION (uniform_int_distribution) shapes them into 1..6.
    // Never use rand() % 6 + 1 for this: it is biased and low quality.
    std::mt19937 engine(42);  // fixed seed => reproducible run
    std::uniform_int_distribution<int> die(1, 6);

    std::cout << "theoretical P(A) = P(sum is 7) = " << P(A, S) << "\n\n";
    std::cout << std::setw(12) << "trials" << std::setw(12) << "hits"
              << std::setw(12) << "estimate" << std::setw(12) << "error" << "\n";

    long long hits = 0, done = 0;
    for (long long target : {100LL, 1000LL, 10000LL, 100000LL, 1000000LL}) {
        // Keep rolling until we have `target` trials in total, reusing the
        // work already done instead of restarting the simulation each time.
        for (; done < target; ++done) {
            Outcome roll{die(engine), die(engine)};
            // A.count(roll) asks "is this outcome an element of the event A?"
            if (A.count(roll)) ++hits;
        }
        double estimate = static_cast<double>(hits) / static_cast<double>(done);
        std::cout << std::setw(12) << done << std::setw(12) << hits
                  << std::setw(12) << estimate
                  << std::setw(12) << std::abs(estimate - P(A, S)) << "\n";
    }
    std::cout << "\n  The error shrinks roughly like 1/sqrt(n): to get one more\n"
                 "  correct digit you need about 100x more trials.\n";

    // ------------------------------------------------------------------------
    rule("SUMMARY");
    // ------------------------------------------------------------------------
    std::cout <<
        "  outcome        one result of the experiment\n"
        "  sample space S set of ALL outcomes\n"
        "  event A        any SUBSET of S\n"
        "  P(A) = |A|/|S| when outcomes are equally likely\n"
        "  axioms         P(A) >= 0,  P(S) = 1,  disjoint => P(AuB) = P(A)+P(B)\n"
        "  derived        P(A^c) = 1-P(A),  P(AuB) = P(A)+P(B)-P(AnB)\n"
        "  frequentist    P(A) = long-run fraction of trials where A occurs\n";

    return 0;
}
