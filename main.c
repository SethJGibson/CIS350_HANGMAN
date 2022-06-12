/*---------------------------------------------------
 Author:      Seth J. Gibson, Jaiden Ortiz, Dennis Salo
 Course:      CIS 350-01
 Description: This program initializes a Red Tab ST7735
                 LCD screen and sends an example menu
                 to the display.
 ---------------------------------------------------*/

#include "msp.h"
#include "WordBank.h"
#include <ST7735.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define MENU_LENGTH 3
#define DIFF_LENGTH 3
#define EASY = 0
#define MEDIUM = 1
#define HARD = 2

void Clock_Init48MHz(void);                         // MCLK and SMCLK initialization
void SysTick_Init();                                // SysTick initialization
void SysTick_Delay(uint16_t delayms);               // SysTick millisecond delay
void SetupPort5Interrupts();                        // Set up interrupts on Port 5
void SetupPort1Interrupts();                        // Set up interrupts on Port 3
void PORT5_IRQHandler(void);                        // Block that executes after PORT5 interrupt (Knob turning)
void PORT1_IRQHandler(void);                        // Block that executes after PORT1 interrupt (Button press)
                                                    // Writes a string to the LCD
void gameInProgressRotate(void);
void gameInProgressButton(void);
void mainMenuRotate(void);
void mainMenuButton(void);
void difficultyRotate(void);
void difficultyButton(void);
void leaderboardRotate(void);
void leaderboardButton(void);

void hangTheMan();
void clearWord();
void gameLose();
void gameWin();
void removeChar(char *str, char letter);
void LCDLineWrite(int16_t a, int16_t b, char line[], int16_t textColor, int16_t backColor, uint8_t pixelSize, uint8_t lineLength);

const unsigned short PoCv2[] = {/* DATA GOES HERE */};  // IGNORE.
// To show images, .bmp files need to be broken down into hex and called as char arrays. The data usually go here.

int i = 0;                      // CodeComposer hates the i in for loops if its not up here
int state = 0;                  // 0 = Game, 1 = Menu, 2 = Difficulty, 3 = Leaderboard
int diffState = 0;              // 0 = Easy, 1 = Medium, 2 = Hard
int firstTime = 1;
volatile uint32_t x = 0;        // Iterator variable, decides the knobs place in the alphabet shown on screen
char letter[5];                 // Current letter from alphabet to be shown on screen
int len = 0;
char word[20] = "";
char correctWord[20] = "TEST";      ///This is meant to hold the correct word to be guessed
char alphabet[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
char workingAlpha[26];
char *mainMenu[MENU_LENGTH] = {"Start", "Difficulty", "Leaderboard"}; 
char *difficulty[3] = {"Easy", "Medium", "Hard"};
int lifeCounter = 0;
int winCounter = 0;
int lifeCounterCheck = 0;

void main(void) {                                                   /* IGNORE THIS BLOCK, its all boring hardware setup */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // Stop WatchDog timer
    Clock_Init48MHz();                              // Initialize clock to 48 MHz
    ST7735_InitR(INITR_REDTAB);                     // Initialize LCD as red tab
    SetupPort5Interrupts();                         // Setup GPIO on port 5 interrupts
    NVIC_EnableIRQ(PORT5_IRQn);                     // Turn on port 5 interrupts
    SetupPort1Interrupts();                         // Setup GPIO on port 1 interrupts
    NVIC_EnableIRQ(PORT1_IRQn);                     // Turn on port 1 interrupts
    __enable_irq();                                 // Enable all interrupts
                                                    /* OK now you can start paying attention again. */

    Output_Clear();                                 // Initial command to clear screen before anything

    uint16_t white = ST7735_Color565(255, 255, 255);    // LCD color macros for black and white
    uint16_t black = ST7735_Color565(0,0,0);

    ST7735_FillScreen(black);                       // Set black background

    chooseWord();                                   //Selecting random word from bank based on difficulty

    strncpy(workingAlpha, alphabet, 26);
    clearWord();

    while(1)                                                // Infinite loops are key to keeping variables updated live on screen
    {
        switch (state) {
            case 0:
                if (firstTime) {
                    // Display Graphics
                    firstTime = 0;
                }
                sprintf(letter, "%c", workingAlpha[x]);                 // Put letter in a string
                LCDLineWrite(16, 60, letter, white, black, 5, 1);   // then print that string
                LCDLineWrite(16, 120, word, white, black, 2, 20);   // The full word goes here too

                if (lifeCounter != lifeCounterCheck) {              // Checks input to see if change has been made
                    hangTheMan();                                   // if there was, add a limb to the m a n
                }

                if (lifeCounterCheck == 6) {                        // Checks if the hangman is completed
                    gameLose();                                     // if he is, end the game
                }

                if (winCounter == len) {                        // Checks if the hangman is completed
                    gameWin();                                     // if he is, end the game
                }
                break;
            case 1:
                if (firstTime) {
                    LCDLineWrite(16, 60, "MENU", white, black, 5, 5);   // Test string to show it entered menu state
                    firstTime = 0;
                }

                break;
            case 2:
                if (firstTime) {

                    firstTime = 0;
                }

                break;
            case 3:
                if (firstTime) {

                    firstTime = 0;
                }

                break;
        }
    }
}

void PORT5_IRQHandler(void)                         // Interrupt handler triggers when the knob turns. This logic decides which letter we're on.
{                                                   // This block is currently unfinished due to some noticeable jank.
    if (P5->IFG & BIT5 )                            // Checking if the Encoder CLK is high
    {
        if (P5->IN & BIT4){                         // If the CLK is high and DT is high, its clockwise
            __delay_cycles(30000);                  // Wait 10 ms and check again. This debounces the input.
            if (P5->IN & BIT4){
                x++;
            }
        }
        else {                                      // If the CLK is high and DT is not high, its CCW
            __delay_cycles(30000);
            if (P5->IN & BIT4){
                x++;                                // Currently it only counts up no matter how you turn it.
            }
        }
    }

    switch (state) {
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
    }

    P5->IFG = 0;                                    // Reset GPIO flag
}

void PORT1_IRQHandler(void)                         // Interrupt handler for the button press. This is where letter select logic goes.
{
    if (P1->IFG & BIT7)                            // Knob has a button built in. This checks if the button signal is high
    {
        switch (state) {
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
        }
    }
    P1->IFG = 0;                                    // reset GPIO flag
}

void gameInProgressRotate(void)
{
    if(x > (strlen(workingAlpha)))                                 // If x reached the end of the alphabet, reset to 0
        x = 0;
}

void gameInProgressButton(void) {
    if(strchr(correctWord, workingAlpha[x]) != NULL)
    {

        for(i = 0; i < strlen(correctWord); i++)            // For loop to find current index of correct guess in correctWord
        {
            if(correctWord[i] == workingAlpha[x])           // Comparing each letter in correctWord with our guess
            {
                strncpy(&word[i], &workingAlpha[x], 1);     // Using "i" to put our guess in the correct position
                winCounter++;
            }
        }
    }
    else {
        lifeCounter++;
    }
    removeChar(workingAlpha, workingAlpha[x]);
}

void mainMenuRotate(void)
{
    if(x > (MENU_LENGTH))                                 // If x reached the end of the menu options, reset to 0
        x = 0;
}

//Menu selection "changes" state
void mainMenuButton(void)
{
    if(x = 0)
        state = 0;
    else if (x = 1)
        state = 2;
    else if (x = 2)
        state = 3;
}

void difficultyRotate(void)
{
    if(x > DIFF_LENGTH)                                 // If x reached the end of the difficulty options, reset to 0
        x = 0;
}

void difficultyButton(void)
{
    diffState = x;                                  //Selected difficulty depends on value of x
}

void leaderboardRotate(void)
{

}

void leaderboardButton(void)
{

}

void hangTheMan() {
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

void clearWord()                // Fills in word space with underscores based on word length
{
    len = 0;
    for(i = 0; correctWord[i] != '\0'; i++)     // get word length
    {
        len++;
    }
    for(i = 0; i < len; i++)                    // then print the underscores accordingly
    {
        strncpy(&word[i], "_", 1);
    }
}

void reset()                    // Clear view and reset all globals
{
    Output_Clear();
    x = 0;
    memset(word, 0, 20);
    strcpy(correctWord, bank[rand() % 25]);         //copy random word from bank to correctWord
    clearWord();
    // clear or reassign correctWord from wordlist. this can be function or we do it right here
    strncpy(workingAlpha, alphabet, 26);    // restore the working alphabet to all 26 letters
    lifeCounter = 0;
    winCounter = 0;
    lifeCounterCheck = 0;
    firstTime = 1;
    state = 1;
}

void gameLose() {               // Game Lost State. Shows losing graphic, then resets.
    for (i = 0; i < 10; i++) {
        LCDLineWrite(0, 70, " YOU LOSE ", ST7735_Color565(255, 244, 32), ST7735_Color565(0xff, 0, 0), 2, 12);
        __delay_cycles(3000000);
        LCDLineWrite(0, 70, " YOU LOSE ", ST7735_Color565(0xff, 0, 0), ST7735_Color565(255, 244, 32), 2, 12);
        __delay_cycles(3000000);
    }
    reset();
}

void gameWin() {               // Game Win State. Shows winning graphic, then resets.
    for (i = 0; i < 10; i++) {
        LCDLineWrite(0, 70, " YOU WIN! ", ST7735_Color565(0, 32, 255), ST7735_Color565(0, 192, 0), 2, 12);
        __delay_cycles(3000000);
        LCDLineWrite(0, 70, " YOU WIN! ", ST7735_Color565(0, 192, 0), ST7735_Color565(0, 32, 255), 2, 12);
        __delay_cycles(3000000);
    }
    reset();
}

void removeChar(char *str, char letter)     // Function for removing a letter from the available working alphabet after a selection
    {
        int i, j;
        int len = strlen(str);

        for(i = 0; i < len; i++)
        {
            if(str[i] == letter)
            {
                for(j = i; j <= len; j++)
                {
                    str[j] = str[j + 1];
                }
                len--;
                i--;
            }
        }
    }

void chooseWord()
{
    char wordOnDeck[20] = "";
    srand(time(NULL));                              //Initialize random function
    //strcpy(correctWord, bank[rand() % 25]);         //copy random word from bank to correctWord
    if(diffState = EASY)
    {
        strcpy(correctWord, bank[rand() % 25]);
    }
    else
    {
        while(strlen(wordOnDeck) < 5 || strlen(wordOnDeck) > 8)
        {
            strcpy(wordOnDeck, bank[rand() % 25]);
        }

        strcpy(correctWord, wordOnDeck);
    }
}


/* LOOK NO FURTHER. The rest is boring initialization shit that has no sway over logic. You're brain's just gonna hurt reading past this line. */

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

      P5->IES &=~ BIT4;                               //Set Rising Edge
      P5->IE |= BIT4;                                 //Enable the interrupt

      P5->SEL1 &= ~BIT5;                              //clear bits 5.5. 5.5 is CLK
      P5->SEL0 &= ~BIT5;
      P5->DIR &= ~BIT5;                               //set as input

      P5->IES |= BIT5;                               //Set Falling Edge
      P5->IE |= BIT5;                                 //Enable the interrupt
      P5 -> IFG = 0;                                  //Set Flag to 0
}

void SetupPort1Interrupts()                         //Set up my interrupts on Port 3
{
    P1->SEL1 &= ~BIT7;                              //clear bits 1.6
    P1->SEL0 &= ~BIT7;
    P1->DIR &= ~BIT7;                               //set as input
    P1 -> REN |= BIT7;                              //set internal resistor
    P1->OUT |= BIT7;
    P1->IES |= BIT7;                                //Set Falling Edge
    P1->IE |= BIT7;                                 //Enable the interrupt
    P1 -> IFG = 0;                                  //Set Flag to 0
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
    CS->KEY = CS_KEY_VAL ;                      // Unlock CS module for register access
    CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;
        while(CS->IFG & CS_IFG_HFXTIFG)
                    CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;

    /* Select MCLK & HSMCLK = HFXT, no divider */
    CS->CTL1 = CS->CTL1 & ~(CS_CTL1_SELM_MASK   |
                            CS_CTL1_DIVM_MASK   |
                            CS_CTL1_SELS_MASK   |
                            CS_CTL1_DIVHS_MASK)   |
                            CS_CTL1_SELM__HFXTCLK |
                            CS_CTL1_SELS__HFXTCLK;

    CS->CTL1 = CS->CTL1 |CS_CTL1_DIVS_2;    // change the SMCLK clock speed to 12 MHz.

    CS->KEY = 0;                            // Lock CS module from unintended accesses
}

void SysTick_Init() {
    SysTick -> CTRL = 0;                            // disable SysTick
    SysTick -> LOAD = 0x00FFFFFF;                   // max reload value
    SysTick -> VAL = 0;                             // any write to current clears it
    SysTick -> CTRL = 0x00000005;                   // enable SysTiick, 3MHz, no interrupts
}

void SysTick_Delay(uint16_t delayms) {
    SysTick -> LOAD = ((delayms * 3000) - 1);       // delay for 1 usecond per delay value
    SysTick -> VAL = 0;                             // any write to CVR clears it
    while ((SysTick -> CTRL & 0x00010000) == 0);    // wait for flag to be set
}
