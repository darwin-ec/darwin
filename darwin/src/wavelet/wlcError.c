/* Wavelet Laboratory
 * Error Message Handling operations.
 *
 * Fausto Espinal, 18 Jan. 97
 */
#include <stdio.h>
#include "wlcore.h"


char *errMsg = NULL;

/* WL_SetErrorMsg
 *
 * Sets the value of errMsg to string.  
 */
void WL_SetErrorMsg(char *errString)
{
    errMsg = errString;
}

/* WL_GetErrorMsg
 *
 * Gets the value of errMsg.  
 */
char *WL_GetErrorMsg(char *errString)
{
    return (errMsg);
}
