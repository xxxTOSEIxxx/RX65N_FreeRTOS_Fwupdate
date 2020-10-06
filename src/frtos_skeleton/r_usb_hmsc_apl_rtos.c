/**********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO 
 * THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2014(2020) Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : r_usb_hmsc_apl.c
 * Description  : USB Host MSC application code
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *         : 08.01.2014 1.00 First Release
 *         : 26.12.2014 1.10 RX71M is added
 *         : 30.09.2015 1.11 RX63N/RX631 is added.
 *         : 30.09.2016 1.20 RX65N/RX651 is added.
 *         : 31.03.2018 1.23 Using Pin setting API.
 *         : 01.03.2020 1.30 RX72N/RX66N is added and uITRON is supported.
 *********************************************************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "r_usb_hmsc_apl.h"
#include "r_usb_hmsc_apl_config.h"


#if USB_SUPPORT_RTOS == USB_APL_ENABLE
#if USB_SUPPORT_MULTI == USB_APL_DISABLE

#include "r_usb_typedef.h"
#include "r_usb_rtos_apl.h"
#include "r_rtos_abstract.h"


#include "Global.h"
extern GLOBAL_INFO_TABLE			g_tGlobalInfo;


#if (BSP_CFG_RTOS_USED == 4)        /* Renesas RI600V4 & RI600PX */
#include "kernel_id.h"
#endif /* (BSP_CFG_RTOS_USED == 4) */

/******************************************************************************
 Private global variables and functions
 ******************************************************************************/
static  FATFS       g_file_object;      /* File system object structure */
//static  uint8_t     g_file_data[FILE_SIZE];
//static  uint8_t     g_isFileWrite;
//static  UINT        g_file_size;

static  rtos_mbx_id_t   g_usb_apl_mbx_id;

const   static FATFS *g_pfat = &g_file_object;
//static  uint8_t g_msc_file[15] =
//{
//    "0:hmscdemo.txt"  /* Drive No. 0 */
//};

static void usb_pin_setting(void);
//static void apl_init(void);

#if (BSP_CFG_RTOS_USED != 0)    /* Use RTOS */
static  rtos_mbx_id_t   g_usb_apl_mbx_id;
#endif /* (BSP_CFG_RTOS_USED != 0) */

/******************************************************************************
 Exported global functions
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_apl_callback
 Description     : Callback function for HMSC application program
 Arguments       : usb_ctrl_t *p_ctrl   : 
                   rtos_task_id_t  cur_task  : 
                   uint8_t    usb_state : 
 Return value    : none
 ******************************************************************************/
void usb_apl_callback (usb_ctrl_t *p_ctrl, rtos_task_id_t cur_task, uint8_t usb_state)
{
    rtos_send_mailbox(&g_usb_apl_mbx_id, (void *)p_ctrl);

} /* End of function usb_apl_callback */

/******************************************************************************
 Function Name   : usb_main
 Description     : Host MSC application main process
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_main (void)
{
	FRESULT     eFileResult = FR_OK;
    usb_ctrl_t  ctrl;
    usb_cfg_t   cfg;
    usb_ctrl_t  *p_mess;
//    FIL         file;
//    FRESULT     ret;
#if (BSP_CFG_RTOS_USED == 1)                                /* FreeRTOS */
    rtos_mbx_info_t     mbx_info;
#endif /* (BSP_CFG_RTOS_USED == 1) */

#if (BSP_CFG_RTOS_USED == 1)                                /* FreeRTOS */
    mbx_info.length = USB_APL_QUEUE_SIZE;
    rtos_create_mailbox(&g_usb_apl_mbx_id, &mbx_info);      /* For APL */
#elif (BSP_CFG_RTOS_USED == 4)                              /* Renesas RI600V4 & RI600PX */
    g_usb_apl_mbx_id = ID_USB_APL_MBX;
#else
    /*  */
#endif /* (BSP_CFG_RTOS_USED == 1) */

//    apl_init();

    usb_pin_setting();  /* USB pin function and port mode setting. */

#if ((USE_USBIP & USE_USBIP0) == USE_USBIP0)
    ctrl.module     = USB_IP0;
    ctrl.type       = USB_HMSC;
    cfg.usb_speed   = USB_FS;
    cfg.usb_mode    = USB_HOST;
    R_USB_Open(&ctrl, &cfg);
#endif

#if((USE_USBIP & USE_USBIP1) == USE_USBIP1)
    ctrl.module     = USB_IP1;
    ctrl.type       = USB_HMSC;
    cfg.usb_speed   = USB_FS;
    cfg.usb_mode    = USB_HOST;
    R_USB_Open(&ctrl, &cfg);
#endif

    R_USB_Callback((usb_callback_t *)usb_apl_callback);

    while (1)
    {
        rtos_receive_mailbox(&g_usb_apl_mbx_id, (void **)&p_mess, RTOS_FOREVER);

        ctrl = *p_mess;

        switch (ctrl.event)
        {
            case USB_STS_CONFIGURED :
                /* Create a file object. */
            	eFileResult = f_mount((FATFS *) g_pfat,(const TCHAR*)"0:", 1);
            	if (eFileResult != FR_OK)
            	{
            		printf("f_mount Error. [eFileResult:%d]\n",eFileResult);
            	}
            	else
            	{
					R_USB_HmscGetSem();
					g_tGlobalInfo.eUsbKind = USB_KIND_CONNECT;
					g_tGlobalInfo.eLedKind = LED_KIND_ON;
					R_USB_HmscRelSem();

					printf("USB_STS_CONFIGURED\n");
            	}
                break;

            case USB_STS_DETACH :
            	R_USB_HmscGetSem();
                g_tGlobalInfo.eUsbKind = USB_KIND_DISCONNECT;
                g_tGlobalInfo.eLedKind = LED_KIND_OFF;
                R_USB_HmscRelSem();

                printf("USB_STS_DETACH\n");
                break;

            default :
            	break;

        } /* switch( event ) */
    } /* while(1) */
} /* End of function usb_main */

#if 0
/******************************************************************************
Function Name   : file_read_task
Description     : File Read Task in MSC device
Arguments       : none
Return value    : none
******************************************************************************/
#if (BSP_CFG_RTOS_USED == 4)        /* Renesas RI600V4 & RI600PX */
void file_read_task (VP_INT b)
#else  /* (BSP_CFG_RTOS_USED == 4) */
void file_read_task(void)
#endif /* (BSP_CFG_RTOS_USED == 4) */
{
    FIL         file;
    FRESULT     ret;

    while (1)
    {
        if (USB_APL_YES == g_isFileWrite)
        {
            R_USB_HmscGetSem();
            /* AA */
            ret = f_open(&file, (const char *)g_msc_file, (FA_OPEN_ALWAYS | FA_READ));
            if (FR_OK == ret)
            {
                ret = f_read(&file, g_file_data, sizeof(g_file_data), (UINT *)&g_file_size);
                if (FR_OK == ret)
                {
                    ret = f_close(&file);
                }
            }

            if (FR_OK != ret)
            {
                g_isFileWrite = USB_APL_NO;
            }
            R_USB_HmscRelSem();
            /* BB */
        }
    }
}   /* end of task file_read_task */
#endif


/******************************************************************************
 Function Name   : usb_pin_setting
 Description     : USB pin setting processing
 Arguments       : none
 Return value    : none
 ******************************************************************************/
static void usb_pin_setting (void)
{
#if ((USE_USBIP & USE_USBIP0) == USE_USBIP0)
    R_USB_PinSet_USB0_HOST();
#endif

#if((USE_USBIP & USE_USBIP1) == USE_USBIP1)
    R_USB_PinSet_USBA_HOST();
#endif
} /* End of function usb_pin_setting */

#if 0
/******************************************************************************
 Function Name   : apl_init
 Description     : Host MSC Sample APL Initialize Command Send
 Arguments       : none
 Return value    : none
 ******************************************************************************/
static void apl_init (void)
{
    uint16_t     i;

    g_isFileWrite = USB_APL_NO;

    for (i = 0; i < FILE_SIZE; i++)
    {
        g_file_data[i] = (uint8_t)(0x30 + (i % 10));
    }
} /* End of function apl_init */
#endif

/******************************************************************************
 Other functions
 ******************************************************************************/
#endif /* USB_SUPPORT_MULTI == USB_APL_DISABLE */
#endif /* USB_SUPPORT_RTOS == USB_APL_ENABLE */

/******************************************************************************
 End  Of File
 ******************************************************************************/
