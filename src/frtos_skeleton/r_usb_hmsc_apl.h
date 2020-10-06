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
 * Copyright (C) 2014(2020) Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * File Name    : r_usb_hmsc_apl.h
 * Description  : USB Host MSC application code
 ***********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *         : 08.01.2014 1.00 First Release
 *         : 26.12.2014 1.10 RX71M is added
 *         : 30.09.2015 1.11 RX63N/RX631 is added.
 *         : 30.09.2016 1.20 RX65N/RX651 is added.
 *         : 01.03.2020 1.30 RX72N/RX66N is added and uITRON is supported.
 ***********************************************************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "r_usb_basic_if.h"
#include "r_usb_hmsc_if.h"
#include "ff.h"             /* TFAT header */
#include "r_usb_basic_pinset.h"

/******************************************************************************
 Macro definitions
 ******************************************************************************/
#define USE_USBIP0                  (0x01u)     /* USB0 module */
#define USE_USBIP1                  (0x02u)     /* USB1 module */

#define USB_APL_DISABLE             (0)
#define USB_APL_ENABLE              (1)

#define USB_APL_YES                 (1)
#define USB_APL_NO                  (0)

#define FILE_SIZE                   (512)
#define NUM_USBIP                   (2)   /* multi */

/** Support Strage Drive number
 * #define MAX_DRIVE_NUM                     (1)
 * #define MAX_DRIVE_NUM                     (2)
 * #define MAX_DRIVE_NUM                     (3)
 * #define MAX_DRIVE_NUM                     (4)
 * **/
#define MAX_DRIVE_NUM               (4)

/*****************************************************************************
 Enumerated Types
 ******************************************************************************/

/******************************************************************************
 Exported global variables
 ******************************************************************************/

/*****************************************************************************
 Function
 ******************************************************************************/

/******************************************************************************
 End  Of File
 ******************************************************************************/

