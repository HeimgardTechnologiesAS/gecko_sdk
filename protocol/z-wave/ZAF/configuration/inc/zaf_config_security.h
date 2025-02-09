/**
 * @file
 * @copyright 2022 Silicon Laboratories Inc.
 */
#ifndef ZAF_CONFIG_SECURITY_H_
#define ZAF_CONFIG_SECURITY_H_

#include "zaf_config.h"
#include "ZW_security_api.h"

#if ZAF_CONFIG_REQUEST_KEY_S0==1
#define S0_BIT                  SECURITY_KEY_S0_BIT
#else
#define S0_BIT                  0
#endif

#if ZAF_CONFIG_REQUEST_KEY_S2_UNAUTHENTICATED==1
#define S2_UNAUTHENTICATED_BIT  SECURITY_KEY_S2_UNAUTHENTICATED_BIT
#else
#define S2_UNAUTHENTICATED_BIT  0
#endif

#if ZAF_CONFIG_REQUEST_KEY_S2_AUTHENTICATED==1
#define S2_AUTHENTICATED_BIT    SECURITY_KEY_S2_AUTHENTICATED_BIT
#else
#define S2_AUTHENTICATED_BIT    0
#endif

#if ZAF_CONFIG_REQUEST_KEY_S2_ACCESS==1
#define S2_ACCESS_BIT           SECURITY_KEY_S2_ACCESS_BIT
#else
#define S2_ACCESS_BIT           0
#endif

#define ZAF_CONFIG_REQUESTED_SECURITY_KEYS (S0_BIT | S2_UNAUTHENTICATED_BIT | S2_AUTHENTICATED_BIT | S2_ACCESS_BIT)

#endif /* ZAF_CONFIG_SECURITY_H_ */
