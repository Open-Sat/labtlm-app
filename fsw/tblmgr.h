/*
** $Id: $
** 
** Purpose: Provide a class to manage the application's tables
**
** Notes:
**   1. The tables are formatted using XML and reply on the open source EXPAT
**      library.  This is a prototype and EXPAT has not been tested so it is
**      not recommended that it be used on a flight project until it has been
**      formally tested.
**   2. Regardless of the parser used the design is reusable. 
**
** References:
**   1. OpenSat Object-Based Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/

#ifndef _tblmgr_
#define _tblmgr_

/*
** Includes
*/

#include <expat.h>
#include "app_config.h"
#include "pktmgr.h"

/*
** XML tag definitions.
*/
#define TBLMGR_XML_EL_PKT_ENTRY    "entry"

#define TBLMGR_XML_AT_STREAM_ID    "stream-id"
#define TBLMGR_XML_AT_PRIORITY     "priority"
#define TBLMGR_XML_AT_RELIABILITY  "reliability"
#define TBLMGR_XML_AT_BUF_LIM      "buf-limit"

#define TBLMGR_BUFFSIZE  8192

/*
** TBLMGR_LoadTblCmd Load Type options
*/
#define TBLMGR_LOAD_TBL_REPLACE    0
#define TBLMGR_LOAD_TBL_UPDATE     1


/*
** Event Message IDs
*/

#define TBLMGR_CREATE_ERR_EID           (TBLMGR_BASE_EID + 0)
#define TBLMGR_FILE_OPEN_ERR_EID        (TBLMGR_BASE_EID + 1)
#define TBLMGR_FILE_READ_ERR_EID        (TBLMGR_BASE_EID + 2)
#define TBLMGR_PARSE_ERR_EID            (TBLMGR_BASE_EID + 3)
#define TBLMGR_WRITE_CFE_HDR_ERR_EID    (TBLMGR_BASE_EID + 4)
#define TBLMGR_CREATE_MSG_DUMP_ERR_EID  (TBLMGR_BASE_EID + 5)
#define TBLMGR_CMD_LOAD_TYPE_ERR_EID    (TBLMGR_BASE_EID + 6)
#define TBLMGR_CMD_LOAD_PARSE_ERR_EID   (TBLMGR_BASE_EID + 7)

#define TBLMGR_TOTAL_EID  8

/*
** Type Definitions
*/

typedef struct
{
   uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE]; /* cFE Software Bus Command Message Header     */
   char    FileName[OS_MAX_PATH_LEN];      /* ASCII text string of full path and filename */
   uint16  LoadType;                       /* Replace or update table records             */

} TBLMGR_LoadTblCmd;
#define TBLMGR_LOAD_TBL_CMD_DATA_LEN  (sizeof(TBLMGR_LoadTblCmd) - CFE_SB_CMD_HDR_SIZE)

typedef struct
{
   uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE]; /* cFE Software Bus Command Message Header    */
   char    FileName[OS_MAX_PATH_LEN];      /*ASCII text string of full path and filename */

} TBLMGR_DumpTblCmd;
#define TBLMGR_DUMP_TBL_CMD_DATA_LEN  (sizeof(TBLMGR_DumpTblCmd) - CFE_SB_CMD_HDR_SIZE)


/*
**  Local table copies used for table load commands
**  - Even though only one table being managed, defined separate struct for PKTMGR in
**    case things expand in the future
*/
typedef struct {

   boolean       LoadActive;
   boolean       LastLoadValid;
   uint16        AttrErrCnt;
   PKTMGR_Table  Local;
   boolean       Modified[PKTMGR_TBL_MAX_ENTRY_ID];

} TBLMGR_PktTbl;

typedef struct {

   TBLMGR_PktTbl PktTbl;

} TBLMGR_Class;

/*
** Exported Functions
*/

/******************************************************************************
** Function: TBLMGR_Constructor
**
** Initialize the Table Manager object.
*/
void TBLMGR_Constructor(TBLMGR_Class* TblMgrPtr,
                        char* PktMgrFilePathName);


/******************************************************************************
** Function: TBLMGR_ResetStatus
**
** Reset counters and status flags to a known reset state.  The behavior of the scheduler
** should not be impacted. The intent is to clear counters/flags for telemetry.
**
** Notes:
**   1. See the SCHTBL_Class definition for the effected data.
**
*/
void TBLMGR_ResetStatus(void);


/******************************************************************************
** Function: TBLMGR_LoadPktTable
**
** Command to load the telemetry packet table.
**
** Notes:
**  1. Function signature must match cmdmgr's CMDMGR_CmdFuncPtr.
**
*/
boolean TBLMGR_LoadPktTable(const CFE_SB_MsgPtr_t MsgPtr);


/******************************************************************************
** Function: TBLMGR_DumpPktTable
**
** Command to dump the telemetry packet table.
**
** Notes:
**  1. Function signature must match cmdmgr's CMDMGR_CmdFuncPtr.
**
*/
boolean TBLMGR_DumpPktTable(const CFE_SB_MsgPtr_t MsgPtr);


#endif /* _tblmgr_ */
