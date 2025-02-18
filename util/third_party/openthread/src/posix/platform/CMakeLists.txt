#
#  Copyright (c) 2019, The OpenThread Authors.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of the copyright holder nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

set(OT_PLATFORM_LIB "openthread-posix" PARENT_SCOPE)

add_library(ot-posix-config INTERFACE)

option(OT_DAEMON "Enable daemon mode" OFF)
if(OT_DAEMON)
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_CONFIG_DAEMON_ENABLE=1"
    )

    # We have to add this definition to `ot-config` because openthread core
    # libraries will set config file to "openthrad-core-posix-config.h" which
    # depends on `OPENTHREAD_POSIX_CONFIG_DAEMON_ENABLE` to correctly enable
    # PLATFORM_NETIF and PLATFORM_UDP features. Otherwise, openthread core and
    # posix libraries will use different feature definitions.
    list(APPEND OT_PLATFORM_DEFINES "OPENTHREAD_POSIX_CONFIG_DAEMON_ENABLE=1")
    set(OT_PLATFORM_DEFINES ${OT_PLATFORM_DEFINES} PARENT_SCOPE)
endif()

option(OT_POSIX_INSTALL_EXTERNAL_ROUTES "Install External Routes as IPv6 routes" ON)
if(OT_POSIX_INSTALL_EXTERNAL_ROUTES)
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_CONFIG_INSTALL_EXTERNAL_ROUTES_ENABLE=1"
    )
else()
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_CONFIG_INSTALL_EXTERNAL_ROUTES_ENABLE=0"
    )
endif()

option(OT_POSIX_VIRTUAL_TIME "enable virtual time" OFF)
if(OT_POSIX_VIRTUAL_TIME)
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_VIRTUAL_TIME=1"
    )
endif()

option(OT_POSIX_MAX_POWER_TABLE  "enable max power table" OFF)
if(OT_POSIX_MAX_POWER_TABLE)
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_CONFIG_MAX_POWER_TABLE_ENABLE=1"
    )
endif()

option(OT_POSIX_SECURE_SETTINGS "enable secure settings" OFF)
if (OT_POSIX_SECURE_SETTINGS)
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_CONFIG_SECURE_SETTINGS_ENABLE=1"
    )
endif()

set(OT_POSIX_CONFIG_RCP_BUS "" CACHE STRING "RCP bus type")
if(OT_POSIX_CONFIG_RCP_BUS)
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_CONFIG_RCP_BUS=OT_POSIX_RCP_BUS_${OT_POSIX_CONFIG_RCP_BUS}"
    )
endif()

set(OT_POSIX_NAT64_CIDR "192.168.255.0/24" CACHE STRING "NAT64 CIDR for OpenThread NAT64")
if(OT_POSIX_NAT64_CIDR)
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_CONFIG_NAT64_CIDR=\"${OT_POSIX_NAT64_CIDR}\""
    )
endif()

if(NOT OT_CONFIG)
    set(OT_CONFIG "openthread-core-posix-config.h" PARENT_SCOPE)
endif()

set(OT_POSIX_CONFIG_RCP_VENDOR_INTERFACE "vendor_interface_example.cpp"
       CACHE STRING "vendor interface implementation")

set(CMAKE_EXE_LINKER_FLAGS "-rdynamic ${CMAKE_EXE_LINKER_FLAGS}" PARENT_SCOPE)

if(OT_ANDROID_NDK)
    target_compile_options(ot-posix-config INTERFACE -Wno-sign-compare)
endif()

add_library(openthread-posix
    alarm.cpp
    backbone.cpp
    backtrace.cpp
    daemon.cpp
    entropy.cpp
    firewall.cpp
    hdlc_interface.cpp
    infra_if.cpp
    logging.cpp
    mainloop.cpp
    memory.cpp
    misc.cpp
    multicast_routing.cpp
    netif.cpp
    radio.cpp
    radio_url.cpp
    settings.cpp
    spi_interface.cpp
    system.cpp
    trel.cpp
    udp.cpp
    utils.cpp
    virtual_time.cpp
    ${OT_POSIX_CONFIG_RCP_VENDOR_INTERFACE}
)

target_link_libraries(openthread-posix
    PUBLIC
        openthread-platform
    PRIVATE
        openthread-url
        ot-config-ftd
        ot-config
        ot-posix-config
        $<$<NOT:$<BOOL:${OT_ANDROID_NDK}>>:util>
        $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:rt>
)

option(OT_TARGET_OPENWRT "enable openthread posix for OpenWRT" OFF)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND NOT OT_TARGET_OPENWRT)
    target_compile_definitions(ot-posix-config
        INTERFACE "OPENTHREAD_POSIX_CONFIG_NAT64_AIL_PREFIX_ENABLE=1"
    )
    target_link_libraries(openthread-posix PRIVATE anl)
endif()

target_compile_definitions(openthread-posix
    PUBLIC
        ${OT_PUBLIC_DEFINES}
    PRIVATE
        OPENTHREAD_FTD=1
        OPENTHREAD_MTD=0
)

target_compile_options(openthread-posix PRIVATE
    ${OT_CFLAGS}
)

target_include_directories(openthread-posix PRIVATE
    ${OT_PUBLIC_INCLUDES}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/third_party/mbedtls/repo/include
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/posix/platform/include
)

if(OT_POSIX_CONFIG_RCP_BUS MATCHES "CPC")
    set(CPCD_SOURCE_DIR "" CACHE STRING "Location of cpc daemon project")

    if (NOT CPCD_SOURCE_DIR)
        CHECK_INCLUDE_FILE(sl_cpc.h CPC_HEADER_FOUND)
        if (NOT CPC_HEADER_FOUND)
            message(STATUS "CPC is not installed.")
            message(FATAL_ERROR "Could not locate CPC daemon sources.\n"
                                "Please define the CMake variable 'CPCD_SOURCE_DIR'.\n"
                                "'CPCD_SOURCE_DIR' is an absolute path to the CPC Daemon sources")
        else()
            message(STATUS "Using installed CPC header")
        endif()

    else()
        message(STATUS  "Including specified CPC daemon source...\n"
                            "    CPCD_SOURCE_DIR = ${CPCD_SOURCE_DIR}")
        if (NOT IS_ABSOLUTE ${CPCD_SOURCE_DIR})
            message(FATAL_ERROR "'CPCD_SOURCE_DIR' must be an absolute path.\nCPCD_SOURCE_DIR=\"${CPCD_SOURCE_DIR}\"")
        endif()

        if (NOT TARGET cpc)
            # Add cpcd to the build tree
            message(STATUS  "Building cpc library from source...")
            add_subdirectory(${CPCD_SOURCE_DIR} ${CPCD_SOURCE_DIR}/build)
        else()
            message(STATUS "CPC targets already defined - not adding subdirectory ${CPCD_SOURCE_DIR}")
        endif()
        target_include_directories(openthread-posix PRIVATE
            ${CPCD_SOURCE_DIR}/lib
        )
    endif()

    target_sources(openthread-posix PRIVATE cpc_interface.cpp)
    target_link_libraries(openthread-posix PRIVATE cpc)

endif()
