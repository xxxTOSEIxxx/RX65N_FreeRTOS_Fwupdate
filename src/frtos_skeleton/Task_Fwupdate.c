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
#include "r_flash_rx_if.h"
#include "MotorolaStype.h"


#include "Global.h"
extern GLOBAL_INFO_TABLE			g_tGlobalInfo;


typedef struct
{
	STYPE_RECORD_TABLE				tStypeRecord;
	STYPE_FLASH_INFO_TABLE			tStypeFlashInfo;

} FWUPDATE_GLOBAL_TABLE;

FWUPDATE_GLOBAL_TABLE				g_Fwupdate;




void Task_Fwupdate(void * pvParameters)
{
	MOTOROLA_STYPE_RESULT_ENUM 		eResult = MOTOROLA_STYPE_RESULT_SUCCESS;
	FRESULT							eFileResult = FR_OK;
	FIL								file;
	flash_err_t						eFlashResult = FLASH_SUCCESS;
//	flash_bank_t 					eBankInfo;							// 起動バンク情報


	// SW
	PORTA.PDR.BIT.B2 = 0;
	PORTA.PCR.BIT.B2 = 1;


	// USB接続待ち
	while(1)
	{
		if (g_tGlobalInfo.eUsbKind == USB_KIND_CONNECT)
		{
			vTaskDelay(500);
			break;
		}

		vTaskDelay(500);
	}

	printf("update Start!\n");
	g_tGlobalInfo.eLedKind = LED_KIND_PROCESS;

	// ファイルオープン
	eFileResult = f_open(&file, "update.mot", (FA_OPEN_EXISTING | FA_READ));
	if (eFileResult != FR_OK)
	{
		printf("f_open Error. [eFileResult:%d]\n",eFileResult);
		g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
		goto Task_Fwupdate_EndProc_Label;
	}

	// フラッシュモジュールオープン
	eFlashResult = R_FLASH_Open();
	if (eFlashResult != FLASH_SUCCESS)
	{
		printf("R_FLASH_Open Error. [eFlashResult:%d]\n",eFlashResult);
		g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
		goto Task_Fwupdate_EndProc_Label;
	}

#if 0
	// 起動バンク取得
	eFlashResult = R_FLASH_Control(FLASH_CMD_BANK_GET,&eBankInfo);
	if (eFlashResult != FLASH_SUCCESS)
	{
		printf("R_FLASH_Control(FLASH_CMD_BANK_GET) Error. [eFlashResult:%d]\n",eFlashResult);
		g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
		goto Task_Fwupdate_EndProc_Label;
	}
#endif

	// イレース
	eFlashResult = R_FLASH_Erase(FLASH_CF_BLOCK_38,38);
	if (eFlashResult != FLASH_SUCCESS)
	{
		printf("R_FLASH_Erase Error. [Address：%p, eFlashResult:%d]\n",FLASH_CF_BLOCK_38,eFlashResult);
		g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
		goto Task_Fwupdate_EndProc_Label;
	}

#if 0
	// モトローラ S-Typeファイルの解析
	while(1)
	{
		eResult = ReadStypeRecord(&file,&g_Fwupdate.tStypeRecord, &g_Fwupdate.tStypeFlashInfo);
		if (eResult != MOTOROLA_STYPE_RESULT_SUCCESS)
		{
			break;
		}
	}
#endif

	printf("update Success!\n");
	g_tGlobalInfo.eLedKind = LED_KIND_ON;

Task_Fwupdate_EndProc_Label:

	// ファイルクローズ
	f_close(&file);

	while(1)
	{
		vTaskDelay(100);
	}
}
