
/*
 * Filename: to-lab_testrunner.c
 *
 * Purpose: Run the unit tests for the telemetry output lab application.
 *
 */

/*
 * Includes
 */

#include "uttest.h"

/*
 *  Text parsers text cases
 */

void CMDMGR_AddTestCase(void);
void TBLMGR_AddTestCase(void);
void PKTMGR_AddTestCase(void);

/*
 * Function Definitions
 */

int main(void)
{   

   CMDMGR_AddTestCase();
   TBLMGR_AddTestCase();
   PKTMGR_AddTestCase();

   printf("main: After test cases added and before ut run\n");

   return(UtTest_Run());

} /* End main() */
