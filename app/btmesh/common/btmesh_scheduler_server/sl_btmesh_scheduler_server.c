/***************************************************************************//**
 * @file
 * @brief Scheduler Server module
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdbool.h>
#include "em_common.h"
#include "sl_status.h"
#include "sl_bt_api.h"
#include "sl_btmesh_api.h"
#include "sl_btmesh_dcd.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#endif // SL_CATALOG_APP_LOG_PRESENT

#include "sl_btmesh_scheduler_server.h"
#include "sl_btmesh_scheduler_server_config.h"

// Warning! The app_btmesh_util shall be included after the component configuration
// header file in order to provide the component specific logging macro.
#include "app_btmesh_util.h"

/***************************************************************************//**
 * @addtogroup Scheduler Server
 * @{
 ******************************************************************************/

/*******************************************************************************
 * Scheduler initialization.
 * This should be called at each boot if provisioning is already done.
 * Otherwise this function should be called after provisioning is completed.
 *
 * @param[in] element Index of the element where scheduler model is initialized.
 *
 * @return Status of the initialization operation.
 *         Returns bg_err_success (0) if succeed, non-zero otherwise.
 ******************************************************************************/
uint16_t sl_btmesh_scheduler_init(void)
{
  // Initialize scheduler server models
  sl_status_t result = sl_btmesh_scheduler_server_init(BTMESH_SCHEDULER_SERVER_MAIN);

  log_status_error_f(result, "sl_btmesh_scheduler_server_init failed" NL);

  return result;
}

/***************************************************************************//**
 * Handling of scheduler server action changed event.
 *
 * @param[in] evt  Pointer to scheduler server action changed event.
 ******************************************************************************/
static void handle_scheduler_server_action_changed_event(
  sl_btmesh_evt_scheduler_server_action_changed_t *evt)
{
  log_info("evt:gecko_evt_mesh_scheduler_server_action_changed_id, \
elem_index=%u, index=%u, ", evt->elem_index, evt->index);
  if (evt->year == 100) {
    log_append_info("year=Every, ");
  } else {
    log_append_info("year=%u, ", evt->year);
  }
  log_append_info("month=0x%03x, ", evt->month);
  if (evt->day) {
    log_append_info("day=0x%02x, ", evt->day);
  } else {
    log_append_info("day=Every, ");
  }
  if (evt->hour == 0x18) {
    log_append_info("hour=Every, ");
  } else if (evt->hour == 0x19) {
    log_append_info("hour=Random, ");
  } else {
    log_append_info("hour=%u, ", evt->hour);
  }
  if (evt->minute == 0x3c) {
    log_append_info("minute=Every, ");
  } else if (evt->minute == 0x3d) {
    log_append_info("minute=Every 15, ");
  } else if (evt->minute == 0x3e) {
    log_append_info("minute=Every 20, ");
  } else if (evt->minute == 0x3f) {
    log_append_info("minute=Random, ");
  } else {
    log_append_info("minute=%u, ", evt->minute);
  }
  if (evt->second == 0x3c) {
    log_append_info("second=Every, ");
  } else if (evt->second == 0x3d) {
    log_append_info("second=Every 15, ");
  } else if (evt->second == 0x3e) {
    log_append_info("second=Every 20, ");
  } else if (evt->second == 0x3f) {
    log_append_info("second=Random, ");
  } else {
    log_append_info("second=%u, ", evt->second);
  }
  log_append_info("day of the week=0x%02x, ", evt->day_of_week);
  log_append_info("action=");
  switch (evt->action) {
    case 0x0:
      log_append_info("Turn Off");
      break;

    case 0x1:
      log_append_info("Turn On");
      break;

    case 0x2:
      log_append_info("Scene Recall");
      break;

    case 0xf:
      log_append_info("No action");
      break;

    default:
      break;
  }
  log_append_info(", transition time=%lu ms, ", evt->transition_time_ms);
  if (evt->scene_number) {
    log_append_info("scene number=0x%04x", evt->scene_number);
  } else {
    log_append_info("scene number=No scene");
  }
  log_append_info(NL);
}

/*******************************************************************************
 * Handling of mesh scheduler events.
 * It handles:
 *  - scheduler_server_action_changed
 *
 * @param[in] evt  Pointer to incoming scheduler event.
 ******************************************************************************/
void sl_btmesh_scheduler_server_on_event(sl_btmesh_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_btmesh_evt_scheduler_server_action_changed_id:
      handle_scheduler_server_action_changed_event(
        &(evt->data.evt_scheduler_server_action_changed));
      break;

    case sl_btmesh_evt_node_initialized_id:
      if (evt->data.evt_node_initialized.provisioned) {
        sl_btmesh_scheduler_init();
      }
      break;

    case sl_btmesh_evt_node_provisioned_id:
      sl_btmesh_scheduler_init();
      break;

    default:
      break;
  }
}

/** @} (end addtogroup Scheduler Server) */
