#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include "WordBank.h"


char correctWord[20] = "TEST";
char alphabet[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
char workingAlpha[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
char word[20] = "TEST";
char EEPROM_Write[5][9] = { "5555 AAA", "4444 AAB", "3333 ABA", "1111 ABB" };
char* leaderboardLine = { "2222 BAB" };
char x;
diffState = 1;
easy = 1;
med = 2;
hard = 3;
char easy_test_pass[20] = "WINS";
char med_test_pass[20] = "FIGHT";
char hard_test_pass[20] = "ONGOING";
char easy_test_fail[20] = "FAIL";
char med_test_fail[20] = "FAIL";
char hard_test_fail[20] = "FAIL";

int main()
{

    ///Clear word test

    puts("********CLEAR WORD TEST********");

    srand(time(NULL));
    strcpy(correctWord, bank[rand() % 25]);
    printf("CORRECT WORD Before: %s\n", word);
    clearWord();
    assert(strcmp(word, "____")); ///Checking if word was cleared
    printf("CORRECT WORD After: %s\n\n", word);

    ///RemoveChar test

    puts("********REMOVE CHARACTER TEST********");

    printf("Length Before removeChar: %d\n", strlen(workingAlpha));
    removeChar(workingAlpha, workingAlpha[x]);
    printf("Length After removeChar: %d\n\n", strlen(workingAlpha));

    ///adjustLeaderboard Test

    puts("********ADJUST LEADERBOARD TEST********");

    puts("Before adjustLeaderboard: ");
    for (int i = 0; i < strlen(EEPROM_Write); i++)
    {
        puts(EEPROM_Write[i]);
    }

    adjustLeaderBoard(leaderboardLine);
    puts("\nAfter adjustLeaderboard");

    for (int i = 0; i < strlen(EEPROM_Write); i++)
    {
        puts(EEPROM_Write[i]);
    }

    puts("\n");

    ///Alphabet test

    puts("********ALPHABET TEST********");

    for (int i = 65; i <= 90; i++)
    {
        assert(strchr(alphabet, i));
    }

    puts("\nAll letters present");

    ///Letter Selection Test

    puts("********LETTER SELECT TEST********");

    printf("%d", letterSelect(4));

    chooseWord_testEasy_Pass();
    printf("\n");
    chooseWord_testEasy_Fail();
    printf("\n");
    chooseWord_testMed_Pass();
    printf("\n");
    chooseWord_testMed_Fail();
    printf("\n");
    chooseWord_testHard_Pass();
    printf("\n");
    chooseWord_testHard_Fail();

    return 0;
}
//
void clearWord()                // Fills in word space with underscores based on word length
{
    int len = 0;
    for (int i = 0; correctWord[i] != '\0'; i++)     // get word length
    {
        len++;
    }
    for (int i = 0; i < len; i++)                    // then print the underscores accordingly
    {
        strncpy(&word[i], "_", 1);
    }
}

void adjustLeaderBoard(char line[])
{
    // Takes in leaderboard entry and position,
    // logic reformats leaderboard arrays to fit it in, inserts it into the right spot, or slides scores down if higher score inserted
    // also deletes last score in leaderboard if it needs to slide values down

    int place = 0;

    //compare to strings in EEPROM_Write
    for (int i = 0; i < 6; i++)
    {
        if (strncmp(line, EEPROM_Write[i], 4) > 0)
        {
            place = i;
            break;
        }
    }

    //shifting words in EEPROM_Write after placing line
    for (int i = 5; i > place; i--)
    {
        strncpy(EEPROM_Write[i], EEPROM_Write[i - 1], 8);
    }

    strncpy(EEPROM_Write[place], line, 8); //placing word in EEPROM_Write
}

void removeChar(char* str, char charToRemove)
{
    int i, j;
    int len = strlen(str);

    for (i = 0; i < len; i++)
    {
        if (str[i] == charToRemove)
        {
            for (j = i; j < len; j++)
            {
                str[j] = str[j + 1];
            }
            len--;
            i--;
        }
    }
}


int letterSelect(int x)
{
    //printf("Hello");
    for (int i = 0; i < strlen(correctWord); i++)                ///For loop to find current index of correct guess in correctWord
    {
        if (correctWord[i] == workingAlpha[x])                       ///Comparing each letter in correctWord with our guess
        {
            strncpy(&word[i], &workingAlpha[x], 1);                 ///Using "i" to put our guess in the correct position
        }
    }
    // removeChar(workingAlpha, workingAlpha[x]);
    //printf("%s", word);

    //putchar(workingAlpha[x]);

    if (strchr(correctWord, workingAlpha[x]))
        return 1;
    else
        return 0;
}

//easy test
void chooseWord_testEasy_Pass() {
    srand(time(NULL));
    if (easy == 1) {
        strcpy(correctWord, bank_EasyTest[rand() % 1]);
        if (strcmp(correctWord, easy_test_pass) == 0) {
            printf("Easy test has passed. End of Test | ");
        }
        else {
            printf("Easy test failed word was %s should be WINS. ", correctWord);
        }
    }
}

void chooseWord_testEasy_Fail() {
    srand(time(NULL));
    if (easy == 1) {
        strcpy(correctWord, bank_EasyTest[rand() % 1]);
        if (strcmp(correctWord, easy_test_fail) == 0) {
            printf("Easy test has passed ");
        }
        else {
            printf("Easy test failed word was %s should be WINS. END OF TEST | ", easy_test_fail);
        }
    }
}
// MEDIUM
void chooseWord_testMed_Pass() {
    srand(time(NULL));
    if (med == 2) {
        strcpy(correctWord, bank_MedTest[rand() % 1]);
        if (strcmp(correctWord, med_test_pass) == 0) {
            printf("Medium test has passed. End of Test | ");
        }
        else {
            printf("Medium test failed word was %s should be FIGHTS. ", correctWord);
        }
    }
}

void chooseWord_testMed_Fail() {
    srand(time(NULL));
    if (med == 2) {
        strcpy(correctWord, bank_MedTest[rand() % 1]);
        if (strcmp(correctWord, med_test_fail) == 0) {
            printf("Med test has passed End of Test ");
        }
        else {
            printf("medium test failed word was %s should be FIGHTS. END OF TEST | ", med_test_fail);
        }
    }
}

//HARD
void chooseWord_testHard_Pass() {
    srand(time(NULL));
    if (hard == 3) {
        strcpy(correctWord, bank_HardTest[rand() % 1]);
        if (strcmp(correctWord, hard_test_pass) == 0) {
            printf("Hard test has passed. End of Test | ");
        }
        else {
            printf("Hard test failed word was %s should be FIGHTS. ", correctWord);
        }
    }
}

void chooseWord_testHard_Fail() {
    srand(time(NULL));
    if (hard == 3) {
        strcpy(correctWord, bank_HardTest[rand() % 1]);
        if (strcmp(correctWord, hard_test_fail) == 0) {
            printf("Hard test has passed End of Test ");
        }
        else {
            printf("Hard test failed word was %s should be FIGHTS. END OF TEST | ", hard_test_fail);
        }
    }
}