/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef __HAL_EINT_H__
#define __HAL_EINT_H__
#include "hal_platform.h"



#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup hal_eint_enum Enum
  * @{
  */

/** @brief This emun defines the EINT trigger mode.  */
typedef enum {
    HAL_EINT_LEVEL_LOW     = 0,                 /**< Level and low trigger. */
    HAL_EINT_LEVEL_HIGH    = 1,                 /**< Level and high trigger. */
    HAL_EINT_EDGE_FALLING  = 2,                 /**< Edge and falling trigger. */
    HAL_EINT_EDGE_RISING   = 3,                 /**< Edge and rising trigger. */
    HAL_EINT_EDGE_FALLING_AND_RISING = 4        /**< Edge and falling or rising trigger. */
} hal_eint_trigger_mode_t;


/** @brief  This enum define the API return type.  */
typedef enum {
    HAL_EINT_STATUS_ERROR_EINT_NUMBER  = -3,     /**< EINT error number. */
    HAL_EINT_STATUS_INVALID_PARAMETER  = -2,     /**< EINT error invalid parameter. */
    HAL_EINT_STATUS_ERROR              = -1,     /**< EINT undefined error. */
    HAL_EINT_STATUS_OK                 = 0       /**< EINT operation completed successfully. */
} hal_eint_status_t;


/**
  * @}
  */


/** @defgroup hal_eint_struct Struct
  * @{
  */

/** @brief This structure defines the initial configuration structure. For more information please refer to #hal_eint_init(). */
typedef struct {
    hal_eint_trigger_mode_t trigger_mode;      /**< EINT trigger mode. */
    uint32_t debounce_time;                    /**< EINT hardware debounce time in milliseconds. EINT debounce is disabled when the debounce time is set to zero. */
#ifdef HAL_EINT_FEATURE_FIRQ
    bool     firq_enable;                      /**< EINT firq mode enable. The true indicates fast eint, flase indicates normal eint */
#endif
} hal_eint_config_t;

/**
  * @}
  */


/** @defgroup hal_eint_typedef Typedef
  * @{
  */
/** @brief  This defines the callback function prototype.
 *          A callback function should be registered for every EINT in use.
 *          This function will be called after an EINT interrupt is triggered in the EINT ISR.
 *          For more details about the callback function, please refer to hal_eint_register_callback().
 *  @param [out] user_data is the parameter which is set manually using hal_eint_register_callback() function.
 */
typedef void (*hal_eint_callback_t)(void *user_data);

/**
  * @}
  */


/*****************************************************************************
* Functions
*****************************************************************************/

/**
 * @brief This function initializes the EINT number, it sets the EINT trigger mode and debounce time.
 * @param[in] eint_number is the EINT number, the value is HAL_EINT_0~HAL_EINT_MAX-1.
 * @param[in] eint_config is the initial configuration parameter. EINT debounce is disabled when debounce time is set to zero. For more details, please refer to #hal_eint_config_t.
 * @return    To indicate whether this function call is successful.
 *            If the return value is #HAL_EINT_STATUS_OK, the operation completed successfully;
 *            If the return value is #HAL_EINT_STATUS_INVALID_PARAMETER, a wrong parameter is given, the parameter must be verified.
 * @sa  hal_eint_deinit()
 * @par       Example
 * Sample code please refer to @ref HAL_EINT_Driver_Usage_Chapter
*/
hal_eint_status_t hal_eint_init(hal_eint_number_t eint_number, const hal_eint_config_t *eint_config);


/**
 * @brief This function deinitializes the EINT number, it resets the EINT trigger mode and debounce time.
 * @param[in] eint_number is the EINT number, the value is HAL_EINT_0~HAL_EINT_MAX-1.
 * @return    To indicate whether this function call is successful.
 *            If the return value is #HAL_EINT_STATUS_OK, the operation completed successfully;
 *            If the return value is #HAL_EINT_STATUS_INVALID_PARAMETER, a wrong parameter is given, the parameter must be verified.
 * @sa  hal_eint_deinit()
 * @par       Example
 * Sample code please refer to @ref HAL_EINT_Driver_Usage_Chapter
*/
hal_eint_status_t hal_eint_deinit(hal_eint_number_t eint_number);


/**
 * @brief This function registers a callback function for a specified EINT number.
 * @param[in] eint_number is the EINT number, the value is HAL_EINT_0~HAL_EINT_MAX-1, please refer to #hal_eint_number_t.
 * @param[in] callback is the function given by the user, which will be called at EINT ISR routine.
 * @param[in] user_data is a reserved parameter for user.
 * @return    To indicate whether this function call is successful.
 *            If the return value is #HAL_EINT_STATUS_OK, the operation completed successfully;
 *            If the return value is #HAL_EINT_STATUS_INVALID_PARAMETER, a wrong parameter is given, the parameter must be verified.
 * @sa  hal_eint_init()
 * @par       Example
 * Sample code please refer to @ref HAL_EINT_Driver_Usage_Chapter
 */
hal_eint_status_t hal_eint_register_callback(hal_eint_number_t eint_number,
        hal_eint_callback_t callback,
        void *user_data);




/**
 * @brief This function masks the dedicated EINT source.
 * @param[in] eint_number is the EINT number, the value is from HAL_EINT_0 to HAL_EINT_MAX-1, please refer to #hal_eint_number_t.
 * @return    To indicate whether this function call is successful.
 *            If the return value is #HAL_EINT_STATUS_OK,  the operation completed successfully;
 *            If the return value is #HAL_EINT_STATUS_INVALID_PARAMETER, a wrong parameter is given, the parameter must be verified.
 * @sa  #hal_eint_unmask
 */
hal_eint_status_t hal_eint_mask(hal_eint_number_t eint_number);


/**
 * @brief This function unmasks the dedicated EINT source.
 * @param[in] eint_number is the EINT number, the value is HAL_EINT_0~HAL_EINT_MAX-1, please refer to #hal_eint_number_t.
 * @return    To indicate whether this function call is successful.
 *            If the return value is #HAL_EINT_STATUS_OK,  the operation completed successfully;
 *            If the return value is #HAL_EINT_STATUS_INVALID_PARAMETER, a wrong parameter is given, the parameter must be verified.
 * @sa  hal_eint_mask()
 */
hal_eint_status_t hal_eint_unmask(hal_eint_number_t eint_number);





#ifdef __cplusplus
}
#endif


/**
* @}
* @}
*/

#endif /* __HAL_EINT_H__ */


