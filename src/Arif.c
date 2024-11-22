// CAN COMMUNICATION INCLUDED
// Scoring System 

#include <stdio.h>
#include <stdlib.h>
#include "NUC1xx.h"
#include "GPIO.h"
#include "CAN.h"
#include "SYS.h"
#include "LCD.h"
#include "Scankey.h"
#include "Seven_Segment.h"

#define MAX_ROUNDS 10 // Maximum number of rounds


char TEXT0[16] = "CAN :           ";
char TEXT1[16] = "ID  :           ";
char TEXT2[16] = "Data:           ";
char TEXT3[16] = "                ";


// Global variables
uint8_t target_number = 0;   // Target number to press
uint8_t is_target_set = 0;   // Flag to indicate if target is set
uint8_t score = 0;           // Player's score
uint8_t round = 1;           // Current round
uint8_t other_score = 0;   // Variable to track the other player's score
uint8_t other_scoree= 0; 


// Function prototypes
void CAN_ShowMsg(STR_CANMSG_T* Msg);
void StartGameRound(void);
void CheckInput(uint8_t key);
void showFinalScore(void);
void seg_display(int16_t value);


// Interrupt Service Routine / Callback function
STR_CANMSG_T rrMsg;


void CAN_CallbackFn(uint32_t u32IIDR)
{
    if (u32IIDR == 1)
    {
        DrvCAN_ReadMsgObj(0, TRUE, &rrMsg);
        CAN_ShowMsg(&rrMsg);

        switch (rrMsg.Data[0])
        {
        case 'T': // 'T' indicates target number
            target_number = rrMsg.Data[1] - '0'; // Extract target number
            is_target_set = 1; // Activate the target
            break;

        case 'S': // 'S' indicates score update
            // Parse the scores sent in the message
            if (rrMsg.Data[1] == '1')
                other_score = rrMsg.Data[2] - '0';
            else if (rrMsg.Data[1] == '2')
                other_scoree = rrMsg.Data[2] - '0';
            break;

        case 'L': // 'L' indicates the game is over
            showFinalScore(); // Call the game-ending function
            break;

        case 'N': // 'N' indicates next round
            round = rrMsg.Data[1] - '0'; // Sync round number
            StartGameRound(); // Start the new round
            break;

        default:
            // Handle unexpected messages if needed
            break;
        }
    }
}


// Display Message on LCD
void CAN_ShowMsg(STR_CANMSG_T* Msg)
{
    uint8_t i;
    sprintf(TEXT1 + 6, "%x", Msg->Id);
   // print_Line(1, TEXT1);

    for (i = 0; i < Msg->DLC; i++)
        sprintf(TEXT2 + 6 + i, "%c", Msg->Data[i]);
    //print_Line(2, TEXT2);
}


// TX send ID & Data
void CAN_TX(uint8_t number, char type)
{
    STR_CANMSG_T tMsg;

    /* Send an 11-bits message */
    tMsg.FrameType = DATA_FRAME;
    tMsg.IdType = CAN_STD_ID; // Standard 11-bit ID
    tMsg.Id = 0x700; // Fixed ID for simplicity
    tMsg.DLC = 2; // Data length = 2 bytes

    tMsg.Data[0] = type; // Type: 'T' for target, 'W' for win
    tMsg.Data[1] = 0x30 + number; // Send number as ASCII

    if (DrvCAN_SetTxMsgObj(MSG(1), &tMsg) < 0)
        return; // Failed to set message

    DrvCAN_SetTxRqst(MSG(1)); // Transmit the message
}


// RX set Mask Filter
void SetMaskFilter()
{
    STR_CANMASK_T MaskMsg;

    DrvCAN_EnableInt(CAN_CON_IE);
    DrvCAN_InstallCallback(CALLBACK_MSG, (CAN_CALLBACK)CAN_CallbackFn);

    MaskMsg.u8Xtd = 1;
    MaskMsg.u8Dir = 1;
    MaskMsg.u8IdType = 0; // Standard ID
    MaskMsg.u32Id = 0x700;
    DrvCAN_SetMsgObjMask(MSG(0), &MaskMsg);
    DrvCAN_SetRxMsgObj(MSG(0), CAN_STD_ID, 0x7FF, TRUE);
}


// Start a new game round
void StartGameRound()
{
    if (round > MAX_ROUNDS)
    {
        showFinalScore(); // End game if max rounds are reached
        return;
    }

    target_number = (rand() % 9) + 1; // Generate random target number (1-9)
    is_target_set = 1; // Mark the target as active

    // Display the round and target on the LCD
    clear_LCD();
    char roundInfo[16];
    sprintf(roundInfo, "Round: %d", round);
    print_Line(0, roundInfo);
    sprintf(TEXT3, "Target: %d   ", target_number);
    print_Line(1, TEXT3);

    // Notify the other boards of the new target
    CAN_TX(target_number, 'T');
}


// Check if input matches the target
void CheckInput(uint8_t key)
{
    if (!is_target_set)
        return;

    if (key == target_number)
    {
        // Player wins this round
        score++;
        is_target_set = 0; // Reset target flag

        // Update score on the seven-segment display
        seg_display(score);

        // Check if the score reached 10
        

        // Notify the other board to move to the next round
        CAN_TX(round + 1, 'N'); // Send 'N' message with the next round number

        round++; // Move to the next round
        StartGameRound();
    }
    else
    {
        sprintf(TEXT3, "Try Again!   ");
       // print_Line(3, TEXT3);
    }
}


// Display the final score
void showFinalScore()
{
    char scoreStr[16];
    clear_LCD(); // Clear the LCD for the final message

    // Display final score
    sprintf(scoreStr, "Your Score: %d", score);
    print_Line(0, "Game Over!");
    print_Line(1, scoreStr);

    // Determine the winner among the three players
    if (score > other_score && score > other_scoree)
    {
        print_Line(2, "You Win!");
    }
    else if (other_score > score && other_score > other_scoree)
    {
        print_Line(2, "Opponent 1 Wins!");
    }
    else if (other_scoree > score && other_scoree > other_score)
    {
        print_Line(2, "Opponent 2 Wins!");
    }
    else
    {
        print_Line(2, "You Lose!");
    }

    // Signal game over (optional buzzer logic)
    DrvGPIO_ClrBit(E_GPB, 11); // Turn ON the buzzer
    DrvSYS_Delay(1000000);     // Delay for buzzer sound
    DrvGPIO_SetBit(E_GPB, 11); // Turn OFF the buzzer

    // Blink the final score on the 7-segment display
    for (int i = 0; i < 6; i++)
    {
        seg_display(score);
        DrvSYS_Delay(500000); // 500ms on
        close_seven_segment(); // Turn off the display
        DrvSYS_Delay(500000); // 500ms off
    }

    // Stop the game by notifying other players
    CAN_TX(0, 'L'); // Broadcast 'L' for game over
    is_target_set = 0;        // Disable target
    round = MAX_ROUNDS + 1;   // Prevent further rounds
}


// Display an integer on two or more 7-segment displays
void seg_display(int16_t value)
{
    int8_t tens = value / 10;
    int8_t units = value % 10;

    if (value < 10)
    {
        close_seven_segment();
        show_seven_segment(0, units);
    }
    else
    {
        close_seven_segment();
        show_seven_segment(1, tens);
        show_seven_segment(0, units);
    }
}


// Main function
int main(void)
{
    uint8_t keyin;

    UNLOCKREG();
    DrvSYS_SetOscCtrl(E_SYS_XTL12M, 1);
    DrvSYS_Delay(5000); /* Delay for Xtal stable */
    DrvSYS_SelectHCLKSource(0);
    LOCKREG();

    init_LCD(); // Initialize LCD panel
    clear_LCD();
    OpenKeyPad(); // Initialize Keypad

    DrvGPIO_Open(E_GPB, 12, E_IO_OUTPUT); // CAN Transceiver setting
    DrvGPIO_ClrBit(E_GPB, 12);

    DrvGPIO_InitFunction(E_FUNC_CAN0);
    DrvCAN_Init();

    DrvCAN_Open(100); // Set CAN speed to 100 Kbps

    srand(1234); // Fixed seed for random numbers (replace with dynamic seed if needed)
    SetMaskFilter();

    StartGameRound(); // Start the first round

    while (round <= MAX_ROUNDS)
    {
        keyin = ScanKey(); // Scan keypad to input a number
        if (keyin != 0)
        {
            CheckInput(keyin); // Check if the input matches the target
        }
    }

    while (1); // End of game, wait indefinitely
}
