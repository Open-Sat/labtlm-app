/*
** $Id: $
** 
** Purpose: Define platform configurations for the Lab Telemetry Output application
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

#ifndef _labtlm_platform_cfg_
#define _labtlm_platform_cfg_

/*
** Includes
*/

/******************************************************************************
** Lab Telemetry Output Application Macros
*/

#define  LABTLM_RUNLOOP_DELAY    500  /* Delay in milliseconds */

#define  LABTLM_TLM_PORT     1235

#define  LABTLM_DEF_PKTTBL_FILE_NAME "/cf/labtlm_pkttbl.xml"

#define  LABTLM_CMD_MID           0x1880
#define  LABTLM_SEND_HK_MID       0x1881
#define  LABTLM_TLM_HK_MID        0x0880
#define  LABTLM_TLM_DATA_TYPE_MID 0x0881

/******************************************************************************
** Uplink object Macros
*/

#define  UPLINK_RECV_BUFF_LEN  512

#endif /* _labtlm_platform_cfg_ */
