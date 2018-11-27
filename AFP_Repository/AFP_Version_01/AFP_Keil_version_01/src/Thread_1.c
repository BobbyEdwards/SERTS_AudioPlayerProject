#include "cmsis_os.h"  // CMSIS RTOS header file
#include "Board_LED.h"
#include "UART_driver.h"
#include "stdint.h"                     // data type definitions
#include "stdio.h"                      // file I/O functions
#include "rl_usb.h"                     // Keil.MDK-Pro::USB:CORE
#include "rl_fs.h"                      // Keil.MDK-Pro::File System:CORE
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio.h"
#include <stdio.h>

#define LED_Green   0
#define LED_Orange  1
#define LED_Red     2
#define LED_Blue    3

////////////////////////////////////////////////
// State Machine definitions
enum state{
  NoState,
  Idle,
  List,
};

enum commands{
  ListFiles,
  SendComplete,
  SendFiles
};

#define Show_Files_char "1"
#define Receive_File_char "4"

#define Play_char "P"
#define Pause_char "S"

char *StartFileList_msg = "2\n";
char *EndFileList_msg = "3\n";
//////////////////////////////////////////////////////////
void Control (void const *argument); // thread function
osThreadId tid_Control; // thread id
osThreadDef (Control, osPriorityNormal, 1, 0); // thread object

// Command queue from Rx_Command to Controller
osMessageQId mid_CMDQueue; // message queue for commands to Thread
osMessageQDef (CMDQueue, 1, uint32_t); // message queue object

// UART receive thread
void Rx_Command (void const *argument);  // thread function
osThreadId tid_RX_Command;  // thread id
osThreadDef (Rx_Command, osPriorityNormal, 1, 0); // thread object

// Blue Blink thread
void FS_Thread (void const *argument);                             // thread function
osThreadId tid_FS_Thread;                                          // thread id
osThreadDef (FS_Thread, osPriorityNormal, 1, 0);                   // thread object

void Process_Event(uint16_t event){
  static uint16_t   Current_State = NoState; // Current state of the SM
  switch(Current_State){
    case NoState:
      // Next State
      Current_State = Idle;
      // Exit actions
      // Transition actions
      // State1 entry actions
      LED_On(LED_Red);
      break;
    case Idle:
      if(event == SendFiles){
        Current_State = List;
        // Exit actions
        LED_Off(LED_Red);
        // Transition actions
        // List entry actions
        LED_On(LED_Green);
      }
      break;
    case List:
      if(event == SendComplete){
        Current_State = Idle;
        // Exit actions
        LED_Off(LED_Green);
        // Transition actions
        // Idle state entry actions
        LED_On(LED_Red);
      }
      break;
    default:
      break;
  } // end case(Current_State)
} // Process_Event


void Init_Thread (void) {
   LED_Initialize(); // Initialize the LEDs
   UART_Init(); // Initialize the UART
  // Create queues
   mid_CMDQueue = osMessageCreate (osMessageQ(CMDQueue), NULL);  // create msg queue
	if (!mid_CMDQueue)return; // Message Queue object not created, handle failure
		
  // Create threads
   tid_RX_Command = osThreadCreate (osThread(Rx_Command), NULL);
  if (!tid_RX_Command) return;
   tid_Control = osThreadCreate (osThread(Control), NULL);
  if (!tid_Control) return;
  tid_FS_Thread = osThreadCreate (osThread(FS_Thread), NULL);
  if (!tid_FS_Thread) return;
  }

// Thread function
void Control(void const *arg){
  osEvent evt; // Receive message object
  Process_Event(0); // Initialize the State Machine
   while(1){
    evt = osMessageGet (mid_CMDQueue, osWaitForever); // receive command
      if (evt.status == osEventMessage) { // check for valid message
      Process_Event(evt.value.v); // Process event
    }
   }
}

void Rx_Command (void const *argument){
   char rx_char[2]={0,0};
	 char fileName[50] = {0};
   while(1){
      UART_receive(rx_char, 1); // Wait for command from PC GUI
    // Check for the type of character received
      if(!strcmp(rx_char, Show_Files_char)){
         // Trigger1 received
         osMessagePut (mid_CMDQueue, SendFiles, osWaitForever);
      }
			if (!strcmp(rx_char, Receive_File_char)){
        // Trigger2 received
				UART_receivestring(fileName, 50);
				LED_On(LED_Orange);
				LED_Off(LED_Orange);
      }
			if(!strcmp(rx_char, Play_char)){
         // Trigger1 received
         //osMessagePut (mid_CMDQueue, SendFiles, osWaitForever);
				LED_Off(LED_Orange);
				LED_On(LED_Green);
      }
			if(!strcmp(rx_char, Pause_char)){
         // Trigger1 received
         //osMessagePut (mid_CMDQueue, SendFiles, osWaitForever);
				LED_Off(LED_Green);
				LED_On(LED_Orange);
      }
			// end if
   }
} // end Rx_Command

void FS_Thread(void const *arg){
//=====================================================================================
	// USB Setup
	usbStatus ustatus; // USB driver status variable
	uint8_t drivenum = 0; // Using U0: drive number
	char *drive_name = "U0:"; // USB drive name
	fsStatus fstatus; // file system status variable
	//static FILE *f;
	
	//LED_On(LED_Green);
	ustatus = USBH_Initialize (drivenum); // initialize the USB Host
	if (ustatus == usbOK){
		// loop until the device is OK, may be delay from Initialize
		ustatus = USBH_Device_GetStatus (drivenum); // get the status of the USB device
		while(ustatus != usbOK){
			ustatus = USBH_Device_GetStatus (drivenum); // get the status of the USB device
		}
		// initialize the drive
		fstatus = finit (drive_name);
		if (fstatus != fsOK){
			// handle the error, finit didn't work
		} // end if
		// Mount the drive
		fstatus = fmount (drive_name);
		if (fstatus != fsOK){
			// handle the error, fmount didn't work
		} // end if 
		// file system and drive are good to go	
//=====================================================================================
		// Send the file names here
		osEvent evt; // Receive message object
		while(1){
			evt = osMessageGet (mid_CMDQueue, osWaitForever); // receive command
      if (evt.status == osEventMessage) { // check for valid message
				if (evt.value.v == SendFiles){	// check for send files command
					fsFileInfo info;
					info.fileID = 0;                             // info.fileID must be set to 0
					// let GUI know that file names will be sent
					UART_send(StartFileList_msg, strlen(StartFileList_msg));
					while (ffind ("U0:*.*", &info) == fsOK) {     // find whatever is in drive "R0:"
						if(info.size >= 1){
							LED_On(LED_Blue);
							UART_send(info.name, strlen(info.name));
							UART_send("\r\n", 2);
							LED_Off(LED_Blue);
						}
					}	// while sending files loop
					// done sending files so send notification that files have been sent
					UART_send(EndFileList_msg, strlen(EndFileList_msg));
					osMessagePut(mid_CMDQueue, SendComplete, osWaitForever);
				}	// if send files command
			} // if valis message
		}	// continuous loop checking message	
	}	// if usb status is ok
} // FS_Thread
