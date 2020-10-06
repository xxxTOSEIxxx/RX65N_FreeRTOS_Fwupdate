/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "task_function.h"
#include "platform.h"

#include "Global.h"
extern GLOBAL_INFO_TABLE			g_tGlobalInfo;


#define	ON							( 1 )
#define OFF							( 0 )
#define PORING_TIME					( 100 )

typedef struct
{
	uint8_t							Led;					// LED制御
	int16_t							Time;					// 制御時間
} LED_CONTROL_INFO_TABLE;

LED_CONTROL_INFO_TABLE				g_LedControlInfo[LED_KIND_MAX][10] =
{
		{ { OFF,  1000 }, { OFF,    -1}, { OFF,     -1}, { OFF,    -1}, { OFF,     -1}, { OFF,    -1}, { OFF,     -1}, { OFF,    -1}, { OFF,    -1}, { OFF,    -1} },	// LED_KIND_OFF
		{ { ON ,  1000 }, { OFF,    -1}, { OFF,     -1}, { OFF,    -1}, { OFF,     -1}, { OFF,    -1}, { OFF,     -1}, { OFF,    -1}, { OFF,    -1}, { OFF,    -1} },	// LED_KIND_ON
		{ { ON ,   500 }, { OFF,   500}, { OFF,     -1}, { OFF,    -1}, { OFF,     -1}, { OFF,    -1}, { OFF,     -1}, { OFF,    -1}, { OFF,    -1}, { OFF,    -1} },	// LED_KIND_PROCESS
		{ { ON ,   100 }, { OFF,   100}, { ON ,   100 }, { OFF,   100}, { ON ,   100 }, { OFF,   100}, { ON ,   100 }, { OFF,   500}, { OFF,    -1}, { OFF,    -1} },	// LED_KIND_ERROR
};


void Task_Led(void * pvParameters)
{
	uint8_t						index = 0;
	uint8_t						ControlLed = LED_KIND_MAX;
	uint16_t					ControlTime = 0;


	g_tGlobalInfo.eLedKind = LED_KIND_OFF;

	// LED
	PORTA.PDR.BIT.B1 = 1;
	PORTA.PODR.BIT.B1 = 0;

	while(1)
	{
		if (g_tGlobalInfo.eLedKind != ControlLed)
		{
			index = 0;
			ControlLed = g_tGlobalInfo.eLedKind;
			ControlTime = 0;
		}

		if (g_LedControlInfo[ControlLed][index].Time <= ControlTime)
		{
			ControlTime = 0;
			index++;
			if (g_LedControlInfo[ControlLed][index].Time == -1)
			{
				index = 0;
			}
		}

		PORTA.PODR.BIT.B1 = g_LedControlInfo[ControlLed][index].Led;

		vTaskDelay(PORING_TIME);
		ControlTime += PORING_TIME;
	}
}
