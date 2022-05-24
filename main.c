/*---------------------------------------------------
 Author:      Seth J. Gibson (For now)
 Course:      CIS 350-01
 Description: This program initializes a Red Tab ST7735
                 LCD screen and sends an example menu
                 to the display.
 ---------------------------------------------------*/

#include "msp.h"
#include <ST7735.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void Clock_Init48MHz(void);                         // MCLK and SMCLK initialization
void SysTick_Init();                                // SysTick initialization
void SysTick_Delay(uint16_t delayms);               // SysTick millisecond delay
void SetupPort5Interrupts();                        // Set up interrupts on Port 5
void SetupPort1Interrupts();                        // Set up interrupts on Port 3
void PORT5_IRQHandler(void);                        // Block that executes after PORT5 interrupt (Knob turning)
void PORT1_IRQHandler(void);                        // Block that executes after PORT1 interrupt (Button press)
                                                    // Writes a string to the LCD
void LCDLineWrite(int16_t a, int16_t b, char line[], int16_t textColor, int16_t backColor, uint8_t pixelSize, uint8_t lineLength);

const unsigned short PoCv2[] = {/* DATA GOES HERE */};  // IGNORE.
// To show images, .bmp files need to be broken down into hex and called as char arrays. The data usually go here.

int i = 0;                      // CodeComposer hates the i in for loops if its not up here
volatile uint32_t x = 0;        // Iterator variable, decides the knobs place in the alphabet shown on screen
char letter[5];                 // Current letter from alphabet to be shown on screen
char word[20] = "T E S T";
char alphabet[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

void main(void)
{                                                   /* IGNORE THIS BLOCK, its all boring hardware setup */
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

    /* 12. THE HANGED MAN */
    ST7735_FillRect(82, 22, 15, 15, white);     // Head
    ST7735_FillRect(82, 38, 15, 35, white);     // Torso
    ST7735_FillRect(75, 38, 6, 30, white);      // ArmL
    ST7735_FillRect(98, 38, 6, 30, white);      // ArmR
    ST7735_FillRect(82, 74, 6, 30, white);      // LegL
    ST7735_FillRect(91, 74, 6, 30, white);      // LegR

    while(1)                                                // Infinite loops are key to keeping variables updated live on screen
    {
        sprintf(letter, "%c", alphabet[x]);                 // Put letter in a string
        LCDLineWrite(16, 60, letter, white, black, 5, 1);   // then print that string
        LCDLineWrite(16, 120, word, white, black, 2, 20);   // The full word goes here too
    }
}

void PORT5_IRQHandler(void)                         // Interrupt handler triggers when the knob turns. This logic decides which letter we're on.
{                                                   // This block is currently unfinished due to some noticeable jank.
    if (P5->IFG & BIT5 )                            // Checking if the Encoder CLK is high
    {
        if(x > 25){                                 // If x reached the end of the alphabet, reset to 0
            x = 0;
        }
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

    P5->IFG = 0;                                    // Reset GPIO flag
}

void PORT1_IRQHandler(void)                         // Interrupt handler for the button press. This is where letter select logic goes.
{
    if (P1->IFG & BIT6 )                            // Knob has a button built in. This checks if the button signal is high
    {
        if(strchr(correctWord, alphabet[x]) != NULL)			///FIXME: This logic currently only adds one letter to word
        {								///even if there are more than one of same letter
		
            for(i = 0; i < strlen(correctWord); i++)                ///For loop to find current index of correct guess in correctWord
            {
                if(correctWord[i] == alphabet[x])                       ///Comparing each letter in correctWord with our guess
                {
                    strncpy(&word[i], &alphabet[x], 1);                 ///Using "i" to put our guess in the correct position
                }
            }
            //strncpy(&word[2], &alphabet[x], 1);                 //Copies letter onto string word
            ///strncat(word, &alphabet[x], 1);             // This currently does not update the word on screen. Haven't figured out why yet.
        }
    }
    P1->IFG = 0;                                    // reset GPIO flag
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
    P1->SEL1 &= ~BIT6;                              //clear bits 1.6
    P1->SEL0 &= ~BIT6;
    P1->DIR &= ~BIT6;                               //set as input
    P1 -> REN |= BIT6;                              //set internal resistor
    P1->OUT |= BIT6;
    P1->IES |= BIT6;                                //Set Falling Edge
    P1->IE |= BIT6;                                 //Enable the interrupt
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
