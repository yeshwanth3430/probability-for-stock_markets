// ============================================================================
//  1.1 - A GENTLE INTRODUCTION TO PROBABILITY
//  
//
//  Build & run (quote the name, it has spaces):
//      g++ -std=c++17 -Wall -Wextra -O2 "1.1Gentle Introduction to Probability:.cpp" -o p11
//      ./p11
//
//  ---------------------------------------------------------------------------
//  THIS LESSON HAS EXACTLY TWO IDEAS. That is all. Nothing else.
//  ---------------------------------------------------------------------------
//
//    IDEA 1 - THE VOCABULARY
//
//      experiment      something random we do        flip a coin
//      outcome         one possible result           "Heads"
//      sample space S  the set of ALL outcomes       { Heads, Tails }
//      event A         any GROUP of outcomes         "I get Heads"
//
//    IDEA 2 - THE NAIVE DEFINITION OF PROBABILITY
//
//      If every outcome is equally likely, then:
//
//                       |A|      how many outcomes are in A
//              P(A) =  ----- = ----------------------------
//                       |S|      how many outcomes in total
//
//      That is it. Count the ones you want, divide by the total.
//      "Naive" is the real textbook name for it - not an insult.
//
//  We will use these two ideas on four different worlds: COINS, DICE, CARDS,
//  and the STOCK MARKET. The point of this lesson is that it is the SAME
//  counting every time. Only the story changes.
//
//  ONE WARNING: the naive definition only works when every outcome is equally
//  likely. A fair coin, a fair die, a well shuffled deck: fine. A loaded die
//  or a real stock: not fine. We handle those in later lessons.
// ============================================================================

#include <iomanip>    // std::setw, std::setprecision - for tidy columns
#include <iostream>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// A tiny helper we will reuse in every single section.
//
// `favorable` = |A|, the number of outcomes in our event.
// `total`     = |S|, the number of outcomes in the sample space.
//
// Note static_cast<double>. If we wrote favorable / total with two ints, C++
// would do INTEGER division and 1 / 2 would come out as 0. Casting one side to
// double forces real division. This is the classic beginner C++ bug.
// ----------------------------------------------------------------------------
void showProbability(const std::string& description, int favorable, int total) {
    double p = static_cast<double>(favorable) / static_cast<double>(total);

    std::cout << "    " << std::left << std::setw(34) << description
              << std::right << std::setw(3) << favorable << " / " << total
              << "  =  " << std::fixed << std::setprecision(4) << p
              << "   (" << std::setprecision(1) << p * 100.0 << "%)\n"
              << std::setprecision(4);
}

void title(const std::string& text) {
    std::cout << "\n" << std::string(66, '=') << "\n  " << text << "\n"
              << std::string(66, '=') << "\n";
}

// ============================================================================
int main() {
    std::cout << std::fixed << std::setprecision(4);

    // ========================================================================
    title("PART 1 - THE COIN (the smallest possible experiment)");
    // ========================================================================
    // A coin has two sides, so the sample space has two outcomes.
    // We store it in a vector, which is just a resizable list.
    std::vector<std::string> coin = {"Heads", "Tails"};

    std::cout << "\n  Experiment: flip one fair coin.\n";
    std::cout << "  Sample space S = { ";
    for (const std::string& outcome : coin) std::cout << outcome << " ";
    std::cout << "}\n";
    std::cout << "  |S| = " << coin.size() << " outcomes\n\n";

    // The event "I get Heads" contains exactly 1 of the 2 outcomes.
    showProbability("P(Heads)", 1, 2);
    showProbability("P(Tails)", 1, 2);
    // An event can hold MORE THAN ONE outcome. "Heads or Tails" holds both.
    showProbability("P(Heads or Tails)", 2, 2);
    // ...and it can hold NONE. That event is impossible.
    showProbability("P(the coin lands on its edge)", 0, 2);

    std::cout << "\n  Already the two limits show up: an event you are sure of\n"
              << "  has probability 1, and an impossible one has probability 0.\n"
              << "  Every probability lives between those two numbers.\n";

    // ---- now flip TWO coins -------------------------------------------------
    // Each flip has 2 results, so two flips have 2 x 2 = 4 possible outcomes.
    // That multiplication is the only counting trick in this lesson.
    std::cout << "\n  Now flip TWO coins. Every combination of first and second:\n";

    std::vector<std::string> twoCoins;
    for (const std::string& first : coin)
        for (const std::string& second : coin)
            twoCoins.push_back(first.substr(0, 1) + second.substr(0, 1));  // "HT"

    std::cout << "  S = { ";
    for (const std::string& outcome : twoCoins) std::cout << outcome << " ";
    std::cout << "}     |S| = " << twoCoins.size() << "\n\n";

    // To find |A| we walk the whole sample space and count what we want.
    // This counting loop is the pattern for the entire rest of the file.
    int twoHeads = 0, exactlyOneHead = 0, atLeastOneHead = 0;
    for (const std::string& outcome : twoCoins) {
        int heads = 0;
        if (outcome[0] == 'H') ++heads;
        if (outcome[1] == 'H') ++heads;

        if (heads == 2) ++twoHeads;
        if (heads == 1) ++exactlyOneHead;
        if (heads >= 1) ++atLeastOneHead;
    }

    const int nCoins = static_cast<int>(twoCoins.size());
    showProbability("P(two Heads)",        twoHeads,       nCoins);
    showProbability("P(exactly one Head)", exactlyOneHead, nCoins);
    showProbability("P(at least one Head)", atLeastOneHead, nCoins);

    std::cout << "\n  Careful: 'exactly one Head' has probability 2/4, not 1/4,\n"
              << "  because BOTH HT and TH give one head. Listing the sample\n"
              << "  space is what stops you from missing outcomes like that.\n";

    // ========================================================================
    title("PART 2 - THE DICE (same idea, six outcomes instead of two)");
    // ========================================================================
    std::cout << "\n  Experiment: roll one fair six-sided die.\n";
    std::cout << "  S = { 1 2 3 4 5 6 }   |S| = 6\n\n";

    // Same counting loop, new world.
    int rolledFour = 0, evenFaces = 0, biggerThanFour = 0;
    for (int face = 1; face <= 6; ++face) {
        if (face == 4)     ++rolledFour;
        if (face % 2 == 0) ++evenFaces;        // % is the remainder operator
        if (face > 4)      ++biggerThanFour;
    }

    showProbability("P(roll a 4)",          rolledFour,     6);
    showProbability("P(roll an even face)", evenFaces,      6);
    showProbability("P(roll higher than 4)", biggerThanFour, 6);

    // ---- two dice -----------------------------------------------------------
    // 6 faces x 6 faces = 36 outcomes. The two dice are different colours, so
    // (2,5) and (5,2) are two SEPARATE outcomes. That is why it is 36 and not 21.
    std::cout << "\n  Now roll TWO dice (a red one and a blue one).\n"
              << "  |S| = 6 x 6 = 36 outcomes\n\n";

    int sumIsSeven = 0, doubles = 0, sumIsTwo = 0;
    for (int red = 1; red <= 6; ++red) {
        for (int blue = 1; blue <= 6; ++blue) {
            if (red + blue == 7) ++sumIsSeven;
            if (red == blue)     ++doubles;
            if (red + blue == 2) ++sumIsTwo;
        }
    }

    showProbability("P(the sum is 7)",       sumIsSeven, 36);
    showProbability("P(doubles)",            doubles,    36);
    showProbability("P(the sum is 2)",       sumIsTwo,   36);

    // ------------------------------------------------------------------------
    // WHERE DID 6, 6 AND 1 COME FROM? And how do you get them QUICKLY,
    // without writing out 36 pairs every time? This is the useful part.
    // ------------------------------------------------------------------------

    // First, see the thing. Here is the entire sample space with the SUM
    // written into each of the 36 boxes.
    std::cout << "\n  Every one of the 36 outcomes, showing its SUM\n"
                 "  (red die down the side, blue die across the top):\n\n";

    std::cout << std::left << std::setw(16) << "         blue ->" << std::right;
    for (int blue = 1; blue <= 6; ++blue) std::cout << std::setw(4) << blue;
    std::cout << "\n";
    for (int red = 1; red <= 6; ++red) {
        std::cout << std::left << std::setw(16)
                  << ("   red " + std::to_string(red)) << std::right;
        for (int blue = 1; blue <= 6; ++blue)
            std::cout << std::setw(4) << red + blue;
        std::cout << "\n";
    }
    std::cout << "\n  Look at the 7s: they run along a diagonal, six boxes of it.\n"
                 "  The single 2 sits alone in the top-left corner. That is the\n"
                 "  whole answer - but counting boxes by eye gets slow fast.\n";

    // ---- THE SHORTCUT -------------------------------------------------------
    // The trick you will use for the rest of your life:
    //     FIX ONE DIE, then ask how many choices the OTHER die has left.
    // Once the red die is chosen, the sum you want FORCES the blue die to one
    // specific value. So the only question is: is that forced value legal?
    std::cout << "\n  SHORTCUT - do not list, FORCE.\n"
                 "  Fix the red die. Your target sum then forces exactly one\n"
                 "  value on the blue die. Just check whether it is a real face.\n";

    std::cout << "\n  Target sum 7   ->   blue must be 7 - red:\n";
    for (int red = 1; red <= 6; ++red) {
        int blue = 7 - red;
        std::cout << "       red " << red << "  ->  blue must be " << std::setw(2) << blue
                  << "   " << (blue >= 1 && blue <= 6 ? "yes, a real face" : "IMPOSSIBLE")
                  << "\n";
    }
    std::cout << "  All 6 choices of red work, so the event has 6 outcomes.\n";

    std::cout << "\n  Target sum 2   ->   blue must be 2 - red:\n";
    for (int red = 1; red <= 6; ++red) {
        int blue = 2 - red;
        std::cout << "       red " << red << "  ->  blue must be " << std::setw(2) << blue
                  << "   " << (blue >= 1 && blue <= 6 ? "yes, a real face"
                                                      : "IMPOSSIBLE, no such face")
                  << "\n";
    }
    std::cout << "  Only red = 1 survives, so the event has 1 outcome. That is\n"
                 "  why P(sum 2) = 1/36 while P(sum 7) = 6/36.\n";

    // The same shortcut as a loop. Notice it never visits all 36 boxes: it
    // walks the 6 red values and tests the single blue value each one forces.
    // 6 checks instead of 36 - and with ten dice it would be the difference
    // between 60 checks and sixty million.
    std::cout << "\n  Run that shortcut for every possible sum:\n\n"
                 "     sum   ways   out of 36\n";
    int grandTotal = 0;
    for (int k = 2; k <= 12; ++k) {
        int ways = 0;
        for (int red = 1; red <= 6; ++red) {
            int blue = k - red;                       // forced by the target sum
            if (blue >= 1 && blue <= 6) ++ways;       // is it a real face?
        }
        grandTotal += ways;
        std::cout << "     " << std::setw(3) << k << std::setw(7) << ways << "   "
                  << std::string(ways, '#') << "\n";
    }
    // The dashes and the total sit under the "ways" column, which ends at
    // column 15: 5 leading spaces + setw(3) for the sum + setw(7) for the count.
    std::cout << std::string(11, ' ') << "----\n"
              << "     total" << std::setw(5) << grandTotal
              << "   <- every outcome landed in exactly\n"
              << std::string(22, ' ') << "one sum, so this HAS to be 36\n";

    std::cout << "\n  The counts climb 1,2,3,4,5,6 then fall 5,4,3,2,1 - a triangle\n"
                 "  peaking at 7. 7 is the most likely sum simply because more\n"
                 "  boxes produce it. The 36 OUTCOMES are equally likely; the\n"
                 "  SUMS built from them are not.\n";

    std::cout << "\n  Same shortcut for doubles: fix red, and blue is forced to\n"
                 "  equal it. All 6 choices work, so 6 outcomes -> 6/36.\n";

    // ========================================================================
    title("PART 3 - THE CARDS (52 outcomes, still just counting)");
    // ========================================================================
    // ------------------------------------------------------------------------
    // FIRST: what IS a deck of cards? If you have never used one, none of the
    // probabilities below will mean anything. So here is the whole thing.
    //
    //   A standard deck has 52 cards, and every card has TWO labels:
    //   its SUIT and its RANK.
    //
    //   THE 4 SUITS - the symbol printed on the card:
    //        Hearts    (red)
    //        Diamonds  (red)
    //        Clubs     (black)
    //        Spades    (black)
    //   Two suits are red and two are black, so the deck is 26 red, 26 black.
    //
    //   THE 13 RANKS - the value printed on the card, low to high:
    //        2 3 4 5 6 7 8 9 10   Jack   Queen   King   Ace
    //        \_______________/    \______________/       \_/
    //          9 number cards      3 face cards         the Ace
    //
    //   Every suit contains all 13 ranks exactly once. So:
    //
    //        4 suits  x  13 ranks  =  52 cards
    //
    //   That is the multiplication rule again - the same one that gave us
    //   4 outcomes for two coins and 36 for two dice.
    // ------------------------------------------------------------------------

    // A card has a rank and a suit, so we make a small struct to hold both.
    // A struct is just a bundle of variables with a name.
    struct Card {
        int rank;   // 0..12  ->  2,3,4,5,6,7,8,9,10,Jack,Queen,King,Ace
        int suit;   // 0..3   ->  Hearts, Diamonds, Clubs, Spades
    };

    // Short codes for printing the grid, and full names for reading.
    const std::string RANK_CODES[13] = {"2","3","4","5","6","7","8","9","10",
                                        "J","Q","K","A"};
    const std::string SUIT_NAMES[4]  = {"Hearts","Diamonds","Clubs","Spades"};
    const std::string SUIT_CODES[4]  = {"H","D","C","S"};
    const std::string SUIT_COLOR[4]  = {"red","red","black","black"};

    // Build the deck: every suit paired with every rank. 4 x 13 = 52.
    std::vector<Card> deck;
    for (int suit = 0; suit < 4; ++suit)
        for (int rank = 0; rank < 13; ++rank)
            deck.push_back(Card{rank, suit});

    const int nCards = static_cast<int>(deck.size());

    // ---- print the ENTIRE sample space, all 52 cards ------------------------
    // With coins and dice we could list S in one line. A deck is bigger, so we
    // lay it out as a grid: one row per suit, one column per rank. Seeing all
    // 52 boxes at once is what makes "13 out of 52" obvious instead of abstract.
    std::cout << "\n  The whole deck - this grid IS the sample space S:\n\n";
    // The indent (19) must match the row label width below: 2 + 9 + 8.
    // The trailing space on each rank code lines the header up with the RANK
    // part of "2H", not with the suit letter.
    std::cout << std::string(19, ' ');
    for (const std::string& r : RANK_CODES) std::cout << std::setw(4) << (r + " ");
    std::cout << "\n";
    for (int suit = 0; suit < 4; ++suit) {
        std::cout << "  " << std::left << std::setw(9) << SUIT_NAMES[suit]
                  << std::setw(8) << ("(" + SUIT_COLOR[suit] + ")") << std::right;
        for (int rank = 0; rank < 13; ++rank)
            std::cout << std::setw(4) << (RANK_CODES[rank] + SUIT_CODES[suit]);
        std::cout << "\n";
    }

    // ---- count the categories, don't trust me, count them -------------------
    // Every number here comes from a loop over the deck. Nothing is typed in
    // by hand, so you can change the deck and the counts follow.
    int redCards = 0, blackCards = 0;
    int numberCards = 0, faceCards = 0, aces = 0;
    for (const Card& c : deck) {
        if (c.suit <= 1)                 ++redCards;      // 0=Hearts, 1=Diamonds
        else                             ++blackCards;    // 2=Clubs,  3=Spades

        if (c.rank <= 8)                 ++numberCards;   // ranks 2..10
        else if (c.rank <= 11)           ++faceCards;     // Jack, Queen, King
        else                             ++aces;          // Ace
    }

    std::cout << "\n  Counting up the categories:\n"
              << "     red cards    (Hearts + Diamonds) : " << redCards    << "\n"
              << "     black cards  (Clubs + Spades)    : " << blackCards  << "\n"
              << "     number cards (2 through 10)      : " << numberCards << "\n"
              << "     face cards   (J, Q, K)           : " << faceCards   << "\n"
              << "     aces                             : " << aces        << "\n"
              << "     -------------------------------------------\n"
              << "     " << numberCards << " + " << faceCards << " + " << aces
              << " = " << numberCards + faceCards + aces
              << "   and   " << redCards << " + " << blackCards << " = "
              << redCards + blackCards << "\n"
              << "  Two different ways of slicing the deck, both totalling 52.\n";

    // ---- NOW the probabilities ---------------------------------------------
    std::cout << "\n  Experiment: shuffle well, draw ONE card.\n"
              << "  |S| = " << nCards << " equally likely outcomes.\n\n";

    // Same counting loop as the coin, the die and the market. Never changes.
    int hearts = 0, spades = 0, kings = 0, redKings = 0, aceOfSpades = 0;
    for (const Card& c : deck) {
        if (c.suit == 0)                  ++hearts;       // 0 = Hearts
        if (c.suit == 3)                  ++spades;       // 3 = Spades
        if (c.rank == 11)                 ++kings;        // 11 = King
        if (c.rank == 11 && c.suit <= 1)  ++redKings;     // King AND red
        if (c.rank == 12 && c.suit == 3)  ++aceOfSpades;  // one single card
    }

    showProbability("P(a red card)",         redCards,    nCards);
    showProbability("P(a Heart)",            hearts,      nCards);
    showProbability("P(a Spade)",            spades,      nCards);
    showProbability("P(a number card 2-10)", numberCards, nCards);
    showProbability("P(a face card J/Q/K)",  faceCards,   nCards);
    showProbability("P(a King)",             kings,       nCards);
    showProbability("P(a RED King)",         redKings,    nCards);
    showProbability("P(the Ace of Spades)",  aceOfSpades, nCards);

    std::cout << "\n  Read these off the grid above and they make sense:\n"
              << "     P(Heart) = 13/52 = 1/4  -> one whole row out of four\n"
              << "     P(King)  =  4/52 = 1/13 -> one whole column out of 13\n"
              << "     P(red King) = 2/52      -> where that row-pair and\n"
              << "                                that column cross\n"
              << "     P(Ace of Spades) = 1/52 -> a single box in the grid\n"
              << "\n  The rarest event here is one exact card. The more outcomes\n"
              << "  an event collects, the bigger its probability - that is all\n"
              << "  P(A) = |A|/|S| is really saying.\n";

    // ========================================================================
    title("PART 4 - THE STOCK MARKET (the same maths, with money)");
    // ========================================================================
    // Here is the payoff of this lesson. A stock is not a coin or a die, but if
    // we model its day as a few equally likely moves, the counting is IDENTICAL.
    //
    // Think of it as a "market die" with six faces. Each face is one possible
    // percentage move for the day, and each is equally likely.
    const double MOVES[6] = { -5.0, -2.0, -0.5, +0.5, +2.0, +5.0 };

    std::cout << "\n  Experiment: one trading day for a stock whose six equally\n"
              << "  likely daily moves are:  ";
    for (double m : MOVES)
        std::cout << std::showpos << std::setprecision(1) << m << "%  ";
    std::cout << std::noshowpos << std::setprecision(4) << "\n  |S| = 6 outcomes\n\n";

    int greenDay = 0, redDay = 0, bigDrop = 0, smallMove = 0;
    for (double move : MOVES) {
        if (move > 0)                     ++greenDay;    // the stock rose
        if (move < 0)                     ++redDay;      // the stock fell
        if (move <= -2.0)                 ++bigDrop;     // fell 2% or more
        if (move > -1.0 && move < 1.0)    ++smallMove;   // a quiet day
    }

    showProbability("P(the stock rises)",      greenDay,  6);
    showProbability("P(the stock falls)",      redDay,    6);
    showProbability("P(it drops 2% or more)",  bigDrop,   6);
    showProbability("P(a quiet day, under 1%)", smallMove, 6);

    // ---- two stocks, exactly like the two dice ------------------------------
    std::cout << "\n  Now hold TWO of these stocks at once. Just like two dice:\n"
              << "  |S| = 6 x 6 = 36 possible trading days\n\n";

    int bothUp = 0, atLeastOneUp = 0, bothDown = 0;
    for (double stockA : MOVES) {
        for (double stockB : MOVES) {
            if (stockA > 0 && stockB > 0) ++bothUp;
            if (stockA > 0 || stockB > 0) ++atLeastOneUp;   // || means OR
            if (stockA < 0 && stockB < 0) ++bothDown;
        }
    }

    showProbability("P(both stocks rise)",     bothUp,       36);
    showProbability("P(at least one rises)",   atLeastOneUp, 36);
    showProbability("P(both stocks fall)",     bothDown,     36);

    std::cout << "\n  'P(both rise)' is 9/36, the same shape of answer as\n"
              << "  'P(doubles)' with the dice. Coins, dice, cards, money -\n"
              << "  one formula, four costumes.\n";

    // ========================================================================
    title("WHAT YOU LEARNED IN 1.1");
    // ========================================================================
    std::cout <<
        "\n  experiment      the random thing we do\n"
        "  outcome         one possible result\n"
        "  sample space S  the set of ALL outcomes\n"
        "  event A         any group of outcomes we care about\n"
        "\n"
        "  THE NAIVE DEFINITION:   P(A) = |A| / |S|\n"
        "  (only valid when every outcome is equally likely)\n"
        "\n"
        "  P(A) is always between 0 and 1.\n"
        "      P = 0  ->  impossible\n"
        "      P = 1  ->  certain\n"
        "\n"
        "  Counting trick used throughout: if one step has m results and the\n"
        "  next has n, then together they have m x n. Two coins 2x2 = 4,\n"
        "  two dice 6x6 = 36, a deck 4x13 = 52, two stocks 6x6 = 36.\n"
        "\n"
        "  COMING LATER (not needed yet):\n"
        "      the axioms of probability, P(not A) = 1 - P(A),\n"
        "      P(A or B) = P(A) + P(B) - P(A and B), conditional probability,\n"
        "      Bayes' theorem, and unequal / real-world probabilities.\n"
        "\n"
        "  Play with it: change a number in MOVES, add a face to the die, or\n"
        "  count a new event. Every result above came from a counting loop.\n";

    return 0;
}
