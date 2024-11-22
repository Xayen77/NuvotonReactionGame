// NOT INCLUDE CAN SERIAL COMMUNICATION

#include "GPIO.h"
#include "Scankey.h"
#include "LCD.h"
#include "Seven_Segment.h"
#include <stdlib.h>  // For random number generation
#include <stdio.h>   // For sprintf

void delay(void); // Function prototype for delay

void initGame() {
    // Initialize the LCD, keypad, 7-segment, and other peripherals
    init_LCD();        // Initialize LCD
    clear_LCD();       // Clear LCD
    OpenKeyPad();      // Initialize Keypad
    close_seven_segment(); // Ensure 7-segment display is off initially
}

void displayWelcome() {
    clear_LCD();
    print_Line(0, "REACTION GAME");
		print_Line(1,"VishallZwanArif"); 
	  print_Line(2, "Press any key"); 
		print_Line(3, "to start"); 
		
}

void displayTargetKey(uint8_t targetKey) {
    char message[16]; // Declare variables at the top of the block
    clear_LCD();
    sprintf(message, "Press Key: %d", targetKey);
    print_Line(0, message);
    delay();  // Delay added to allow the user time to see the target key
}


void displayCongrats() {
    clear_LCD();
    print_Line(0, "Congratulations!");
}

void updateSevenSegment(uint16_t score) {
    uint8_t tens, units;

    // Extract tens and units digits
    tens  = (score / 10) % 10;  // Tens place (0-9)
    units = score % 10;         // Units place (0-9)

    // Turn off all segments first (clear the display)
    close_seven_segment();

    // Show tens and units place on the 7-segment display
    show_seven_segment(0, tens); // Segment 0 = tens place
    show_seven_segment(1, units); // Segment 1 = units place
}




void buzzerSound() {
    // Sound the buzzer for success
    DrvGPIO_SetBit(E_GPB, 10); // Hypothetical buzzer pin
    delay();                   // Short delay
    DrvGPIO_ClrBit(E_GPB, 10);
}

int main(void) {
    uint16_t score = 0;
    uint8_t targetKey = 0, pressedKey = 0;

    initGame();
    displayWelcome();

    // Wait for any key press to start
    while (ScanKey() == 0);

    while (1) {
        // Generate and display a random target key (1-9)
        targetKey = (rand() % 9) + 1;
        displayTargetKey(targetKey);

        // Wait until the correct key is pressed
        do {
            pressedKey = ScanKey();
            delay(); // Small delay to reduce CPU load
        } while (pressedKey == 0 || pressedKey != targetKey);

        // Process correct key press
        if (pressedKey == targetKey) {
            score += 5;              // Increment score by 5
            buzzerSound();           // Play success sound
            displayCongrats();       // Show success message
            updateSevenSegment(score); // Display the updated score

            delay(); // Short delay before generating a new target
        }
    }

    return 0;
}
