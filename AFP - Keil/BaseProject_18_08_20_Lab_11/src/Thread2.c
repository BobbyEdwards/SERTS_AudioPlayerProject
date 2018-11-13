
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
#include "math.h"
#include "arm_math.h" // header for DSP library
#include <stdio.h>

// LED constants
#define LED_Green   0
#define LED_Orange  1
#define LED_Red     2
#define LED_Blue    3

//========================================================
#define NUM_CHAN	2 // number of audio channels
#define NUM_POINTS 1024 // number of points per channel
#define BUF_LEN NUM_CHAN*NUM_POINTS // length of the audio buffer
/* buffer used for audio play */
int16_t Audio_Buffer1[BUF_LEN];
int16_t Audio_Buffer2[BUF_LEN];
//========================================================

void FS (void const *arg);                           // function prototype for Thread_1
osThreadDef (FS, osPriorityNormal, 1, 0);            // define Thread_1

osSemaphoreDef (SEM0);    // Declare semaphore
osSemaphoreId  (SEM0_id); // Semaphore ID

#define MSGQUEUE_OBJECTS    1                 // number of Message Queue Objects
osMessageQId mid_MsgQueue;                     // message queue id
osMessageQDef (MsgQueue, MSGQUEUE_OBJECTS, int32_t);    // message queue object
#define Buffer1 1
#define Buffer2 2

void Init_Thread (void) {

	osThreadId id; // holds the returned thread create ID
	
	LED_Initialize(); // Initialize the LEDs
	UART_Init(); // Initialize the UART
	
	SEM0_id = osSemaphoreCreate(osSemaphore(SEM0), 0);  // Create semaphore
	
	mid_MsgQueue = osMessageCreate (osMessageQ(MsgQueue), NULL);  // create msg queue
	if (!mid_MsgQueue) {
		; // Message Queue object not created, handle failure
	}
	
  id = osThreadCreate (osThread (FS), NULL);         // create the thread
  if (id == NULL) {                                        // handle thread creation
    // Failed to create a thread
  };
}

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

// pointer to file type for files on USB device
FILE *f;

// Thread function
void FS (void const *argument) {
	usbStatus ustatus; // USB driver status variable
	uint8_t drivenum = 0; // Using U0: drive number
	char *drive_name = "U0:"; // USB drive name
	fsStatus fstatus; // file system status variable
	WAVHEADER header;
	static uint8_t rtrn = 0;
	//bool enable = true;
	
	//========================================================
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
		f = fopen ("Test.wav","r");// open a file on the USB device
		if (f != NULL) {
			fread((void *)&header, sizeof(header), 1, f);
			//fclose (f); // close the file
		} // end if file opened
	} // end if USBH_Initialize
	//========================================================
	
	// initialize the audio output
	rtrn = BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 0x46, header.sample_rate);
	if (rtrn != AUDIO_OK)return;
	
	// load data for the first audio buffer===================
	fread(Audio_Buffer1, 2, BUF_LEN, f);
	LED_On(LED_Blue);
	
	// Start the audio player
  BSP_AUDIO_OUT_Play((uint16_t *)Audio_Buffer1, 2*BUF_LEN);
	
	// Infinite loop
	uint8_t bufferNum = 2;	// keeps track of the next buffer to use
	
	// while not disabled
  while(!feof(f)){
		// write data to buffers
		// use first buffer
		if (bufferNum == 1){
			bufferNum = 2;
			// load data for the audio buffer2
			fread(Audio_Buffer2, 2, BUF_LEN, f);
			// send message about buffer2
			osMessagePut(mid_MsgQueue, Buffer2, osWaitForever);
			osSemaphoreWait(SEM0_id, osWaitForever);
		} else if(bufferNum == 2){	// use second buffer
			bufferNum = 1;
			// load data for the audio buffer1
			fread(Audio_Buffer1, 2, BUF_LEN, f);
			// send message about buffer1
			osMessagePut(mid_MsgQueue, Buffer1, osWaitForever);
			osSemaphoreWait(SEM0_id, osWaitForever);
		}	// end else if
  }	// end while loop
	// mute audio
	BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
	fclose(f);	// close the file
}

/* User Callbacks: user has to implement these functions if they are needed. */
/* This function is called when the requested data has been completely transferred. */
void    BSP_AUDIO_OUT_TransferComplete_CallBack(void){
	osEvent evt;
	evt = osMessageGet (mid_MsgQueue, 0);
	if (evt.status == osEventMessage) {
		osSemaphoreRelease(SEM0_id);
		if (evt.value.v == Buffer1) {	// if told to go to buffer 1
			BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)Audio_Buffer1, BUF_LEN);
		} else if (evt.value.v == Buffer2) {	// else if told to go to buffer2
			BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)Audio_Buffer2, BUF_LEN);
		}
	}
}

/* This function is called when half of the requested buffer has been transferred. */
void    BSP_AUDIO_OUT_HalfTransfer_CallBack(void){
}

/* This function is called when an Interrupt due to transfer error or peripheral
   error occurs. */
void    BSP_AUDIO_OUT_Error_CallBack(void){
		while(1){
		}
}



