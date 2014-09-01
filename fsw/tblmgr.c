/* 
** File:
**   $Id: $
**
** Purpose: Provide a class to manage the application's tables.
**
** Notes
**   1. 
**
** References:
**   1.CFS Object-based Application Developers Guide.
**
**
** $Date: $
** $Revision: $
** $Log: $
*/

/*
** Include Files:
*/

#include <string.h>
#include "tblmgr.h"


/*
** Global File Data
*/

static TBLMGR_Class* TblMgr = NULL;

static int  TblEntryId;
static char TblFileBuff[TBLMGR_BUFFSIZE];  /* XML table file buffer could be large so don't put on the stack */

/*
** Local File Function Prototypes
*/

typedef void (*DumpTableFuncPtr)(int32 FileHandle);   /* File-specific dump table function */

/******************************************************************************
** Function: DumpTableToFile
**
** General purpose function that takes a file pointer to the table-specific
** processing.
**
*/
static boolean DumpTableToFile(const char* FileName, const char* FileDescr, DumpTableFuncPtr DumpTableFunc);


/******************************************************************************
** Function: ParseXmlFile
**
** General purpose XML file parser.  The Start and End element callback
** functions do the file-specific processing.
**
*/
static boolean ParseXmlFile(const char* FileName,
                            XML_StartElementHandler StartElementHandler,
                            XML_EndElementHandler   EndElementHandler);

/******************************************************************************
** Functions: DumpPktTable, PktTblStartElement, PktTblEndElement
**
** These functions provide the packet table specific processing for XML parsing
** and for dump file formatting.
**
*/
static void DumpPktTable(int32 FileHandle);
static void XMLCALL PktTblStartElement(void *data, const char *el, const char **attr);
static void XMLCALL PktTblEndElement(void *data, const char *el);


/******************************************************************************
** Function: TBLMGR_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void TBLMGR_Constructor(TBLMGR_Class* TblMgrPtr,
                        char* PktTblFilePathName)
{

   uint16 entry;
   TblMgr = TblMgrPtr;

   /*
    * Set all data and flags to zero.
    * AttrErrCnt - Incremented by XML parser
    */
   CFE_PSP_MemSet(&(TblMgr->PktTbl), 0, sizeof(TBLMGR_PktTbl));
   for (entry=0; entry < PKTMGR_TBL_MAX_ENTRY_ID; entry++)
      TblMgr->PktTbl.Local.Entry[entry].StreamId = PKTMGR_UNUSED_MSG_ID;

   TblMgr->PktTbl.LoadActive = TRUE;

   /* printf ("TBLMGR_Constructor() - before Parse MsgTbl\n"); */
   /* CFE_EVS_SendEvent(999,CFE_EVS_INFORMATION,"TBLMGR_Constructor() - before Parse PktTbl"); */

   if (ParseXmlFile(PktTblFilePathName, PktTblStartElement, PktTblEndElement) &&
       TblMgr->PktTbl.AttrErrCnt == 0)
   {

      /* printf ("TBLMGR_Constructor() - before Parse MsgTbl\n"); */
      TblMgr->PktTbl.LastLoadValid = (PKTMGR_LoadTable(&(TblMgr->PktTbl.Local)) == TRUE);

   } /* End if successful MSGTBL parse */

   TblMgr->PktTbl.LoadActive = FALSE;
 
} /* End TBLMGR_Constructor() */


/******************************************************************************
** Function: TBLMGR_ResetStatus
**
*/
void TBLMGR_ResetStatus()
{

   CFE_PSP_MemSet((void *)TblMgr, 0, sizeof(TBLMGR_Class));

} /* End TBLMGR_ResetStatus() */


/******************************************************************************
** Function: TBLMGR_LoadPktTable
**
*/
boolean TBLMGR_LoadPktTable(const CFE_SB_MsgPtr_t MsgPtr)
{
   const  TBLMGR_LoadTblCmd *LoadTblCmd = (const TBLMGR_LoadTblCmd *) MsgPtr;
   int    entry;


   /*
    * Set all data & flags to zero then set all StreamIds to UNUSED.
    * AttrErrCnt - Incremented by XML parser
    */
   CFE_PSP_MemSet(&TblMgr->PktTbl, 0, sizeof(TBLMGR_PktTbl));
   for (entry=0; entry < PKTMGR_TBL_MAX_ENTRY_ID; entry++)
      TblMgr->PktTbl.Local.Entry[entry].StreamId = PKTMGR_UNUSED_MSG_ID;

   TblMgr->PktTbl.LoadActive = TRUE;

   TblEntryId = 0;
   if (ParseXmlFile(LoadTblCmd->FileName, PktTblStartElement, PktTblEndElement) &&
       TblMgr->PktTbl.AttrErrCnt == 0)
   {

      if (LoadTblCmd->LoadType == TBLMGR_LOAD_TBL_REPLACE)
      {
         TblMgr->PktTbl.LastLoadValid = (PKTMGR_LoadTable(&(TblMgr->PktTbl.Local)) == TRUE);

      } /* End if replace entire table */
      else if (LoadTblCmd->LoadType == TBLMGR_LOAD_TBL_UPDATE)
      {
         TblMgr->PktTbl.LastLoadValid = TRUE;
         for (entry=0; entry < PKTMGR_TBL_MAX_ENTRY_ID; entry++)
         {

            if (TblMgr->PktTbl.Modified[entry])
            {
               if (!PKTMGR_LoadTableEntry(entry, &(TblMgr->PktTbl.Local.Entry[entry])))
                  TblMgr->PktTbl.LastLoadValid = FALSE;
            }

         } /* End pkt loop */

      } /* End if update individual records */
      else
      {
         CFE_EVS_SendEvent(TBLMGR_CMD_LOAD_TYPE_ERR_EID,CFE_EVS_ERROR,"TBLMGR: Invalid packet table command load type %d",LoadTblCmd->LoadType);
      }

   } /* End if successful parse */
   else
   {
      CFE_EVS_SendEvent(TBLMGR_CMD_LOAD_PARSE_ERR_EID,CFE_EVS_ERROR,"TBLMGR: Packet table command file parsing failed");
   }

   return TblMgr->PktTbl.LastLoadValid;

} /* End of TBLMGR_LoadPktTable() */


/******************************************************************************
** Function: TBLMGR_DumpPktTable
**
*/
boolean TBLMGR_DumpPktTable(const CFE_SB_MsgPtr_t MsgPtr)
{
   const  TBLMGR_DumpTblCmd *DumpTblCmd = (const TBLMGR_DumpTblCmd *) MsgPtr;
   char   FileName[OS_MAX_PATH_LEN];

   /* Copy the commanded filename into local buffer to ensure size limitation and to allow for modification */
   CFE_PSP_MemCpy(FileName, (void *)DumpTblCmd->FileName, OS_MAX_PATH_LEN);

   /* Check to see if a default filename should be used */
   if (FileName[0] == '\0')
   {
      strncpy(FileName, TBLMGR_DEF_PKT_TBL_DUMP_FILE, OS_MAX_PATH_LEN);
   }

   /* Make sure all strings are null terminated before attempting to process them */
   FileName[OS_MAX_PATH_LEN-1] = '\0';

   return DumpTableToFile(FileName,"Telemetry Output App's Packet Table",DumpPktTable);

} /* End of TBLMGR_DumpPktTable() */



/******************************************************************************
**
** PktTblStartElement
**
** Callback function for the XML parser. It assumes global variables have been
** properly setup before the parsing begins.
**
*/
static void XMLCALL PktTblStartElement(void *data, const char *el, const char **attr)
{

   int     i;
   uint16  ProcessedAttr = 0x0F;  /* 4 Attribute bits */
   uint16  StreamId    = 0;
   uint16  Priority    = 0;
   uint16  Reliability = 0;
   uint16  BuffLim     = 0;

   if (strcmp(el,TBLMGR_XML_EL_PKT_ENTRY) == 0)
   {

      for (i = 0; attr[i]; i += 2) {
         if (strcmp(attr[i],TBLMGR_XML_AT_STREAM_ID)==0) {
            /* printf("%s\n", attr[i + 1]); */
            StreamId = atoi(attr[i + 1]);
            ProcessedAttr &= 0xFE;
         } else if (strcmp(attr[i],TBLMGR_XML_AT_PRIORITY)==0)
         {
            Priority = atoi(attr[i + 1]);
            ProcessedAttr &= 0xFD;
         } else if (strcmp(attr[i],TBLMGR_XML_AT_RELIABILITY)==0)
         {
            Reliability = atoi(attr[i + 1]);
            ProcessedAttr &= 0xFB;
         } else if (strcmp(attr[i],TBLMGR_XML_AT_BUF_LIM)==0)
         {
            BuffLim = atoi(attr[i + 1]);
            ProcessedAttr &= 0xF7;
         }
      } /* End attribute loop */

      if (ProcessedAttr == 0)
      {
        /* TODO - Add error checking */
        TblMgr->PktTbl.Local.Entry[TblEntryId].StreamId        = StreamId;
        TblMgr->PktTbl.Local.Entry[TblEntryId].Qos.Priority    = Priority;
        TblMgr->PktTbl.Local.Entry[TblEntryId].Qos.Reliability = Reliability;
        TblMgr->PktTbl.Local.Entry[TblEntryId].BuffLim         = BuffLim;
        TblMgr->PktTbl.Modified[TblEntryId] = TRUE;
        /* printf ("TblMgr->PktTbl.Local.Entry[%d].StreamId = 0x%04X\n", TblEntryId, TblMgr->PktTbl.Local.Entry[TblEntryId].StreamId); */
        TblEntryId++;
      }
      else
      {
         TblMgr->PktTbl.AttrErrCnt++;
      }


  } /* End if PKTTBL_XML_EL_ENTRY found */

} /* End PktTblStartElement() */

static void XMLCALL PktTblEndElement(void *data, const char *el)
{

} /* End PktTblEndElement() */


/******************************************************************************
**
** DumpPktTable
**
** Dump the packet table contents to a file as text. The function signature
** must correspond to the DumpTableFuncPtr type.
**
*/
static void DumpPktTable(int32 FileHandle)
{
   const  PKTMGR_Table  *PktTblPtr;
   char   DumpRecord[256];
   int    i;

   sprintf(DumpRecord,"\nEntry: Stream ID, Buffer Limit, QoS (Priority,Reliability)\n");
   OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

   PktTblPtr = PKTMGR_GetTblPtr();

   for (i=0; i < PKTMGR_TBL_MAX_ENTRY_ID; i++)
   {
      sprintf(DumpRecord,"%03d: 0x%04X, %03d, (%d,%d)\n",
              i, PktTblPtr->Entry[i].StreamId,PktTblPtr->Entry[i].BuffLim,
              PktTblPtr->Entry[i].Qos.Priority, PktTblPtr->Entry[i].Qos.Reliability);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
   }

} /* End DumpPktTable() */


/******************************************************************************
**
** ParseXmlFile
**
** Parse an XML file
*/
static boolean ParseXmlFile(const char* FilePathName,
                            XML_StartElementHandler StartElementHandler,
                            XML_EndElementHandler   EndElementHandler)
{

   int      FileHandle;
   int32    ReadStatus;
   boolean  Done;
   boolean  ParseErr  = FALSE;
   boolean  RetStatus = FALSE;

   XML_Parser XmlParser = XML_ParserCreate(NULL);
   if (! XmlParser) {
      CFE_EVS_SendEvent(TBLMGR_CREATE_ERR_EID, CFE_EVS_ERROR, "Failed to allocate memory for XML parser");
   }
   else {

      XML_SetElementHandler(XmlParser, StartElementHandler, EndElementHandler);

      FileHandle = OS_open(FilePathName, OS_READ_ONLY, 0);
      
      if (FileHandle >= 0) {

         Done = FALSE;

         while (!Done) {

            ReadStatus = OS_read(FileHandle, TblFileBuff, TBLMGR_BUFFSIZE);

            if ( ReadStatus == OS_FS_ERROR )
            {
               CFE_EVS_SendEvent(TBLMGR_FILE_READ_ERR_EID, CFE_EVS_ERROR, "File read error, EC = 0x%08X",ReadStatus);
               Done = TRUE;
               ParseErr = TRUE;
            }
            else if ( ReadStatus == 0 ) /* EOF reached */
            {
                Done = TRUE;
            }
            else {

               /* ReadStatus contains number of bytes read */
               if (XML_Parse(XmlParser, TblFileBuff, ReadStatus, Done) == XML_STATUS_ERROR) {

                  CFE_EVS_SendEvent(TBLMGR_PARSE_ERR_EID, CFE_EVS_ERROR, "Parse error at line %l, error code = %s",
                                    XML_GetCurrentLineNumber(XmlParser),
                                    XML_ErrorString(XML_GetErrorCode(XmlParser)));
                  Done = TRUE;
                  ParseErr = TRUE;

               } /* End if valid parse */
            } /* End if valid fread */
         } /* End file read loop */

         RetStatus = !ParseErr;
         
         OS_close(FileHandle);

      } /* End if file opened */
      else
      {
      
          CFE_EVS_SendEvent(TBLMGR_FILE_OPEN_ERR_EID, CFE_EVS_ERROR, "File open error for %s, Error = %d",
                            FilePathName, FileHandle );
      }

      XML_ParserFree(XmlParser);

   } /* end if parser allocated */

   return RetStatus;

} /* End ParseXmlFile() */


/******************************************************************************
** Function: DumpTableToFile
**
*/
static boolean DumpTableToFile(const char* FileName, const char* FileDescr, DumpTableFuncPtr DumpTableFunc)
{

   CFE_FS_Header_t  CfeStdFileHeader;
   int32            FileHandle;
   int32            FileStatus;
   boolean          RetStatus = FALSE;

   /* Create a new dump file, overwriting anything that may have existed previously */
   FileHandle = OS_creat(FileName, OS_WRITE_ONLY);

   if (FileHandle >= OS_FS_SUCCESS)
   {
      /* Initialize the standard cFE File Header for the Dump File */
      CfeStdFileHeader.SubType = 0x74786574;
      strcpy(&CfeStdFileHeader.Description[0], FileDescr);

      /* Output the Standard cFE File Header to the Dump File */
      FileStatus = CFE_FS_WriteHeader(FileHandle, &CfeStdFileHeader);

      if (FileStatus == sizeof(CFE_FS_Header_t))
      {
         (DumpTableFunc)(FileHandle);
         RetStatus = TRUE;

      } /* End if successfully wrote file header */
      else
      {
          CFE_EVS_SendEvent(TBLMGR_WRITE_CFE_HDR_ERR_EID,
                            CFE_EVS_ERROR,
                            "Error writing cFE File Header to '%s', Status=0x%08X",
                            FileName, FileStatus);

      }
   } /* End if file create */
   else
   {

        CFE_EVS_SendEvent(TBLMGR_CREATE_MSG_DUMP_ERR_EID,
                          CFE_EVS_ERROR,
                          "Error creating CDS dump file '%s', Status=0x%08X",
                          FileName, FileHandle);

    }

   OS_close(FileHandle);

   return RetStatus;

} /* End of DumpTableToFile() */


/* end of file */
