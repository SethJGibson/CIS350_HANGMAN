/*! \file WordBank.h
    \brief A word bank that holds all possible words used in-game.

    This files holds all 25 words that can be potentially selected
    at random as the correct word. Placed in a header file to avoid
    clutter.

    \author Seth J. Gibson, Jaiden Ortiz, Dennis Salo
    \version 1.0
    \since 1.0
*/

#ifndef WORDBANK_H_
#define WORDBANK_H_

/// String array that contains the full word bank
extern char* bank[] = {
        "WINS", "LOSE", "BANK", "BIKE", "KITE",
        "FIGHT", "MIGHT", "BEACH", "ADULT", "STACK",
        "STRING", "FLIGHT", "ENTITY", "EMPIRE", "FOLLOW",
        "ALCOHOL", "COLLEGE", "CAPTURE", "FICTION", "DESPITE",
        "ONGOING", "OUTSIDE", "PACKAGE", "OVERALL", "NOTHING"
};

#endif  // WORDBANK_H_