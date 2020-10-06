//*************************************************************************************************
// モトローラー S-Typeレコード解析
//*************************************************************************************************
#include "MotorolaStype.h"
#include "platform.h"
#include "r_usb_hmsc_apl_config.h"
#include "r_usb_hmsc_apl.h"
#include "r_flash_rx_if.h"
#include "ff.h"
#include "stdio.h"
#include "stdlib.h"


#define DEBUG_PRINTF



//-----------------------------------------------------------------------------
// プロトタイプ宣言
//-----------------------------------------------------------------------------
// 16進数の文字を数値に変換
static uint8_t atohex(const char ch);

// 2バイトのHEX文字を数値に変換（例："3D" → 61(0x3D)）
static uint8_t StrHexToNum(const char* pszHex);

#if 0
// 2バイトの10進数文字を数値に変換（例："91" → 91(0x5B)）
static uint8_t StrNumToNum(const char* pszNum);
#endif

// チェックサム
MOTOROLA_STYPE_RESULT_ENUM CheckSum(const char* pData, uint8_t DataSize, uint8_t CheckSum);

// S0レコード解析
static MOTOROLA_STYPE_RESULT_ENUM Analyze_S0(FIL *pFile, STYPE_RECORD_TABLE* ptStypeRecord);

// S3レコード解析
static MOTOROLA_STYPE_RESULT_ENUM Analyze_S3(FIL *pFile, STYPE_RECORD_TABLE* ptStypeRecord, STYPE_FLASH_INFO_TABLE* ptStypeFlashInfo);

// S7レコード解析
static MOTOROLA_STYPE_RESULT_ENUM Analyze_S7(FIL *pFile, STYPE_RECORD_TABLE* ptStypeRecord);



//-----------------------------------------------------------------------------
// 16進数の文字を数値に変換
//-----------------------------------------------------------------------------
static uint8_t atohex(const char ch)
{
	uint8_t						Hex = 0x00;

	switch (ch) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		Hex = ch - '0';
		break;
	case 'A':
	case 'a':
		Hex = 0x0A;
		break;
	case 'B':
	case 'b':
		Hex = 0x0B;
		break;
	case 'C':
	case 'c':
		Hex = 0x0C;
		break;
	case 'D':
	case 'd':
		Hex = 0x0D;
		break;
	case 'E':
	case 'e':
		Hex = 0x0E;
		break;
	case 'F':
	case 'f':
		Hex = 0x0f;
		break;
	default:
		Hex = 0x00;
		break;
	}

	return Hex;
}


//-----------------------------------------------------------------------------
// 2バイトのHEX文字を数値に変換（例："3D" → 61(0x3D)）
//-----------------------------------------------------------------------------
static uint8_t StrHexToNum(const char* pszHex)
{
	uint8_t						Hex = 0x00;


	Hex = atohex(pszHex[0]) << 4;
	Hex += atohex(pszHex[1]);

	return Hex;
}


#if 0
//-----------------------------------------------------------------------------
// 2バイトの10進数文字を数値に変換（例："91" → 91(0x5B)）
//-----------------------------------------------------------------------------
static uint8_t StrNumToNum(const char* pszNum)
{
	uint8_t						Num = 0x00;


	if ((pszNum[0] >= '0') && (pszNum[0] <= '9'))
	{
		Num += pszNum[0] - '0';
	}
	Num = Num * 10;

	if ((pszNum[1] >= '0') && (pszNum[1] <= '9'))
	{
		Num += pszNum[1] - '0';
	}

	return Num;
}
#endif

//-----------------------------------------------------------------------------
// チェックサム
//-----------------------------------------------------------------------------
MOTOROLA_STYPE_RESULT_ENUM CheckSum(const char* pData, uint8_t DataSize, uint8_t CheckSum)
{
	uint8_t						Sum = 0x00;
	uint8_t						i = 0;


	// 指定された領域(pData)からデータサイズ(DataSize)分のCheckSumを算出する
	for (i = 0; i < DataSize; i += 2)
	{
		Sum += StrHexToNum(&pData[i]);
	}
	Sum = ~Sum;


	return ((Sum == CheckSum) ? MOTOROLA_STYPE_RESULT_SUCCESS : MOTOROLA_STYPE_RESULT_ERROR_CHECK_SUM);
}


//-----------------------------------------------------------------------------
// S0レコード解析
//-----------------------------------------------------------------------------
static MOTOROLA_STYPE_RESULT_ENUM Analyze_S0(FIL *pFile, STYPE_RECORD_TABLE* ptStypeRecord)
{
	FRESULT						eFileResult = FR_OK;
	MOTOROLA_STYPE_RESULT_ENUM	eResult = MOTOROLA_STYPE_RESULT_SUCCESS;
	UINT						ReadNum = 0;


	// S0のレコードサイズは固定なので、S0レコードサイズ分読み込む
	eFileResult = f_read(pFile, ptStypeRecord->tS0.sz0E,sizeof(S0_RECORD_TABLE), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != sizeof(S0_RECORD_TABLE))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	// "E0"チェック
	if ((ptStypeRecord->tS0.sz0E[0] != '0') || (ptStypeRecord->tS0.sz0E[1] != 'E'))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	// "0000"チェック
	for (int i = 0; i < 4; i++)
	{
		if (ptStypeRecord->tS0.sz0000[i] != '0')
		{
			return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
		}
	}

	// ニューラインチェック
	if ((ptStypeRecord->tS0.szNewLine[0] != 0x0D) || (ptStypeRecord->tS0.szNewLine[1] != 0x0A))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	// チェクサム
	eResult = CheckSum((const char*)&ptStypeRecord->tS0,
		(sizeof(S0_RECORD_TABLE) - (CHECK_SUM_SIZE + NEW_LINE_SIZE)), StrHexToNum(ptStypeRecord->tS0.szChecksum));

	return eResult;
}


//-----------------------------------------------------------------------------
// S3レコード解析
//-----------------------------------------------------------------------------
static MOTOROLA_STYPE_RESULT_ENUM Analyze_S3(FIL *pFile, STYPE_RECORD_TABLE* ptStypeRecord, STYPE_FLASH_INFO_TABLE* ptStypeFlashInfo)
{
	FRESULT						eFileResult = FR_OK;
	MOTOROLA_STYPE_RESULT_ENUM	eResult = MOTOROLA_STYPE_RESULT_SUCCESS;
	UINT						ReadNum = 0;
	uint8_t						RecordLength = 0;
	uint32_t					i = 0;


	memset(ptStypeFlashInfo, 0x00, sizeof(STYPE_FLASH_INFO_TABLE));

	// S3のレコード長を読み込む
	eFileResult = f_read(pFile, ptStypeRecord->tS3.szLength,sizeof(ptStypeRecord->tS3.szLength), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != sizeof(ptStypeRecord->tS3.szLength))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	RecordLength = StrHexToNum(ptStypeRecord->tS3.szLength);					// レコード長(16進数) : アドレス(4:8/2) + データ(*x2) + CheckSum(1)

	// ロードアドレスを読み込む
	eFileResult = f_read(pFile, ptStypeRecord->tS3.szAddress,sizeof(ptStypeRecord->tS3.szAddress), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != sizeof(ptStypeRecord->tS3.szAddress))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	for (i = 0; i < sizeof(ptStypeRecord->tS3.szAddress); i++)
	{
		ptStypeFlashInfo->Address += atohex(ptStypeRecord->tS3.szAddress[i]);
		if (i < (sizeof(ptStypeRecord->tS3.szAddress) - 1))
		{
			ptStypeFlashInfo->Address = ptStypeFlashInfo->Address << 4;
		}
	}

	// コードを読み込む
	ptStypeFlashInfo->DataSize = RecordLength - (4 + 1);						// レコード長(4) と CheckSum(1)分を引いた値がデータサイズとなる
	eFileResult = f_read(pFile, ptStypeRecord->tS3.szCode,(ptStypeFlashInfo->DataSize * 2), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != (ptStypeFlashInfo->DataSize * 2))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	for (i = 0; i < ptStypeFlashInfo->DataSize; i++)
	{
		ptStypeFlashInfo->Data[i] = StrHexToNum(&ptStypeRecord->tS3.szCode[(i * 2)]);
	}

	// チェクサム・ニューラインを読み込む
	eFileResult = f_read(pFile, ptStypeRecord->tS3.szChecksum,(CHECK_SUM_SIZE + NEW_LINE_SIZE), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != (CHECK_SUM_SIZE + NEW_LINE_SIZE))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	// ニュー・ラインチェック
	if ((ptStypeRecord->tS3.szNewLine[0] != 0x0D) || (ptStypeRecord->tS3.szNewLine[1] != 0x0A))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	// チェクサム(レコード長 + アドレス + データ)
	eResult = CheckSum(ptStypeRecord->tS3.szLength,
		(sizeof(ptStypeRecord->tS3.szLength) + sizeof(ptStypeRecord->tS3.szAddress) + (ptStypeFlashInfo->DataSize * 2)),
		StrHexToNum(ptStypeRecord->tS3.szChecksum));

#ifdef DEBUG_PRINTF
	//  フラッシュ書込み情報を表示する(DEBUG用)
	printf("[%08X] : ", ptStypeFlashInfo->Address);
	for (uint8_t j = 0; j < ptStypeFlashInfo->DataSize; j++)
	{
		printf("%02X ", ptStypeFlashInfo->Data[j]);
	}
	printf("\n");
#endif	// #ifdef DEBUG_PRINTF

	return eResult;
}


//-----------------------------------------------------------------------------
// S7レコード解析
//-----------------------------------------------------------------------------
static MOTOROLA_STYPE_RESULT_ENUM Analyze_S7(FIL *pFile, STYPE_RECORD_TABLE* ptStypeRecord)
{
	FRESULT						eFileResult = FR_OK;
	MOTOROLA_STYPE_RESULT_ENUM	eResult = MOTOROLA_STYPE_RESULT_SUCCESS;
	UINT						ReadNum = 0;
	uint32_t					i = 0;
	uint32_t					EntryPointAddress = 0x00000000;


	// S7のレコード長を読み込む
	eFileResult = f_read(pFile, ptStypeRecord->tS7.szLength,sizeof(ptStypeRecord->tS7.szLength), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != sizeof(ptStypeRecord->tS7.szLength))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	// エントリポイントアドレスを読み込む
	eFileResult = f_read(pFile, ptStypeRecord->tS7.szEntryPointAddress,sizeof(ptStypeRecord->tS7.szEntryPointAddress), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != sizeof(ptStypeRecord->tS7.szEntryPointAddress))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	for (i = 0; i < sizeof(ptStypeRecord->tS7.szEntryPointAddress); i++)
	{
		EntryPointAddress += atohex(ptStypeRecord->tS7.szEntryPointAddress[i]);
		if (i < (sizeof(ptStypeRecord->tS7.szEntryPointAddress) - 1))
		{
			EntryPointAddress = EntryPointAddress << 4;
		}
	}

	// チェクサム・ニューラインを読み込む
	eFileResult = f_read(pFile, ptStypeRecord->tS7.szChecksum,(CHECK_SUM_SIZE + NEW_LINE_SIZE), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != (CHECK_SUM_SIZE + NEW_LINE_SIZE))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	// ニュー・ラインチェック
	if ((ptStypeRecord->tS7.szNewLine[0] != 0x0D) || (ptStypeRecord->tS7.szNewLine[1] != 0x0A))
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	// チェクサム(レコード長 + エントリポイントアドレス)
	eResult = CheckSum(ptStypeRecord->tS7.szLength,
		(sizeof(ptStypeRecord->tS7.szLength) + sizeof(ptStypeRecord->tS7.szEntryPointAddress)),
		StrHexToNum(ptStypeRecord->tS7.szChecksum));

#ifdef DEBUG_PRINTF
	//  エントリポイント情報を表示する(DEBUG用)
	printf("+++ Entry Point Address : %08X +++\n", EntryPointAddress);
#endif	// #ifdef DEBUG_PRINTF

	return eResult;
}


//-----------------------------------------------------------------------------
// S-Typeレコード読込み
//-----------------------------------------------------------------------------
MOTOROLA_STYPE_RESULT_ENUM ReadStypeRecord(FIL *pFile, STYPE_RECORD_TABLE* ptStypeRecord, STYPE_FLASH_INFO_TABLE* ptStypeFlashInfo)
{
	FRESULT						eFileResult = FR_OK;
	MOTOROLA_STYPE_RESULT_ENUM	eResult = MOTOROLA_STYPE_RESULT_SUCCESS;
	UINT						ReadNum = 0;


	// 2文字読み込んで、S-Typeレコードタイプを調べる
	eFileResult = f_read(pFile, ptStypeRecord->szRecordName,sizeof(ptStypeRecord->szRecordName), &ReadNum);
	if (eFileResult != FR_OK)
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}
	if (ReadNum != 2)
	{
		return MOTOROLA_STYPE_RESULT_FILE_EOF;
	}

	// レコード種別チェック
	if (ptStypeRecord->szRecordName[0] != 'S')
	{
		return MOTOROLA_STYPE_RESULT_ERROR_ANALYZE;
	}

	switch (ptStypeRecord->szRecordName[1]) {
	case '0':
		// S-Type 0 レコード解析処理
		ptStypeRecord->eStypeRecordKind = STYPE_RECORD_KIND_S0;
		eResult = Analyze_S0(pFile, ptStypeRecord);
		break;
	case '3':
		// S-Type 3 レコード解析処理
		ptStypeRecord->eStypeRecordKind = STYPE_RECORD_KIND_S3;
		eResult = Analyze_S3(pFile, ptStypeRecord, ptStypeFlashInfo);
		break;
	case '7':
		// S-Type 7 レコード解析処理
		ptStypeRecord->eStypeRecordKind = STYPE_RECORD_KIND_S7;
		eResult = Analyze_S7(pFile, ptStypeRecord);
		break;
	case '1':
	case '2':
	case '8':
	case '9':
	default:
		eResult = MOTOROLA_STYPE_RESULT_ERROR_UNKNOWN_STYPE;
		break;
	}

	return eResult;
}

