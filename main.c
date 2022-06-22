/*! \file main.c
    \brief A Hangman game on an MSP432 embedded system.

    This program displays a Hangman game to an 128x160 display
    and is controlled by the player through a rotary encoder.
    Game includes menu options to change difficulty and view
    externally-saved leaderboard.

    \author Seth J. Gibson, Jaiden Ortiz, Dennis Salo
    \version 2.0
    \since 0.1
*/

#include "msp.h"
#include "WordBank.h"
#include <ST7735.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

/// Number of options in main menu
#define MENU_LENGTH 3

/// Number of options for game difficulty
#define DIFF_LENGTH 3

/**
* Initializes MCLK and SMCLK initialization to 48 MHz
*/
void Clock_Init48MHz(void);

/**
* Enables SysTick, then initializes system tick speed to
  3MHz with no interrupts.
*/
void SysTick_Init();

/**
* Delay function that utilizes enabled SysTick.
* @param delayms Length of delay in milliseconds
*/
void SysTick_Delay(uint16_t delayms);

/**
* Initializes PORTX Interrupts on P5.4 and P5.5. These
* are used for the CLK and DT pins of the rotary encoder.
*/
void SetupPort5Interrupts();

/**
* Initializes PORTX Interrupts on P1.7. This pin
* is used for the button on the rotary encoder.
*/
void SetupPort1Interrupts();

/**
* Function that allows user to iterate through
* options via rotary encoder signals. Occurs after
* PORT5 interrupt.
*/
void PORT5_IRQHandler(void);

/**
* Function that allows user to select options via
* button on the rotary encoder. Occurs after PORT1
* interrupt.
*/
void PORT1_IRQHandler(void);
                                 
/**
* Function that handles the logic of the rotary 
* encoder rotation while in the GameInProgress state
*/
void gameInProgressRotate(void);

/**
* Function that handles the logic of the rotary
* encoder button while in the GameInProgress state
*/
void gameInProgressButton(void);

/**
* Function that handles the logic of the rotary
* encoder rotation while in the MainMenu state
*/
void mainMenuRotate(void);

/**
* Function that handles the logic of the rotary
* encoder button while in the MainMenu state
*/
void mainMenuButton(void);

/**
* Function that handles the logic of the rotary
* encoder rotation while in the Difficulty state
*/
void difficultyRotate(void);

/**
* Function that handles the logic of the rotary
* encoder button while in the Difficulty state
*/
void difficultyButton(void);

/**
* Function that handles the logic of the rotary
* encoder rotation while in the Leaderboard state
*/
void leaderboardRotate(void);

/**
* Function that handles the logic of the rotary
* encoder button while in the Leaderboard state
*/
void leaderboardButton(void);

/**
* Function that handles the logic of the rotary
* encoder rotation while in the LeaderboardNameEntry state
*/
void leaderboardNameEntryRotate(void);

/**
* Function that handles the logic of the rotary
* encoder button while in the LeaderboardNameEntry state
*/
void leaderboardNameEntryButton(void);

/**
* This function adds a limb to the Hangman by incrementing
* the lifeCounter global and drawing a limb according to
* the "Easy" difficulty penalty.
*/
void hangTheManE();

/**
* This function adds two limbs to the Hangman by incrementing
* the lifeCounter global and drawing a limb according to
* the "Medium" difficulty penalty.
*/
void hangTheManM();

/**
* This function adds two limbs to the Hangman by incrementing
* the lifeCounter global and drawing a limb according to
* the "Hard" difficulty penalty.
*/
void hangTheManH();

/**
* This function fills in the space allotted for the hangman
* word with underscores based on word length.
*/
void clearWord();

/**
* This function resets all global variables to their values
* on startup.
*/
void reset();

/**
* This function declares the losing state of the game by
* sending flashing graphics to the LCD and calls reset().
*/
void gameLose();

/**
* This function declares the winning state of the game by
* sending flashing graphics to the LCD and calls reset().
*/
void gameWin();

/**
* This function removes a letter from the available
* alphabet after a selection has been made, to
* avoid repeat selections.
* @param str Character array for Available alphabet
* @param letter Letter selected by the user
*/
void removeChar(char* str, char letter);

/**
* This function randomly selects a word from the 
* word bank according to the difficulty set by the user.
*/
void chooseWord();

/**
* This function writes a string to the LCD.
* @param a X coordinate for the placement of the string
* @param b Y coordinate for the placement of the string
* @param line[] String to be printed
* @param textColor RGB color value of the text displayed
* @param backColor RGB color value of the space behind the displayed text
* @param pixelSize Size of the text displayed
* @param lineLength Amount of characters in the string
*/
void LCDLineWrite(int16_t a, int16_t b, char line[], int16_t textColor, int16_t backColor, uint8_t pixelSize, uint8_t lineLength);

/**
* This function initializes I2C communicationn with the external
* EEPROM integrated circuit. 
*/
void I2C1_init(void);

/**
* This function writes data to the external EEPROM chip using
* the "burst-write" method.
* @param slaveAddr Designated I2C address of EEPROM chip
* @param memAddr Memory address on chip to write data to
* @param byteCount Amount of bytes being written to the memory address
* @param data Unsigned string to write to EEPROM chip
*/
int I2C1_burstWrite(int slaveAddr, unsigned int memAddr, int byteCount, unsigned char* data);

/**
* This function reads data from the external EEPROM chip using
* the "burst-read" method.
* @param slaveAddr Designated I2C address of EEPROM chip
* @param memAddr Memory address on chip to read data from
* @param byteCount Amount of bytes being read from the memory address
* @param data Unsigned string to read from EEPROM chip
*/
int I2C1_burstRead(int slaveAddr, unsigned int memAddr, int byteCount, unsigned char* data);

/**
* This function displays a single leaderboard entry to the LCD
* @param line String entry from leaderboard
* @param addr Place of entry on the leaderboard (1 - 6)
*/
void Display_EEPROM(char line[], int addr);

/**
* This function takes in a leaderboard entry and inserts it into the 
* leaderboard, adjusting the rest of the leaderboard accordingly.
* @param line String entry for leaderboard
*/
void adjustLeaderBoard(char line[]);

/**
* This function writes a leadderboard entry to a specified leaderboard address
* @param line String entry for leaderboard
* @param addr Place of entry on the leaderboard (1 - 6)
*/
void writeToLeaderBoard(char line[], int addr);

/**
* This function reads a leadderboard entry from a specified leaderboard address
* @param addr Place of entry on the leaderboard (1 - 6)
*/
void readFromLeaderBoard(int addr);

/// CodeComposer must have the i in for loops declared as global
int i = 0; 

///  Variable that determines the state of the system. 0 = Game, 1 = Menu, 2 = Difficulty, 3 = Leaderboard, 4 = Leaderboard Name Entry
int state = 1;                  

///  Variable that determines the difficulty of the game. 0 = Easy, 1 = Medium, 2 = Hard
int diffState = 0;   

///  Variable that determines if a state has been entered for the first time or not
int firstTime = 1;

///  Variable that holds the score the player achieved by playing the game
int score = 0;

///  String used to display the value of the score to the LCD
char scoreString[4];

/// Iterator variable that decides the queued letter in the alphabet shown on screen
volatile uint32_t x = 0;

/// Queued letter from alphabet to be shown on screen
char letter[5];

/// Global length value for finding length of multiple strings
int len = 0;

/// String displayed on the screen, populated with the correct word as game progresses
char word[20] = "";

/// String that contains the correct letter 
char correctWord[20] = "TEST";

/// String that contains a full copy of the alphabet. Available alphabet is copied from this source
char alphabet[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

/// String that contains the working copy of the alphabet. Letters are removed as game progresses
char workingAlpha[26];

/// String array that holds all the menu options displayed to the LCD
char* mainMenu[MENU_LENGTH] = { "Start", "Difficulty", "Leaderboard" };

/// String array that holds all the difficulty options displayed to the LCD
char* difficulty[3] = { "Easy", "Medium", "Hard" };

/// Global value that counts the amount of failed guesses that have been made
int lifeCounter = 0;

/// Global value that counts the amount of successful guesses that have been made
int winCounter = 0;

/// Global value that verifies the amount of failed guesses that have been made
int lifeCounterCheck = 0;

/// Macros for the game logic to determine the difficulty of the game
int EASY = 0, MEDIUM = 1, HARD = 2;

/// Macro for holding the I2C Write address required to operate the EEPROM chip
#define EEPROM_SLAVE_ADDR_WRITE 0x50

/// Macro for holding the I2C Read address required to operate the EEPROM chip
#define EEPROM_SLAVE_ADDR_READ  0x51

/// String array for a local copy of the leaderboard
char EEPROM_Write[6][8] = {
                                    "0000 AAA",
                                    "0000 AAA",
                                    "0000 AAA",
                                    "0000 AAA",
                                    "0000 AAA",
                                    "0000 AAA"
};

/// Variable used to fill the username string for a leaderboard entry
int nameSelect = 0;

/// String used to store the username for a leaderboard entry
char nameCharSelect[3];

/// Placeholder variable holding the data for an in-progress leaderboard entry
char leaderBoardEntry[] = "0000 AAA";     

void main(void) {                                                   
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // Stop WatchDog timer
    Clock_Init48MHz();                              // Initialize clock to 48 MHz
    ST7735_InitR(INITR_REDTAB);                     // Initialize LCD as red tab
    SetupPort5Interrupts();                         // Setup GPIO on port 5 interrupts
    NVIC_EnableIRQ(PORT5_IRQn);                     // Turn on port 5 interrupts
    SetupPort1Interrupts();                         // Setup GPIO on port 1 interrupts
    NVIC_EnableIRQ(PORT1_IRQn);                     // Turn on port 1 interrupts
    I2C1_init();
    __enable_irq();                                 // Enable all interrupts
                                                    /* OK now you can start paying attention again. */

    uint16_t white = ST7735_Color565(255, 255, 255);    // LCD color macros for black and white
    uint16_t black = ST7735_Color565(0, 0, 0);

    readFromLeaderBoard(1);                         // Read in the current leaderboard from the EEPROM chip
    readFromLeaderBoard(2);
    readFromLeaderBoard(3);
    readFromLeaderBoard(4);
    readFromLeaderBoard(5);
    readFromLeaderBoard(6);

    Output_Clear();                                 // Initial command to clear screen before anything

    ST7735_FillScreen(black);                       // Set black background

    chooseWord();                                   // Selecting random word from bank based on difficulty

    strncpy(workingAlpha, alphabet, 26);            // Fill working alphabet with a full alphabet
    clearWord();                                    // Fill displayed word with underscores to indicate length

    while (1)                                                // Infinite loops are key to keeping variables updated live on screen
    {
        switch (state) {
        case 0:
            if (firstTime && state == 0) {                                      // If the state machine first enters,
                ST7735_FillScreen(black);                                       // Clear screen
                LCDLineWrite(10, 5, "    SCORE:    ", white, black, 1, 14);     // Display Graphics
                firstTime = 0;
            }

            sprintf(letter, "%c", workingAlpha[x]);             // Put letter in a string
            LCDLineWrite(16, 60, letter, white, black, 5, 1);   // then print that string
            LCDLineWrite(16, 120, word, white, black, 2, 20);   // The full word goes here too

            if (lifeCounter != lifeCounterCheck) {              // Checks input to see if change has been made
                if (diffState == EASY) {                        // Remove amount of limbs according to difficulty
                    hangTheManE();
                }
                else if (diffState == MEDIUM) { 
                    hangTheManM();
                }
                else {               
                    hangTheManH();
                }
            }

            if (diffState == EASY) {                            // End game according to difficulty
                if (lifeCounter == lifeCounterCheck & lifeCounterCheck == 6) {
                    gameLose();
                    break;
                }
            }
            if (diffState == MEDIUM) {
                if (lifeCounter == lifeCounterCheck & lifeCounterCheck == 3) {
                    gameLose();
                    break;
                }
            }
            if (diffState == HARD) {
                if (lifeCounter == lifeCounterCheck & lifeCounterCheck == 2) {
                    gameLose();
                    break;
                }
            }

            if (winCounter == len) {                            // Checks if the hangman is completed
                gameWin();                                      // if he is, end the game
            }

            sprintf(scoreString, "%5d", score);                     // Continuously update the users score
            LCDLineWrite(70, 5, scoreString, white, black, 1, 5);  

            break;
        case 1:
            if (firstTime && state == 1) {                                          // If the state machine first enters,
                ST7735_FillScreen(black);                                           // Clear screen & Display Graphics
                LCDLineWrite(20, 20, "HANGMAN", white, black, 2, 8);
                LCDLineWrite(50, 70, "Play", white, black, 1, 5);
                LCDLineWrite(33, 90, "Difficulty", white, black, 1, 11);            
                LCDLineWrite(30, 110, "Leaderboard", white, black, 1, 12);          
                LCDLineWrite(7, 150, "KILLROOM Games 2022", white, black, 1, 19);
                firstTime = 0;
            }

            switch (x) {                                            // Print cursor according to placement of rotary encoder
            case 0:
                LCDLineWrite(43, 70, ">", white, black, 1, 1);
                LCDLineWrite(26, 90, " ", white, black, 1, 1);
                LCDLineWrite(23, 110, " ", white, black, 1, 1);
                break;
            case 1:
                LCDLineWrite(43, 70, " ", white, black, 1, 1);
                LCDLineWrite(26, 90, ">", white, black, 1, 1);
                LCDLineWrite(23, 110, " ", white, black, 1, 1);
                break;
            case 2:
                LCDLineWrite(43, 70, " ", white, black, 1, 1);
                LCDLineWrite(26, 90, " ", white, black, 1, 1);
                LCDLineWrite(23, 110, ">", white, black, 1, 1);
                break;
            }

            break;
        case 2:
            if (firstTime && state == 2) {                      // If the state machine first enters,
                ST7735_FillScreen(black);                       // Clear screen & display graphics

                LCDLineWrite(5, 10, "DIFFICULTY",               
                    ST7735_Color565(0xff, 0xff, 0xff),
                    ST7735_Color565(0, 0, 0), 2, 10);
                LCDLineWrite(10, 110, "PENALTY:   LIMB(S)",
                    ST7735_Color565(0xff, 0xff, 0xff),
                    ST7735_Color565(0, 0, 0), 1, 18);

                firstTime = 0;
            }

            switch (x) {                                        // Print difficulty option and details according to placement of rotary encoder
            case (0):
                LCDLineWrite(5, 60, "   EASY   ", ST7735_Color565(0, 128, 0), black, 2, 10);
                LCDLineWrite(65, 110, "1", white, black, 1, 1);
                LCDLineWrite(-5, 125, "    Words are small    ", white, black, 1, 24);
                break;
            case (1):
                LCDLineWrite(5, 60, "  MEDIUM  ", ST7735_Color565(255, 218, 35), black, 2, 10);
                LCDLineWrite(65, 110, "2", white, black, 1, 1);
                LCDLineWrite(10, 125, "  Words are big  ", white, black, 1, 17);
                break;
            case (2):
                LCDLineWrite(5, 60, "   HARD   ", ST7735_Color565(255, 0, 0), black, 2, 10);
                LCDLineWrite(65, 110, "3", white, black, 1, 1);
                LCDLineWrite(7, 125, "Words are extra big", white, black, 1, 19);
                break;
            }

            break;
        case 3:
            if (firstTime && state == 3) {                      // If the state machine first enters,
                ST7735_FillScreen(black);                       // Clear screen & display entirety of leaderboard

                readFromLeaderBoard(1);
                Display_EEPROM(EEPROM_Write[0], 1);
                readFromLeaderBoard(2);
                Display_EEPROM(EEPROM_Write[1], 2);
                readFromLeaderBoard(3);
                Display_EEPROM(EEPROM_Write[2], 3);
                readFromLeaderBoard(4);
                Display_EEPROM(EEPROM_Write[3], 4);
                readFromLeaderBoard(5);
                Display_EEPROM(EEPROM_Write[4], 5);
                readFromLeaderBoard(6);
                Display_EEPROM(EEPROM_Write[5], 6);

                firstTime = 0;
            }

            break;
        case 4:                                             
            if (firstTime && state == 4) {                      // If the state machine first enters,
                ST7735_FillScreen(black);                       // Clear screen & display graphics
                sprintf(scoreString, "%04d", score);

                LCDLineWrite(28, 10, "YOU MADE THE",
                    ST7735_Color565(0xff, 0xff, 0xff),
                    ST7735_Color565(0, 0, 0), 1, 12);
                LCDLineWrite(28, 20, "LEADERBOARD!",
                    ST7735_Color565(0xff, 0xff, 0xff),
                    ST7735_Color565(0, 0, 0), 1, 12);
                LCDLineWrite(12, 40, "YOUR SCORE: ",
                    ST7735_Color565(0xff, 0xff, 0xff),
                    ST7735_Color565(0, 0, 0), 1, 12);
                LCDLineWrite(90, 40, scoreString,
                    ST7735_Color565(0xff, 0xff, 0xff),
                    ST7735_Color565(0, 0, 0), 1, 4);
                LCDLineWrite(10, 130, "ENTER NAME:    / 3",
                    ST7735_Color565(0xff, 0xff, 0xff),
                    ST7735_Color565(0, 0, 0), 1, 19);

                firstTime = 0;
            }

            switch (nameSelect) {                               // Show user which characters they are selecting for their 3-character-long username
            case (0):
                LCDLineWrite(88, 130, "1", white, black, 1, 1);
                break;
            case (1):
                LCDLineWrite(88, 130, "2", white, black, 1, 1);
                break;
            case (2):
                LCDLineWrite(88, 130, "3", white, black, 1, 1);
                break;
            }

            sprintf(letter, "%c", alphabet[x]);                 // Put letter in a string
            LCDLineWrite(53, 70, letter, white, black, 5, 1);   // then print that string

            break;
        }
    }
}

void PORT5_IRQHandler(void)                         // Interrupt handler triggers when the knob turns. This logic decides which letter we're on.
{                                                   // This block is currently unfinished due to some noticeable jank.
    if (P5->IFG & BIT5)                             // Checking if the Encoder CLK is high
    {
        if (P5->IN & BIT4) {                        // If the CLK is high and DT is high, its clockwise
            __delay_cycles(30000);                  // Wait 10 ms and check again. This debounces the input.
            if (P5->IN & BIT4) {
                x++;
            }
        }
        else {                                      // If the CLK is high and DT is not high, its CCW
            __delay_cycles(30000);
            if (P5->IN & BIT4) {
                x++;                                // Currently it only counts up no matter how you turn it.
            }
        }
    }

    switch (state) {                                // Call the rotation function according to its respective state
    case 0:
        gameInProgressRotate();
        break;
    case 1:
        mainMenuRotate();
        break;
    case 2:
        difficultyRotate();
        break;
    case 3:
        leaderboardRotate();
        break;
    case 4:
        leaderboardNameEntryRotate();
        break;
    }

    P5->IFG = 0;                                    // Reset GPIO flag
}

void PORT1_IRQHandler(void)                         // Interrupt handler for the button press. This is where letter select logic goes.
{
    if (P1->IFG & BIT7)                             // Knob has a button built in. This checks if the button signal is high
    {
        switch (state) {                            // Call the rotation function according to its respective state
        case 0:
            gameInProgressButton();
            break;
        case 1:
            mainMenuButton();
            break;
        case 2:
            difficultyButton();
            break;
        case 3:
            leaderboardButton();
            break;
        case 4:
            leaderboardNameEntryButton();
            break;
        }
    }
    P1->IFG = 0;                                    // reset GPIO flag
}

void gameInProgressRotate(void)
{
    if (x > (strlen(workingAlpha)))                 // If x reached the end of the alphabet, reset to 0
        x = 0;
}

void gameInProgressButton(void) {
    if (strchr(correctWord, workingAlpha[x]) != NULL)           // If the user made a correct guess,
    {
        for (i = 0; i < strlen(correctWord); i++)               // For loop to find current index of correct guess in correctWord
        {
            if (correctWord[i] == workingAlpha[x])              // Comparing each letter in correctWord with our guess
            {
                strncpy(&word[i], &workingAlpha[x], 1);         // Using "i" to put our guess in the correct position
                winCounter++;
                score += 1000;
            }
        }
    }
    else {                                                      // Otherwise, the user guessed wrong
        lifeCounter++;                                          // Add limb(s) to HANGMAN
        score -= 250;                                           // and reduce score
    }
    removeChar(workingAlpha, workingAlpha[x]);                  // Finally, remove guess from working alphabet
}

void mainMenuRotate(void)
{
    if (x > (MENU_LENGTH))                                 // If x reached the end of the menu options, reset to 0
        x = 0;
}

void mainMenuButton(void)
{
    if (x == 0)                                         // Menu button function selects new states to enter
        state = 0;
    else if (x == 1)
        state = 2;
    else if (x == 2)
        state = 3;
    reset();
}

void difficultyRotate(void)
{
    if (x > DIFF_LENGTH)                                // If x reached the end of the difficulty options, reset to 0
        x = 0;
}

void difficultyButton(void)
{
    diffState = x;                                      // Selected difficulty depends on value of x
    state = 1;                                          // When selected, return to main menu
    reset();
}

void leaderboardRotate(void)
{
    // This function does nothing, so here's a secret code just for you:
    // ROT47 *@F 2C6 2 362FE:7F= >2?]
}

void leaderboardButton(void)
{
    state = 1;                                          // When bored, the user presses the button to return to main menu
    reset();
}

void leaderboardNameEntryRotate(void)
{
    if (x > (strlen(alphabet)))                         // If x reached the end of the alphabet, reset to 0
        x = 0;
}

void leaderboardNameEntryButton(void)
{
    nameCharSelect[nameSelect] = alphabet[x];           // Add the selected letter to the username
    nameSelect++;

    if (nameSelect > 2) {                               // If all letters have been added,
        sprintf(leaderBoardEntry, "%04d %c%c%c", score, nameCharSelect[0], nameCharSelect[1], nameCharSelect[2]);   // Assemble leaderboard entry

        adjustLeaderBoard(leaderBoardEntry);            // Add leaderboard entry  to local copy of leaderboard,

        writeToLeaderBoard(EEPROM_Write[0], 1);         // then write local leaderboard to EEPROM chip
        writeToLeaderBoard(EEPROM_Write[1], 2);
        writeToLeaderBoard(EEPROM_Write[2], 3);
        writeToLeaderBoard(EEPROM_Write[3], 4);
        writeToLeaderBoard(EEPROM_Write[4], 5);
        writeToLeaderBoard(EEPROM_Write[5], 6);

        state = 1;                                      // Finally, return to menu
        reset();
    }
}

void hangTheManE() {
    uint16_t white = ST7735_Color565(255, 255, 255);    // LCD color macros for white

    switch (lifeCounter) {
    case 1:
        ST7735_FillRect(82, 22, 15, 15, white);     // Head
        break;
    case 2:
        ST7735_FillRect(82, 38, 15, 35, white);     // Torso
        break;
    case 3:
        ST7735_FillRect(75, 38, 6, 30, white);      // ArmL
        break;
    case 4:
        ST7735_FillRect(98, 38, 6, 30, white);      // ArmR
        break;
    case 5:
        ST7735_FillRect(82, 74, 6, 30, white);      // LegL
        break;
    case 6:
        ST7735_FillRect(91, 74, 6, 30, white);      // LegR
        break;
    }

    lifeCounterCheck = lifeCounter;
}

void hangTheManM() {
    uint16_t white = ST7735_Color565(255, 255, 255);    // LCD color macros for white

    switch (lifeCounter) {
    case 1:
        ST7735_FillRect(82, 22, 15, 15, white);     // Head
        ST7735_FillRect(82, 38, 15, 35, white);     // Torso
        break;
    case 2:
        ST7735_FillRect(75, 38, 6, 30, white);      // ArmL
        ST7735_FillRect(98, 38, 6, 30, white);      // ArmR
        break;
    case 3:
        ST7735_FillRect(82, 74, 6, 30, white);      // LegL
        ST7735_FillRect(91, 74, 6, 30, white);      // LegR
        break;
    }

    lifeCounterCheck = lifeCounter;
}

void hangTheManH() {
    uint16_t white = ST7735_Color565(255, 255, 255);    // LCD color macros for white

    switch (lifeCounter) {
    case 1:
        ST7735_FillRect(82, 22, 15, 15, white);     // Head
        ST7735_FillRect(82, 38, 15, 35, white);     // Torso
        ST7735_FillRect(75, 38, 6, 30, white);      // ArmL
        break;
    case 2:
        ST7735_FillRect(98, 38, 6, 30, white);      // ArmR
        ST7735_FillRect(82, 74, 6, 30, white);      // LegL
        ST7735_FillRect(91, 74, 6, 30, white);      // LegR
        break;
    }

    lifeCounterCheck = lifeCounter;
}

void clearWord()                // Fills in word space with underscores based on word length
{
    len = 0;
    for (i = 0; correctWord[i] != '\0'; i++)     // get word length
    {
        len++;
    }
    for (i = 0; i < len; i++)                    // then print the underscores accordingly
    {
        strncpy(&word[i], "_", 1);
    }
}

void reset(void)                    // Clear view and reset all globals
{
    Output_Clear();
    ST7735_FillScreen(ST7735_Color565(0, 0, 0));
    x = 0;
    memset(word, 0, 20);
    chooseWord();
    clearWord();
    strncpy(workingAlpha, alphabet, 26);    // restore the working alphabet to all 26 letters
    lifeCounter = 0;
    winCounter = 0;
    lifeCounterCheck = 0;
    firstTime = 1;
    score = 0;
}

void gameLose() {               // Game Lost State. Shows losing graphic, then resets.
    for (i = 0; i < 10; i++) {
        LCDLineWrite(0, 70, " YOU LOSE ", ST7735_Color565(255, 244, 32), ST7735_Color565(0xff, 0, 0), 2, 12);
        __delay_cycles(3000000);
        LCDLineWrite(0, 70, " YOU LOSE ", ST7735_Color565(0xff, 0, 0), ST7735_Color565(255, 244, 32), 2, 12);
        __delay_cycles(3000000);
    }
    state = 1;
    reset();
}

void gameWin() {               // Game Win State. Shows winning graphic, then resets.
    for (i = 0; i < 10; i++) {
        LCDLineWrite(0, 70, " YOU WIN! ", ST7735_Color565(0, 32, 255), ST7735_Color565(0, 192, 0), 2, 12);
        __delay_cycles(3000000);
        LCDLineWrite(0, 70, " YOU WIN! ", ST7735_Color565(0, 192, 0), ST7735_Color565(0, 32, 255), 2, 12);
        __delay_cycles(3000000);
    }

    if (score > 0) {
        state = 4;
        x = 0;
        firstTime = 1;
    }
    else {
        state = 1;
        reset();
    }
}

void removeChar(char* str, char letter)     // Function for removing a letter from the available working alphabet after a selection
{
    int i, j;
    int len = strlen(str);

    for (i = 0; i < len; i++)
    {
        if (str[i] == letter)
        {
            for (j = i; j <= len; j++)
            {
                str[j] = str[j + 1];
            }
            len--;
            i--;
        }
    }
}

void chooseWord() {                         // Function for selecting a letter from the word bank depending on the difficulty
    srand(time(NULL));
    if (diffState == EASY)
    {
        strcpy(correctWord, bankE[rand() % 25]);
    }
    else if (diffState == MEDIUM)
    {
        strcpy(correctWord, bankM[rand() % 15]);
    }
    else
    {
        strcpy(correctWord, bankH[rand() % 5]);
    }
}

void Display_EEPROM(char line[], int addr) {    // Function for writing leaderboard entry to a specified address
    sprintf(EEPROM_Write[addr - 1], "%c%c%c%c%c%c%c%c", line[0], line[1], line[2], line[3], line[4], line[5], line[6], line[7]);    // Place string in local copy of leaderboard,
    LCDLineWrite(15, (addr * 20), EEPROM_Write[addr - 1], ST7735_Color565(0xff, 0xff, 0xff), ST7735_Color565(0, 0, 0), 2, 9);       // then print that string
}

void adjustLeaderBoard(char line[]) {           // Function for adding leaderboard entry to local copy of leaderboard
    int place = 0;
    
    for (i = 0; i < 6; i++)                     // Compare to strings in EEPROM_Write
    {
        if (strncmp(EEPROM_Write[i], line, 4) > 0)
            place++;
        if (strncmp(EEPROM_Write[i], line, 4) == 0) {
            place++;
            break;
        }
    }

    for (i = 5; i > place; i--)                 // Shifting words in EEPROM_Write after placing line
    {
        strncpy(EEPROM_Write[i], EEPROM_Write[i - 1], 8);

    }

    strncpy(EEPROM_Write[place], line, 8);      // Placing word in EEPROM_Write
}

void writeToLeaderBoard(char line[], int memAddr) {                 // Function for writing leaderboard entry to external leaderboard
    memAddr *= 40;                                                  // Space out addresses by 40
    I2C1_burstWrite(EEPROM_SLAVE_ADDR_WRITE, memAddr, 8, line);     // then write leaderboard entry to EEPROM
}

void readFromLeaderBoard(int addr) {                                // Function for reading leaderboard entry from external leaderboard
    int memAddr = addr * 40;
    char EEPROM_Read[23];

    I2C1_burstRead(EEPROM_SLAVE_ADDR_WRITE, memAddr, 8, EEPROM_Read);       // Read RTC Time Information from EEPROM

    strncpy(EEPROM_Write[(addr - 1)], EEPROM_Read, 8);
}

/* LOOK NO FURTHER. The rest is boring initialization that has no sway over logic. You're brain's just gonna hurt reading past this line. */

void LCDLineWrite(int16_t a, int16_t b, char line[], int16_t textColor, int16_t backColor, uint8_t pixelSize, uint8_t lineLength) {
    int i = 0;
    int spacing = (6 * pixelSize);

    for (i; i < lineLength; i++) {
        ST7735_DrawCharS((a + (spacing * i)), b, line[i], textColor, backColor, pixelSize);
    }
}

void SetupPort5Interrupts()                         //Set up interrupts on Port 5
{
    P5->SEL1 &= ~BIT4;                              //clear bits 5.4. 5.4 is DT
    P5->SEL0 &= ~BIT4;
    P5->DIR &= ~BIT4;                               //set as input

    P5->IES &= ~BIT4;                               //Set Rising Edge
    P5->IE |= BIT4;                                 //Enable the interrupt

    P5->SEL1 &= ~BIT5;                              //clear bits 5.5. 5.5 is CLK
    P5->SEL0 &= ~BIT5;
    P5->DIR &= ~BIT5;                               //set as input

    P5->IES |= BIT5;                                //Set Falling Edge
    P5->IE |= BIT5;                                 //Enable the interrupt
    P5->IFG = 0;                                    //Set Flag to 0
}

void SetupPort1Interrupts()                         //Set up my interrupts on Port 3
{
    P1->SEL1 &= ~BIT7;                              //clear bits 1.6
    P1->SEL0 &= ~BIT7;
    P1->DIR &= ~BIT7;                               //set as input
    P1->REN |= BIT7;                                //set internal resistor
    P1->OUT |= BIT7;
    P1->IES |= BIT7;                                //Set Falling Edge
    P1->IE |= BIT7;                                 //Enable the interrupt
    P1->IFG = 0;                                    //Set Flag to 0
}

void Clock_Init48MHz(void)
{
    // Configure Flash wait-state to 1 for both banks 0 & 1
    FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) |
        FLCTL_BANK0_RDCTL_WAIT_1;
    FLCTL->BANK1_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) |
        FLCTL_BANK1_RDCTL_WAIT_1;

    //Configure HFXT to use 48MHz crystal, source to MCLK & HSMCLK*
    PJ->SEL0 |= BIT2 | BIT3;                    // Configure PJ.2/3 for HFXT function
    PJ->SEL1 &= ~(BIT2 | BIT3);
    CS->KEY = CS_KEY_VAL;                      // Unlock CS module for register access
    CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;
    while (CS->IFG & CS_IFG_HFXTIFG)
        CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;

    /* Select MCLK & HSMCLK = HFXT, no divider */
    CS->CTL1 = CS->CTL1 & ~(CS_CTL1_SELM_MASK |
        CS_CTL1_DIVM_MASK |
        CS_CTL1_SELS_MASK |
        CS_CTL1_DIVHS_MASK) |
        CS_CTL1_SELM__HFXTCLK |
        CS_CTL1_SELS__HFXTCLK;

    CS->CTL1 = CS->CTL1 | CS_CTL1_DIVS_2;    // change the SMCLK clock speed to 12 MHz.

    CS->KEY = 0;                            // Lock CS module from unintended accesses
}

void SysTick_Init() {
    SysTick->CTRL = 0;                            // disable SysTick
    SysTick->LOAD = 0x00FFFFFF;                   // max reload value
    SysTick->VAL = 0;                             // any write to current clears it
    SysTick->CTRL = 0x00000005;                   // enable SysTiick, 3MHz, no interrupts
}

void SysTick_Delay(uint16_t delayms) {
    SysTick->LOAD = ((delayms * 3000) - 1);       // delay for 1 usecond per delay value
    SysTick->VAL = 0;                             // any write to CVR clears it
    while ((SysTick->CTRL & 0x00010000) == 0);    // wait for flag to be set
}

void I2C1_init(void)
{
    EUSCI_B1->CTLW0 |= 1;                   // disable UCB1 during config
    EUSCI_B1->CTLW0 = 0x0F81;               // 7 bit slave addr, master, I2C, synch Mode, use SMCLK
    EUSCI_B1->BRW = 30;                    // set clock prescaler 3MHz/30 = 100kHz;

    // Initialize P6.4 and P6.5 for I2C

    P6->SEL0 |= 0x30;           // P6.4 SDA P6.5 SCL
    P6->SEL1 &= ~0x30;
    EUSCI_B1->CTLW0 &= ~1;    // enable UCB1 after configuration
}

int I2C1_burstWrite(int slaveAddr, unsigned int memAddr, int byteCount, unsigned char* data)
{
    if (byteCount <= 0)
        return -1;                      // -1 if no write was performed

    EUSCI_B1->I2CSA = slaveAddr;        // setup slave address
    EUSCI_B1->CTLW0 |= 1;               // EUSCIB1 reset held for configuration
    EUSCI_B1->CTLW0 |= 0x0010;          // enable transmitter
    EUSCI_B1->CTLW0 |= 0x0002;          // Generate Start and send slave address
    EUSCI_B1->CTLW0 &= ~1;              // EUSCIB1 reset disabled for operation

    while (!(EUSCI_B1->IFG & 2));        // wait until port is ready for transmit "waiting for ACK?"
    EUSCI_B1->TXBUF = memAddr;         // send memory address to slave

    // send data one byte at a time //

    do {
        while (!(EUSCI_B1->IFG & 2));    // wait until port is ready for transmit "waiting for ACK?"
        EUSCI_B1->TXBUF = *data++;      // send data to slave
        byteCount--;                    // decrement byte count
    } while (byteCount > 0);

    while (!(EUSCI_B1->IFG & 2));
    EUSCI_B1->CTLW0 |= 0x0004;          // send STOP
    __delay_cycles(3000000);
    while ((EUSCI_B1->CTLW0 & 4) == 1);         // wait until stop and sent

    return 0;
}

int I2C1_burstRead(int slaveAddr, unsigned int memAddr, int byteCount, unsigned char* data)
{
    if (byteCount <= 0)
        return -1;                      // -1 if no write was performed

    EUSCI_B1->CTLW0 |= 1;               // EUSCIB1 reset held for configuration
    EUSCI_B1->I2CSA = slaveAddr;        // setup slave address
    EUSCI_B1->CTLW0 |= 0x0010;          // enable transmitter       (Sets BIT 4 -- "Received break characters set UCRXIFG) GOOD
    EUSCI_B1->CTLW0 |= 0x0002;          // Generate Start and send slave address    ("next frame transmitted is a break or break/synch")

    EUSCI_B1->CTLW0 &= ~1;              // EUSCIB1 reset disabled for operation

    while (EUSCI_B1->CTLW0 & 2);        // wait until restart is finished
    EUSCI_B1->TXBUF = memAddr;         // send memory address to slave

    while (!(EUSCI_B1->IFG & 2));        // wait until last transmit is complete
    EUSCI_B1->CTLW0 &= ~0x00010;        // Enable receiver
    EUSCI_B1->CTLW0 |= 0x0002;          // Generate RESTART and send Slave Address  ("next frame transmitted is a break or break/synch")
    while (EUSCI_B1->CTLW0 & 2);        // wait until restart is finished

    // Receive data one byte at a time
    do {
        if (byteCount == 1)
            EUSCI_B1->CTLW0 |= 0x0004;      // setup to send Stop after last byte is received

        while (!(EUSCI_B1->IFG & 1));       // wait until data is received
        *data++ = EUSCI_B1->RXBUF;          // read the received data
        byteCount--;
    } while (byteCount);

    while (EUSCI_B1->CTLW0 & 4);           // wait until STOP is sent

    return 0;                               // no error //
}