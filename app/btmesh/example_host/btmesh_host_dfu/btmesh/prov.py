# Copyright 2022 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

import logging

from bgapi.bglib import CommandFailedError

from bgapix.bglibx import BGLibExt
from bgapix.slstatus import SlStatus

from . import util
from .db import BtmeshDatabase, Node
from .errors import BtmeshError, BtmeshErrorCode
from .event import LocalEventBus

logger = logging.getLogger(__name__)


class Provisioner:
    NETKEY_IDX = 0

    def __init__(self, lib: BGLibExt, db: BtmeshDatabase, evtbus: LocalEventBus):
        self.lib = lib
        self.db = db
        self.evtbus = evtbus

    def init(self):
        # Initialize the NCP device as provisioner
        try:
            self.lib.btmesh.prov.init()
            event = self.lib.wait_events(
                ["btmesh_evt_prov_initialized", "btmesh_evt_prov_initialization_failed"]
            )[0]
            if event == "btmesh_evt_prov_initialized":
                return event.networks
            else:
                raise BtmeshError(
                    BtmeshErrorCode.PROV_INIT_FAILED,
                    f"Provisioner initialization failed. ({event.errorcode})",
                    event=event,
                )
        except CommandFailedError as e:
            if e.errorcode != SlStatus.INVALID_STATE:
                raise
        # The get networks should not fail unless there is a fundamental crypto
        # issue on the NCP node
        response = self.lib.btmesh.node.get_networks()
        # The response.networks contains the netkey indexes bytes and each
        # netkey index is two bytes so the byte count shall be divided by 2
        # to determine the number of networks on the provisioner
        return len(response.networks) // 2

    @property
    def prov_node(self) -> Node:
        return self.db.get_node_by_uuid(self.db.prov_uuid)

    def create_network(
        self, netkey=bytes(), appkeys=[bytes()], prov_node_name="Provisioner"
    ):
        # One network key is created with one bound application key
        # If the key is a zero length bytes then the key is random generated
        self.lib.btmesh.prov.create_network(self.NETKEY_IDX, netkey)
        rsp = self.lib.btmesh.node.get_uuid()
        uuid = rsp.uuid
        rsp = self.lib.btmesh.prov.get_ddb_entry(uuid)
        prov_node = Node(
            uuid=uuid,
            devkey=rsp.device_key,
            prim_addr=rsp.address,
            elem_count=rsp.elements,
            name=prov_node_name,
        )
        self.db.add_node(prov_node)
        self.db.prov_uuid = uuid
        for appkey_index, appkey in enumerate(appkeys):
            self.lib.btmesh.prov.create_appkey(self.NETKEY_IDX, appkey_index, appkey)
            prov_node.add_appkey_index(appkey_index)

    def scan_unprov_beacons_gen(self, max_time=2.0):
        try:
            self.lib.btmesh.prov.scan_unprov_beacons()
        except CommandFailedError as e:
            # The INVALID_STATE error is raised if the scanning is already
            # active which is not an error in this case
            if e.errorcode != SlStatus.INVALID_STATE:
                raise
        try:
            for unprov_beacon in self.lib.gen_events(
                event_selector="btmesh_evt_prov_unprov_beacon",
                max_time=max_time,
                timeout=None,
            ):
                yield unprov_beacon
        except Exception:
            try:
                self.lib.btmesh.prov.stop_scan_unprov_beacons()
            except CommandFailedError as e:
                # If an event handler raises an exception when a reset occurs
                # then this stop call will fail. This function call is used for
                # cleanup purposes so raising another exception inside an
                # exception shall be avoided.
                logger.error("Failed to stop scanning for unprovisioned beacons.")
            raise
        else:
            self.lib.btmesh.prov.stop_scan_unprov_beacons()

    def scan_unprov_beacons(self, max_time=2.0):
        return list(self.scan_unprov_beacons_gen(max_time=max_time))

    def provision_adv_device(self, uuids, max_time=20):
        if not isinstance(uuids, (list, tuple)):
            uuids = [uuids]
        for uuid in uuids:
            # Create provisioning session command fails only:
            # - SL_STATUS_INVALID_PARAMETER: Provisioning session with the same
            #   uuid is active
            # - SL_STATUS_NO_MORE_RESOURCE: Maximum number of provisioning
            #   sessions would be exceeded.
            # Note: The max number of provisioning sessions is configurable in
            #       sl_btmesh_config.h of NCP node by c macro:
            #       SL_BTMESH_CONFIG_MAX_PROV_SESSIONS
            # The above errors are not possible because only one provisioning
            # session can be active at the same time because the app code runs
            # on a single thread.
            self.lib.btmesh.prov.create_provisioning_session(self.NETKEY_IDX, uuid, 10)
            # The provision ADV device command may fail:
            # - SL_STATUS_INVALID_PARAMETER: If the device is already provisioned
            # - SL_STATUS_BT_MESH_LIMIT_REACHED: There is no space in the device
            #   database on the NCP node
            # - SL_STATUS_BT_MESH_DOES_NOT_EXIST: No provisioning session is
            #   created previously for the passed uuid
            # - SL_STATUS_NO_MORE_RESOURCE: Memory allocation fails
            # Note: The max number of devices in device database is configurable
            #       in sl_btmesh_config.h of NCP node by c macro:
            #       SL_BTMESH_CONFIG_MAX_PROVISIONED_DEVICES
            self.lib.btmesh.prov.provision_adv_device(uuid)
            event = self.lib.wait_events(
                [
                    "btmesh_evt_prov_device_provisioned",
                    "btmesh_evt_prov_provisioning_failed",
                ],
                max_time=max_time,
            )[0]
            if event == "btmesh_evt_prov_provisioning_failed":
                raise BtmeshError(
                    BtmeshErrorCode.PROVISIONING_FAILED,
                    f"Provisioning {event.uuid.hex()} device failed "
                    f"with {util.prov_failure_reason_str(event.reason)}.",
                    event=event,
                )
            elif event == "btmesh_evt_prov_device_provisioned":
                rsp = self.lib.btmesh.prov.get_ddb_entry(uuid)
                node = Node(
                    uuid=uuid,
                    devkey=rsp.device_key,
                    prim_addr=rsp.address,
                    elem_count=rsp.elements,
                )
                self.db.add_node(node)
