// ============================================================================
//  1.3 - SET THEORY IN PROBABILITY: SAMPLE SPACES AND EVENTS
//
//  Build & run (quote the name, it has spaces and a colon):
//      g++ -std=c++17 -Wall -Wextra -O2 "1.3 Set Theory in Probability: Sample Spaces and Events.cpp" -o p13
//      ./p13
//
//  ---------------------------------------------------------------------------
//  WHERE WE ARE
//  ---------------------------------------------------------------------------
//  1.1  P(A) = ways A can happen / things that can happen
//  1.2  how to COUNT those ways when you cannot list them
//  1.3  the LANGUAGE underneath all of it                     <- you are here
//
//  1.1 quietly used words like "or", "and", "not" without ever saying what
//  they mean. This lesson makes them exact, because in probability they are
//  not English words at all - they are SET OPERATIONS.
//
//       "A or B"    ->  A union B          written  A u B
//       "A and B"   ->  A intersect B      written  A n B
//       "not A"     ->  the complement     written  A^c
//
//  (On paper the union symbol is a big U and the intersection is an upside
//  down U. We type them as u and n here.)
//
//  ---------------------------------------------------------------------------
//  THE ONE IDEA
//  ---------------------------------------------------------------------------
//       the SAMPLE SPACE S is a set - the set of everything that can happen
//       an EVENT is a SUBSET of S - any collection of outcomes you care about
//
//  That is it. Once you accept those two lines, every rule of probability
//  turns into a rule about sets, and you can DRAW it instead of memorising it.
// ============================================================================

#include <algorithm>  // set_union, set_intersection, set_difference, includes
#include <iomanip>
#include <iostream>
#include <iterator>   // std::inserter
#include <set>
#include <string>
#include <vector>

// ============================================================================
//  A SET, AND THE THREE THINGS YOU CAN DO TO ONE
//
//  std::set is the perfect tool here: it stores each element once, keeps them
//  sorted, and the standard library ships the set operations we need.
// ============================================================================
using Set = std::set<std::string>;

// "A or B"  - everything in A, everything in B, no duplicates.
Set setUnion(const Set& a, const Set& b) {
    Set out;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(),
                   std::inserter(out, out.end()));
    return out;
}

// "A and B" - only the things in BOTH.
Set setIntersect(const Set& a, const Set& b) {
    Set out;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                          std::inserter(out, out.end()));
    return out;
}

// "A but not B" - everything in A with B's members removed.
Set setMinus(const Set& a, const Set& b) {
    Set out;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(),
                        std::inserter(out, out.end()));
    return out;
}

// "not A" - everything in the sample space that is NOT in A.
// Note this is meaningless without S. "Not football" only makes sense once
// you say who the everyone is. That is why S must be fixed first.
Set complement(const Set& S, const Set& a) { return setMinus(S, a); }

// Two events are DISJOINT (mutually exclusive) when they share nothing,
// meaning they cannot both happen on the same try.
bool areDisjoint(const Set& a, const Set& b) { return setIntersect(a, b).empty(); }

// Is every member of A also in B?
bool isSubset(const Set& a, const Set& b) {
    return std::includes(b.begin(), b.end(), a.begin(), a.end());
}

// ============================================================================
//  LAYOUT HELPERS  (same 70-column page as 1.2, so the lessons look alike)
// ============================================================================
const int PAGE = 70;

std::string repeat(const std::string& unit, int times) {
    std::string s;
    for (int i = 0; i < times; ++i) s += unit;
    return s;
}

// Sets are written with curly braces and commas: {Ann, Cal, Eve}
std::string show(const Set& s) {
    if (s.empty()) return "{ }";
    std::string out = "{";
    bool first = true;
    for (const std::string& e : s) {
        if (!first) out += ", ";
        out += e;
        first = false;
    }
    return out + "}";
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

// One set, labelled, with its size. |A| is standard notation for "how many
// elements are in A" - and it is the top of the P(A) fraction from 1.1.
void showSet(const std::string& label, const Set& s) {
    const std::string body = show(s);
    if (body.size() <= 30) {
        std::cout << "        " << std::left << std::setw(22) << label
                  << std::setw(30) << body << "size " << s.size() << "\n";
    } else {
        // Too wide for the column - give it its own line instead of pushing
        // the size marker off the 70-column page.
        std::cout << "        " << std::left << std::setw(22) << label
                  << "size " << s.size() << "\n"
                  << "          " << body << "\n";
    }
}

// Two sets that a rule claims are equal. Print both, then say whether the
// claim survived. This is how every law below gets tested, never asserted.
void verifySet(const std::string& claim, const Set& lhs, const Set& rhs) {
    std::cout << "\n  " << (lhs == rhs ? "ok " : "XX ") << claim << "\n"
              << "        left  " << show(lhs) << "\n"
              << "        right " << show(rhs) << "\n";
}

// Numeric check, for the probability rules at the end.
void verifyNumber(const std::string& claim, double lhs, double rhs) {
    bool same = (lhs - rhs < 1e-12) && (rhs - lhs < 1e-12);
    std::cout << "  " << (same ? "ok " : "XX ") << std::left << std::setw(38)
              << claim << std::right << std::fixed << std::setprecision(4)
              << lhs << "  vs  " << rhs << "\n";
}

// P(A) = |A| / |S|, straight from 1.1. Every probability below is this.
void probability(const std::string& description, const Set& a, const Set& S) {
    double p = static_cast<double>(a.size()) / static_cast<double>(S.size());
    std::cout << "        " << std::left << std::setw(30) << description
              << std::right << std::setw(2) << a.size() << " / " << S.size()
              << "  =  " << std::fixed << std::setprecision(4) << p << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ========================================================================
    part(1, "A SET IS JUST A COLLECTION", "no maths yet");
    // ========================================================================
    // Everything in this lesson is built on one very ordinary idea, so it is
    // worth saying plainly before any symbols appear.
    question("What is a set? A collection of things, written in");
    std::cout << "      curly braces, where each thing appears once and the\n"
                 "      order is irrelevant.\n";

    note("The five friends in our club:");
    const Set club = {"Ann", "Ben", "Cal", "Dee", "Eve"};
    showSet("club", club);

    std::cout << "\n"
        "      Three pieces of notation and we are done:\n"
        "\n"
        "         Ann is IN the club          written  Ann in club\n"
        "         how many are in it          written  |club| = "
              << club.size() << "\n"
        "         a set with nothing in it    written  { }  (the EMPTY set)\n"
        "\n"
        "      {Ann, Ben} and {Ben, Ann} are the SAME set. Order never\n"
        "      matters in a set - which is exactly the 'order ignored'\n"
        "      idea from 1.2, now with a name.\n";

    // ========================================================================
    part(2, "THE SAMPLE SPACE S", "the universe of outcomes");
    // ========================================================================
    question("We draw ONE name out of a hat. What can happen?");

    const Set S = club;                       // the sample space for this lesson
    note("The sample space S is the set of every possible outcome:");
    showSet("S", S);

    std::cout << "\n"
        "      S is the universe for this experiment. Nothing outside S\n"
        "      can ever happen, and every question we ask from here on is\n"
        "      a question about pieces of S.\n";

    note("It matters that we FIX S first. 'Not football' is a");
    std::cout << "      meaningless phrase until somebody says who everyone is.\n"
                 "      Change S and every complement in the lesson changes too.\n";

    // ========================================================================
    part(3, "AN EVENT IS A SUBSET OF S", "any slice you care about");
    // ========================================================================
    // This is the sentence the whole lesson exists to make precise.
    question("What is an EVENT? Any subset of S. That is the whole");
    std::cout << "      definition - a collection of outcomes you care about.\n";

    const Set football = {"Ann", "Cal", "Eve"};   // A
    const Set chess    = {"Cal", "Dee"};          // B

    note("Two events, from what the club actually does:");
    showSet("A = plays football", football);
    showSet("B = plays chess", chess);

    std::cout << "\n"
        "      Both are subsets of S: every name inside them is a name\n"
        "      that could actually be drawn.\n";
    std::cout << "\n        A subset of S?  " << (isSubset(football, S) ? "yes" : "no")
              << "          B subset of S?  " << (isSubset(chess, S) ? "yes" : "no")
              << "\n";

    note("Two events sit at the extremes, and both are legal:");
    showSet("S itself (certain)", S);
    showSet("{ } the empty event", Set{});
    std::cout << "\n"
        "      S is the event 'something happens' - it always does.\n"
        "      { } is the event 'nothing happens' - it never does.\n"
        "      Everything interesting lives between those two.\n";

    // ========================================================================
    part(4, "THE PICTURE: A VENN DIAGRAM", "draw it, do not memorise it");
    // ========================================================================
    // Once you can draw the box, you never need to remember a single rule -
    // you read the answer off the picture.
    question("Where does everybody actually sit?");

    std::cout << "\n"
        "    +-- S -------------------------------------------------+\n"
        "    |                                                      |\n"
        "    |   +-- A: football ------+                            |\n"
        "    |   |                     |                            |\n"
        "    |   |   Ann      +--------+---- B: chess ----+         |\n"
        "    |   |            |        |                  |         |\n"
        "    |   |   Eve      |  Cal   |       Dee        |         |\n"
        "    |   |            |        |                  |         |\n"
        "    |   +------------+--------+                  |         |\n"
        "    |                |                           |         |\n"
        "    |                +---------------------------+         |\n"
        "    |                                                      |\n"
        "    |   Ben              <- inside S, outside both circles  |\n"
        "    +------------------------------------------------------+\n";

    note("Four regions, and every single person is in exactly one:");
    const Set onlyA   = setMinus(football, chess);
    const Set bothAB  = setIntersect(football, chess);
    const Set onlyB   = setMinus(chess, football);
    const Set neither = complement(S, setUnion(football, chess));

    showSet("football only", onlyA);
    showSet("both", bothAB);
    showSet("chess only", onlyB);
    showSet("neither", neither);

    std::cout << "\n        " << onlyA.size() << " + " << bothAB.size() << " + "
              << onlyB.size() << " + " << neither.size() << "  =  "
              << onlyA.size() + bothAB.size() + onlyB.size() + neither.size()
              << "  =  |S|\n";
    note("Nobody is counted twice and nobody is missed. Keep that");
    std::cout << "      picture in mind - PART 8 is entirely about the trouble\n"
                 "      you get into when you forget it.\n";

    // ========================================================================
    part(5, "UNION - the word 'OR'", "A u B");
    // ========================================================================
    question("Who plays football OR chess?");

    note("Union means: in A, or in B, or in both. In probability");
    std::cout << "      'or' ALWAYS includes 'both' - it is never the either/or\n"
                 "      of everyday speech. Cal plays both, and Cal is in A u B.\n";

    const Set unionAB = setUnion(football, chess);
    std::cout << "\n";
    showSet("A = football", football);
    showSet("B = chess", chess);
    showSet("A u B", unionAB);

    answer("A u B = " + show(unionAB) + "  -  everyone who plays something");
    note("Read it off the Venn: football only + both + chess only.");
    verifySet("A u B is the three regions inside the circles",
              unionAB, setUnion(setUnion(onlyA, bothAB), onlyB));

    // ========================================================================
    part(6, "INTERSECTION - the word 'AND'", "A n B");
    // ========================================================================
    question("Who plays football AND chess?");

    const Set interAB = setIntersect(football, chess);
    std::cout << "\n";
    showSet("A n B", interAB);

    answer("A n B = " + show(interAB) + "  -  only Cal is in both circles");

    note("Intersection is the overlap, and it is always SMALLER");
    std::cout << "      than the things it came from. Adding a requirement can\n"
                 "      never give you more people:\n\n"
                 "         |A| = " << football.size()
              << "      |B| = " << chess.size()
              << "      |A n B| = " << interAB.size() << "\n";

    note("When the overlap is empty the events are MUTUALLY");
    std::cout << "      EXCLUSIVE - they cannot both happen. Football and chess\n"
                 "      are not; but football and 'plays nothing' are:\n";
    std::cout << "\n        " << std::left << std::setw(16) << "A n B"
              << std::setw(10) << show(interAB) << "disjoint? "
              << (areDisjoint(football, chess) ? "yes" : "no") << "\n"
              << "        " << std::setw(16) << "A n neither"
              << std::setw(10) << show(setIntersect(football, neither))
              << "disjoint? "
              << (areDisjoint(football, neither) ? "yes" : "no")
              << std::right << "\n";

    // ========================================================================
    part(7, "COMPLEMENT - the word 'NOT'", "A^c");
    // ========================================================================
    question("Who does NOT play football?");

    const Set notFootball = complement(S, football);
    const Set notChess    = complement(S, chess);
    std::cout << "\n";
    showSet("S", S);
    showSet("A = football", football);
    showSet("A^c = not football", notFootball);

    answer("A^c = " + show(notFootball));

    note("A and A^c always do two things at once:");
    std::cout << "\n"
        "         they share nothing        A n A^c = "
              << show(setIntersect(football, notFootball)) << "\n"
        "         together they are all     A u A^c = "
              << show(setUnion(football, notFootball)) << "\n";
    verifySet("A u A^c rebuilds the whole of S",
              setUnion(football, notFootball), S);

    note("That is why sizes must add up, and it is the reason the");
    std::cout << "      complement rule P(A^c) = 1 - P(A) works in PART 10:\n\n"
                 "         |A| + |A^c|  =  " << football.size() << " + "
              << notFootball.size() << "  =  " << S.size() << "  =  |S|\n";

    // ========================================================================
    part(8, "DE MORGAN'S LAWS", "where intuition fails");
    // ========================================================================
    // The one place beginners reliably guess wrong, so it gets its own part
    // and both laws get checked against real sets.
    question("What does 'does NOT play both sports' mean?");

    note("The trap: people read 'not both' as 'plays neither'.");
    std::cout << "      Those are different sets, and the Venn diagram settles\n"
                 "      it instantly. De Morgan's two laws are the rule:\n\n"
                 "         (A u B)^c  =  A^c n B^c      not(either) = neither\n"
                 "         (A n B)^c  =  A^c u B^c      not(both)   = misses one\n";

    std::cout << "\n";
    showSet("A^c = not football", notFootball);
    showSet("B^c = not chess", notChess);

    verifySet("(A u B)^c = A^c n B^c",
              complement(S, setUnion(football, chess)),
              setIntersect(notFootball, notChess));

    verifySet("(A n B)^c = A^c u B^c",
              complement(S, setIntersect(football, chess)),
              setUnion(notFootball, notChess));

    note("Look at what those two answers actually are:");
    std::cout << "\n"
        "         not(plays either) = " << show(complement(S, unionAB)) << "\n"
        "                             only Ben, who plays nothing\n"
        "\n"
        "         not(plays both)   = " << show(complement(S, interAB)) << "\n"
        "                             everyone except Cal - including Ann,\n"
        "                             who does play a sport\n"
        "\n"
        "      Completely different sets. 'Not both' is a much weaker\n"
        "      statement than 'neither', and mixing them up is the most\n"
        "      common mistake in this whole subject.\n";

    // ========================================================================
    part(9, "PARTITION", "no gaps, no overlaps");
    // ========================================================================
    question("How do I chop S into pieces safely?");

    note("A PARTITION is a set of events that are all disjoint and");
    std::cout << "      together cover the whole of S. Every outcome lands in\n"
                 "      exactly one piece - no gaps, no double counting.\n";

    const std::vector<std::pair<std::string, Set>> partition = {
        {"football only", onlyA}, {"both", bothAB},
        {"chess only",    onlyB}, {"neither", neither}};

    std::cout << "\n";
    Set rebuilt;
    size_t tally = 0;
    for (const auto& piece : partition) {
        showSet(piece.first, piece.second);
        rebuilt = setUnion(rebuilt, piece.second);
        tally += piece.second.size();
    }

    // Both halves of the definition have to hold, so test both.
    bool allDisjoint = true;
    for (size_t i = 0; i < partition.size(); ++i)
        for (size_t j = i + 1; j < partition.size(); ++j)
            if (!areDisjoint(partition[i].second, partition[j].second))
                allDisjoint = false;

    std::cout << "\n        every pair disjoint?   "
              << (allDisjoint ? "yes - no overlaps" : "NO") << "\n"
              << "        sizes add to |S|?      " << tally << " = " << S.size()
              << "\n";
    verifySet("the pieces rebuild S exactly", rebuilt, S);

    note("A and A^c are the simplest partition of all - two pieces.");
    std::cout << "      Partitions are how you break a hard question into easy\n"
                 "      ones, and they come back for good in Bayes' theorem.\n";

    // ========================================================================
    part(10, "BACK TO PROBABILITY", "P(A) = |A| / |S|");
    // ========================================================================
    // All the set work now pays off: each rule of probability is a rule about
    // sets we already proved, divided through by |S|.
    question("Draw one name at random. All five are equally likely.");

    std::cout << "\n";
    probability("P(A) plays football", football, S);
    probability("P(B) plays chess", chess, S);
    probability("P(A n B) plays both", interAB, S);
    probability("P(A u B) plays either", unionAB, S);
    probability("P(A^c) no football", notFootball, S);

    const double pA  = static_cast<double>(football.size()) / S.size();
    const double pB  = static_cast<double>(chess.size())    / S.size();
    const double pAB = static_cast<double>(interAB.size())  / S.size();
    const double pU  = static_cast<double>(unionAB.size())  / S.size();
    const double pAc = static_cast<double>(notFootball.size()) / S.size();

    note("RULE 1 - the complement rule, straight from PART 7:");
    std::cout << "      A and A^c are disjoint and rebuild S, so their sizes\n"
                 "      add to |S| and their probabilities add to 1.\n\n";
    verifyNumber("P(A^c) = 1 - P(A)", pAc, 1.0 - pA);

    note("RULE 2 - the addition rule, straight from PART 4's Venn:");
    std::cout << "      Add |A| and |B| and Cal gets counted twice, once in\n"
                 "      each circle. Subtract the overlap once to fix it.\n\n"
                 "         P(A) + P(B)          = " << pA + pB
              << "   <- too big, Cal twice\n"
                 "         minus P(A n B)       = " << pAB << "\n"
                 "         P(A u B)             = " << pU << "\n\n";
    verifyNumber("P(A u B) = P(A) + P(B) - P(A n B)", pU, pA + pB - pAB);

    note("RULE 3 - when events are DISJOINT the overlap is empty,");
    std::cout << "      so the subtraction disappears and you may simply add.\n"
                 "      Football and 'plays nothing' never happen together:\n\n";
    const double pNeither = static_cast<double>(neither.size()) / S.size();
    verifyNumber("disjoint: P(A u C) = P(A) + P(C)",
                 static_cast<double>(setUnion(football, neither).size()) / S.size(),
                 pA + pNeither);

    note("RULE 4 - a partition's probabilities always total 1,");
    std::cout << "      because the pieces rebuild S and never overlap:\n\n";
    double partitionTotal = 0.0;
    for (const auto& piece : partition) {
        partitionTotal += static_cast<double>(piece.second.size()) / S.size();
        std::cout << "         " << std::left << std::setw(20) << piece.first
                  << std::right << static_cast<double>(piece.second.size()) / S.size()
                  << "\n";
    }
    std::cout << "\n";
    verifyNumber("all four pieces sum to 1", partitionTotal, 1.0);

    // ========================================================================
    part(11, "THE SAME SETS, THREE OTHER WORLDS", "it is always the same");
    // ========================================================================
    // Nothing here is new. It is the identical machinery pointed at 1.1's
    // other sample spaces, to show that the club was not a special case.
    question("Does any of this depend on it being a club of friends?");

    // ---- one die ------------------------------------------------------------
    const Set die     = {"1", "2", "3", "4", "5", "6"};
    const Set even    = {"2", "4", "6"};
    const Set biggerThan3 = {"4", "5", "6"};

    std::cout << "\n      DICE - roll one die.\n\n";
    showSet("S", die);
    showSet("A = even", even);
    showSet("B = bigger than 3", biggerThan3);
    showSet("A u B", setUnion(even, biggerThan3));
    showSet("A n B", setIntersect(even, biggerThan3));
    showSet("A^c = odd", complement(die, even));
    verifyNumber("P(A u B) = P(A)+P(B)-P(A n B)",
                 static_cast<double>(setUnion(even, biggerThan3).size()) / die.size(),
                 static_cast<double>(even.size()) / die.size()
                 + static_cast<double>(biggerThan3.size()) / die.size()
                 - static_cast<double>(setIntersect(even, biggerThan3).size()) / die.size());

    // ---- cards --------------------------------------------------------------
    // Only the four aces, so the whole sample space still fits on screen.
    const Set aces      = {"Ace-H", "Ace-D", "Ace-C", "Ace-S"};
    const Set redAces   = {"Ace-H", "Ace-D"};

    std::cout << "\n      CARDS - draw one of the four aces (H D C S = the suits).\n\n";
    showSet("S", aces);
    showSet("A = red", redAces);
    showSet("A^c = black", complement(aces, redAces));
    verifyNumber("P(red) + P(black) = 1",
                 static_cast<double>(redAces.size()) / aces.size()
                 + static_cast<double>(complement(aces, redAces).size()) / aces.size(),
                 1.0);

    // ---- market -------------------------------------------------------------
    // The four worlds of 1.1 again. A trading day is a sample space too.
    const Set day     = {"-5%", "-2%", "-0.5%", "+0.5%", "+2%", "+5%"};
    const Set up      = {"+0.5%", "+2%", "+5%"};
    const Set bigMove = {"-5%", "-2%", "+2%", "+5%"};

    std::cout << "\n      MARKET - one trading day, six equally likely moves.\n\n";
    showSet("S", day);
    showSet("A = the stock rose", up);
    showSet("B = a big move", bigMove);
    showSet("A n B = big rise", setIntersect(up, bigMove));
    showSet("A u B = rose or big", setUnion(up, bigMove));
    showSet("A^c = did not rise", complement(day, up));

    note("'A big move that was not a rise' is a set expression too:");
    std::cout << "        B n A^c  =  " << show(setIntersect(bigMove, complement(day, up)))
              << "\n";
    verifySet("B n A^c = B minus A", setIntersect(bigMove, complement(day, up)),
              setMinus(bigMove, up));

    note("Same three operations every time. The story changes, the");
    std::cout << "      set theory does not.\n";

    // ========================================================================
    part(12, "WHAT YOU LEARNED", "1.3 summary");
    // ========================================================================
    std::cout <<
        "\n"
        "      THE TWO DEFINITIONS\n"
        "          sample space S    the set of everything that can happen\n"
        "          event A           any SUBSET of S\n"
        "\n"
        "      THE THREE OPERATIONS\n"
        "          A u B   union          'or'    in A, or B, or both\n"
        "          A n B   intersection   'and'   in both\n"
        "          A^c     complement     'not'   in S but not in A\n"
        "\n"
        "      WORDS WORTH KNOWING\n"
        "          disjoint / mutually exclusive   A n B = { }\n"
        "          partition    disjoint pieces that rebuild all of S\n"
        "          { }          the empty event, which never happens\n"
        "          S            the certain event, which always happens\n"
        "\n"
        "      DE MORGAN - the one people get wrong\n"
        "          (A u B)^c = A^c n B^c      not(either)  =  neither\n"
        "          (A n B)^c = A^c u B^c      not(both)    =  misses one\n"
        "\n"
        "      AND SO, IN PROBABILITY\n"
        "          P(A^c)   = 1 - P(A)\n"
        "          P(A u B) = P(A) + P(B) - P(A n B)\n"
        "          disjoint => P(A u B) = P(A) + P(B)\n"
        "          a partition's probabilities always total 1\n"
        "\n"
        "      Every one of those came from the Venn diagram in PART 4.\n"
        "      Draw the box, put everybody in a region, read the answer.\n"
        "\n"
        "      COMING LATER\n"
        "          the axioms of probability, conditional probability\n"
        "          P(A given B), independence, and Bayes' theorem.\n";

    std::cout << "\n" << repeat("=", PAGE) << "\n";
    return 0;
}
