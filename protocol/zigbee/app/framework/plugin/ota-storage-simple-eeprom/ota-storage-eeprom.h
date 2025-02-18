/***************************************************************************//**
 * @file
 * @brief APIs and defines for the OTA Storage Simple EEPROM plugin, which
 *        manages storing an OTA file in the EEPROM.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifdef UC_BUILD
#include "ota-storage-simple-eeprom-config.h"
#include "sl_component_catalog.h"
#if (EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_SOC_BOOTLOADING_SUPPORT == 1)
#define SOC_BOOTLOADING_SUPPORT
#endif
#if (EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT == 1)
#define READ_MODIFY_WRITE_SUPPORT
#endif
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  #include "test-harness-config.h"
#endif //SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
#else // !UC_BUILD
#ifdef EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_SOC_BOOTLOADING_SUPPORT
#define SOC_BOOTLOADING_SUPPORT
#endif
#if (EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT == TRUE)
#define READ_MODIFY_WRITE_SUPPORT
#endif
#endif // UC_BUILD

#if !defined(OTA_STORAGE_EEPROM_INTERNAL_HEADER)
#error "Do not include this header with files outside of app/framework/plugin/ota-storage-simple-eeprom/"
#endif

// Layout of the EEPROM data for OTA is dependent upon whether SOC Bootloading support is
// enabled.  When it is NOT enabled, the layout of data is pretty
// straightforward:

// LAYOUT 1
//
// EEPROM Start                     ---------------
// Image Info Meta-data             |    Info     |
//                                  ---------------
// OTA Header (offset 0 for the     |    OTA      |
//             OTA storage module)  |   Header    |
//                                  ---------------
// Rest of OTA image                |             |
//  (upgrade image data)            |     EBL     |
//                                  |      +      |
//                                  |  Signature  |
// End of OTA image                 ---------------
//                                  |             |
// ... (unused) ...                 |             |
//                                  |             |
// EEPROM End                       ---------------

// For SOC Bootloading, we must put the EBL at the top of the EEPROM
// since existing bootloaders have no knowledge of OTA headers.

// LAYOUT 2
//
// EEPROM Start                     ---------------
// Beginning of EBL file            |             |
//  (upgrade image data)            |    EBL      |
//                                  |             |
// End of EBL data                  |             |
//                                  ---------------
// Other OTA image data             |             |
//  (e.g. signature)                |  Signature  |
//                                  |             |
// End of OTA image                 ---------------
//                                  |             |
// ... (unused) ...                 |             |
//                                  |             |
//                                  ---------------
// Image Info Meta-data             |    Info     |
//                                  |             |
//                                  |             |
//                                  |             |
//                                  ---------------
// OTA Header (offset 0 for the     |    OTA      |
//             OTA storage module)  |   Header    |
//  ...extra space...               |             |
// EEPROM End                       ---------------

// The Image Info Meta-data is DIFFERENT than previous versions of this plugin.
// The meta-data NOW contains the following data:
//   Ember Magic Number (8 bytes)
//     [different than the OTA file magic number]
//   Version Tag (2 bytes)
//   EBL Start offset within OTA file (4 bytes)
//   Saved Download Offset data (100 bytes)
//     This comes in 2 flavors:
//     1. For read-modify-write flash drivers this will be a simple 4-byte
//        counter indicating the last downloaded offset.  It will only be updated
//        based on the plugin option...
//          EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_DOWNLOAD_OFFSET_SAVE_RATE
//        Rest of the space is unused.
//     2. For flash drivers without read-modify-write support, we will use
//        a byte-mask indicating the last full flash page of written data.
//        The byte-mask will have negative logic (0xFF means flash page not
//        downloaded) and requires one byte per page of the EEPROM space
//        allocated for the OTA code.
//        In other words if the client is given 200k of download space within
//        the EEPROM, and a flash page is 4k, then we need 50 bytes for the
//        byte-mask.

/**
 * @defgroup ota-storage-simple-eeprom OTA Storage Simple EEPROM
 * @ingroup component
 * @brief API and Callbacks for the OTA Storage Simple EEPROM Component
 *
 * This is a driver for the Over-the-Air simple storage module component.
 * It uses an EEPROM as the underlying storage device. It provides a means
 * to record data being read or written, as well as metadata with
 * information about how far along a client download is.
 * It can be used either by an OTA Client or an OTA Server.
 * Note that this component assumes that the flash storage does not have
 * read-modify-write support. Users should ensure this value matches the flash
 * storage device used by the application. A mismatch between the
 * project-configured value and the actual flash storage support value
 * will result in an application that asserts upon startup.
 *
 */

/**
 * @addtogroup ota-storage-simple-eeprom
 * @{
 */

#if defined(EMBER_TEST)
// So that we don't need separately configured applications, we play games
// with the #defines generated by App Builder.  We still need to compile
// different applications, but that can be more easily done by passing
// in -D parameters globally.

  #if (EMBER_TEST_OTA_EEPROM_PAGE_ERASE == 1)
    #undef READ_MODIFY_WRITE_SUPPORT
  #endif //EMBER_TEST_OTA_EEPROM_PAGE_ERASE

  #if (EMBER_TEST_OTA_EEPROM_SOC_BOOTLOAD == 1)
    #define SOC_BOOTLOADING_SUPPORT
  #endif //EMBER_TEST_OTA_EEPROM_SOC_BOOTLOAD

  #if defined EMBER_TEST_OTA_EEPROM_NONZERO_START_OFFSET
    #undef EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_STORAGE_START
// This is just one page size offset
    #define EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_STORAGE_START   2048
  #endif // EMBER_TEST_OTA_EEPROM_NONZERO_OFFSETS
#endif

#if defined(SOC_BOOTLOADING_SUPPORT)
  #define SOC_BOOTLOADING_SUPPORT_ENABLED true
  #define SOC_BOOTLOADING_SUPPORT_TEXT "yes"
#else
  #define SOC_BOOTLOADING_SUPPORT_ENABLED false
  #define SOC_BOOTLOADING_SUPPORT_TEXT "no"
#endif

#ifdef READ_MODIFY_WRITE_SUPPORT
  #define READ_MODIFY_WRITE_SUPPORT_TEXT "yes"
#else
  #define READ_MODIFY_WRITE_SUPPORT_TEXT "no"
#endif

// I like shorter names, so redefine the App. Builder names.
#define EEPROM_START EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_STORAGE_START
#define EEPROM_END   EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_STORAGE_END
#define SAVE_RATE    EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_DOWNLOAD_OFFSET_SAVE_RATE

#define DEFAULT_SLOT      0
#define USE_FIRST_SLOT    0
#define USE_LAST_SLOT     1
#define USE_SPECIFIC_SLOT 2
#define DO_NOT_USE_SLOTS  3
#define SLOT_STRATEGY EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_GECKO_BOOTLOADER_STORAGE_SUPPORT

#define MAX_IMAGE_INFO_AND_OTA_HEADER_SIZE  2048 // bytes

#define MAX_WORD_SIZE 4

// The following indexes are all relative to the start of the Image Info section

#define MAGIC_NUMBER_INDEX   0
#define MAGIC_NUMBER_SIZE    8
#define VERSION_NUMBER_INDEX (MAGIC_NUMBER_INDEX + MAGIC_NUMBER_SIZE)
#define VERSION_NUMBER_SIZE  4

// Magic number to indicate that the stored download meta-data is valid.
// Magic Number = 1-951-0200 (Ember's main phone number in hex)
// Version = 0x00 0x01 (big endian).
#define MAGIC_NUMBER   0x01, 0x09, 0x05, 0x01, 0x00, 0x02, 0x00, 0x00

// Version number information
//   0x00000001 - Addition of the Page-erase required support
//   0x00000002 - Support for 2-byte word sizes (previously-previously 1-byte was assumed)
//   0X00000003 - Support for 4-byte word sizes (previously 2-byte was assumed)
//     We changed the size of the bytemask to 512 to support Local Storage.
//     which means bumping the version number.
#define VERSION_NUMBER 0x00, 0x00, 0x00, 0x03

// The Offset within the OTA file (relative to offset 0) where the
// EBL data starts.  We assume EBL data starts right after the 1st
// tag meta-data.  However OTA headers are variable in size so we
// must keep track of how big the header is.
// This #define is actually the LOCATION where that offset is stored,
// not the actual offset.
#define EBL_START_OFFSET_INDEX      (VERSION_NUMBER_INDEX + VERSION_NUMBER_SIZE)
#define EBL_START_OFFSET_SIZE       4

// The last recorded offset we downloaded.  This may not be the same as the
// value stored in RAM by the OTA cluster itself.  This value will be
// the absolute offset of the file, regardless of the re-mapping
// this code does.  It is an offset understood by the OTA storage interfaces
// relative to the start of the OTA file.
#define SAVED_DOWNLOAD_OFFSET_INDEX (EBL_START_OFFSET_INDEX + EBL_START_OFFSET_SIZE)

// If we are using a Bytemask (Page erase required mode), then this is the maximum size.
// The bytemask limits the maximum size of the download image because it determines
// how many pages we can record as "fully downloaded".  This allows the code to
// recover from a reboot in the middle of the download.  A further complication is
// that this must be related to the word size of the EEPROM and the page size.
// In a perfect world we would size this based on the part but because the EEPROM
// info is likely contained within the bootloader we can't determine this except
// at runtime.  This is sized for the worst case scenario as follows:
//   512k image size (3588) / 2048 page size (local storage bootloader)
//     = 256 bytes * 4-byte word size = 1024
// Most other parts use a 2048 or 4096 page size and are not 2-byte word sizes,
// so we are don't need this many bytes.
#define MAX_BYTEMASK_LENGTH 1024

#define OTA_HEADER_INDEX (SAVED_DOWNLOAD_OFFSET_INDEX + MAX_BYTEMASK_LENGTH)

// The minimum offset we will write that determines if we store the current
// download offset persistently.  This is equal to the minimum OTA header size.
#define MINIMUM_FILE_SIZE_TO_STORE_OFFSET (OTA_MINIMUM_HEADER_LENGTH)

/**
 * @name API
 * @{
 */

uint32_t otaStorageEepromGetStorageStartAddress();
uint32_t otaStorageEepromGetStorageEndAddress();
uint32_t otaStorageEepromGetImageInfoStartAddress();

/** @} */ // end of name API

void emAfOtaStorageWriteInt32uToEeprom(uint32_t value, uint32_t realOffset);
uint32_t emAfOtaStorageReadInt32uFromEeprom(uint32_t realOffset);
void emAfStorageEepromUpdateDownloadOffset(uint32_t offset, bool finalOffset);

bool emAfOtaStorageDriverGetRealOffset(uint32_t* offset,
                                       uint32_t* length);
/**
 * @name Callbacks
 * @{
 */

/** @brief Erase complete.
 *
 * Called when an EEPROM erase operation has completed.
 *
 * @param success The result of the erase operation. Ver.: always
 */
void emberAfPluginOtaStorageSimpleEepromEraseCompleteCallback(bool success);
/** @} */ // end of name Callbacks
/** @} */ // end of ota-storage-simple-eeprom

#ifdef UC_BUILD
extern sl_zigbee_event_t emberAfPluginOtaStorageSimpleEepromPageEraseEvent;
extern void emberAfPluginOtaStorageSimpleEepromPageEraseEventHandler(SLXU_UC_EVENT);
#define eraseEvent (&emberAfPluginOtaStorageSimpleEepromPageEraseEvent)
#else
extern EmberEventControl emberAfPluginOtaStorageSimpleEepromPageEraseEventControl;
#define eraseEvent emberAfPluginOtaStorageSimpleEepromPageEraseEventControl
#endif

bool emAfOtaStorageCheckDownloadMetaData(void);
void emAfOtaStorageWriteDownloadMetaData(void);
void emAfOtaStorageEepromInit(void);
uint8_t emAfOtaStorageDriverGetWordSize(void);

#if defined(SOC_BOOTLOADING_SUPPORT)
uint32_t emAfGetEblStartOffset(void);
#endif

// Very very verbose debug printing.
//#define DEBUG_PRINT
#if defined(DEBUG_PRINT)
  #define debugPrint(...) otaPrintln(__VA_ARGS__)
  #define debugFlush()    emberAfCoreFlush()
#else
  #define debugPrint(...)
  #define debugFlush()
#endif
