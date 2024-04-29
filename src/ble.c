/***********************************************************************
 * @file ble.c
 * @version 0.0.1
 * @brief Function source file for setting up bLuetooth events & event handler
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

#include "ble.h"
#include "app.h"
#include "timers.h"
#include "irq.h"
#include "lcd.h"
#include "gatt_db.h"
#include "sl_bt_api.h"
#include "stdbool.h"
#include "adc.h"
// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

// BLE private data
// file = ble.c
#include "ble_device_type.h"
#include "ble.h"


#if DEVICE_IS_BLE_SERVER == 0

//d84d8df7-9b1c-4dfc-b25d-61f9e8b2a49e - smoke service
static const uint8_t smokeService[16] = {0x9e, 0xa4, 0xb2, 0xe8, 0xf9, 0x61, 0x5d, 0xb2, 0xfc, 0x4d, 0x1c, 0x9b, 0xf7, 0x8d, 0x4d, 0xd8};

//e9958983-800c-4b3f-9d3f-e1c352d025ec - smoke char
static const uint8_t smoke_char[16] = {0xec, 0x25, 0xd0, 0x52, 0xc3, 0xe1, 0x3f, 0x9d, 0x3f, 0x4b, 0x0c, 0x80, 0x83, 0x89, 0x95, 0xe9};

//1d9f9050-5651-46a2-b5f4-6c9545cc1ab1 - light service
static const uint8_t lightService[16] = {0xb1, 0x1a, 0xcc, 0x45, 0x95, 0x6c, 0xf4, 0xb5, 0xa2, 0x46, 0x51, 0x56, 0x50, 0x90, 0x9f, 0x1d};

//33aef90b-a996-4ede-9b73-99a7b209e9fb - light char
static const uint8_t light_char[16] = {0xfb, 0xe9, 0x09, 0xb2, 0xa7, 0x99, 0x73, 0x9b, 0xde, 0x4e, 0x96, 0xa9, 0x0b, 0xf9, 0xae, 0x33};


#endif

// BLE private data
ble_data_struct_t ble_data;

sl_status_t sc = 0; // status code

//array for hard-coded server address for the gecko board we are using as server
uint8_t server_addr[6] = SERVER_BT_ADDRESS;

uint8_t handle;
uint32_t interval_adv = 400; //250msec Value = 250*1.6 = 400
uint32_t interval_conn = 60; //75 msec Value =  75/1.25 = 60
uint16_t slave_latency = 4; //4 slave latency - slave can be off air upto 300msec.
uint16_t spvsn_timeout = 76;   //(4+1)(75msec * 2) = 750. Value = 750/10 = 75 supervision timeout
uint16_t min_ce_length = 0;
uint16_t max_ce_length = 4;
uint16_t scan_interval = 80; // 80*0.625 = 50msec
uint16_t scan_window = 40; // 40*0.625 = 25msec

//#if DEVICE_IS_BLE_SERVER == 0
////service UUID value 00000001-38c8-433e-87ec-652a2d136289
//
////1d9f9050-5651-46a2-b5f4-6c9545cc1ab1
//static const uint8_t lightService[16] = {0xb1,0x1a,0xcc,0x45,0x95,0x6c,0xf4,0xb5,0xa2,0x46,0x51,0x56,0x50,0x90,0x9f,0x1d};
//
////33aef90b-a996-4ede-9b73-99a7b209e9fb
//
//static const uint8_t light_char[16] = {0xfb, 0xe9, 0x09, 0xb2, 0xa7, 0x99, 0x73, 0x9b, 0xde, 0x4e, 0x96, 0xa9, 0x0b, 0xf9, 0xae, 0x33};
//#endif

/*
 *this is the declaration function that returns a pointer to the BLE private data
 * @param null
 * @return null
 */
ble_data_struct_t* getBleDataPtr ()
{
  return (&ble_data);
} // getBleDataPtr()


#if DEVICE_IS_BLE_SERVER == 1
// Declare memory for the queue/buffer, and our write and read pointers.
queue_struct_t my_queue[QUEUE_DEPTH]; // the queue
uint32_t wptr = 0;              // write pointer
uint32_t rptr = 0;              // read pointer
bool queue_status = 0;

// ---------------------------------------------------------------------
// Private function used only by this .c file.
// Compute the next ptr value. Given a valid ptr value, compute the next valid
// value of the ptr and return it.
// Isolation of functionality: This defines "how" a pointer advances.
// ---------------------------------------------------------------------
static uint32_t
nextPtr (uint32_t ptr)
{

  if ((ptr + 1) == QUEUE_DEPTH)
    {
      return 0; // Queue is full with approach 0
    }
  else
    return (ptr + 1);

} // nextPtr()

// -----------------------------------------------------------------------------------------------------------------------------
// Public function.
// This function writes an entry to the queue if the the queue is not full.
// Input parameter "charHandle" should be written to queue_struct_t element "charHandle".
// Input parameter "bufLength" should be written to queue_struct_t element "bufLength"
// The bytes pointed at by input parameter "buffer" should be written to queue_struct_t element "buffer"
// Returns bool false if successful or true if writing to a full fifo.
// i.e. false means no error, true means an error occurred.
// ------------------------------------------------------------------------------------------------------------------------------
bool
write_queue (uint16_t charHandle, uint32_t bufLength, uint8_t *buffer)
{

  if ((MIN_BUFFER_LENGTH < bufLength) && (bufLength < MAX_BUFFER_LENGTH))
    {
      if (nextPtr (wptr) != rptr)
        {
          my_queue[wptr].charHandle = charHandle;
          my_queue[wptr].bufLength = bufLength;
          memcpy (my_queue[wptr].buffer, buffer, bufLength);
          wptr = nextPtr (wptr);
          return false;
        }

      else
        return true;
    }

  else
    return true;

} // write_queue()

// ---------------------------------------------------------------------
// Public function.
// This function reads an entry from the queue, and returns values to the
// caller. The values from the queue entry are returned by writing
// the values to variables declared by the caller, where the caller is passing
// in pointers to charHandle, bufLength and buffer. The caller's code will look like this:
//
//   uint16_t     charHandle;
//   uint32_t     bufLength;
//   uint8_t      buffer[5];
//
//   status = read_queue (&charHandle, &bufLength, &buffer[0]);
//
// Returns bool false if successful or true if reading from an empty fifo.
// i.e. false means no error, true means an error occurred.
// ---------------------------------------------------------------------
bool
read_queue (uint16_t *charHandle, uint32_t *bufLength, uint8_t *buffer)
{

  if (rptr == wptr)
    {
      return true;
    }

  else
    {

      *charHandle = my_queue[rptr].charHandle;
      *bufLength = my_queue[rptr].bufLength;
      memcpy (buffer, my_queue[rptr].buffer, *bufLength);
      rptr = nextPtr (rptr);
      return false;

    }
} // read_queue()

// ---------------------------------------------------------------------
// Public function.
// Function that computes the number of written entries currently in the queue. If there
// are 3 entries in the queue, it should return 3. If the queue is empty it should
// return 0. If the queue is full it should return either QUEUE_DEPTH if
// USE_ALL_ENTRIES==1 otherwise returns QUEUE_DEPTH-1.
// ---------------------------------------------------------------------
uint32_t
get_queue_depth ()
{
  if (rptr < wptr)
    {
      return (wptr - rptr);
    }
  else if (rptr == wptr)
    {
      return 0;
    }
  else
    {
      return (QUEUE_DEPTH - rptr + wptr);
    }

} // get_queue_depth()


/*
 *Function that reads and sends the temperature measurement to client (EFR Connect app) when Indications are On

 * @param null
 * @return null
 */
void ble_ambient(){

  ble_data_struct_t *bleData = getBleDataPtr ();
  uint8_t light_status[1];

  //light_status[0] = lightState;
  //button_state[1] = 0;

  if (bleData->connected == true)
    {
      uint8_t lightState = convert_light();
      light_status[0] = lightState;
      light_status[1] = 0;
      sl_status_t sc = sl_bt_gatt_server_write_attribute_value (
          gattdb_light_status, 0, sizeof(light_status[0]), &light_status[0]);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r",
              (unsigned int)sc);
        }

      if (bleData->light_indication == true) //connected & in flight
        {


              sc = sl_bt_gatt_server_send_indication (
                  bleData->connection_handle,
                  gattdb_light_status,
                  1, &light_status[0]);
              if (sc != SL_STATUS_OK)
                {
                  LOG_ERROR(
                      "sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r",
                      (unsigned int)sc);
                }
              else
                {
                  bleData->indication_in_flight = true;
                  if(lightState == 1)
                    {
                  displayPrintf (DISPLAY_ROW_TEMPVALUE, "Buzzer is On\r\n");
                    }
                  else
                    {
                      displayPrintf (DISPLAY_ROW_TEMPVALUE, "Buzzer is Off\r\n");
                    }
                }
            }

        }


} //void ble_button ()

void ble_smoke()
{

  ble_data_struct_t *bleData = getBleDataPtr ();
  uint8_t smoke_status[1];

  //light_status[0] = lightState;
  //button_state[1] = 0;

  if (bleData->connected == true)
    {
      uint8_t smokeState = convert_smoke();
      smoke_status[0] = smokeState;
      smoke_status[1] = 0;
      sl_status_t sc = sl_bt_gatt_server_write_attribute_value (
          gattdb_light_status, 0, sizeof(smoke_status[0]), &smoke_status[0]);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r",
              (unsigned int)sc);
        }

      if (bleData->light_indication == true) //connected & in flight
        {


              sc = sl_bt_gatt_server_send_indication (
                  bleData->connection_handle,
                  gattdb_light_status,
                  1, &smoke_status[0]);
              if (sc != SL_STATUS_OK)
                {
                  LOG_ERROR(
                      "sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r",
                      (unsigned int)sc);
                }
              else
                {
                  bleData->indication_in_flight = true;
                  if(smokeState == 1)
                    {
                  displayPrintf (DISPLAY_ROW_ACTION, "Light is On\r\n");
                    }
                  else
                    {
                      displayPrintf (DISPLAY_ROW_ACTION, "Light is Off\r\n");
                    }
                }
            }

        }

}
#endif
/*
 *Function to implement event handler that handles various events occuring while transmitting temperature to the Client

 * @param bluetooth event occuring
 * @return null
 */
void handle_ble_event (sl_bt_msg_t *evt)
{

  ble_data_struct_t *bleData = getBleDataPtr ();
  switch (SL_BT_MSG_ID(evt->header))
    {

    //****************************case 1 - sl_bt_evt_system_boot_id********************************************************
    //    PACKSTRUCT( struct sl_bt_evt_system_boot_s
    //    {
    //      uint16_t major;      /**< Major release version */
    //      uint16_t minor;      /**< Minor release version */
    //      uint16_t patch;      /**< Patch release number */
    //      uint16_t build;      /**< Build number */
    //      uint32_t bootloader; /**< Bootloader version */
    //      uint16_t hw;         /**< Hardware type */
    //      uint32_t hash;       /**< Version hash */
    //    });
    //*********************************************************************************************************************
    case sl_bt_evt_system_boot_id:

      displayInit();
      sc = sl_bt_system_get_identity_address (&(bleData->myAddress),
                                              &(bleData->myAddressType));
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_system_get_identity_address() returned != 0 status=0x%04x\n\r",
              (unsigned int)sc);
        }

#if DEVICE_IS_BLE_SERVER

      sc = sl_bt_advertiser_create_set (&(bleData->advertisingSetHandle));
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_advertiser_create_set() returned != 0 status=0x%04x\n\r",
              (unsigned int)sc);
        }

      //      sl_status_t sl_bt_advertiser_set_timing(uint8_t handle,
      //                                              uint32_t interval_min,
      //                                               uint32_t interval_max,
      //                                              uint16_t duration,
      //                                              uint8_t maxevents);
      sc = sl_bt_advertiser_set_timing (bleData->advertisingSetHandle,
                                        interval_adv, interval_adv, 0, 0);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_advertiser_set_timing() returned != 0 status=0x%04x\n\r",
              (unsigned int)sc);
        }

      sc = sl_bt_advertiser_start (bleData->advertisingSetHandle,
                                   sl_bt_advertiser_general_discoverable,
                                   sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r",
                    (unsigned int)sc);
        }


      //Device in "Advertising" state
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
      displayPrintf(DISPLAY_ROW_NAME, "Server");
#else
      sc = sl_bt_scanner_set_mode(sl_bt_gap_phy_1m, 0);
       if (sc != SL_STATUS_OK)
         {
           LOG_ERROR("sl_bt_scanner_set_mode() returned != 0 status=0x%04x\n\r",
                     (unsigned int)sc);
         }

       sc = sl_bt_scanner_set_timing(sl_bt_gap_1m_phy, scan_interval, scan_window);
       if (sc != SL_STATUS_OK)
         {
           LOG_ERROR("sl_bt_scanner_set_timing() returned != 0 status=0x%04x\n\r",
                     (unsigned int)sc);
         }

       sc = sl_bt_connection_set_default_parameters(interval_conn, interval_conn, slave_latency, spvsn_timeout, min_ce_length, max_ce_length);
       if (sc != SL_STATUS_OK)
         {
           LOG_ERROR("sl_bt_connection_set_default_parameters() returned != 0 status=0x%04x\n\r",
                     (unsigned int)sc);
         }

       sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
       if (sc != SL_STATUS_OK)
         {
           LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x\n\r",
                     (unsigned int)sc);
         }

       displayPrintf(DISPLAY_ROW_NAME, "Client");
       displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");


#endif

      displayPrintf(DISPLAY_ROW_BTADDR, "%02X:%02X:%02X:%02X:%02X:%02X",
                    bleData->myAddress.addr[0],
                    bleData->myAddress.addr[1],
                    bleData->myAddress.addr[2],
                    bleData->myAddress.addr[3],
                    bleData->myAddress.addr[4],
                    bleData->myAddress.addr[5]);
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "EcoSensor");

      bleData->connected = false;
      bleData->light_indication = false;
      bleData->smoke_indication = false;
      bleData->indication_in_flight = false;
      bleData->gatt_procedure_ongoing = false;

      break;

      //****************************case 2 - sl_bt_evt_connection_opened_id********************************************************
      //      PACKSTRUCT( struct sl_bt_evt_connection_opened_s
      //      {
      //        bd_addr address;      /**< Remote device address */
      //        uint8_t address_type; /**< Enum @ref sl_bt_gap_address_type_t. Remote device
      //                                   address type. Values:
      //                                     - <b>sl_bt_gap_public_address (0x0):</b> Public
      //                                       device address
      //                                     - <b>sl_bt_gap_static_address (0x1):</b> Static
      //                                       device address
      //                                     - <b>sl_bt_gap_random_resolvable_address
      //                                       (0x2):</b> Resolvable private random address
      //                                     - <b>sl_bt_gap_random_nonresolvable_address
      //                                       (0x3):</b> Non-resolvable private random
      //                                       address */
      //        uint8_t master;       /**< Device role in connection. Values:
      //                                     - <b>0:</b> Peripheral
      //                                     - <b>1:</b> Central */
      //        uint8_t connection;   /**< Handle for new connection */
      //        uint8_t bonding;      /**< Bonding handle. Values:
      //                                     - <b>SL_BT_INVALID_BONDING_HANDLE (0xff):</b> No
      //                                       bonding
      //                                     - <b>Other:</b> Bonding handle */
      //        uint8_t advertiser;   /**< The local advertising set that this connection was
      //                                   opened to. Values:
      //                                     - <b>SL_BT_INVALID_ADVERTISING_SET_HANDLE
      //                                       (0xff):</b> Invalid value or not applicable.
      //                                       Ignore this field
      //                                     - <b>Other:</b> The advertising set handle */
      //      });
      //
      //*********************************************************************************************************************
    case sl_bt_evt_connection_opened_id:

      bleData->connected = true;

      bleData->connection_handle = evt->data.evt_connection_opened.connection;

#if DEVICE_IS_BLE_SERVER

      sc = sl_bt_advertiser_stop (bleData->advertisingSetHandle);

      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x\n\r",
                    (unsigned int)sc);
        }

      //      sl_status_t sl_bt_connection_set_parameters(uint8_t connection,
      //                                                  uint16_t min_interval,
      //                                                  uint16_t max_interval,
      //                                                  uint16_t latency,
      //                                                  uint16_t timeout,
      //                                                  uint16_t min_ce_length,
      //                                                  uint16_t max_ce_length);
      sc = sl_bt_connection_set_parameters (bleData->connection_handle,
                                            interval_conn, interval_conn,
                                            slave_latency, spvsn_timeout, 0, 0);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_connection_set_parameters() returned != 0 status=0x%04x\n\r",
              (unsigned int)sc);
        }
#endif
      //Device in "Connected" state
      displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
#if DEVICE_IS_BLE_SERVER == 0

      displayPrintf(DISPLAY_ROW_BTADDR2, "%02X:%02X:%02X:%02X:%02X:%02X",
                                             server_addr[0],
                                             server_addr[1],
                                             server_addr[2],
                                             server_addr[3],
                                             server_addr[4],
                                             server_addr[5]);

#endif


      break;

      //****************************case 3 - sl_bt_evt_connection_closed_id********************************************************
      //      PACKSTRUCT( struct sl_bt_evt_connection_closed_s
      //      {
      //        uint16_t reason;     /**< Reason of connection close */
      //        uint8_t  connection; /**< Handle of the closed connection */
      //***************************************************************************************************************************
    case sl_bt_evt_connection_closed_id:

      displayPrintf (DISPLAY_ROW_9, "");
      displayPrintf (DISPLAY_ROW_TEMPVALUE, "");
      displayPrintf (DISPLAY_ROW_PASSKEY, "");
      displayPrintf (DISPLAY_ROW_ACTION, "");

      bleData->connected = false;
      bleData->light_indication = false;
      bleData->smoke_indication = false;
      bleData->indication_in_flight = false;
      bleData->gatt_procedure_ongoing = false;

#if DEVICE_IS_BLE_SERVER

      //      sl_status_t sl_bt_advertiser_start(uint8_t handle,
      //                                         uint8_t discover,
      //                                         uint8_t connect);
      sc = sl_bt_advertiser_start (bleData->advertisingSetHandle,
                                   sl_bt_advertiser_general_discoverable,
                                   sl_bt_advertiser_connectable_scannable);

      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r",
                    (unsigned int)sc);
        }

      //Connection closed. Device in "Advertising" state
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
      //Clear temperature value displayed
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
#else
      //start scanning in client for server when connection is closed
      sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x\n\r",
                    (unsigned int)sc);
        }

       displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
#endif
            //Clear temperature value displayed
       //display temperature measurement & client address on client gecko
       displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
       displayPrintf(DISPLAY_ROW_BTADDR2, "");


      break;

      //****************************case 4 - sl_bt_evt_connection_parameters_id********************************************************
      //      PACKSTRUCT( struct sl_bt_evt_connection_parameters_s
      //      {
      //        uint8_t  connection;    /**< Connection handle */
      //        uint16_t interval;      /**< Connection interval. Time = Value x 1.25 ms */
      //        uint16_t latency;       /**< Peripheral latency (how many connection intervals
      //                                     the peripheral can skip) */
      //        uint16_t timeout;       /**< Supervision timeout. Time = Value x 10 ms */
      //        uint8_t  security_mode; /**< Enum @ref sl_bt_connection_security_t. Connection
      //                                     security mode. Values:
      //                                       - <b>sl_bt_connection_mode1_level1 (0x0):</b>
      //                                         No security
      //                                       - <b>sl_bt_connection_mode1_level2 (0x1):</b>
      //                                         Unauthenticated pairing with encryption
      //                                       - <b>sl_bt_connection_mode1_level3 (0x2):</b>
      //                                         Authenticated pairing with encryption
      //                                       - <b>sl_bt_connection_mode1_level4 (0x3):</b>
      //                                         Authenticated Secure Connections pairing with
      //                                         encryption using a 128-bit strength
      //                                         encryption key */
      //        uint16_t txsize;        /**< Maximum Data Channel PDU Payload size that the
      //                                     controller can send in an air packet */
      //      });
      //*********************************************************************************************************************

    case sl_bt_evt_connection_parameters_id:

//Used to print the values of connection parameters for checking
//      LOG_INFO(
//          "Connection parameters are : connection = %d\r\n  interval = msec%d\r\n latency = %d\r\n timeout = msec%d\r\n security mode = %d\r\n ",
//          (int)(evt->data.evt_connection_parameters.connection),
//          (int)(evt->data.evt_connection_parameters.interval*(1.25)),
//          (int)(evt->data.evt_connection_parameters.latency),
//          (int)(evt->data.evt_connection_parameters.timeout*10),
//          (int)(evt->data.evt_connection_parameters.security_mode));
//
//      break;

    case sl_bt_evt_system_external_signal_id:
      //nothing now

      break;

    case sl_bt_evt_system_soft_timer_id:

      if (evt->data.evt_system_soft_timer.handle == 0)
        {
          displayUpdate ();
        }

#if DEVICE_IS_BLE_SERVER == 1
      else if (evt->data.evt_system_soft_timer.handle == 1)
        {
          //dequeue the fifo
          if (!bleData->indication_in_flight && get_queue_depth () > 0)
            {
              uint16_t read_charHandle;         // GATT DB handle from gatt_db.h
              uint32_t read_bufLength; // Number of bytes written to field buffer[5]
              uint8_t read_buffer[MAX_BUFFER_LENGTH];
              queue_status = read_queue (&read_charHandle, &read_bufLength,
                                         read_buffer);

              if (queue_status == 1)
                {
                  LOG_ERROR("Indication dequeue failed\n\r");
                }
              else
                {
                  sc = sl_bt_gatt_server_send_indication (
                      bleData->connection_handle, read_charHandle,
                      read_bufLength, read_buffer);
                  if (sc != SL_STATUS_OK)
                    {
                      LOG_ERROR(
                          "sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r",
                          (unsigned int)sc);
                    }
                  else
                    {
                      bleData->indication_in_flight = true;
                      bleData->queue_entries--;
                    }

                }
            }
        }
#endif
      break;
#if DEVICE_IS_BLE_SERVER
      //****************************case 6 - sl_bt_evt_gatt_server_characteristic_status_id********************************************************
      //      PACKSTRUCT( struct sl_bt_evt_gatt_server_characteristic_status_s
      //      {
      //        uint8_t  connection;          /**< Connection handle */
      //        uint16_t characteristic;      /**< GATT characteristic handle. This value is
      //                                           normally received from the
      //                                           gatt_characteristic event. */
      //        uint8_t  status_flags;        /**< Enum @ref
      //                                           sl_bt_gatt_server_characteristic_status_flag_t.
      //                                           Describes whether Client Characteristic
      //                                           Configuration was changed or if a
      //                                           confirmation was received. Values:
      //                                             - <b>sl_bt_gatt_server_client_config
      //                                               (0x1):</b> Characteristic client
      //                                               configuration has been changed.
      //                                             - <b>sl_bt_gatt_server_confirmation
      //                                               (0x2):</b> Characteristic confirmation
      //                                               has been received. */
      //        uint16_t client_config_flags; /**< Enum @ref
      //                                           sl_bt_gatt_server_client_configuration_t.
      //                                           This field carries the new value of the
      //                                           Client Characteristic Configuration. If the
      //                                           status_flags is 0x2 (confirmation
      //                                           received), the value of this field can be
      //                                           ignored. */
      //        uint16_t client_config;       /**< The handle of client-config descriptor. */
      //      });
      //*********************************************************************************************************************

    case sl_bt_evt_gatt_server_characteristic_status_id:

      if (evt->data.evt_gatt_server_characteristic_status.characteristic
          == gattdb_light_status)
        {
          // client characteristic configuration changed by remote GATT client
          if (sl_bt_gatt_server_client_config
              == (sl_bt_gatt_server_characteristic_status_flag_t) evt->data.evt_gatt_server_characteristic_status.status_flags)
            {

              if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
                  == gatt_disable)
                {
                  bleData->light_indication = false;
                  //Clear temperature value displayed
                  displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
                }

              // confirmation of indication received from remove GATT client
              else if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
                  == gatt_indication)
                {
                  bleData->light_indication = true;
                }
            }

          if (evt->data.evt_gatt_server_characteristic_status.characteristic
              == gattdb_smoke_status)
            {
              // client characteristic configuration changed by remote GATT client
              if (sl_bt_gatt_server_client_config
                  == (sl_bt_gatt_server_characteristic_status_flag_t) evt->data.evt_gatt_server_characteristic_status.status_flags)
                {

                  if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
                      == gatt_disable)
                    {
                      bleData->smoke_indication = false;
                      //Clear temperature value displayed
                      gpioLed1SetOff ();
                      displayPrintf (DISPLAY_ROW_TEMPVALUE, "");
                    }

                  // confirmation of indication received from remove GATT client
                  else if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
                      == gatt_indication)
                    {
                      bleData->smoke_indication = true;
                      gpioLed1SetOn ();
                    }
                }

              if (sl_bt_gatt_server_confirmation
                  == (sl_bt_gatt_server_characteristic_status_flag_t) evt->data.evt_gatt_server_characteristic_status.status_flags)
                {
                  bleData->indication_in_flight = false;
                }
            }  //track indication bool

          if (sl_bt_gatt_server_confirmation
              == (sl_bt_gatt_server_characteristic_status_flag_t) evt->data.evt_gatt_server_characteristic_status.status_flags)
            {
              bleData->indication_in_flight = false;
            }
        }  //track indication bool

      break;

      //****************************case 7 - sl_bt_evt_gatt_server_indication_timeout_id********************************************************
      //      PACKSTRUCT( struct sl_bt_evt_gatt_server_indication_timeout_s
      //      {
      //        uint8_t connection; /**< Connection handle */
      //      });
      //*********************************************************************************************************************
    case sl_bt_evt_gatt_server_indication_timeout_id:

      LOG_INFO("server indication timeout\n\r");
      bleData->light_indication = false;
      bleData->smoke_indication = false;

      break;

#else

    case sl_bt_evt_scanner_scan_report_id:

          if (evt->data.evt_scanner_scan_report.packet_type == 0) {
                    //check whether it is desired server
                    if((evt->data.evt_scanner_scan_report.address.addr[0] == server_addr[0]) &&
                        (evt->data.evt_scanner_scan_report.address.addr[1] == server_addr[1]) &&
                        (evt->data.evt_scanner_scan_report.address.addr[2] == server_addr[2]) &&
                        (evt->data.evt_scanner_scan_report.address.addr[3] == server_addr[3]) &&
                        (evt->data.evt_scanner_scan_report.address.addr[4] == server_addr[4]) &&
                        (evt->data.evt_scanner_scan_report.address.addr[5] == server_addr[5]) &&
                        (evt->data.evt_scanner_scan_report.address_type==0)) {

                        sc = sl_bt_scanner_stop();


          if (sc != SL_STATUS_OK)
             {
                LOG_ERROR("sl_bt_scanner_stop() returned != 0 status=0x%04x\n\r",
                                  (unsigned int)sc);
             }

          sc = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address, evt->data.evt_scanner_scan_report.address_type, sl_bt_gap_1m_phy, NULL);
          if (sc != SL_STATUS_OK)
             {
                LOG_ERROR("sl_bt_connection_open() returned != 0 status=0x%04x\n\r",
                                  (unsigned int)sc);
             }
                    }
          }
          break;


          case sl_bt_evt_gatt_procedure_completed_id:

            //Check for gatt procedure completion
            bleData->gatt_procedure_ongoing = false;
          break;


          case sl_bt_evt_gatt_service_id:
            //Check for temperature service handle
            if(memcmp(evt->data.evt_gatt_service.uuid.data, lightService, sizeof(lightService)) == 0)
              {
                //LOG_INFO("Got Thermo service handle");
                bleData->light_service_handle = evt->data.evt_gatt_service.service;
              }
            if(memcmp(evt->data.evt_gatt_service.uuid.data, smokeService, sizeof(smokeService)) == 0)
              {
                //LOG_INFO("Got button service handle");
                bleData->smoke_service_handle = evt->data.evt_gatt_service.service;
              }
            break;


          case sl_bt_evt_gatt_characteristic_id:
            //Check for temperature characteristics handle
            if(memcmp(evt->data.evt_gatt_characteristic.uuid.data, light_char, sizeof(light_char)) == 0)
              {
                //LOG_INFO("Got thermo char handle");
                bleData->light_chars_handle = evt->data.evt_gatt_characteristic.characteristic;
              }
            if(memcmp(evt->data.evt_gatt_characteristic.uuid.data, smoke_char, sizeof(smoke_char)) == 0)
              {
                //LOG_INFO("Got button char handle");
                bleData->smoke_chars_handle = evt->data.evt_gatt_characteristic.characteristic;
              }
            break;

          case sl_bt_evt_gatt_characteristic_value_id:

            /***************************************************************************//**
             *
             * Send a confirmation to a remote GATT server after receiving a characteristic
             * indication. The @ref sl_bt_evt_gatt_characteristic_value event carries the @p
             * att_opcode containing @ref sl_bt_gatt_handle_value_indication (0x1d), which
             * reveals that an indication has been received and must be confirmed with this
             * command. The confirmation needs to be sent within 30 seconds, otherwise
             * further GATT transactions are not allowed by the remote side.
             *
             * @param[in] connection Connection handle
             *
             * @return SL_STATUS_OK if successful. Error code otherwise.
             *
             ******************************************************************************/
//            sc = sl_bt_gatt_send_characteristic_confirmation(bleData->connection_handle);
//            if (sc != SL_STATUS_OK)
//               {
//                  LOG_ERROR("sl_bt_gatt_send_characteristic_confirmation() returned != 0 status=0x%04x\n\r",
//                                    (unsigned int)sc);
//               }
//
//            bleData->light_chars_value = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
//
//            //Calculate the temperature and display it on the client LCD
//            //temperature_in_c_client = FLOAT_TO_INT32(bleData->chars_value);
//            if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
//                displayPrintf(DISPLAY_ROW_9, "Buzzer On");
//            }
//            else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
//                displayPrintf(DISPLAY_ROW_9, "Buzzer Off");
//            }
//
//            displayPrintf(DISPLAY_ROW_TEMPVALUE, "Light = %x", bleData->light_chars_value);
//
//            break;

            if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication) {
                 sc = sl_bt_gatt_send_characteristic_confirmation(bleData->connection_handle);

                 if(sc != SL_STATUS_OK) {
                     LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
                 }
             }

             if(evt->data.evt_gatt_characteristic_value.characteristic == bleData->light_chars_handle) {
                 //save value got from server in a variable
                 bleData->light_chars_value = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
                             //Calculate the temperature and display it on the client LCD
                             //temperature_in_c_client = FLOAT_TO_INT32(bleData->chars_value);
                             if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
                                 displayPrintf(DISPLAY_ROW_9, "Buzzer On");
                             }
                             else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
                                 displayPrintf(DISPLAY_ROW_9, "Buzzer Off");
                             }

                // temperature_in_c_client = FLOAT_TO_INT32((bleData->chars_value));
                 displayPrintf(DISPLAY_ROW_TEMPVALUE, "Light = %d lux",bleData->light_chars_value);
             }

//             //check if we got a read response, if yes show button on lcd
//             if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_read_response) {
//                 if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
//                     displayPrintf(DISPLAY_ROW_9, "Button Pressed");
//                 }
//                 else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
//                     displayPrintf(DISPLAY_ROW_9, "Button Released");
//                 }
//             }

             if(evt->data.evt_gatt_characteristic_value.characteristic == bleData->smoke_chars_handle) {
                 //save value got from server in a variable
                 bleData->smoke_chars_value = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
                             //Calculate the temperature and display it on the client LCD
                             //temperature_in_c_client = FLOAT_TO_INT32(bleData->chars_value);
                             if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
                                 displayPrintf(DISPLAY_ROW_9, "LED On");
                             }
                             else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
                                 displayPrintf(DISPLAY_ROW_9, "LED Off");
                             }

                // temperature_in_c_client = FLOAT_TO_INT32((bleData->chars_value));
                 displayPrintf(DISPLAY_ROW_TEMPVALUE, "Smoke = %d percent",bleData->smoke_chars_value);
             }

             break;



    #endif




        }

    }//void handle_ble_event (sl_bt_msg_t *evt)

