/***********************************************************************
 * @file ble.h
 * @version 0.0.1
 * @brief Function header file for setting up bLuetooth events & event handler
 *
 * @author Sriya Garde, sriya.garde@colorado.edu
 * @date Feb 25, 2024
 *
 * @institution University of Colorado Boulder (UCB)
 * @course ECEN 5823-001: IoT Embedded Firmware (Fall 2020)
 * @instructor David Sluiter
 *
 * @assignment ecen5823-assignment5-sriyagarde
 * @due Feb 25, 2024
 *
 * @resources IoTEF Lecture Slides 09,10,11

 * an0026-efm32-letimer Application note
 * an0012-efm32-gpio Application note
 * an0004.1-efm32-cmu Application note
 * an0007.1-efr32-efm32-series-1-energymodes Application note
 * EFR32xG1 Wireless Gecko Reference Manual
 * ug343-multinode-energy-profiler
 *
 * @copyright All rights reserved. Distribution allowed only for the
 * use of assignment grading. Use of code excerpts allowed at the
 * discretion of author. Contact for permission.
 */

#ifndef SRC_BLE_H_
#define SRC_BLE_H_
#include "stdbool.h"
#include "stdint.h"
#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "src/i2c.h"
#include "app.h"
#include "sl_bgapi.h"
#include "em_letimer.h"
#include "em_gpio.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "ble_device_type.h"
#include "ble.h"
#include "sl_bt_api.h"

// This is the number of entries in the queue. Please leave
// this value set to 16.
#define QUEUE_DEPTH      (16)
// Student edit:
//   define this to 1 if your design uses all array entries
//   define this to 0 if your design leaves 1 array entry empty
#define USE_ALL_ENTRIES  (0)

#define MAX_BUFFER_LENGTH  (5)
#define MIN_BUFFER_LENGTH  (1)

typedef struct {

  uint16_t       charHandle;                 // GATT DB handle from gatt_db.h
  uint32_t       bufLength;                  // Number of bytes written to field buffer[5]
  uint8_t        buffer[MAX_BUFFER_LENGTH];  // The actual data buffer for the indication,
                                             //   need 5-bytes for HTM and 1-byte for button_state.
                                             //   For testing, test lengths 1 through 5,
                                             //   a length of 0 shall be considered an
                                             //   error, as well as lengths > 5

} queue_struct_t;


// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition
typedef struct {
// values that are common to servers and clients
bd_addr myAddress;
uint8_t myAddressType;

// values unique for server
// The advertising set handle allocated from Bluetooth stack.
uint8_t advertisingSetHandle;
uint8_t connection_handle;           // true when in an open connection
bool connected;
//bool indication;
bool light_indication;
bool smoke_indication;
bool indication_in_flight;       // true when an indication is in-flight

bool gatt_procedure_ongoing;

uint32_t queue_entries;
//uint32_t temp_service_handle;
uint32_t light_service_handle;
uint32_t smoke_service_handle;

//uint16_t chars_handle;
uint16_t light_chars_handle;
uint16_t smoke_chars_handle;

//uint8_t * chars_value;
uint8_t * light_chars_value;
uint8_t * smoke_chars_value;

// values unique for client

} ble_data_struct_t;

static uint32_t nextPtr(uint32_t ptr);
bool     write_queue      (uint16_t  charHandle, uint32_t  bufLength, uint8_t *buffer);
bool     read_queue       (uint16_t *charHandle, uint32_t *bufLength, uint8_t *buffer);
uint32_t get_queue_depth  (void);

// function prototypes

ble_data_struct_t*  getBleDataPtr(void);

void ble_ambient();

void ble_smoke();

void handle_ble_event(sl_bt_msg_t *evt);

#endif /* SRC_BLE_H_ */
