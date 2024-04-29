/***********************************************************************
 * @file scheduler.c
 * @version 0.0.1
 * @brief Function source file for setting up Scheduler
 *
 * @author Sriya Garde, sriya.garde@colorado.edu
 * @date Feb 15, 2024
 *
 * @institution University of Colorado Boulder (UCB)
 * @course ECEN 5823-001: IoT Embedded Firmware (Fall 2020)
 * @instructor David Sluiter
 *
 * @assignment ecen5823-assignment4-sriyagarde
 * @due Feb 15, 2024
 *
 * @resources IoTEF Lecture Slides 08

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

#include "scheduler.h"
#include "app.h"
#include "em_core.h"
#include "em_cmu.h"
#include "em_i2c.h"
#include "em_adc.h"
#include "stdbool.h"
#include "app.h"
#include "gpio.h"
#include "timers.h"
#include "i2c.h"
#include "irq.h"
#include "adc.h"
#include "lcd.h"
#include "sl_bt_api.h"
#include "ble.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

////Definitions to enclose critical sections of the ISR which shouldn't be modified by other peripheral interrupts
//#define CORE_DECLARE_IRQ_STATE CORE_irqState_t irqState
//#define CORE_ENTER_CRITICAL() irqState = CORE_EnterCritical()
//#define CORE_EXIT_CRITICAL() CORE_ExitCritical(irqState)


uint32_t myEvents = eventDefault;

sl_status_t rc=0;
//Characteristic Descriptor 2905

#if DEVICE_IS_BLE_SERVER == 0


//1d9f9050-5651-46a2-b5f4-6c9545cc1ab1
static const uint8_t lightService[16] = {0xb1, 0x1a, 0xcc, 0x45, 0x95, 0x6c, 0xf4, 0xb5, 0xa2, 0x46, 0x51, 0x56, 0x50, 0x90, 0x9f, 0x1d};

//33aef90b-a996-4ede-9b73-99a7b209e9fb
static const uint8_t light_char[16] = {0xfb, 0xe9, 0x09, 0xb2, 0xa7, 0x99, 0x73, 0x9b, 0xde, 0x4e, 0x96, 0xa9, 0x0b, 0xf9, 0xae, 0x33};

//d84d8df7-9b1c-4dfc-b25d-61f9e8b2a49e - smoke service
static const uint8_t smokeService[16] = {0x9e, 0xa4, 0xb2, 0xe8, 0xf9, 0x61, 0x5d, 0xb2, 0xfc, 0x4d, 0x1c, 0x9b, 0xf7, 0x8d, 0x4d, 0xd8};

//e9958983-800c-4b3f-9d3f-e1c352d025ec - smoke char
static const uint8_t smoke_char[16] = {0xec, 0x25, 0xd0, 0x52, 0xc3, 0xe1, 0x3f, 0x9d, 0x3f, 0x4b, 0x0c, 0x80, 0x83, 0x89, 0x95, 0xe9};


#endif

//Enum to define the states for I2C Temperature measurement state machine
typedef enum uint32_t {
  state0_Idle,
  state1_StartInit,
  state2_ADCRead,
  state0_smoke_Idle,
  state1_smoke_StartInit,
  state2_smoke_ADCRead,
  //state3_WaittoRead,
  //state4_I2CRead,
  state0_Client_Idle,
  state0_Client_smoke_discover,
  state1_Client_light_Getservice,
  state1_Client_smoke_Getservice,
  state2_Client_light_Getchars,
  state2_Client_smoke_Getchars,
  state3_Client_SetIndication,
  state4_Client_Connclose

} State_t;

/*
 *Scheduler function to perform actions when Underflow flag for LETIMER0 IRQ is set

 * @param null
 * @return null
 */
void schedulerSetEventUFFlag()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(eventTimerUFflag) ;
 // myEvents |= eventTimerUFflag;

  CORE_EXIT_CRITICAL();
}//schedulerSetEventUFFlag()

/*
 *Scheduler function to perform actions when COMP1 flag for LETIMER0 IRQ is set

 * @param null
 * @return null
 */
void schedulerSetEventCOMP1Flag()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();

  sl_bt_external_signal(eventTimerCOMP1flag);
  //myEvents |= eventTimerCOMP1flag;

  CORE_EXIT_CRITICAL();
}//schedulerSetEventCOMP1Flag()



#if DEVICE_IS_BLE_SERVER
/*
 *State machine function to get the next state to retain and perform the flow for i2c transfer of temperature measurement

 * @param the bluetooth event occuring to be handled by the temperature state machine
 * @return null
 */
void light_state_machine(sl_bt_msg_t *evt)
{
  ble_data_struct_t *bleData = getBleDataPtr();

  if((SL_BT_MSG_ID(evt->header)==sl_bt_evt_system_external_signal_id)
       && (bleData->connected==true)
       && (bleData->light_indication==true))
    {
  //  State_t currentState;
  static State_t nextState = state0_Idle;
  //  currentState = nextState;
  switch(nextState)
  {
    case state0_Idle :
      LOG_INFO("Entering Idle state0 Light sensor\r\n");
      nextState = state0_Idle;
      if(evt->data.evt_system_external_signal.extsignals & eventTimerUFflag)
        {
          light_sensor_enable();
          timerWaitUs_irq(8000);
          nextState = state1_StartInit;
        }
      break;

    case state1_StartInit :
      LOG_INFO("Entering Start_Init state1 Light sensor\r\n");
      nextState = state1_StartInit;
      if(evt->data.evt_system_external_signal.extsignals & eventTimerUFflag)
          //eventTimerCOMP1flag
        {
          //sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
          initADC();
          //timerWaitUs_irq(3000);
          nextState = state2_ADCRead;
        }
      break;

    case state2_ADCRead :
      LOG_INFO("Entering ADCRead state2 Light sensor\r\n");
      nextState = state2_ADCRead;
      if(evt->data.evt_system_external_signal.extsignals & eventTimerUFflag)
        {
          //sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          //ADC_Start(ADC0, adcStartSingle);
          ble_ambient();
          timerWaitUs_irq(3000);
          light_sensor_disable();
          nextState = state0_smoke_Idle;
        }
      break;

    case state0_smoke_Idle :
          LOG_INFO("Entering Idle state0 Smoke Sensor\r\n");
          nextState = state0_smoke_Idle;
          if(evt->data.evt_system_external_signal.extsignals & eventTimerCOMP1flag)
            {
              smoke_sensor_enable();
              timerWaitUs_irq(8000);
              nextState = state1_smoke_StartInit;
            }
          break;

        case state1_smoke_StartInit :
          LOG_INFO("Entering Start_Init state1 Smoke Sensor\r\n");
          nextState = state1_smoke_StartInit;
          if(evt->data.evt_system_external_signal.extsignals & eventTimerCOMP1flag)
              //eventTimerCOMP1flag
            {
              //sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
              initADC();
              timerWaitUs_irq(3000);
              nextState = state2_smoke_ADCRead;
            }
          break;

        case state2_smoke_ADCRead :
          LOG_INFO("Entering ADCRead state2 Smoke Sensor\r\n");
          nextState = state2_smoke_ADCRead;
          if(evt->data.evt_system_external_signal.extsignals & eventTimerCOMP1flag)
            {
              //sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
              //ADC_Start(ADC0, adcStartSingle);
              ble_smoke();
              timerWaitUs_irq(3000);
              smoke_sensor_disable();
              nextState = state0_Idle;
            }
          break;

    default :
      LOG_ERROR("Error in state machine functioning \r\n");
      break;
  }

    }


}//light_state_machine(uint32_t event)


#else

/*
 *State machine function to discover the client gatt [procedures for temperature measurements

 * @param the bluetooth event occuring to be handled by the discovery state machine
 * @return null
 */
void discovery_state_machine(sl_bt_msg_t *evt){

  ble_data_struct_t *bleData = getBleDataPtr();


    static State_t nextState = state0_Client_Idle;
    if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id) {
        nextState = state0_Client_Idle;
    }
    switch(nextState)
    {
      case state0_Client_Idle:
        nextState = state0_Client_Idle;
        if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_opened_id){
            bleData->gatt_procedure_ongoing = true;
            rc = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connection_handle,
                                                              sizeof(lightService),
                                                              (const uint8_t*)lightService);
            if (rc != SL_STATUS_OK)
                        {
                          LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() returned != 0 status=0x%04x\n\r",(unsigned int)rc);
                        }

            nextState = state1_Client_light_Getservice;
        }


      break;

      case state1_Client_light_Getservice:
        nextState = state1_Client_light_Getservice;
        if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)//to be defined in ble.c for client events
          {
            bleData->gatt_procedure_ongoing = true;
            rc = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connection_handle,
                                                             bleData->light_service_handle,
                                                             sizeof(light_char),
                                                             (const uint8_t*)light_char);
            if (rc != SL_STATUS_OK)
                        {
                          LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() returned != 0 status=0x%04x\n\r",(unsigned int)rc);
                        }

            nextState = state2_Client_light_Getchars;
        }

        break;

        case state2_Client_light_Getchars:
          nextState = state2_Client_light_Getchars;
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)//to be defined in ble.c for client events
            {
              bleData->gatt_procedure_ongoing = true;

              rc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                              bleData->light_chars_handle,
                                                              sl_bt_gatt_indication);
              if (rc != SL_STATUS_OK)
                {
                  LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r",(unsigned int)rc);
                }
              displayPrintf(DISPLAY_ROW_CONNECTION, "Handling indications");
              nextState = state0_Client_smoke_discover;
            }
          break;

        case state0_Client_smoke_discover:

          nextState = state0_Client_smoke_discover;
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_opened_id){
              bleData->gatt_procedure_ongoing = true;
              rc = sl_bt_gatt_discover_primary_services_by_uuid(bleData->connection_handle,
                                                                sizeof(smokeService),
                                                                (const uint8_t*)smokeService);
              if (rc != SL_STATUS_OK)
                          {
                            LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() returned != 0 status=0x%04x\n\r",(unsigned int)rc);
                          }

              nextState = state1_Client_smoke_Getservice;
          }
          break;


        case state1_Client_smoke_Getservice:
          nextState = state1_Client_smoke_Getservice;
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)//to be defined in ble.c for client events
            {
              bleData->gatt_procedure_ongoing = true;
              rc = sl_bt_gatt_discover_characteristics_by_uuid(bleData->connection_handle,
                                                               bleData->smoke_service_handle,
                                                               sizeof(smoke_char),
                                                               (const uint8_t*)smoke_char);
              if (rc != SL_STATUS_OK)
                          {
                            LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() returned != 0 status=0x%04x\n\r",(unsigned int)rc);
                          }

              nextState = state2_Client_smoke_Getchars;
          }

          break;

        case state2_Client_smoke_Getchars:
          nextState = state2_Client_smoke_Getchars;
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)//to be defined in ble.c for client events
            {
              bleData->gatt_procedure_ongoing = true;

              rc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                              bleData->smoke_chars_handle,
                                                              sl_bt_gatt_indication);
              if (rc != SL_STATUS_OK)
                {
                  LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r",(unsigned int)rc);
                }
              displayPrintf(DISPLAY_ROW_CONNECTION, "Handling indications");
              nextState = state3_Client_SetIndication;
            }
          break;


        case state3_Client_SetIndication:
          nextState = state3_Client_SetIndication;
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)//to be defined in ble.c for client events
            {
              nextState = state4_Client_Connclose;
            }
          break;

        case state4_Client_Connclose:
          nextState = state4_Client_Connclose;
          if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id){
              nextState = state0_Client_Idle;
          }
          break;

        default:
          LOG_ERROR("Error in state machine functioning \r\n");
                break;


    }

}



#endif

