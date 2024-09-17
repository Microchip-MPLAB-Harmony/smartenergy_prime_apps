/*******************************************************************************
  Modem Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    modem.h

  Summary:
    MODEM : Modem Application for PRIME Base Node

  Description:
    This header file defines the serialization interface of the PRIME primitives 
    through the USI for the Base Node.
*******************************************************************************/

#ifndef MODEM_H_INCLUDED
#define MODEM_H_INCLUDED

#include <stdint.h>
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************    
// *****************************************************************************
/* PRIME modem message command communication enumeration

 Summary:
    PRIME modem communication command messages.

 Description:
    This enumeration lists the message communication commands.

 Remarks:
    None.
*/
typedef enum 
{
    /* Null Convergence Layer establish request and indication commands */
    APP_MODEM_CL_NULL_ESTABLISH_REQUEST_CMD                     = 0x01,
    APP_MODEM_CL_NULL_ESTABLISH_INDICATION_CMD                  = 0x02,
    APP_MODEM_CL_NULL_ESTABLISH_CONFIRM_CMD                     = 0x03,
    APP_MODEM_CL_NULL_ESTABLISH_RESPONSE_CMD                    = 0x04,
    /* Null Convergence Layer release request and indication commands */            
    APP_MODEM_CL_NULL_RELEASE_REQUEST_CMD                       = 0x05,
    APP_MODEM_CL_NULL_RELEASE_INDICATION_CMD                    = 0x06,
    APP_MODEM_CL_NULL_RELEASE_CONFIRM_CMD                       = 0x07,
    APP_MODEM_CL_NULL_RELEASE_RESPONSE_CMD                      = 0x08,
    /* Null Convergence Layer join request and indication commands */               
    APP_MODEM_CL_NULL_JOIN_REQUEST_CMD                          = 0x09,
    APP_MODEM_CL_NULL_JOIN_INDICATION_CMD                       = 0x0A,
    APP_MODEM_CL_NULL_JOIN_RESPONSE_CMD                         = 0x0B,
    APP_MODEM_CL_NULL_JOIN_CONFIRM_CMD                          = 0x0C,
    /* Null Convergence Layer leave request and indication commands */               
    APP_MODEM_CL_NULL_LEAVE_REQUEST_CMD                         = 0x0D,
    APP_MODEM_CL_NULL_LEAVE_CONFIRM_CMD                         = 0x0E,
    APP_MODEM_CL_NULL_LEAVE_INDICATION_CMD                      = 0x0F,
    /* Null Convergence Layer data request and indication commands */            
    APP_MODEM_CL_NULL_DATA_REQUEST_CMD                          = 0x10,
    APP_MODEM_CL_NULL_DATA_CONFIRM_CMD                          = 0x11,
    APP_MODEM_CL_NULL_DATA_INDICATION_CMD                       = 0x12,
    /* PLME request and confirm commands */            
    APP_MODEM_CL_NULL_PLME_RESET_REQUEST_CMD                    = 0x13,
    APP_MODEM_CL_NULL_PLME_RESET_CONFIRM_CMD                    = 0x14,
    APP_MODEM_CL_NULL_PLME_SLEEP_REQUEST_CMD                    = 0x15,
    APP_MODEM_CL_NULL_PLME_SLEEP_CONFIRM_CMD                    = 0x16,
    APP_MODEM_CL_NULL_PLME_RESUME_REQUEST_CMD                   = 0x17,
    APP_MODEM_CL_NULL_PLME_RESUME_CONFIRM_CMD                   = 0x18,
    APP_MODEM_CL_NULL_PLME_TESTMODE_REQUEST_CMD                 = 0x19,
    APP_MODEM_CL_NULL_PLME_TESTMODE_CONFIRM_CMD                 = 0x1A,
    APP_MODEM_CL_NULL_PLME_GET_REQUEST_CMD                      = 0x1B,
    APP_MODEM_CL_NULL_PLME_GET_CONFIRM_CMD                      = 0x1C,
    APP_MODEM_CL_NULL_PLME_SET_REQUEST_CMD                      = 0x1D,
    APP_MODEM_CL_NULL_PLME_SET_CONFIRM_CMD                      = 0x1E,
    /* MLME request and confirm commands */             
    APP_MODEM_CL_NULL_MLME_REGISTER_REQUEST_CMD                 = 0x1F,
    APP_MODEM_CL_NULL_MLME_REGISTER_CONFIRM_CMD                 = 0x20,
    APP_MODEM_CL_NULL_MLME_REGISTER_INDICATION_CMD              = 0x21,
    APP_MODEM_CL_NULL_MLME_UNREGISTER_REQUEST_CMD               = 0x22,
    APP_MODEM_CL_NULL_MLME_UNREGISTER_CONFIRM_CMD               = 0x23,
    APP_MODEM_CL_NULL_MLME_UNREGISTER_INDICATION_CMD            = 0x24,
    APP_MODEM_CL_NULL_MLME_PROMOTE_REQUEST_CMD                  = 0x25,
    APP_MODEM_CL_NULL_MLME_PROMOTE_CONFIRM_CMD                  = 0x26,
    APP_MODEM_CL_NULL_MLME_PROMOTE_INDICATION_CMD               = 0x27,
    APP_MODEM_CL_NULL_MLME_DEMOTE_REQUEST_CMD                   = 0x28,
    APP_MODEM_CL_NULL_MLME_DEMOTE_CONFIRM_CMD                   = 0x29,
    APP_MODEM_CL_NULL_MLME_DEMOTE_INDICATION_CMD                = 0x2A,
    APP_MODEM_CL_NULL_MLME_RESET_REQUEST_CMD                    = 0x2B,
    APP_MODEM_CL_NULL_MLME_RESET_CONFIRM_CMD                    = 0x2C,
    APP_MODEM_CL_NULL_MLME_GET_REQUEST_CMD                      = 0x2D,
    APP_MODEM_CL_NULL_MLME_GET_CONFIRM_CMD                      = 0x2E,
    APP_MODEM_CL_NULL_MLME_LIST_GET_REQUEST_CMD                 = 0x2F,
    APP_MODEM_CL_NULL_MLME_LIST_GET_CONFIRM_CMD                 = 0x30,
    APP_MODEM_CL_NULL_MLME_SET_REQUEST_CMD                      = 0x31,
    APP_MODEM_CL_NULL_MLME_SET_CONFIRM_CMD                      = 0x32,
    /* 4-32 Convergence Layer request and indication commands */  
    APP_MODEM_CL_432_ESTABLISH_REQUEST_CMD                      = 0x33,
    APP_MODEM_CL_432_ESTABLISH_CONFIRM_CMD                      = 0x34,
    APP_MODEM_CL_432_RELEASE_REQUEST_CMD                        = 0x35,
    APP_MODEM_CL_432_RELEASE_CONFIRM_CMD                        = 0x36,
    APP_MODEM_CL_432_DL_DATA_REQUEST_CMD                        = 0x37,
    APP_MODEM_CL_432_DL_DATA_INDICATION_CMD                     = 0x38,
    APP_MODEM_CL_432_DL_DATA_CONFIRM_CMD                        = 0x39,
    APP_MODEM_CL_432_DL_JOIN_INDICATION_CMD                     = 0x3A,
    APP_MODEM_CL_432_DL_LEAVE_INDICATION_CMD                    = 0x3B,
    APP_MODEM_CL_432_REDIRECT_RESPONSE_CMD                      = 0x3C,

    /* Base Management firmware upgrade request commands */             
    APP_MODEM_BMNG_FUP_CLEAR_TARGET_REQUEST_CMD                 = 0x3D,
    APP_MODEM_BMNG_FUP_ADD_TARGET_REQUEST_CMD                   = 0x3E,
    APP_MODEM_BMNG_FUP_SET_FW_DATA_REQUEST_CMD                  = 0x3F,
    APP_MODEM_BMNG_FUP_SET_UPGRADE_REQUEST_CMD                  = 0x40,
    APP_MODEM_BMNG_FUP_INIT_FILE_TX_REQUEST_CMD                 = 0x41,
    APP_MODEM_BMNG_FUP_DATA_FRAME_REQUEST_CMD                   = 0x42,
    APP_MODEM_BMNG_FUP_CHECK_CRC_REQUEST_CMD                    = 0x43,
    APP_MODEM_BMNG_FUP_ABORT_FU_REQUEST_CMD                     = 0x44,
    APP_MODEM_BMNG_FUP_START_FU_REQUEST_CMD                     = 0x45,
    APP_MODEM_BMNG_FUP_SET_MATCH_RULE_REQUEST_CMD               = 0x46,
    APP_MODEM_BMNG_FUP_GET_VERSION_REQUEST_CMD                  = 0x47,
    APP_MODEM_BMNG_FUP_GET_STATE_REQUEST_CMD                    = 0x48,
    APP_MODEM_BMNG_FUP_ACK_CMD                                  = 0x49,
    APP_MODEM_BMNG_FUP_STATUS_INDICATION_CMD                    = 0x4A,
    APP_MODEM_BMNG_FUP_STATUS_ERROR_INDICATION_CMD              = 0x4B,
    APP_MODEM_BMNG_FUP_VERSION_INDICATION_CMD                   = 0x4C,
    APP_MODEM_BMNG_FUP_KILL_INDICATION_CMD                      = 0x4D,
    APP_MODEM_BMNG_FUP_SET_SIGNATURE_DATA_REQUEST_CMD           = 0x4E,
    APP_MODEM_BMNG_NETWORK_EVENT_CMD                            = 0x4F,
    /* Base Management PRIME profile primitives request commands */
    APP_MODEM_BMNG_PPROF_GET_REQUEST_CMD                        = 0x50,
    APP_MODEM_BMNG_PPROF_SET_REQUEST_CMD                        = 0x51,
    APP_MODEM_BMNG_PPROF_RESET_REQUEST_CMD                      = 0x52,
    APP_MODEM_BMNG_PPROF_REBOOT_REQUEST_CMD                     = 0x53,
    APP_MODEM_BMNG_PPROF_GET_ENHANCED_REQUEST_CMD               = 0x54,
    APP_MODEM_BMNG_PPROF_ACK_CMD                                = 0x55,
    APP_MODEM_BMNG_PPROF_GET_RESPONSE_CMD                       = 0x56,
    APP_MODEM_BMNG_PPROF_GET_ENHANCED_RESPONSE_CMD              = 0x57,
    APP_MODEM_BMNG_PPROF_GET_ZC_RESPONSE_CMD                    = 0x58,
    APP_MODEM_BMNG_PPROF_ZC_DIFF_REQUEST_CMD                    = 0x59,
    APP_MODEM_BMNG_PPROF_ZC_DIFF_RESPONSE_CMD                   = 0x5A,
    /* Base Management whitelist request commands */
    APP_MODEM_BMNG_WHITELIST_ADD_REQUEST_CMD                    = 0x5B,
    APP_MODEM_BMNG_WHITELIST_REMOVE_REQUEST_CMD                 = 0x5C,
    APP_MODEM_BMNG_WHITELIST_ACK_CMD                            = 0x5D,
            
    /* Modem application IPV6 request and confirm commands */
    APP_MODEM_IPV6_ESTABLISH_REQUEST_CMD                        = 0x5F,
    APP_MODEM_IPV6_ESTABLISH_CONFIRM_CMD                        = 0x60,
    APP_MODEM_IPV6_RELEASE_REQUEST_CMD                          = 0x61,
    APP_MODEM_IPV6_RELEASE_CONFIRM_CMD                          = 0x62,
    APP_MODEM_IPV6_REGISTER_REQUEST_CMD                         = 0x63,
    APP_MODEM_IPV6_REGISTER_CONFIRM_CMD                         = 0x64,
    APP_MODEM_IPV6_UNREGISTER_REQUEST_CMD                       = 0x65,
    APP_MODEM_IPV6_UNREGISTER_CONFIRM_CMD                       = 0x66,
    APP_MODEM_IPV6_DATA_REQUEST_CMD                             = 0x67,
    APP_MODEM_IPV6_DATA_INDICATION_CMD                          = 0x68,
    APP_MODEM_IPV6_DATA_CONFIRM_CMD                             = 0x69,
    APP_MODEM_IPV6_MUL_JOIN_REQUEST_CMD                         = 0x6A,
    APP_MODEM_IPV6_MUL_JOIN_CONFIRM_CMD                         = 0x6B,
    APP_MODEM_IPV6_MUL_LEAVE_REQUEST_CMD                        = 0x6C,
    APP_MODEM_IPV6_MUL_LEAVE_CONFIRM_CMD                        = 0x6D,
            
    /* MLME MultiPhy request and confirm commands */
    APP_MODEM_CL_NULL_MLME_MP_PROMOTE_REQUEST_CMD               = 0x6E,
    APP_MODEM_CL_NULL_MLME_MP_PROMOTE_CONFIRM_CMD               = 0x6F,
    APP_MODEM_CL_NULL_MLME_MP_PROMOTE_INDICATION_CMD            = 0x70,
    APP_MODEM_CL_NULL_MLME_MP_DEMOTE_REQUEST_CMD                = 0x71,
    APP_MODEM_CL_NULL_MLME_MP_DEMOTE_CONFIRM_CMD                = 0x72,
    APP_MODEM_CL_NULL_MLME_MP_DEMOTE_INDICATION_CMD             = 0x73,
            
    APP_MODEM_API_ERROR_CMD
} APP_MODEM_PRIME_API_CMD;

typedef enum
{
    APP_MODEM_STATE_INIT=0,
    APP_MODEM_STATE_CONFIGURE,
    APP_MODEM_STATE_TASKS,
            
} APP_MODEM_STATES;

/* Errors in the modem application */
#define APP_MODEM_ERR_UNKNOWN_CMD       9500
#define APP_MODEM_ERR_MSG_TOO_BIG       9501
#define APP_MODEM_ERR_QUEUE_FULL        9502

/* Modem interface */
void APP_Modem_Initialize(void);
void APP_Modem_Tasks(void);
uint8_t APP_Modem_TxdataIndication(void);
uint8_t APP_Modem_RxdataIndication(void);

#endif /* MODEM_H_INCLUDED */
