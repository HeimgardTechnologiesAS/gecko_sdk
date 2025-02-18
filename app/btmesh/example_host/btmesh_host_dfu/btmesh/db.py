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

import dataclasses
import operator
from typing import Callable, ClassVar, Dict, List, Mapping, Optional, Union

from . import util
from .event import LocalEvent
from .mdl import NamedModelID
from .statedict import StateDictObject


class Network(StateDictObject):
    def __init__(self, netkey_index, netkey):
        self.netkey_index = netkey_index
        self.netkey = netkey
        self.appkeys = []


class Node(StateDictObject):
    @classmethod
    def is_name_valid(cls, node_name):
        return util.is_name_valid(node_name)

    @classmethod
    def is_uuid_valid(cls, uuid):
        return util.is_uuid_valid(uuid)

    @classmethod
    def is_addr_valid(cls, addr):
        return util.is_unicast_address(addr)

    def __init__(
        self,
        uuid,
        devkey,
        prim_addr,
        elem_count,
        name=None,
        appkey_indexes=[],
        dcd=None,
    ):
        self.uuid = uuid
        self.devkey = StateDictObject.to_bytes(devkey)
        self.prim_addr = prim_addr
        self.elem_count = elem_count
        if name:
            self.name = name
        else:
            self.name = f"Node_{self.prim_addr:04X}"
        self._appkey_indexes = []
        for appkey_index in appkey_indexes:
            self.add_appkey_index(appkey_index)
        if dcd is None:
            self.dcd = dcd
        elif isinstance(dcd, Mapping):
            self.dcd = DCD.create_from_dict(dcd)
        elif isinstance(dcd, DCD):
            self.dcd = dcd
        else:
            self.raise_construction_error("dcd", dcd, type_error=True)

    @property
    def uuid(self):
        return self._uuid

    @uuid.setter
    def uuid(self, value):
        if not self.is_uuid_valid(value):
            self.raise_construction_error("uuid", value)
        self._uuid = StateDictObject.to_bytes(value)

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, value):
        if not self.is_name_valid(value):
            self.raise_construction_error("name", value)
        self._name = value

    @property
    def elem_addrs(self):
        return [
            addr for addr in range(self.prim_addr, self.prim_addr + self.elem_count)
        ]

    def is_elem_addr(self, addr):
        return self.prim_addr <= addr < (self.prim_addr + self.elem_count)

    def get_elem_addrs(self, elem_indexes):
        if isinstance(elem_indexes, int):
            elem_indexes = [elem_indexes]
        elem_addrs = []
        for elem_idx in elem_indexes:
            if self.elem_count <= elem_idx:
                raise ValueError(
                    f"Node ({self.uuid.hex()}) element index "
                    f"{elem_idx} does not exits."
                )
            elem_addrs.append(self.prim_addr + elem_idx)
        return elem_addrs

    def get_elem_index(self, addr):
        if not self.is_elem_addr(addr):
            raise ValueError(
                f"Node ({self.uuid.hex()}) does not have 0x{addr:04X} element address."
            )
        return addr - self.prim_addr

    def add_appkey_index(self, appkey_index):
        if appkey_index in self._appkey_indexes:
            raise ValueError(
                f"Node ({self.uuid.hex()}) failed to add {appkey_index} "
                f"appkey_index because it already exists."
            )
        self._appkey_indexes.append(appkey_index)

    def remove_appkey_index(self, appkey_index):
        if appkey_index not in self._appkey_indexes:
            raise ValueError(
                f"Node ({self.uuid.hex()}) failed to remove {appkey_index} "
                f"appkey_index because it does not exists."
            )
        self._appkey_indexes.remove(appkey_index)

    def has_appkey_index(self, appkey_index):
        return appkey_index in self._appkey_indexes

    @property
    def appkey_indexes(self):
        return (appkey_index for appkey_index in self._appkey_indexes)

    def __eq__(self, other) -> bool:
        if isinstance(other, self.__class__):
            return self.uuid == other.uuid
        else:
            return False

    def __hash__(self) -> int:
        return hash(self.uuid)


class DCD(StateDictObject):
    def __init__(self, cid, pid, vid, crpl, relay, proxy, friend, lpn, elements=[]):
        super().__init__()
        self.cid = cid
        self.pid = pid
        self.vid = vid
        self.crpl = crpl
        self.relay = relay
        self.proxy = proxy
        self.friend = friend
        self.lpn = lpn
        self.elements: List[DCDElement] = []
        for elem in elements:
            if isinstance(elem, Mapping):
                self.elements.append(DCDElement.create_from_dict(elem))
            elif isinstance(elem, DCDElement):
                self.elements.append(elem)
            else:
                self.raise_construction_error("element", elem, type_error=True)


class DCDElement(StateDictObject):
    def __init__(self, idx, loc, models=[]):
        super().__init__()
        self.idx = idx
        self.loc = loc
        self.models: List[ModelID] = []
        for mdl in models:
            if isinstance(mdl, Mapping):
                self.models.append(ModelID.create_from_dict(mdl))
            elif isinstance(mdl, ModelID):
                self.models.append(mdl)
            else:
                self.raise_construction_error("model", mdl, type_error=True)


class ModelID(StateDictObject):
    SIG_MODEL_VENDOR_ID = 0xFFFF
    MODEL_ID_MIN = 0
    MODEL_ID_MAX = 0xFFFF
    VENDOR_ID_MIN = 0
    VENDOR_ID_MAX = 0xFFFF

    def __init__(self, model_id, vendor_id=0xFFFF):
        super().__init__()
        NamedModelID.validate_value(model_id)
        if isinstance(model_id, tuple):
            # The validate_value guarantees that the tuple length is two
            vendor_id, model_id = model_id
        elif isinstance(model_id, NamedModelID):
            vendor_id, model_id = model_id.value
        if (vendor_id < self.VENDOR_ID_MIN) or (self.VENDOR_ID_MAX < vendor_id):
            raise ValueError(
                f"Vendor ID {vendor_id} is not in "
                f"[{self.VENDOR_ID_MIN},0x{self.VENDOR_ID_MAX:04X}] range."
            )
        if (model_id < self.MODEL_ID_MIN) or (self.MODEL_ID_MAX < model_id):
            raise ValueError(
                f"Model ID {model_id} is not in "
                f"[{self.MODEL_ID_MIN},0x{self.MODEL_ID_MAX:04X}] range."
            )
        self._vendor_id = vendor_id
        self._model_id = model_id

    @property
    def vendor_id(self):
        return self._vendor_id

    @property
    def model_id(self):
        return self._model_id

    def pretty_name(self):
        if NamedModelID.value_exist(self.to_tuple()):
            return NamedModelID.get_pretty_name((self.vendor_id, self.model_id))
        else:
            return str(self)

    def to_tuple(self):
        return (self.vendor_id, self.model_id)

    def is_sig_model(self) -> bool:
        return self.vendor_id == ModelID.SIG_MODEL_VENDOR_ID

    def is_vendor_model(self) -> bool:
        return not self.is_sig_model()

    def __eq__(self, other):
        if isinstance(other, ModelID):
            return (self.vendor_id == other.vendor_id) and (
                self.model_id == other.model_id
            )
        return False

    def __hash__(self):
        return hash((self.vendor_id, self.model_id))

    def __str__(self):
        return f"0x{self.vendor_id:04X}:0x{self.model_id:04X}"


@dataclasses.dataclass
class BtmeshDbClearedEvent(LocalEvent):
    name: ClassVar[str] = "btmesh_levt_db_cleared"


@dataclasses.dataclass
class BtmeshDbNodeRemoved(LocalEvent):
    name: ClassVar[str] = "btmesh_levt_db_node_removed"
    node: Node
    node_memento: Dict[str, object] = dataclasses.field(
        default_factory=dict,
    )


class BtmeshDatabase(StateDictObject):
    def __init__(
        self,
        networks=[],
        nodes=[],
        prov_uuid=None,
        emit: Optional[Callable[[LocalEvent], None]] = None,
    ):
        self.networks = [nw for nw in networks]
        self.nodes: List[Node] = []
        self.prov_uuid = prov_uuid
        for node in nodes:
            if isinstance(node, Mapping):
                self.nodes.append(Node.create_from_dict(node))
            elif isinstance(node, Node):
                self.nodes.append(node)
            else:
                self.raise_construction_error("node", node, type_error=True)
        if emit is None:
            self.emit = self.null_emitter
        else:
            self.emit = emit

    def null_emitter(self, event: LocalEvent):
        pass

    @property
    def prov_uuid(self):
        return self._prov_uuid

    @prov_uuid.setter
    def prov_uuid(self, value):
        if value is None:
            self._prov_uuid = None
        else:
            if not util.is_uuid_valid(value):
                self.raise_construction_error("prov_uuid", value)
            self._prov_uuid = StateDictObject.to_bytes(value)

    def set_event_emitter(self, emit_func: Callable[[LocalEvent], None]):
        self.emit = emit_func

    def clear(self):
        self.networks.clear()
        self.nodes.clear()
        db_clr_event = BtmeshDbClearedEvent()
        self.emit(db_clr_event)

    def add_node(self, node: Node):
        if node.uuid not in (n.uuid for n in self.nodes):
            self.nodes.append(node)

    def node_uuid_exists(self, uuid):
        node = next((n for n in self.nodes if n.uuid == uuid), None)
        return node is not None

    def get_node_by_uuid(self, uuid):
        node = next((n for n in self.nodes if n.uuid == uuid), None)
        if not node:
            raise ValueError(f'Node uuid "{uuid.hex()}" does not exist.')
        return node

    def node_name_exist(self, name):
        node = next((n for n in self.nodes if n.name == name), None)
        return node is not None

    def get_node_by_name(self, name):
        node = next((n for n in self.nodes if n.name == name), None)
        if not node:
            raise ValueError(f'Node name "{name}" does not exist.')
        return node

    def node_prim_addr_exist(self, prim_addr):
        node = next((n for n in self.nodes if n.prim_addr == prim_addr), None)
        return node is not None

    def get_node_by_prim_addr(self, prim_addr):
        node = next((n for n in self.nodes if n.prim_addr == prim_addr), None)
        if not node:
            raise ValueError(f"Node primary address 0x{prim_addr:04X} does not exist.")
        return node

    def node_elem_addr_exist(self, elem_addr):
        node = next((n for n in self.nodes if n.is_elem_addr(elem_addr)), None)
        return node is not None

    def get_node_by_elem_addr(self, elem_addr):
        node = next((n for n in self.nodes if n.is_elem_addr(elem_addr)), None)
        if not node:
            raise ValueError(f"Node element address 0x{elem_addr:04X} does not exist.")
        return node

    def rename_node(self, node: Node, new_name: str):
        if self.node_name_exist(new_name):
            raise ValueError(
                f'Node rename failed because "{new_name}" name already exists.'
            )
        node.name = new_name

    def remove_node(
        self, node: Node, node_memento: Optional[Dict[str, object]] = None
    ) -> Node:
        self.nodes.remove(node)
        self.emit(BtmeshDbNodeRemoved(node, node_memento))
        return node

    def remove_node_by_name(
        self, name: str, node_memento: Optional[Dict[str, object]] = None
    ) -> Node:
        node = self.get_node_by_name(name)
        return self.remove_node(node, node_memento)

    def remove_node_by_uuid(
        self, uuid: bytes, node_memento: Optional[Dict[str, object]] = None
    ) -> Node:
        node = self.get_node_by_uuid(uuid)
        return self.remove_node(node, node_memento)

    def remove_node_by_prim_addr(
        self, prim_addr: int, node_memento: Optional[Dict[str, object]] = None
    ) -> Node:
        node = self.get_node_by_prim_addr(prim_addr)
        return self.remove_node(node, node_memento)

    def remove_node_by_elem_addr(
        self, elem_addr: int, node_memento: Optional[Dict[str, object]] = None
    ) -> Node:
        node = self.get_node_by_elem_addr(elem_addr)
        return self.remove_node(node, node_memento)

    def get_node_list(self, nodefilter=None, order_property=None, reverse=False):
        identity = lambda n: n
        nodefilter = nodefilter if nodefilter else identity
        node_gen = (n for n in self.nodes if nodefilter(n))
        if order_property:
            key = operator.attrgetter(order_property)
            return sorted(node_gen, key=key, reverse=reverse)
        else:
            return list(node_gen)

    def get_elem_addr_list(
        self,
        elem_addr_filter=None,
        nodefilter=None,
        node_order_property=None,
        reverse=False,
    ):
        elem_addrs = []
        identity = lambda n: n
        elem_addr_filter = elem_addr_filter if elem_addr_filter else identity
        nodes = self.get_node_list(
            nodefilter=nodefilter, order_property=node_order_property
        )
        for node in nodes:
            elem_addrs.extend(
                addr for addr in node.elem_addrs if elem_addr_filter(addr)
            )
        if reverse:
            elem_addrs.reverse()
        return elem_addrs


class FWID(StateDictObject):
    @classmethod
    def from_bytes(cls, raw_fwid):
        util.validate_raw_fwid(raw_fwid)
        cid = int.from_bytes(raw_fwid[0 : util.COMPANY_ID_SIZE], byteorder="little")
        version_info = raw_fwid[util.COMPANY_ID_SIZE :]
        return cls(cid, version_info)

    def __init__(self, company_id: int, version_info: Union[bytes, str]):
        self.company_id = company_id
        self.version_info = version_info

    @property
    def company_id(self) -> int:
        return self._company_id

    @company_id.setter
    def company_id(self, value: int) -> None:
        util.validate_company_id(value)
        self._company_id = value

    @property
    def version_info(self) -> bytes:
        return self._version_info

    @version_info.setter
    def version_info(self, value) -> None:
        self._version_info = StateDictObject.to_bytes(value)

    def to_bytes(self) -> bytes:
        return self.company_id.to_bytes(2, byteorder="little") + self.version_info

    def to_str(self, hex_format=True):
        cid_str = f"0x{self.company_id:04X}"
        if hex_format:
            version_info_str = self.version_info.hex()
        else:
            version_info_str = self.version_info.decode("utf-8")
        return f"{cid_str}:{version_info_str}"

    def __hash__(self) -> int:
        return hash(self.to_bytes())

    def __str__(self):
        return self.to_str()

    def __repr__(self):
        return f"FWID(0x{self.company_id:04X},{repr(self.version_info)})"
