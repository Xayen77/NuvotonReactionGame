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
void seg_display(int16_t value);

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

// Display the final score with blinking effect
// Display the final score with blinking effect
// Display the final score with blinking effect and buzzer sound
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
        seg_display(score); // Use seg_display() to handle single or two-digit scores
        DrvSYS_Delay(500000);         // 500ms on
        close_seven_segment();        // Turn off the display
        DrvSYS_Delay(500000);         // 500ms off
    }

    // Keep the final score displayed after blinking
    seg_display(score);

    // Turn OFF the buzzer after the blinking effect
    DrvGPIO_SetBit(E_GPB, 11); // Turn OFF the buzzer (GPB11 = 1)
}



// Display an integer on two or more 7-segment displays
void seg_display(int16_t value) {
    int8_t tens, units;

    // Calculate tens and units place
    tens = value / 10;
    units = value % 10;

    // If the score is single-digit, display only the units place
    if (value < 10) {
        close_seven_segment();
        show_seven_segment(0, units); // Units place on the first segment
        DrvSYS_Delay(5000);
    } else {
        // For two-digit numbers, display both digits with a slight overlap
        for (int i = 0; i < 5; i++) { // Show each digit alternately multiple times
            close_seven_segment();
            show_seven_segment(1, tens); // Tens place on the second segment
            DrvSYS_Delay(2500);         // Short delay for refresh

            close_seven_segment();
            show_seven_segment(0, units); // Units place on the first segment
            DrvSYS_Delay(2500);         // Short delay for refresh
        }
        // Keep both digits displayed simultaneously after the loop
        close_seven_segment();
        show_seven_segment(1, tens);   // Tens place
        DrvSYS_Delay(5000);           // Ensure enough time for visibility
        show_seven_segment(0, units); // Units place
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

    initializeGame(); // Initialize the game

    while (round <= 10) {
        generateRandomNumber(); // Generate and display a random number

        while (1) {
            keyInput = ScanKey(); // Scan for key press

            if (keyInput > 0 && keyInput <= 9) { // Valid key press
                if (keyInput == targetNumber) {
                    score++; // Increment score for correct guess
                    seg_display(score); // Display score using seg_display()
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
