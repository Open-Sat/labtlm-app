/* 
** File:
**   $Id: $
**
** Purpose: Manage the output of telemetry packets from the SB to a UDP socket.
**
** Notes:
**   1. This is non-flight code so an attempt has been made to balance keeping
**      it simple while making it robust. Limiting the number of configuration
**      parameters and integration items (message IDs, perf IDs, etc) was
**      also taken into consideration.
**   2. Event message filters are not used since this is for test environments.
**      This may be reconsidered if event flooding ever becomes a problem.
**   3. Performance traces are not included.
**   4. Most functions are global to assist in unit testing
**
** References:
**   1. NASA GSFC Code 582 C Coding Standards
**   2. CFS Object-based Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/

/*
** Includes
*/

#include <string.h>
#include "labtlm.h"


/*
** Local Function Prototypes
*/

static int32 InitApp(void);
static void InitDataTypePkt(void);
static void ProcessCommands(void);

/*
** Global Data
*/

LABTLM_Class  LabTlm;

LABTLM_HkPkt        LabTlmHkPkt;
LABTLM_DataTypePkt  ToDataTypePkt;

/******************************************************************************
** Function: LABTLM_Main
**
*/
void LABTLM_Main(void)
{

   int32  Status    = CFE_SEVERITY_ERROR;
   uint32 RunStatus = CFE_ES_APP_ERROR;


   Status = CFE_ES_RegisterApp();
   CFE_EVS_Register(NULL,0,0);

   /*
   ** Perform application specific initialization
   */
   if (Status == CFE_SUCCESS)
   {
       OS_printf("TO-LAB: About to call init\n");
       Status = InitApp();
   }

   /*
   ** At this point many flight apps use CFE_ES_WaitForStartupSync() to
   ** synchronize their startup timing with other apps. This is not
   ** needed.
   */

   if (Status == CFE_SUCCESS) RunStatus = CFE_ES_APP_RUN;

   /*
   ** Main process loop
   */
   OS_printf("TO-LAB: About to enter loop\n");
   while (CFE_ES_RunLoop(&RunStatus))
   {

      OS_TaskDelay(LABTLM_RUNLOOP_DELAY);

      PKTMGR_OutputTelemetry();

      ProcessCommands();

   } /* End CFE_ES_RunLoop */


   /* Write to system log in case events not working */

   CFE_ES_WriteToSysLog("LABTLM App terminating, err = 0x%08X\n", Status);

   CFE_EVS_SendEvent(LABTLM_EXIT_ERR_EID, CFE_EVS_CRITICAL, "LABTLM App: terminating, err = 0x%08X", Status);

   CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */

} /* End of LABTLM_Main() */


/******************************************************************************
** Function: LABTLM_NoOpCmd
**
*/

boolean LABTLM_NoOpCmd(const CFE_SB_MsgPtr_t MsgPtr)
{

   CFE_EVS_SendEvent (LABTLM_NOOP_INFO_EID,
                      CFE_EVS_INFORMATION,
                      "No operation command received for Telemetry Output Lab version %d.%d",
                      LABTLM_MAJOR_VERSION,LABTLM_MINOR_VERSION);

   return TRUE;


} /* End LABTLM_NoOpCmd() */


/******************************************************************************
** Function: LABTLM_ResetAppCmd
**
*/

boolean LABTLM_ResetAppCmd(const CFE_SB_MsgPtr_t MsgPtr)
{

   PKTMGR_ResetStatus();
   TBLMGR_ResetStatus();
   CMDMGR_ResetStatus();

   return TRUE;

} /* End LABTLM_ResetAppCmd() */


/******************************************************************************
** Function: LABTLM_SendDataTypeTlmCmd
**
*/

boolean LABTLM_SendDataTypeTlmCmd(const CFE_SB_MsgPtr_t MsgPtr)
{

   int32 Status;

   CFE_SB_TimeStampMsg((CFE_SB_MsgPtr_t) &ToDataTypePkt);
   Status = CFE_SB_SendMsg((CFE_SB_Msg_t *)&ToDataTypePkt);

   return (Status == CFE_SUCCESS);

} /* End LABTLM_SendDataTypeTlmCmd() */

/******************************************************************************
** Function: LABTLM_SendHousekeepingPkt
**
*/
void LABTLM_SendHousekeepingPkt(void)
{

   /*
   ** LABTLM Data
   */

   LabTlmHkPkt.ValidCmdCnt   = LabTlm.CmdMgr.ValidCmdCnt;
   LabTlmHkPkt.InvalidCmdCnt = LabTlm.CmdMgr.InvalidCmdCnt;

   /*
   ** TBLMGR Data
   */

   LabTlmHkPkt.PktTblLoadActive     = LabTlm.TblMgr.PktTbl.LoadActive;
   LabTlmHkPkt.PktTblLastLoadValid  = LabTlm.TblMgr.PktTbl.LastLoadValid;
   LabTlmHkPkt.PktTblAttrErrCnt     = LabTlm.TblMgr.PktTbl.AttrErrCnt;

   /*
   ** PKTMGR Data
   ** - At a minimum all pktmgr variables effected by a reset must be included
   ** - Some of these may be more diagnostic but not enough to warrant a
   **   separate diagnostic. Also easier for the user not to hhave to command it.
   */

   LabTlmHkPkt.TlmSockId = (uint16)LabTlm.PktMgr.TlmSockId;
   strncpy(LabTlmHkPkt.TlmDestIp, LabTlm.PktMgr.TlmDestIp, PKTMGR_IP_STR_LEN);

   CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &LabTlmHkPkt);
   CFE_SB_SendMsg((CFE_SB_Msg_t *) &LabTlmHkPkt);

} /* End LABTLM_SendHousekeepingPkt() */


/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{
    int32 Status = CFE_SUCCESS;

    /*
    ** Initialize 'entity' objects
    */

    PKTMGR_Constructor(&LabTlm.PktMgr, PKTMGR_PIPE_NAME, PKTMGR_PIPE_DEPTH);

    /*
    ** Initialize application managers
    */

    TBLMGR_Constructor(&LabTlm.TblMgr, LABTLM_DEF_PKTTBL_FILE_NAME);

    CFE_SB_CreatePipe(&LabTlm.CmdPipe, CMDMGR_PIPE_DEPTH, CMDMGR_PIPE_NAME);
    CFE_SB_Subscribe(LABTLM_CMD_MID, LabTlm.CmdPipe);
    CFE_SB_Subscribe(LABTLM_SEND_HK_MID, LabTlm.CmdPipe);

    CMDMGR_Constructor(&LabTlm.CmdMgr);
    CMDMGR_RegisterFunc(LABTLM_CMD_RESET_FC,           LABTLM_ResetAppCmd,        0);
    CMDMGR_RegisterFunc(LABTLM_CMD_NOOP_FC,            LABTLM_NoOpCmd,            0);
    CMDMGR_RegisterFunc(LABTLM_CMD_PKT_TBL_LOAD_FC,    TBLMGR_LoadPktTable,      TBLMGR_LOAD_TBL_CMD_DATA_LEN);
    CMDMGR_RegisterFunc(LABTLM_CMD_PKT_TBL_DUMP_FC,    TBLMGR_DumpPktTable,      TBLMGR_DUMP_TBL_CMD_DATA_LEN);
    CMDMGR_RegisterFunc(LABTLM_CMD_ADD_PKT_FC,         PKTMGR_AddPktCmd,         PKKTMGR_ADD_PKT_CMD_DATA_LEN);
    CMDMGR_RegisterFunc(LABTLM_CMD_REMOVE_PKT_FC,      PKTMGR_RemovePktCmd,      PKKTMGR_REMOVE_PKT_CMD_DATA_LEN);
    CMDMGR_RegisterFunc(LABTLM_CMD_REMOVE_ALL_PKTS_FC, PKTMGR_RemoveAllPktsCmd,  0);
    CMDMGR_RegisterFunc(LABTLM_CMD_ENABLE_OUTPUT_FC,   PKTMGR_EnableOutputCmd,   PKKTMGR_ENABLE_OUTPUT_CMD_DATA_LEN);
    CMDMGR_RegisterFunc(LABTLM_CMD_SEND_DATA_TYPES_FC, LABTLM_SendDataTypeTlmCmd, 0);

    CFE_SB_InitMsg(&LabTlmHkPkt, LABTLM_TLM_HK_MID, LABTLM_TLM_HK_LEN, TRUE);
    InitDataTypePkt();

    /*
    ** Application startup event message
    */
    Status = CFE_EVS_SendEvent(LABTLM_INIT_APP_INFO_EID,
                               CFE_EVS_INFORMATION,
                               "TO-LAB Initialized. Version %d.%d.%d.%d",
                               LABTLM_MAJOR_VERSION,
                               LABTLM_MINOR_VERSION,
                               LABTLM_REVISION,
                               LABTLM_MISSION_REV);

    return(Status);

} /* End of InitApp() */

/******************************************************************************
** Function: InitDataTypePkt
**
*/
static void InitDataTypePkt(void)
{

   int16  i;
   char   StringVariable[10] = "ABCDEFGHIJ";

   CFE_SB_InitMsg(&ToDataTypePkt, LABTLM_TLM_DATA_TYPE_MID, LABTLM_TLM_DATA_TYPE_LEN, TRUE);

   ToDataTypePkt.synch = 0x6969;
   #if 0
      ToDataTypePkt.bit1    = 1;
      ToDataTypePkt.bit2    = 0;
      ToDataTypePkt.bit34   = 2;
      ToDataTypePkt.bit56   = 3;
      ToDataTypePkt.bit78   = 1;
      ToDataTypePkt.nibble1 = 0xA;
      ToDataTypePkt.nibble2 = 0x4;
   #endif
      ToDataTypePkt.bl1 = FALSE;
      ToDataTypePkt.bl2 = TRUE;
      ToDataTypePkt.b1  = 16;
      ToDataTypePkt.b2  = 127;
      ToDataTypePkt.b3  = 0x7F;
      ToDataTypePkt.b4  = 0x45;
      ToDataTypePkt.w1  = 0x2468;
      ToDataTypePkt.w2  = 0x7FFF;
      ToDataTypePkt.dw1 = 0x12345678;
      ToDataTypePkt.dw2 = 0x87654321;
      ToDataTypePkt.f1  = 90.01;
      ToDataTypePkt.f2  = .0000045;
      ToDataTypePkt.df1 = 99.9;
      ToDataTypePkt.df2 = .4444;

   for (i=0; i < 10; i++) ToDataTypePkt.str[i] = StringVariable[i];

} /* End InitDataTypePkt() */

/******************************************************************************
** Function: ProcessCommands
**
*/
static void ProcessCommands(void)
{

   int32           Status;
   CFE_SB_Msg_t*   CmdMsgPtr;
   CFE_SB_MsgId_t  MsgId;

   Status = CFE_SB_RcvMsg(&CmdMsgPtr, LabTlm.CmdPipe, CFE_SB_POLL);

   if (Status == CFE_SUCCESS)
   {

      MsgId = CFE_SB_GetMsgId(CmdMsgPtr);

      switch (MsgId)
      {
         case LABTLM_CMD_MID:
            CMDMGR_DispatchFunc(CmdMsgPtr);
            break;

         case LABTLM_SEND_HK_MID:
            LABTLM_SendHousekeepingPkt();
            break;

         default:
            CFE_EVS_SendEvent(LABTLM_INVALID_MID_ERR_EID, CFE_EVS_ERROR,
                              "Received invalid command packet,MID = 0x%4X",MsgId);

            break;

      } /* End Msgid switch */

   } /* End if SB received a packet */

} /* End ProcessCommands() */


/* end of file */
