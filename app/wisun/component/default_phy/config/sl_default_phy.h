/**************************************************************************//**
* @file sl_default_phy.h
* @brief Default PHY selection per board
*******************************************************************************
* # License
* <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

#ifndef APP_DEFAULT_PHY_H
#define APP_DEFAULT_PHY_H

#include "sl_component_catalog.h"

#if defined SL_CATALOG_BRD4163A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 3 // EU
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1a
#elif defined SL_CATALOG_BRD4164A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4170A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4172A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 4 // CN
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4173A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 4 // CN
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4253A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4254A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 3 // EU
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1a
#elif defined SL_CATALOG_BRD4270A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4270B_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4271A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 3 // EU
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1a
#elif defined SL_CATALOG_BRD4272A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 4 // CN
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4273A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4274A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 3 // EU
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1a
#elif defined SL_CATALOG_BRD4400A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4401A_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4400B_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#elif defined SL_CATALOG_BRD4401B_PRESENT
#define APP_SETTINGS_WISUN_DEFAULT_REGULATORY_DOMAIN 1 // NA
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_CLASS 1
#define APP_SETTINGS_WISUN_DEFAULT_OPERATING_MODE 0x1b
#else
#error "this board is not supported"
#endif

#endif // APP_DEFAULT_PHY_H
