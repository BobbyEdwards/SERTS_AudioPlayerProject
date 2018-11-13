
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

void Thread_1 (void const *arg);                           // function prototype for Thread_1
osThreadDef (Thread_1, osPriorityNormal, 1, 0);            // define Thread_1

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
	
	SEM0_id = osSemaphoreCreate(osSemaphore(SEM0), 1);  // Create semaphore with 1 tokens
	
	mid_MsgQueue = osMessageCreate (osMessageQ(MsgQueue), NULL);  // create msg queue
	if (!mid_MsgQueue) {
		; // Message Queue object not created, handle failure
	}
	
  id = osThreadCreate (osThread (Thread_1), NULL);         // create the thread
  if (id == NULL) {                                        // handle thread creation
    // Failed to create a thread
  };
}

// Thread function
void Thread_1 (void const *argument) {
	static uint8_t rtrn = 0;  // return variable
	float32_t Fs = 8000.0; // sample frequency
	float32_t freq = 200.0; // signal frequency in Hz
	int16_t i; // loop counter 
	float32_t tmp; // factor
	float32_t cnt = 0.0f; // time index
	
	tmp = 6.28f*freq/Fs; // only calc this factor once
	
	// initialize the audio output
	rtrn = BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 0x46, 8000);
	if (rtrn != AUDIO_OK)return;
	
	// generate data for the audio buffer
 	for(i=0;i<NUM_POINTS;i++){
		Audio_Buffer1[2*i] = arm_cos_f32(tmp*cnt)*32000.0f; // Left channel
		Audio_Buffer1[2*i+1] = Audio_Buffer1[2*i]; // Right channel
		cnt = cnt + 1.0f;
	}
	
	// Start the audio player
  BSP_AUDIO_OUT_Play((uint16_t *)Audio_Buffer1, BUF_LEN);
	
	// Infinite loop
	uint8_t bufferNum = 1;	// keeps track of the next buffer to use
	//osEvent evt; // Receive message object
  while(1){
		if (bufferNum == 1){
			bufferNum = 2;
			// generate data for the audio buffer2
			for(i=0;i<NUM_POINTS;i++){
				Audio_Buffer2[2*i] = arm_cos_f32(tmp*cnt)*32000.0f; // Left channel
				Audio_Buffer2[2*i+1] = Audio_Buffer2[2*i]; // Right channel
				cnt = cnt + 1.0f;
			}
			osMessagePut(mid_MsgQueue, Buffer1, osWaitForever);
			osSemaphoreWait(SEM0_id, osWaitForever);
		} else if(bufferNum == 2){
			bufferNum = 1;
			// generate data for the audio buffer1
			for(i=0;i<NUM_POINTS;i++){
				Audio_Buffer1[2*i] = arm_cos_f32(tmp*cnt)*32000.0f; // Left channel
				Audio_Buffer1[2*i+1] = Audio_Buffer1[2*i]; // Right channel
				cnt = cnt + 1.0f;
			}
			osMessagePut(mid_MsgQueue, Buffer2, osWaitForever);
			osSemaphoreWait(SEM0_id, osWaitForever);
		}	// end else if
   }	// end while loop
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



