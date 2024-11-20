//NEW REACTIONG GAME ROUGH IDEA CODE 
#include <stdio.h>
#include <stdlib.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "SYS.h"
#include "LCD.h"
#include "Scankey.h"
#include "TIMER.h"            // Include TIMER.c for timer functions
#include "Seven_Segment.h"    // Include Seven_Segment.h for seven-segment control

#define TMR0 0                     // Timer 0 identifier
#define PERIODIC_MODE 1            // Define PERIODIC_MODE for periodic timer mode
#define SYS_CLK_RATE 12000000      // Example system clock rate, replace with your actual clock rate

// Global variables
uint8_t targetNumber = 0;  // Current random number to guess
uint8_t score = 0;         // Player's score
uint8_t round = 1;         // Current round
uint8_t isGameOver = 0;    // Game status (0 - ongoing, 1 - game over)

// Function prototypes
void initializeGame(void);
void generateRandomNumber(void);
void showScore(void);
void Buzz(int durationOn, int durationOff);

// Initialize the game
void initializeGame(void) {
    score = 0;        // Reset the score
    round = 1;        // Start at round 1
    isGameOver = 0;   // Game is not over
    clear_LCD();      // Clear LCD
    print_Line(0, "IOTReaction Game");
    print_Line(1, "VishallZwanArif");
    print_Line(2, "10 Rounds Total");
    DrvSYS_Delay(500000);  // Shorter delay to show welcome message
    clear_LCD();
    
    // Improved seeding using both Timer and a counter (combined entropy)
    srand((unsigned int)(DrvTIMER_GetIntTicks(0) ^ (DrvTIMER_GetIntTicks(1)) ^ (round)));  // Use Timer ticks and round as seed
}

// Generate a random number between 1 and 9
void generateRandomNumber(void) {
    targetNumber = (rand() % 9) + 1; // Random number between 1 and 9
    char numStr[2];
    sprintf(numStr, "%d", targetNumber); // Convert number to string
    clear_LCD();
    print_Line(0, "Number is:");
    print_Line(1, numStr);
}

// Display the final score with blinking effect and buzzer
// Display the final score with a long buzzer sound
void showScore(void) {
    char scoreStr[16];
    sprintf(scoreStr, "Score: %d / 10", score); // Format score string
    clear_LCD();
    print_Line(0, "Game Over!");
    print_Line(1, scoreStr);

    // Start the long buzzer sound
    DrvGPIO_ClrBit(E_GPB, 11); // Turn ON the buzzer (GPB11 = 0)

    // Blink the score on the seven-segment display
    for (int i = 0; i < 6; i++) { // Blink 6 times (3 seconds if delay is 500ms)
        if (score < 10) {
            // Single digit score
            show_seven_segment(0, score);
        } else {
            // Two digit score
            show_seven_segment(0, score / 10); // Tens place
            DrvSYS_Delay(250000); // Short delay for refresh
            show_seven_segment(1, score % 10); // Units place
        }

        DrvSYS_Delay(500000);         // 500ms on
        close_seven_segment();        // Turn off the display
        DrvSYS_Delay(500000);         // 500ms off
    }

    // Turn OFF the buzzer after the blinking effect
    DrvGPIO_SetBit(E_GPB, 11); // Turn OFF the buzzer (GPB11 = 1)

    // Keep the final score displayed after blinking
    if (score < 10) {
        show_seven_segment(0, score);
    } else {
        show_seven_segment(0, score / 10); // Tens place
        DrvSYS_Delay(250000); // Short delay for refresh
        show_seven_segment(1, score % 10); // Units place
    }
}


// Function to control the buzzer
void Buzz(int durationOn, int durationOff) {
    DrvGPIO_ClrBit(E_GPB, 11); // Turn on Buzzer (GPB11 = 0)
    DrvSYS_Delay(durationOn);  // Delay for buzzer ON duration
    DrvGPIO_SetBit(E_GPB, 11); // Turn off Buzzer (GPB11 = 1)
    if (durationOff > 0) {
        DrvSYS_Delay(durationOff); // Delay for buzzer OFF duration
    }
}

// Main function
int main(void) {
    uint8_t keyInput;

    // System initialization
    UNLOCKREG();
    SYSCLK->PWRCON.XTL12M_EN = 1; // Enable external clock (12MHz)
    SYSCLK->CLKSEL0.HCLK_S = 0;   // Select external clock (12MHz)
    LOCKREG();

    init_LCD();  // Initialize the LCD
    clear_LCD();
    OpenKeyPad(); // Initialize keypad scanning
    DrvGPIO_Open(E_GPB, 11, E_IO_OUTPUT); // Initialize GPIO pin GPB11 for controlling Buzzer

    initializeGame(); // Initialize the game

    while (round <= 10) {
        generateRandomNumber(); // Generate and display a random number

        while (1) {
            keyInput = ScanKey(); // Scan for key press

            if (keyInput > 0 && keyInput <= 9) { // Valid key press
                if (keyInput == targetNumber) {
                    score++; // Increment score for correct guess
                    if (score < 10) {
                        show_seven_segment(0, score); // Display score on the first seven-segment
                    } else {
                        show_seven_segment(0, score / 10); // Tens place
                        DrvSYS_Delay(250000); // Short delay for refresh
                        show_seven_segment(1, score % 10); // Units place
                    }
                }
                break; // Move to the next round regardless of correctness
            }
        }

        // Short delay before proceeding to the next round
        DrvSYS_Delay(500000); // Reduced delay for faster transition between rounds

        round++; // Move to the next round
    }

    showScore(); // Display final score
    isGameOver = 1;

    while (1) {
        // Wait here indefinitely after the game ends
    }
}
