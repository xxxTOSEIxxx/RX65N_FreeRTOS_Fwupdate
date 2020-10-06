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
 * File Name    : r_usb_hmsc_apl_config.h
 * Description  : USB Host MSC program configuration file.
 ***********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *         : 08.01.2014 1.00 First Release
 *         : 26.12.2014 1.10 RX71M is added
 *         : 30.09.2015 1.11 RX63N/RX631 is added.
 *         : 30.09.2016 1.20 RX65N/RX651 is added.
 *         : 01.03.2020 1.30 RX72N/RX66N is added and uITRON is supported.
 ***********************************************************************************************************************/

#ifndef R_USB_HMSC_APL_CONFIG_H
#define R_USB_HMSC_APL_CONFIG_H

/******************************************************************************
 User define macro definitions
 ******************************************************************************/

/** [USB module selection setting]
 *  USE_USBIP0              : Uses USB0 module
 *  USE_USBIP1              : Uses USB1 module
 *  USE_USBIP0 | USE_USBIP1 : Uses USB0 and USB1 modules
 */
#define USE_USBIP       (USE_USBIP0)

/** [Select Support Multi Device]
 *  USB_APL_ENABLE        : Support Multi Device
 *  USB_APL_DISABLE       : Not support Multi Device(Support Single device)
 */
#define USB_SUPPORT_MULTI   (USB_APL_DISABLE)

/** [Select Support Free RTOS]
 *  USB_APL_ENABLE        : Support Free RTOS
 *  USB_APL_DISABLE       : Not support Free RTOS(Support non OS)
 */
#define USB_SUPPORT_RTOS    (USB_APL_ENABLE)

#endif  /* R_USB_HMSC_APL_CONFIG_H */
/******************************************************************************
 End  Of File
 ******************************************************************************/

