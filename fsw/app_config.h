/*
** $Id: $
** 
** Purpose: Define configurations for the Lab Telemetry Output application
**
** Notes:
**   1. These macros can only be build with the application and can't
**      have a platform scope because the same file name is used for
**      all applications following the object-based application design.
**
** References:
**   1. CFS Object-based Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/

#ifndef _app_config_
#define _app_config_

/*
** Includes
*/

#include "labtlm_mission_cfg.h"
#include "labtlm_platform_cfg.h"


/******************************************************************************
** Lab Telemetry Output Application Macros
*/

#define  LABTLM_MAJOR_VERSION      1
#define  LABTLM_MINOR_VERSION      0
#define  LABTLM_REVISION           0
#define  LABTLM_MISSION_REV        0


/******************************************************************************
** Command Macros
*/

#define LABTLM_CMD_RESET_FC            0
#define LABTLM_CMD_NOOP_FC             1

#define LABTLM_CMD_PKT_TBL_LOAD_FC     2
#define LABTLM_CMD_PKT_TBL_DUMP_FC     3

#define LABTLM_CMD_ADD_PKT_FC          4
#define LABTLM_CMD_REMOVE_PKT_FC       5
#define LABTLM_CMD_REMOVE_ALL_PKTS_FC  6
#define LABTLM_CMD_ENABLE_OUTPUT_FC    7

#define LABTLM_CMD_SEND_DATA_TYPES_FC  8

#define LABTLM_CMD_TOTAL_FC            9

#define CMDMGR_CMD_FUNC_TOTAL   10
#define CMDMGR_PIPE_DEPTH       10
#define CMDMGR_PIPE_NAME        "LABTLM_CMD_PIPE"
#define CMDMGR_CMD_MSG_TOTAL    2


/******************************************************************************
** Event Macros
*/

#define LABTLM_BASE_EID   0  /* Used by labtlm.h */
#define CMDMGR_BASE_EID  10  /* Used by cmdmgr.h */
#define TBLMGR_BASE_EID  20  /* Used by tblmgr.h */
#define PKTMGR_BASE_EID  30  /* Used by pktmgr.h */


/******************************************************************************
** pktmgr.h Configurations
*/

/*
** Maximum Number of Packet Definitions in Packet Table. Must be greater than zero.
*/

#define PKTMGR_PIPE_DEPTH       132
#define PKTMGR_PIPE_NAME        "LABTLM_PKT_PIPE"

#define PKTMGR_TBL_MAX_ENTRY_ID  64
#define PKTMGR_UNUSED_MSG_ID     (CFE_SB_HIGHEST_VALID_MSGID+1)


/******************************************************************************
** tblmgr.h Configurations
*/

#define TBLMGR_DEF_PKT_TBL_DUMP_FILE LABTLM_DEF_PKTTBL_FILE_NAME

#endif /* _app_config_ */
