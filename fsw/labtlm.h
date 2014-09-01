/*
** $Id: $
** 
** Purpose: Define the Lab Telemetry application. Thsi application receives telemetry packets
**          from the software bus and uses its packet table to determine whether packets
**          should be send over a UDP socket.
**
** Notes:
**   1.
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
#ifndef _labtlm_
#define _labtlm_

/*
** Includes
*/

#include "app_config.h"
#include "cmdmgr.h"
#include "tblmgr.h"
#include "pktmgr.h"

/*
** Macro Definitions
*/

#define LABTLM_INIT_APP_INFO_EID   (LABTLM_BASE_EID + 0)
#define LABTLM_NOOP_INFO_EID       (LABTLM_BASE_EID + 1)
#define LABTLM_EXIT_ERR_EID        (LABTLM_BASE_EID + 2)
#define LABTLM_INVALID_MID_ERR_EID (LABTLM_BASE_EID + 3)

#define LABTLM_TOTAL_EID  4


/*
** Type Definitions
*/

typedef struct
{

   CMDMGR_Class CmdMgr;
   TBLMGR_Class TblMgr;
   PKTMGR_Class PktMgr;

   CFE_SB_PipeId_t CmdPipe;

} LABTLM_Class;

typedef struct
{

   uint8    Header[CFE_SB_TLM_HDR_SIZE];

   /*
   ** CMDMGR Data
   */
   uint16   ValidCmdCnt;
   uint16   InvalidCmdCnt;

   /*
   ** TBLMGR Data
   */

   boolean  PktTblLoadActive;
   boolean  PktTblLastLoadValid;
   uint16   PktTblAttrErrCnt;

   /*
   ** PKTMGR Data
   */

   uint16   TlmSockId;
   char     TlmDestIp[PKTMGR_IP_STR_LEN];

} OS_PACK LABTLM_HkPkt;

#define LABTLM_TLM_HK_LEN sizeof (LABTLM_HkPkt)


typedef struct
{

   uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
   uint16             synch;
#if 0
   bit_field          bit1:1;
   bit_field          bit2:1;
   bit_field          bit34:2;
   bit_field          bit56:2;
   bit_field          bit78:2;

   bit_field          nibble1:4;
   bit_field          nibble2:4;
#endif
   uint8              bl1, bl2;       /* boolean */
   int8               b1, b2, b3, b4;
   int16              w1,w2;
   int32              dw1, dw2;
   float              f1, f2;
   double             df1, df2;
   char               str[10];

} OS_PACK LABTLM_DataTypePkt;

#define LABTLM_TLM_DATA_TYPE_LEN   sizeof (LABTLM_DataTypePkt)


/*
** Exported Data
*/

extern LABTLM_Class  LabTlm;


/*
** Exported Functions
*/

/******************************************************************************
** Function: LABTLM_Main
**
*/
void LABTLM_Main(void);


/******************************************************************************
** Function: LABTLM_NoOpCmd
**
*/
boolean LABTLM_NoOpCmd(const CFE_SB_MsgPtr_t MsgPtr);


/******************************************************************************
** Function: LABTLM_ResetAppCmd
**
*/
boolean LABTLM_ResetAppCmd(const CFE_SB_MsgPtr_t MsgPtr);


#endif /* _labtlm_ */
