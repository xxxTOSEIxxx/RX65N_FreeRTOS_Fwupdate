/***********************************************************************
*
*  FILE        : RX65N_FreeRTOS_Fwupdate.c
*  DATE        : 2020-10-06
*  DESCRIPTION : Main Program
*
*  NOTE:THIS IS A TYPICAL EXAMPLE.
*
***********************************************************************/
#include "FreeRTOS.h"
#include "task.h"

void main_task(void *pvParameters)
{

	/* Create all other application tasks here */

	while(1);

	vTaskDelete(NULL);

}