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

// pointer to file type for files on USB device
FILE *f;
// file name for song selection
char fileName[50] = {0};

// WAVE file header format
typedef struct WAVHEADER {
	unsigned char riff[4];						// RIFF string
	uint32_t overall_size;				// overall size of file in bytes
	unsigned char wave[4];						// WAVE string
	unsigned char fmt_chunk_marker[4];		// fmt string with trailing null char
	uint32_t length_of_fmt;					// length of the format data
	uint16_t format_type;					// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t channels;						// no.of channels
	uint32_t sample_rate;					// sampling rate (blocks per second)
	uint32_t byterate;						// SampleRate * NumChannels * BitsPerSample/8
	uint16_t block_align;					// NumChannels * BitsPerSample/8
	uint16_t bits_per_sample;				// bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned char data_chunk_header [4];		// DATA string or FLLR string
	uint32_t data_size;						// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} WAVHEADER;

#define NUM_CHAN	2 // number of audio channels
#define NUM_POINTS 1024 // number of points per channel
#define BUF_LEN NUM_CHAN*NUM_POINTS // length of the audio buffer
/* buffer used for audio play */
int16_t Audio_Buffer1[BUF_LEN];
int16_t Audio_Buffer2[BUF_LEN];

/* This function is called when half of the requested buffer has been transferred. */
void    BSP_AUDIO_OUT_HalfTransfer_CallBack(void){
}

/* This function is called when an Interrupt due to transfer error or peripheral
   error occurs. */
void    BSP_AUDIO_OUT_Error_CallBack(void){
		while(1){
		}
}

osSemaphoreDef (SEM0);    // Declare semaphore
osSemaphoreId  (SEM0_id); // Semaphore ID

osMessageQId mid_MsgQueue;                     // message queue id
osMessageQDef (MsgQueue, 1, int32_t);    // message queue object

// Command queue from Rx_Command to Controller
osMessageQId mid_CMDQueue; // message queue for commands to Thread
osMessageQDef (CMDQueue, 1, uint32_t); // message queue object

// Command queue from Controller FS_Thread
osMessageQId mid_Command_FS_Queue; // message queue for commands to Thread
osMessageQDef (Command_FS_Queue, 1, uint32_t); // message queue object

//osMessageQId mid_Command_FS_Queue;                     // message queue id
//osMessageQDef (PlayPause_Queue, 1, int32_t);    // message queue object

////////////////////////////////////////////////
// State Machine definitions
enum state{
  NoState,
  Idle,
  List,
	Pause,
	Play
};

enum commands{
  ListFiles,
  SendComplete,
  SendFiles,
	PlayFile,
	PauseFile,
	ResumeFile,
	SongDone
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
      // Idle State entry actions
			LED_Off(LED_Green);
      LED_Off(LED_Red);
			LED_Off(LED_Orange);	//yacht
			LED_Off(LED_Blue);
      break;
    case Idle:
			if (event == PlayFile){
				Current_State = Play;
				// Exit actions
				LED_Off(LED_Red);
				// Transistion actions
				// Play State Entry Actions
				LED_On(LED_Green);
				osMessagePut(mid_Command_FS_Queue, PlayFile, 0);
			}
      else if(event == SendFiles){
        Current_State = List;
        // Exit actions
        LED_Off(LED_Red);
        // Transition actions
        // List entry actions
        LED_On(LED_Green);
				osMessagePut (mid_Command_FS_Queue, SendFiles, 0);
      }
      break;
    case List:
      if(event == SendComplete){
        Current_State = Idle;
        // Exit actions
        LED_Off(LED_Green);
        // Transition actions
        // Idle state entry actions
        //LED_On(LED_Red);
      }
      break;
		case Pause:
			if (event == PlayFile){
				Current_State = Play;
				// Exit actions
				LED_Off(LED_Red);
				// Transistion actions
				// Play State Entry Actions
				LED_On(LED_Green);
				osMessagePut(mid_Command_FS_Queue, ResumeFile, 0);
			}
			break;
		case Play:
			if (event == PauseFile){
				Current_State = Pause;
				// Exit Actions
				LED_Off(LED_Green);
				// Transition Actions
				// Pause State Entry Actions
				LED_On(LED_Red);
				osMessagePut(mid_Command_FS_Queue, PauseFile, 0);
			}
			if (event == SongDone){
				Current_State = Idle;
				// Exit Actions
				LED_Off(LED_Green);
				LED_Off(LED_Red);
				LED_Off(LED_Orange);	//yacht
				LED_Off(LED_Blue);
				// Transition Actions
				// Idle State Entry Actions
				//LED_On(LED_Red);
				//osMessagePut(mid_Command_FS_Queue, PauseFile, 0);	// No action
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
		mid_Command_FS_Queue = osMessageCreate (osMessageQ(Command_FS_Queue), NULL);  // create msg queue
  if (!mid_Command_FS_Queue)return; // Message Queue object not created, handle failure	
	 mid_MsgQueue = osMessageCreate (osMessageQ(MsgQueue), NULL);  // create msg queue
  if (!mid_MsgQueue)return; // Message Queue object not created, handle failure	
	 //mid_Command_FS_Queue = osMessageCreate (osMessageQ(PlayPause_Queue), NULL);  // create msg queue
  //if (!mid_Command_FS_Queue)return; // Message Queue object not created, handle failure	
	
	// Create Semaphore for Buffer
	SEM0_id = osSemaphoreCreate(osSemaphore(SEM0), 0);  // Create semaphore with 1 token
	
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
	 //char fileName[50] = {0};
   while(1){
      UART_receive(rx_char, 1); // Wait for command from PC GUI
    // Check for the type of character received
      if(!strcmp(rx_char, Show_Files_char)){
         // Trigger1 received
         osMessagePut (mid_CMDQueue, SendFiles, osWaitForever);
      }
			else if (!strcmp(rx_char, Receive_File_char)){
        // Trigger2 received
				LED_On(LED_Orange);
				UART_receivestring(fileName, 50);
				
				// Remove last character, this is a return character made by Visual Studio
				int i = 0;
				while(fileName[i] != '\0')
				{
						i++;
						 
				}
				fileName[i-1] = '\0';
				
				LED_Off(LED_Orange);
				//LED_Off(LED_Orange);
      }
			else if(!strcmp(rx_char, Play_char)){
         // Trigger1 received
        osMessagePut (mid_CMDQueue, PlayFile, osWaitForever);
//				LED_Off(LED_Orange);
//				LED_On(LED_Green);
      }
			else if(!strcmp(rx_char, Pause_char)){
         // Trigger1 received
        osMessagePut (mid_CMDQueue, PauseFile, osWaitForever);
//				LED_Off(LED_Green);
//				LED_On(LED_Orange);
      }
			// end if
   }
} // end Rx_Command

/* User Callbacks: user has to implement these functions if they are needed. */
/* This function is called when the requested data has been completely transferred. */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void){
	osEvent event;
	int16_t receivedBuffer;
	
	event = osMessageGet (mid_MsgQueue, 0);        // wait for message
	
  if (event.status == osEventMessage) { // check for valid message
		LED_Off(LED_Orange);
		receivedBuffer = event.value.v;
		osSemaphoreRelease(SEM0_id);
		if (receivedBuffer == 1){
			BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)Audio_Buffer1, BUF_LEN);
		}
		else if (receivedBuffer == 2){
			BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)Audio_Buffer2, BUF_LEN);
		}
	}
}

// FS Thread function
void FS_Thread (void const *argument) {
	//Initialize the USB Host
	usbStatus ustatus; // USB driver status variable
	uint8_t drivenum = 0; // Using U0: drive number
	char *drive_name = "U0:"; // USB drive name
	fsStatus fstatus; // file system status variable
	WAVHEADER header;
	size_t rd;
	uint32_t i;
	static uint8_t rtrn = 0;
	uint8_t rdnum = 1; // read buffer number
	
	osEvent event; // OS Event for Receiving the Play/Pause Command
	
	bool endOfFile = false; // Boolean for keeping track of whether the end of the file has been reached
	int16_t bufferLoaded = 1; // Keeps track of which buffer is currently loaded
	int16_t sizeRead = 0; // Shows number of members read from the file
	int32_t Fs = 44100; // This is the sample Rate/Frequency
	
	
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
		//f = fopen ("Test.wav","r");// open a file on the USB device
		//if (f != NULL) {
			// Read the header data into the header variable
			//fread((void *)&header, sizeof(header), 1, f);
			
			// initialize the audio output
			rtrn = BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 0x46, 44100);
			if (rtrn != AUDIO_OK)return;
			
			// Read the first buffer block of data
		/*
			sizeRead = fread((void *)&Audio_Buffer1, sizeof(Audio_Buffer1), 1, f);
			if (sizeRead < 1){
				endOfFile = true;
			}
			bufferLoaded = 1;
	*/
	// song streaming while loop		
	while(1){
		//Get a message and wait forever
		event = osMessageGet(mid_Command_FS_Queue, osWaitForever); // receive command
		
		// if valid message
		if (event.status == osEventMessage) {
			// switch for messages
			switch(event.value.v) {
				case SendFiles:
					//Send the file list
					fsFileInfo info;
					info.fileID = 0;                             // info.fileID must be set to 0
					// let GUI know that file names will be sent
					UART_send(StartFileList_msg, strlen(StartFileList_msg));
					while (ffind ("U0:*.wav", &info) == fsOK) {     // find whatever is in drive "R0:"
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
					break;
				
				case PlayFile:
					//Open the song file
					// change this to be the selected file name's string
					//f = fopen ("Test.wav","r");// open a file on the USB device
					f = fopen (fileName,"r");// open a file on the USB device
					if(f == NULL){
						break;
					}
					//Read a buffer of data
					// read song header into header variable
					fread((void *)&header, sizeof(header), 1, f);
					
					// load data into first buffer and record the size of the first buffer
					sizeRead = fread((void *)&Audio_Buffer1, sizeof(Audio_Buffer1), 1, f);
					// record the buffer that was loaded
					bufferLoaded = 1;
					
					// Turn off mute
					BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);
					// Start playing the buffer			
					//BSP_AUDIO_OUT_Play function to start playing
					BSP_AUDIO_OUT_Play((uint16_t *)Audio_Buffer1, BUF_LEN);
					
					// NOTE: No break here so it will continue with the next sections too
					
				case ResumeFile:
					// if we are not at the beginning of a song play
					if(event.value.v == ResumeFile){
						BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);
						BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)Audio_Buffer1, BUF_LEN);
					}
					while(sizeRead == 1){
						//Read new buffer of data and send to DMA						
						if (bufferLoaded == 1){
							sizeRead = fread((void *)&Audio_Buffer2, sizeof(Audio_Buffer2), 1, f);
							bufferLoaded = 2;
						}
						else if (bufferLoaded == 2){
							sizeRead = fread((void *)&Audio_Buffer1, sizeof(Audio_Buffer1), 1, f);
							bufferLoaded = 1;
						}
						// send message that a buffer had been loaded
						osMessagePut (mid_MsgQueue, bufferLoaded, osWaitForever); // Send Message
						osSemaphoreWait(SEM0_id, osWaitForever);
						
						//Read a new message WITH ZERO DELAY
						event = osMessageGet(mid_Command_FS_Queue, 0); // receive command
						if (event.status == osEventMessage) { // check for valid message
							if(event.value.v == PauseFile){
								BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
								//the read buffer 1 length to zero; // this will stop all playback
								BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)Audio_Buffer1, 0);
								
								break; // This is not good programming but works
							}
							//if(event.value.v == PauseFile){
								//the read buffer length to zero; // this will stop all playback
							//}
						} // end new message
						if(sizeRead != 1){
							// end of song
							//Send SongDone to state machine
							osMessagePut(mid_CMDQueue, SongDone, osWaitForever);
							// mute audio
							BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
							// close file
							fclose (f); // close the file
						}
					}// end while(the read buffer length is BUF_LEN from previous read)
					break;
				} // end of switch
			} // switch(message)
		} // end while
	} // end if valid message received
//}
} // end Thread