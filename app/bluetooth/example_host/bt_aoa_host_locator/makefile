################################################################################
# General project settings                                                     #
################################################################################

PROJECTNAME = bt_aoa_host_locator
SDK_DIR = ../../../..

# The MQTT component depends on Mosquitto that has to be installed to the
# development environment. By enabling this experimental option, this dependency
# can be eliminated. The mock component has a very limited functionality
# compared to the original one and serves testing purposes only.
MQTT_MOCK ?= 0

# The MQTT_COMPONENT variable is derived automatically based on the MQTT_MOCK.
# Do not edit manually.
MQTT_COMPONENT = 1
ifeq (, $(filter $(MAKECMDGOALS), export))
  ifneq ($(MQTT_MOCK), 0)
    MQTT_COMPONENT = 0
  endif
else
  MQTT_MOCK = 1
endif


################################################################################
# Components                                                                   #
#                                                                              #
# After setting up the toolchain, components can contribute to the project by  #
# appending items to the project variables like INCLUDEPATHS or C_SRC.         #
################################################################################

include $(SDK_DIR)/app/bluetooth/component_host/toolchain.mk
include $(SDK_DIR)/app/bluetooth/component_host/app_log.mk
include $(SDK_DIR)/app/bluetooth/component_host/app_assert.mk
include $(SDK_DIR)/app/bluetooth/component_host/app_signal.mk
include $(SDK_DIR)/app/bluetooth/component_host/cjson.mk
include $(SDK_DIR)/app/bluetooth/component_host/ncp_host_nothread.mk
include $(SDK_DIR)/app/bluetooth/component_host/aoa_cte.mk
include $(SDK_DIR)/app/bluetooth/component_host/aoa_angle.mk
include $(SDK_DIR)/app/bluetooth/component_host/rtl_lib.mk
include $(SDK_DIR)/app/bluetooth/component_host/antenna_array.mk
include ${SDK_DIR}/app/bluetooth/component_host/simple_timer.mk
include ${SDK_DIR}/app/bluetooth/component_host/ncp_reset.mk
include ${SDK_DIR}/app/bluetooth/component_host/ncp_version.mk
ifneq ($(MQTT_COMPONENT), 0)
  include $(SDK_DIR)/app/bluetooth/component_host/mqtt.mk
endif
ifneq ($(MQTT_MOCK), 0)
  include $(SDK_DIR)/app/bluetooth/component_host/mqtt_mock.mk
endif


################################################################################
# Include paths                                                                #
################################################################################

override INCLUDEPATHS += . \
$(SDK_DIR)/app/bluetooth/common_host/aoa_parse \
$(SDK_DIR)/app/bluetooth/common_host/aoa_util \
$(SDK_DIR)/app/bluetooth/common_host/aoa_db \
$(SDK_DIR)/app/bluetooth/common_host/system \
$(SDK_DIR)/platform/common/inc \
$(SDK_DIR)/protocol/bluetooth/inc


################################################################################
# Input files                                                                  #
################################################################################

override C_SRC += \
$(SDK_DIR)/app/bluetooth/common_host/aoa_parse/aoa_parse.c \
$(SDK_DIR)/app/bluetooth/common_host/aoa_parse/aoa_parse_cte.c \
$(SDK_DIR)/app/bluetooth/common_host/aoa_parse/aoa_parse_enum.c \
$(SDK_DIR)/app/bluetooth/common_host/aoa_util/aoa_serdes.c \
$(SDK_DIR)/app/bluetooth/common_host/aoa_util/aoa_util.c \
$(SDK_DIR)/app/bluetooth/common_host/aoa_db/aoa_db.c \
$(SDK_DIR)/app/bluetooth/common_host/system/system.c \
app.c \
main.c

ifneq ($(MQTT_COMPONENT), 0)
ifeq ($(OS), win)
# List of .dll files required by mosquitto.
override LIBS += \
$(EXE_DIR)/mosquitto.dll \
$(EXE_DIR)/libcrypto-1_1-x64.dll \
$(EXE_DIR)/libssl-1_1-x64.dll
endif
endif

PROJ_FILES += $(wildcard config/*)


################################################################################
# Target rules                                                                 #
################################################################################

include $(SDK_DIR)/app/bluetooth/component_host/targets.mk

# Copy .dll files next to the executable to avoid missing DLL errors.
$(EXE_DIR)/%.dll:
	$(shell cp "${MOSQUITTO_DIR}/$*.dll" $(EXE_DIR))
