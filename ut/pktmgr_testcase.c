/*
 * File:
 *   $Id: $
 *
 * Purpose: This file contains a unit test cases for pktmgr.c
 *
 * $Date: $
 * $Revision: $
 * $Log: $
 */

/*
 * Includes
 */

#include "common_types.h"
#include "pktmgr.h"

#include "uttest.h"
#include "utassert.h"
#include "ut_cfe_sb_stubs.h"
#include "ut_cfe_sb_hooks.h"


#define TEST_PIPE_NAME   "Test Pipe"
#define TEST_PIPE_DEPTH   2
#define TEST_PKT_TBL     "pkttbl-lab.xml"

static PKTMGR_Class  UtPktMgr;
static PKTMGR_Table  UtPktTbl;

/*
** Function Definitions
*/

/*******************************************************************
** Function: LoadTestTable
**
*/
static void LoadTestTable(void)
{

   int i;
   PKTMGR_TblEntry *Entry;


   for (i=0; i < PKTMGR_TBL_MAX_ENTRY_ID; i++)
   {
      Entry = &(UtPktTbl.Entry[i]);
      CFE_PSP_MemSet(Entry, 0, sizeof(PKTMGR_TblEntry));
      Entry->StreamId = PKTMGR_UNUSED_MSG_ID;

   } /* End Entry Loop */

   UtPktTbl.Entry[0].StreamId        = 1;
   UtPktTbl.Entry[0].Qos.Reliability = 2;
   UtPktTbl.Entry[0].Qos.Priority    = 3;
   UtPktTbl.Entry[0].BuffLim         = 4;

   UtPktTbl.Entry[1].StreamId        = 11;
   UtPktTbl.Entry[1].Qos.Reliability = 12;
   UtPktTbl.Entry[1].Qos.Priority    = 13;
   UtPktTbl.Entry[1].BuffLim         = 14;

   UtPktTbl.Entry[PKTMGR_TBL_MAX_ENTRY_ID/2].StreamId        = PKTMGR_TBL_MAX_ENTRY_ID/2 + 1;
   UtPktTbl.Entry[PKTMGR_TBL_MAX_ENTRY_ID/2].Qos.Reliability = PKTMGR_TBL_MAX_ENTRY_ID/2 + 2;
   UtPktTbl.Entry[PKTMGR_TBL_MAX_ENTRY_ID/2].Qos.Priority    = PKTMGR_TBL_MAX_ENTRY_ID/2 + 3;
   UtPktTbl.Entry[PKTMGR_TBL_MAX_ENTRY_ID/2].BuffLim         = PKTMGR_TBL_MAX_ENTRY_ID/2 + 4;

   UtPktTbl.Entry[PKTMGR_TBL_MAX_ENTRY_ID-1].StreamId        = PKTMGR_TBL_MAX_ENTRY_ID + 1;
   UtPktTbl.Entry[PKTMGR_TBL_MAX_ENTRY_ID-1].Qos.Reliability = PKTMGR_TBL_MAX_ENTRY_ID + 2;
   UtPktTbl.Entry[PKTMGR_TBL_MAX_ENTRY_ID-1].Qos.Priority    = PKTMGR_TBL_MAX_ENTRY_ID + 3;
   UtPktTbl.Entry[PKTMGR_TBL_MAX_ENTRY_ID-1].BuffLim         = PKTMGR_TBL_MAX_ENTRY_ID + 4;

} /* End LoadTestTable() */

/* PKTMGR_Test01  - Constructor, ResetStatus, GetTblPtr */
void PKTMGR_Test01(void)
{
   const PKTMGR_Table* PktTblPtr = NULL;

   printf("PKTMGR_Test01()\n");

   /* Constructor called as part of setup */
   UtAssert_True(UtPktMgr.Table.Entry[0].StreamId == 0, "UtPktMgr.Table.Entry[0].StreamId == 0");

   PktTblPtr = PKTMGR_GetTblPtr();
   UtAssert_True(PktTblPtr == &(UtPktMgr.Table), "PktTbl.Table");

   PKTMGR_Constructor (&UtPktMgr, TEST_PIPE_NAME, TEST_PIPE_DEPTH);

} /* End PKTMGR_Test01() */


/* PKTMGR_Test02  - LoadTable, LoadTableEntry */
void PKTMGR_Test02(void)
{

   printf("PKTMGR_Test02()\n");

} /* End PKTMGR_Test02() */

/* PKTMGR_Test03  - Functional  */
void PKTMGR_Test03(void)
{

   printf("PKTMGR_Test03()\n");

   /* TODO - Functional test of how PKTMGR is used in TO Lab */

} /* End PKTMGR_Test03() */

/* Initialize test environment to default state for every test */
void PKTMGR_Setup(void)
{
   Ut_CFE_SB_SetReturnCode(UT_CFE_SB_RCVMSG_INDEX, CFE_SB_NO_MESSAGE, 1000);  /* Avoid infinite flush loop for now */
   PKTMGR_Constructor (&UtPktMgr, TEST_PIPE_NAME, TEST_PIPE_DEPTH);
   LoadTestTable();
}

void PKTMGR_TearDown(void)
{
    /* cleanup test environment */
}

void PKTMGR_AddTestCase(void)
{
    UtTest_Add(PKTMGR_Test01, PKTMGR_Setup, PKTMGR_TearDown, "PKTMGR_Test01 - Constructor, ResetStatus, GetTblPtr");
    UtTest_Add(PKTMGR_Test02, PKTMGR_Setup, PKTMGR_TearDown, "PKTMGR_Test02 - LoadTable, LoadTableEntry");
}
