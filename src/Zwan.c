// MOHAMMAD EIZWAN EIZAIDIE BIN MATHEUS
// 22003905
// TEST 1
#include <stdio.h>
#include "NUC1xx.h"
#include "SYS.h"
#include "Seven_Segment.h"
#include "Scankey.h"
#include "LCD.h"

// Static variable to store the score (simulating non-volatile memory)
static int stored_score = 0;

// Function to read the score (you could simulate reading from NVM)
int readScore() {
    return stored_score; // For now, we are just returning the static score variable
}

// Function to write the score (you could simulate writing to NVM)
void writeScore(int score) {
    stored_score = score; // Save the score to the static variable
}

void seg_display(int16_t value) {
    int8_t digit1, digit2;

    if (value >= 10) {
        // For 2-digit score, separate the digits
        digit2 = value / 10;         // Tens place
        digit1 = value % 10;         // Ones place

        // Display both digits simultaneously
        close_seven_segment();  // Clear the 7-segment display before updating
        show_seven_segment(0, digit1);  // Display ones place on the first 7-segment display
				show_seven_segment(1, digit2);  // Display tens place on the second 7-segment display
				DrvSYS_Delay(50000);  // Delay to keep both digits visible for a period


    } else {
        // For single-digit score, show the value on the first display
        digit1 = value;
        close_seven_segment();  // Clear the display before showing the digit
        show_seven_segment(0, digit1);  // Display the single digit on the first display
        DrvSYS_Delay(50000);  // Delay to keep the single digit visible
    }
}




void displayOnLCDWin() {
    clear_LCD();
    print_Line(0, "     YOU WIN!    "); // Display "YOU WIN" message
    print_Line(1, "Press any key    "); // Ask user to press any key
    print_Line(2, "to play again    ");
    print_Line(3, "                ");
}

void resetGame() {
    clear_LCD();
    print_Line(0, "Press 5 to WIN!");
    print_Line(1, "Score:");
}

int main(void) {
    int score = readScore(); // Read the stored score when the program starts
    int last_key = 0;         // To remember the last key pressed (to avoid multiple triggers)

    UNLOCKREG();           // Unlock System Registers
    DrvSYS_Open(48000000); // Set CPU clock to 48 MHz
    LOCKREG();             // Lock System Registers

    // Initialization
    OpenKeyPad();
    init_LCD();
    clear_LCD();

    DrvSYS_Delay(100);

    while (1) {
        // Poll the keypad for a key press
        int key = ScanKey();
			
				resetGame(); // Display initial screen

        

        // Check for key press event (non-blocking)
        if (key != 0 && key != last_key) { // A key was pressed and it’s different from the last one
            last_key = key;  // Update the last key to prevent multiple triggers

            if (key == 5) { // If key 5 is pressed
                score++; // Increment the score

                // Update 7-segment display
                seg_display(score);

                // Display "YOU WIN" on LCD
                displayOnLCDWin();

                // Save the updated score to the "non-volatile" memory
                writeScore(score);
            }
        }

        // Wait for key release to reset the last key state
        if (key == 0 && last_key != 0) { // Key released
            last_key = 0; // Reset last key state after release
        }
    }
}

