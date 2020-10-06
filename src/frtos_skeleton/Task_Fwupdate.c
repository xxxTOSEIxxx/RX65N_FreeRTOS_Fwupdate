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
#include "string.h"


#include "Global.h"
extern GLOBAL_INFO_TABLE			g_tGlobalInfo;


//#define DEBUG_PRINTF


#define	CF_WRITE_MIN_SIZE			( 128 )
#define CF_WRITE_BLOCK_SIZE			( CF_WRITE_MIN_SIZE )


typedef struct
{
	uint32_t						StartAddress;
	uint32_t						EndAddress;

	uint8_t							Data[CF_WRITE_BLOCK_SIZE];
	uint32_t						Size;

} CF_WRITE_INFO_TABLE;






typedef struct
{
	STYPE_RECORD_TABLE				tStypeRecord;
	STYPE_FLASH_INFO_TABLE			tStypeFlashInfo;

	CF_WRITE_INFO_TABLE				tCfWriteInfo;

} FWUPDATE_GLOBAL_TABLE;

FWUPDATE_GLOBAL_TABLE				g_Fwupdate;



//---------------------------------------------------------------------------------
// プログラム書込み処理
// 戻り値： 0:書込み成功 , 1:書込み失敗
//---------------------------------------------------------------------------------
uint8_t Cf_Write(const CF_WRITE_INFO_TABLE* ptCfWriteInfo)
{
	uint8_t							iRet = 0;
	flash_err_t						eFlashResult = FLASH_SUCCESS;
	uint32_t						WriteAddress = 0x00000000;


	WriteAddress = ptCfWriteInfo->StartAddress - (FLASH_CF_HI_BANK_LO_ADDR - FLASH_CF_LO_BANK_LO_ADDR);

#ifdef DEBUG_PRINTF
	printf("---[CF_WRITE_INFO]--------------------------------------------------------\n");
	for (unsigned int j = 0; j < (CF_WRITE_BLOCK_SIZE / 16); j++)
	{
		printf("[%08X (%08X)] : ", (ptCfWriteInfo->StartAddress + (16 * j)), (WriteAddress +(16 * j)));
		for (unsigned int i = 0; i < 16; i++)
		{
			printf("%02X ", ptCfWriteInfo->Data[(16 * j) + i]);
		}
		printf("\n");
	}
	printf("--------------------------------------------------------------------------\n\n");
#endif	// #ifdef DEBUG_PRINTF

#if 0
	// プログラム書込み
	eFlashResult = R_FLASH_Write(ptCfWriteInfo->Data, WriteAddress, CF_WRITE_BLOCK_SIZE);
	if (eFlashResult != FLASH_SUCCESS)
	{
		printf("R_FLASH_Write Error. [eFlashResult:%d, Address:%08X]\n",eFlashResult,WriteAddress);
		iRet = 1;
	}
#endif	// #ifdef DEBUG_PRINTF

	return iRet;
}



void Task_Fwupdate(void * pvParameters)
{
	MOTOROLA_STYPE_RESULT_ENUM 		eResult = MOTOROLA_STYPE_RESULT_SUCCESS;
	FRESULT							eFileResult = FR_OK;
	FIL								file;
	flash_err_t						eFlashResult = FLASH_SUCCESS;
//	flash_bank_t 					eBankInfo;							// 起動バンク情報
	uint8_t							NewRecordFlag = 0;
	uint8_t							Ret = 0;


	// SW
	PORTA.PDR.BIT.B2 = 0;
	PORTA.PCR.BIT.B2 = 1;

	//memset(&g_Fwupdate,0x00,sizeof(g_Fwupdate));

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

	// モトローラ S-Typeファイルの解析
	while(1)
	{
		// モトローラ S-Typeレコード1件分を読込み
		eResult = ReadStypeRecord(&file,&g_Fwupdate.tStypeRecord, &g_Fwupdate.tStypeFlashInfo);
		if (eResult != MOTOROLA_STYPE_RESULT_SUCCESS)
		{
			if (eResult == MOTOROLA_STYPE_RESULT_FILE_EOF)
			{
				// ループを抜ける
				break;
			}
			else
			{
				printf("ReadStypeRecord Error. [eResult:%d]\n",eResult);
				g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
				goto Task_Fwupdate_EndProc_Label;
			}
		}

		NewRecordFlag = 1;

#if 0
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// 解析成功したので、ROMにプログラムを書込む（但しS3レコードのみ）
		if (g_Fwupdate.tStypeRecord.eStypeRecordKind == STYPE_RECORD_KIND_S3)
		{
			// 0xFFF00000 - 0xFFFFFFFFの場合のみ、書込みOK!
			if ((g_Fwupdate.tStypeFlashInfo.Address >= FLASH_CF_HI_BANK_LO_ADDR) && (g_Fwupdate.tStypeFlashInfo.Address <= FLASH_CF_HI_BANK_HI_ADDR))
			{
				// プログラム書込み情報に何も設定されていない場合
				if ((g_Fwupdate.tCfWriteInfo.StartAddress == 0) && (g_Fwupdate.tCfWriteInfo.EndAddress == 0))
				{
					g_Fwupdate.tCfWriteInfo.StartAddress = (g_Fwupdate.tStypeFlashInfo.Address / CF_WRITE_BLOCK_SIZE) * CF_WRITE_BLOCK_SIZE;
					g_Fwupdate.tCfWriteInfo.EndAddress = g_Fwupdate.tCfWriteInfo.StartAddress + (CF_WRITE_BLOCK_SIZE - 1);
					g_Fwupdate.tCfWriteInfo.Size = g_Fwupdate.tStypeFlashInfo.Address % CF_WRITE_BLOCK_SIZE;
				}

				// 1バイトずつ、プログラム書込み情報の書込み範囲を超えていないかをチェックする
				for(uint32_t i = 0 ; i < g_Fwupdate.tStypeFlashInfo.DataSize ; i++)
				{
					// プログラム書込み情報範囲内の場合
					if (g_Fwupdate.tCfWriteInfo.EndAddress >= (g_Fwupdate.tStypeFlashInfo.Address + i))
					{
						// 新しいS-Typeレコードの場合
						if (NewRecordFlag == 1)
						{
							NewRecordFlag = 0;
							g_Fwupdate.tCfWriteInfo.Size += (((g_Fwupdate.tStypeFlashInfo.Address + i) - g_Fwupdate.tCfWriteInfo.StartAddress) - g_Fwupdate.tCfWriteInfo.Size);
						}

						// プログラム書込み情報に書込みデータをセット
						g_Fwupdate.tCfWriteInfo.Data[g_Fwupdate.tCfWriteInfo.Size] = g_Fwupdate.tStypeFlashInfo.Data[i];
						g_Fwupdate.tCfWriteInfo.Size++;

						// プログラム書込み情報の書込みサイズが書込みブロックサイズを超えた場合
						if (g_Fwupdate.tCfWriteInfo.Size >= CF_WRITE_BLOCK_SIZE)
						{
							// 書込み処理
							Ret = Cf_Write(&g_Fwupdate.tCfWriteInfo);
							if (Ret != 0)
							{
								printf("Cf_Write Error.\n");
								g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
								goto Task_Fwupdate_EndProc_Label;
							}

							// 書込み情報の書込み範囲を新たに設定・プログラム書込み情報に書込みデータをセット
							g_Fwupdate.tCfWriteInfo.StartAddress = ((g_Fwupdate.tCfWriteInfo.EndAddress + 1) / CF_WRITE_BLOCK_SIZE) * CF_WRITE_BLOCK_SIZE;
							g_Fwupdate.tCfWriteInfo.EndAddress = g_Fwupdate.tCfWriteInfo.StartAddress + (CF_WRITE_BLOCK_SIZE - 1);
							g_Fwupdate.tCfWriteInfo.Size = 0;
							memset(g_Fwupdate.tCfWriteInfo.Data, 0x00, sizeof(g_Fwupdate.tCfWriteInfo.Data));
						}
					}
					// プログラム書込み情報範囲を超えた場合
					else if (g_Fwupdate.tCfWriteInfo.EndAddress < (g_Fwupdate.tStypeFlashInfo.Address + i))
					{
						// 書込み処理
						g_Fwupdate.tCfWriteInfo.Size = CF_WRITE_BLOCK_SIZE;
						Ret = Cf_Write(&g_Fwupdate.tCfWriteInfo);
						if (Ret != 0)
						{
							printf("Cf_Write Error.\n");
							g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
							goto Task_Fwupdate_EndProc_Label;
						}


						if (i == 0)
						{
							g_Fwupdate.tCfWriteInfo.StartAddress = (g_Fwupdate.tStypeFlashInfo.Address / CF_WRITE_BLOCK_SIZE) * CF_WRITE_BLOCK_SIZE;
							g_Fwupdate.tCfWriteInfo.EndAddress = g_Fwupdate.tCfWriteInfo.StartAddress + (CF_WRITE_BLOCK_SIZE - 1);
							g_Fwupdate.tCfWriteInfo.Size = g_Fwupdate.tStypeFlashInfo.Address % CF_WRITE_BLOCK_SIZE;
							memset(g_Fwupdate.tCfWriteInfo.Data, 0x00, sizeof(g_Fwupdate.tCfWriteInfo.Data));
							g_Fwupdate.tCfWriteInfo.Data[g_Fwupdate.tCfWriteInfo.Size] = g_Fwupdate.tStypeFlashInfo.Data[i];
							g_Fwupdate.tCfWriteInfo.Size++;
						}
						else
						{
							// 書込み情報の書込み範囲を新たに設定・プログラム書込み情報に書込みデータをセット
							g_Fwupdate.tCfWriteInfo.StartAddress = ((g_Fwupdate.tCfWriteInfo.EndAddress + 1) / CF_WRITE_BLOCK_SIZE) * CF_WRITE_BLOCK_SIZE;
							g_Fwupdate.tCfWriteInfo.EndAddress = g_Fwupdate.tCfWriteInfo.StartAddress + (CF_WRITE_BLOCK_SIZE - 1);
							g_Fwupdate.tCfWriteInfo.Size = 0;
							memset(g_Fwupdate.tCfWriteInfo.Data, 0x00, sizeof(g_Fwupdate.tCfWriteInfo.Data));
							g_Fwupdate.tCfWriteInfo.Data[g_Fwupdate.tCfWriteInfo.Size] = g_Fwupdate.tStypeFlashInfo.Data[i];
							g_Fwupdate.tCfWriteInfo.Size++;
						}
					}
					else
					{
						// 処理がここにくることはあり得ないのでエラーとする
						printf("oops...\n");
						g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
						goto Task_Fwupdate_EndProc_Label;
					}
				}
			}
		}
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
	}

#if 0
	// 書込み処理
	if ((g_Fwupdate.tCfWriteInfo.StartAddress != 0) && (g_Fwupdate.tCfWriteInfo.EndAddress != 0))
	{
		Ret = Cf_Write(&g_Fwupdate.tCfWriteInfo);
		if (Ret != 0)
		{
			printf("Cf_Write Error.\n");
			g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
			goto Task_Fwupdate_EndProc_Label;
		}
	}
#endif

#if 0
	// 起動バンク変更
	eFlashResult = R_FLASH_Control(FLASH_CMD_BANK_TOGGLE, NULL);
	if (eFlashResult != FLASH_SUCCESS)
	{
		printf("R_FLASH_Control(FLASH_CMD_BANK_TOGGLE) Error. [eFlashResult:%d]\n",eFlashResult);
		g_tGlobalInfo.eLedKind = LED_KIND_ERROR;
		goto Task_Fwupdate_EndProc_Label;
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
