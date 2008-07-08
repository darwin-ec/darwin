/* Wavelet Laboratory
 * Subband Filter I/O operations.
 *
 * Mike Hilton, 16 May 96
 * Fausto Espinal, 18 Jan. 97
 *
 * A subband filter is a compound object consisting of four filters:
 * the high and lowpass forward transform filters, and the high and
 * low inverse transform filters.  
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wlcore.h"


/* WL_SBFilterLoad
 *
 * Loads a subband filter from a data file.  
 */
int WL_SBFilterLoad (char *fileName, char *filterName, WL_SubbandFilter **newFilter)
{
  WL_SubbandFilter *filter;
  int length, rows, cols;
  dtype *data, *d;
  int i;
  int coef;

  /* read in the data */
  if (WL_ReadAsciiDataFile(fileName, &rows, &cols, &data) != WL_OK)
    return WL_ERROR;
  length = rows * cols;
  d = data;

  /* create a new filter */
  filter = (WL_SubbandFilter *) malloc(sizeof(WL_SubbandFilter));
  *newFilter = filter;
  if (filter == NULL) {
    WL_SetErrorMsg("WL_SBFilterLoad : Unable to malloc subband filter");
    return WL_ERROR;
  }
  filter->info.type = "sbfilter";
  filter->info.plist = NULL;

  /* copy filter name */
  filter->info.name = (char *) malloc(strlen(filterName) + 1);
  if (filter->info.name == NULL) {
    free((char *) filter);
    WL_SetErrorMsg("WL_SBFilterLoad : Unable to malloc subband filter");
    return WL_ERROR;
  }
  strcpy(filter->info.name, filterName);

  /* fill in the data for the four filters */
  for (i = 0; i < 4; i++) {
    /* is there header data for a filter ? */
    if (length - 2 < 0) {
      WL_SetErrorMsg("WL_SBFilterLoad : Bad descriptor file: not enough data");
      return WL_ERROR;
    }
    filter->filters[i].length = (int) (*d++ + 0.5);
    filter->filters[i].offset = (int) (*d++ + 0.5);
    length -= 2;
    /* is the offset legal? */
    if ((filter->filters[i].offset < 0) || 
	(filter->filters[i].offset > filter->filters[i].length)) {
      WL_SetErrorMsg("WL_SBFilterLoad : Bad descriptor file: not enough data");
      return WL_ERROR;
    }
    /* is there enough data left for this filter? */
    if (length < filter->filters[i].length) {
      WL_SetErrorMsg("WL_SBFilterLoad : Bad descriptor file: not enough data");
      return WL_ERROR;
    }

    /* store the filter coefficients into an array */
    filter->filters[i].coefs = 
      (double *) malloc(filter->filters[i].length * sizeof(double));
    if (filter->filters[i].coefs == NULL) {
      WL_SetErrorMsg("WL_SBFilterLoad : unable to malloc sbfilter");
      return WL_ERROR;
    }
    for (coef = 0; coef < filter->filters[i].length; coef++) {
      filter->filters[i].coefs[coef] = *d++;
      length--;
    }
  }
  free(data);
  return WL_OK;
}
