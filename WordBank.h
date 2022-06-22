/*! \file WordBank.h
    \brief A word bank that holds all possible words used in-game.

    This files holds all words that can be potentially selected
    at random as the correct word. Placed in a header file to avoid
    clutter.

    Words are separated into arrays by level of difficulty.

    \author Seth J. Gibson, Jaiden Ortiz, Dennis Salo
    \version 2.0
    \since 1.0
*/

#ifndef WORDBANK_H_
#define WORDBANK_H_

/// String array that contains the "easy" word bank
extern char* bankE[] = {
        "WINS", "LOSE", "BANK", "BIKE", "KITE",
        "ANTS", "PIES", "FLYS", "JUNE", "JULY",
        "BOSSY", "CHESS", "CLASS", "COMMA", "BUNNY",
        "DIZZY", "STALLS", "CRASS", "BLUFF", "DOLLY",
        "NEEDED", "DEEMED", "PEEPER", "HEEDED", "PEEPED"
};

/// String array that contains the "medium" word bank
extern char* bankM[] = {
        "FIGHT", "MIGHT", "BEACH", "ADULT", "STACK",
        "YACHT", "VOCAL", "NOISE", "BRAVE", "TRAIN"
        "STRING", "FLIGHT", "ENTITY", "EMPIRE", "FOLLOW",

};

/// String array that contains the "hard" word bank
extern char* bankH[] = {
        "ONGOING", "OUTSIDE", "PACKAGE", "OVERALL", "NOTHING"
};

#endif  // WORDBANK_H_