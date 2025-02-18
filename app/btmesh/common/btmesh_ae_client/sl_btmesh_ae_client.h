/***************************************************************************//**
 * @file
 * @brief BT Mesh Advertisement Extension Client
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_BTMESH_AE_CLIENT_H
#define SL_BTMESH_AE_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/***************************************************************************//**
 * @addtogroup ae_client BT Mesh Advertisement Extension Client
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * Handle Advertisement Extension Client events.
 *
 * This function is called automatically by Universal Configurator after
 * enabling the component.
 *
 * @param[in] evt  Pointer to incoming event.
 *
 ******************************************************************************/
void sl_btmesh_ae_client_on_event(const sl_btmesh_msg_t *const evt);

/** @} end ae_client */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SL_BTMESH_AE_CLIENT_H
