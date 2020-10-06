/*
 * Global.h
 *
 *  Created on: 2020/09/27
 *      Author: MIBC
 */

#ifndef FRTOS_SKELETON_GLOBAL_H_
#define FRTOS_SKELETON_GLOBAL_H_

// USB接続種別
typedef enum
{
	USB_KIND_DISCONNECT = 0,		// USB未接続
	USB_KIND_CONNECT = 1,			// USB接続

} USB_KIND_ENUM;


// LED制御種別
typedef enum
{
	LED_KIND_OFF = 0,				// OFF
	LED_KIND_ON,					// ON
	LED_KIND_PROCESS,				// 処理中
	LED_KIND_ERROR,					// エラー
	LED_KIND_MAX
} LED_KIND_ENUM;




typedef struct
{
	USB_KIND_ENUM 					eUsbKind;			// USB接続種別
	LED_KIND_ENUM					eLedKind;			// LED制御種別

} GLOBAL_INFO_TABLE;









#endif /* FRTOS_SKELETON_GLOBAL_H_ */
