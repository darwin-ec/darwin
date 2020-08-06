/* Wavelet Laboratory Utilities
 *
 * Mike Hilton, 22 March 1995
 * Fausto Espinal, 18 Jan. 1997
 *
 * This file contains utility functions that support the base Wavelet
 * Laboratory functionality.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "wlcore.h"

/* WL_DtypeCmp
 *
 * Compares two values of type Dtype, returning 1 if A > B,
 * 0 if A == B, or -1 if A < B.  This function is suitable for
 * use with the C-library function qsort.
 */

int WL_DtypeCmp(const void *a, const void *b)   
{                                 
  dtype x = *((dtype *) a);
  dtype y = *((dtype *) b);

  if (x < y) return(-1);
  if (x == y) return(0);
  return(1);
}

/* WL_isDyadic
 *
 * Tests to see if X is a power of 2.  Returns TRUE or FALSE.
 */
int WL_isDyadic(int x)
{
  if (WL_pow2(WL_log2(x)) != x) return 0;
  else return 1;
}


/* WL_log2
 *
 * Computes the largest base 2 logarithm less than or equal to LOG2(X).
 */
int WL_log2(int x)
{
  int n = 0;
  int prod = 1;

  while (prod <= x) {
    n++;
    prod *= 2;
  }
  return(n-1);
}


/* WL_pow2
 *
 * Computes 2 raised to the Xth power.
 */
int WL_pow2(int x)
{
  int n = 1;

  if (x < 0) {
    fprintf(stderr, "\nWL_pow2: negative argument! Returning 0\n");
    return(0);
  }
  while (x-- > 0) n *= 2;
  return(n);
}

/* WL_Swap
 *
 * Swaps to floating point values.     
 */
void WL_Swap(dtype *a, dtype *b)
{
  dtype tmp;

  tmp = *a;
  *a  = *b;
  *b  = tmp;
}

/* WL_ReadAsciiDataFile
 *
 * Reads in the contents of an ascii data file.  The data values may be
 * preceeded by any number of comment lines that begin with the
 * character '#'.
 *
 * Data files represent either vectors or matrices.  If there is more than
 * one value on the first line of data, then the number of items on that
 * first line is taken to be the number of columns of data in a matrix.
 * If the total number of data items read is not evenly divisible by
 * the number of columns detected, the file is assumed to be a vector.
 */
int WL_ReadAsciiDataFile(char *filename, int *rows, 
			 int *cols, dtype **data)
{
  int arraySize;         /* maximum size of filter array */
  char c;                /* temp */
  dtype *values;         /* array of data being read in */
  int i;                 /* loop index */
  FILE *in;              /* file being read */
  int scanned;           /* number of items successfully read in */
  long start;            /* location in file where data begins */
  char temp[128];        /* string used in reading garbage lines */
  int tempsize = 128;    /* length of TEMP string */


  /* open the data file */
  if ((in = fopen(filename, "r")) == NULL){
      WL_SetErrorMsg("unable to open file for reading");
      return WL_ERROR;
  }

  /* throw away any comments that may be at top of file */
  while ((c = getc(in)) == '#') 
    while (!feof(in) && ((c = getc(in)) != '\n'));

  /* advance to first nonspace char */
  while (isspace(c) && !feof(in)) c = getc(in);
  ungetc(c, in);
  /* remember where data begins */
  start = ftell(in);

  /* determine the number of columns per line in the file */
  *cols = 0;
  c = getc(in);
  while (!feof(in)) {
    /* read in whitespace */
    while (isspace(c) && !feof(in) && (c != '\n')) c = getc(in);
    
    /* check for newline */
    if (c == '\n') break;

    /* read in non-whitespace sequence */
    while (!isspace(c) && !feof(in)) c = getc(in);
    (*cols)++;
  }
  /* go back to where data began */
  fseek(in, start, SEEK_SET);

  /* create the initial data array */
  i = 0;
  arraySize = 1024;
  values = (dtype *) malloc(arraySize * sizeof(dtype));
  if (values == NULL){
    fclose(in);
    free((char *) values);
    WL_SetErrorMsg("WL_ReadAsciiDataFile: unable to malloc data array");
    return WL_ERROR;
  }

  /* read in the data */
  while (!feof(in)){
    scanned = fscanf(in, DTYPE_READ_FORMAT, values+i);
    if (scanned != 1){
      fgets(temp, tempsize, in);  /* throw away line */
      if ((temp[0] != '#') && !feof(in)) {
	fclose(in);
	free((char *) values);
        WL_SetErrorMsg("WL_ReadAsciiDataFile: unexpected string in data file");
	return WL_ERROR;
      }
    }
    else i++;
  
    /* double the size of data array if neccessary */
    if (i == arraySize){
      arraySize *= 2;
      values = (dtype *) realloc((char *) values, arraySize * sizeof(dtype));
      if (values == NULL){
	fclose(in);
	free((char *) values);
        WL_SetErrorMsg("WL_ReadAsciiDataFile: unable to malloc data array");
	return WL_ERROR;
      }
    }
  }
  fclose(in);


  /* check to see if the file could contain a matrix */
  if ((i % *cols) != 0) { 
    *cols = 1;
    *rows = i;
  }
  else *rows = i / *cols;

  *data = values;
  return WL_OK;
}

/* WL_ReadPbmFile
 *
 * Given a FILE name, open the file and, if the file is a PBM file,
 * read its contents into a malloc'ed array of dtype.  The size of the 
 * image is returned in the parameters ROWS, and COLS. 
 *
 */
int WL_ReadPbmFile(char *file, int *rows, int *cols,
		   dtype **data)
{
  char  pmode, c, firstchar;
  int   size;
  FILE  *fp;
  dtype *d;
  int row, col;
  int mask;

  /* try to open the file */
  if ((fp = fopen(file, "rb")) == NULL){
      WL_SetErrorMsg("WL_ReadPbmFile: unable to open file for reading.");
      return WL_ERROR;
  }

  /* Check for the correct magic number */
  fscanf(fp, "%c%c\n", &firstchar, &pmode);
  if ((firstchar != 'P') || ((pmode != '4') && (pmode != '1'))) {
      WL_SetErrorMsg("WL_ReadPbmFile: file is not a PBM image file");
      return WL_ERROR;
  }

  /* throw out any comments */
  while ((c = getc(fp)) == '#') 
    while (!feof(fp) && ((c = getc(fp)) != '\n'));
  ungetc(c, fp);
  if (feof(fp)) {
      WL_SetErrorMsg("WL_ReadPbmFile: problems reading PBM file.");
      return WL_ERROR;
  }
  
  /* read the header information */
  fscanf(fp,"%d %d%c", cols, rows, &c);
  size = (*rows) * (*cols);

  /* create the resulting data array */
  *data = (dtype *) malloc(sizeof(dtype) * size);
  if (*data == NULL) {
      WL_SetErrorMsg("WL_ReadPbmFile: unable to malloc image in file.");
      return WL_ERROR;
  }
  d = *data;


  if (pmode == '4') {
    /* pbm raw binary file */
    for (row = 0; row < *rows; row++) {
      col = 0;
      while (col < *cols) {
	int byte = getc(fp);
	if (feof(fp)) {
	  fclose(fp);
	  free((char *) *data);
          WL_SetErrorMsg("WL_ReadPbmFile: Unexpected end of file!");
	  return WL_ERROR;
	}
	for (mask = 0x80; mask != 0; mask >>= 1) {
	  *d++ = (dtype)(mask & byte ? 1 : 0);
	  if (++col >= *cols) break;
	}
      }
    }
    fclose(fp);
  } else {
    while (size-- > 0) {
      int temp;
      char tmpstr[512];
      if (fscanf(fp, "%d", &temp) != 1) {
	/* check to see if encountering a comment */
	fgets(tmpstr, 512, fp);  /* gobble up line */
	if ((tmpstr[0] != '#') && !feof(fp)) {
	  fclose(fp);
	  free((char *) *data);
          WL_SetErrorMsg("WL_ReadPbmFile: unexpected string in data file.");
	  return WL_ERROR;
	}
      }
      else *d++ = (dtype) temp;
    }
    fclose(fp);
  }

  return WL_OK;

}

/* WL_ReadPgmFile
 *
 * Given a FILE name, open the file and, if the file is a PGM file,
 * read its contents into a malloc'ed array of dtype.  The size of the 
 * image is returned in the parameters ROWS, and COLS. 
 */
int WL_ReadPgmFile(char *file, int *rows, int *cols,
		   dtype **data)
{
  char  pmode, c, firstchar;
  int   msize, size;
  FILE  *fp;
  char unsigned *raw, *r;
  size_t bytes_read;
  dtype *d;


  /* try to open the file */
  if ((fp = fopen(file, "rb")) == NULL){
      WL_SetErrorMsg("WL_ReadPgmFile: unable to open file for reading.");
      return WL_ERROR;
  }

  /* Check for the correct magic number */
  fscanf(fp, "%c%c\n", &firstchar, &pmode);
  if ((firstchar != 'P') || ((pmode != '5') && (pmode != '2'))) {
      WL_SetErrorMsg("WL_ReadPgmFile: file is not a PGM image file");
      return WL_ERROR;
  }

  /* throw out any comments */
  while ((c = getc(fp)) == '#') 
    while (!feof(fp) && ((c = getc(fp)) != '\n'));
  ungetc(c, fp);
  if (feof(fp)) {
      WL_SetErrorMsg("WL_ReadPgmFile: problems reading PGM file.");
      return WL_ERROR;
  }
  
  /* read the header information */
  fscanf(fp,"%d %d%c", cols, rows, &c);
  fscanf(fp, "%d%c", &msize, &c);

  /* read in data */
  size = (*rows) * (*cols);
  raw = (unsigned char *) malloc(size);
  if (raw == NULL){
      WL_SetErrorMsg("WL_ReadPgmFile: unable to malloc image in file.");
      return WL_ERROR;
  }

  if (pmode == '5') {
    /* pgm raw binary file */
    bytes_read = fread(raw, sizeof(unsigned char), size, fp);
    fclose(fp);
    if ((int)bytes_read != size){
        WL_SetErrorMsg("WL_ReadPgmFile: problems reading image in file.");
        free(raw);
        return WL_ERROR;
    }
    /* convert the raw character stream into a dtype array */
    *data = (dtype *) malloc(sizeof(dtype) * size);
    if (*data == NULL) {
        WL_SetErrorMsg("WL_ReadPgmFile: unable to malloc image in file.");
        free(raw);
        return WL_ERROR;
    }
    d = *data;
    r = raw;
    while (size-- > 0) *d++ = (dtype) *r++;
    free(raw);
  } else {
    *data = (dtype *) malloc(sizeof(dtype) * size);
    if (*data == NULL) {
        WL_SetErrorMsg("WL_ReadPgmFile: unable to malloc image in file.");
        free(raw);
        return WL_ERROR;
    }
    d = *data;

    while (size-- > 0) {
      int temp;
      char tmpstr[512];
      if (fscanf(fp, "%d", &temp) != 1) {
	/* check to see if encountering a comment */
	fgets(tmpstr, 512, fp);  /* gobble up line */
	if ((tmpstr[0] != '#') && !feof(fp)) {
	  fclose(fp);
	  free((char *) *data);
          WL_SetErrorMsg("WL_ReadPgmFile: unexpected string in data file.");
	  return WL_ERROR;
	}
      }
      else *d++ = (dtype) temp;
    }
    fclose(fp);
  }

  return WL_OK;

}

/* WL_ReadPpmFile
 *
 * Given a FILE name, open the file and, if the file is a PPM file,
 * read its contents into a malloc'ed array of dtype.  The size of the 
 * image is returned in the parameters ROWS, and COLS. 
 */
int WL_ReadPpmFile(char *file, int *rows, int *cols,
		   dtype **data)
{
  char  pmode, c, firstchar;
  int   msize, size;
  FILE  *fp;
  int p, row, col;

  /* try to open the file */
  if ((fp = fopen(file, "rb")) == NULL){
      WL_SetErrorMsg("WL_ReadPpmFile: unable to open file.");
      return WL_ERROR;
  }

  /* Check for the correct magic number */
  fscanf(fp, "%c%c\n", &firstchar, &pmode);
  if ((firstchar != 'P') || ((pmode != '3') && (pmode != '6'))) {
      WL_SetErrorMsg("WL_ReadPpmFile: file is not a PPM image file.");
      return WL_ERROR;
  }

  /* throw out any comments */
  while ((c = getc(fp)) == '#') 
    while (!feof(fp) && ((c = getc(fp)) != '\n'));
  ungetc(c, fp);
  if (feof(fp)) {
      WL_SetErrorMsg("WL_ReadPpmFile: problems reading PGM file.");
      return WL_ERROR;
  }
  
  /* read the header information */
  fscanf(fp,"%d %d%c", cols, rows, &c);
  fscanf(fp, "%d%c", &msize, &c);
  size = (*rows) * (*cols) * 3;

  /* create the result data array */
  *data = (dtype *) malloc(sizeof(dtype) * size);
  if (*data == NULL) {
      WL_SetErrorMsg("WL_ReadPpmFile: unable to malloc image in file.");
      return WL_ERROR;
  }

  if (pmode == '6') {
    /* pgm raw binary file */
    for (row = 0; row < *rows; row++)
      for (col = 0; col < *cols; col++)
	for (p = 0; p < 3; p++) {
	  int x = getc(fp);
	  if (feof(fp)) {
	    fclose(fp);
	    free((char *) *data);
            WL_SetErrorMsg("WL_ReadPpmFile: Unexpected end of file!");
	    return WL_ERROR;
	  }
	  (*data)[p*(*rows)*(*cols)+row*(*cols)+col] = x;
	}
    fclose(fp);
  } else {
    /* ascii ppm file */
    for (row = 0; row < *rows; row++)
      for (col = 0; col < *cols; col++)
	for (p = 0; p < 3; p++) {
	  int temp;
	  char tmpstr[512];
	  if (fscanf(fp, "%d", &temp) != 1) {
	    /* check to see if encountering a comment */
	    fgets(tmpstr, 512, fp);  /* gobble up line */
	    if ((tmpstr[0] != '#') && !feof(fp)) {
	      fclose(fp);
	      free((char *) *data);
              WL_SetErrorMsg("WL_ReadPpmFile: unexpected string in data file.");
	      return WL_ERROR;
	    }
	  }
	  else (*data)[p*(*rows)*(*cols)+row*(*cols)+col] = temp;
	}
    fclose(fp);
  }

  return WL_OK;
}

/* WL_Read16uDataFile
 *
 * Reads the contents of a file containing 16-bit unsigned integer values.
 */
int WL_Read16uDataFile(char *fname, int *n, dtype **data)
{
  unsigned char buf[2];  /* buffer where 16 bit values are read */
  unsigned long fsize;   /* size of the input data file, in bytes */
  int i;                 /* loop index */
  FILE *in;              /* file being read */
  dtype *values;         /* array of data being returned */

  /* open the data file */
  if ((in = fopen(fname, "r")) == NULL) {
      WL_SetErrorMsg("unable to open file for reading.");
      return WL_ERROR;
  }

  /* find out how long the file is */
  fseek(in, 0L, SEEK_END);
  fsize = ftell(in);
  rewind(in);
  *n = fsize/2;

  /* allocate an array to hold the data */
  values = (dtype *) malloc(*n * sizeof(dtype));
  if (values == NULL){
    fclose(in);
    WL_SetErrorMsg("WL_Read16uDataFile: unable to malloc data array");
    return WL_ERROR;
  }

  for (i = 0; i < *n; i++) {
    fread((void *)buf, (size_t) 1, (size_t) 2, in);
    values[i] = buf[0] * 256 + buf[1];
  }
  fclose(in);
  *data = values;
  return WL_OK;
}

/* WL_StripFilename
 *
 * Strips the directories and suffix off of a filename.
 */
void WL_StripFilename(char *infile, char *outfile)
{ 
  char *dot;
  char *slash;

  /* find rightmost '/' in infile */
  slash = strrchr(infile, '/');
  if (slash == NULL) slash = infile;
  else slash++;
  strcpy(outfile, slash);

  /* find rightmost period in outfile */
  dot = strrchr(outfile, '.');
  if (dot != NULL) *dot = '\0';
}

/* WL_WriteAsciiDataFile
 *
 * Write DATA to a file named FILENAME.  Each row is written on a
 * separate line.
 */
int WL_WriteAsciiDataFile(char *filename, dtype *data,
		int rows, int cols)
{
  FILE *out;
  int c;

  if ((out = fopen(filename, "w")) == NULL) {
      WL_SetErrorMsg("unable to open file.");
      return WL_ERROR;
  }

  while (rows-- > 0) {
    c = cols;
    while (c-- > 0) fprintf(out, DTYPE_PRINT_FORMAT " ", *data++);
    fprintf(out, "\n");
  }

  fclose(out);
  return WL_OK;
}
/* WL_WriteGnuplotDataFile
 *
 * Write DATA to a file named FILENAME in a format that Gnuplot can 
 * understand.
 */
int WL_WriteGnuplotDataFile(char *filename, dtype *data,
		int rows, int cols)
{
  FILE *out;
  int c;

  if ((out = fopen(filename, "w")) == NULL) {
      WL_SetErrorMsg("WL_WriteGnuplotDataFile : Unable to open file for output.");
      return WL_ERROR;
  }

  while (rows-- > 0) {
    c = cols;
    while (c-- > 0) fprintf(out, DTYPE_PRINT_FORMAT "\n", *data++);
    fprintf(out, "\n");
  }

  fclose(out);
  return WL_OK;
}

/* WL_WritePbmFile
 *
 * Write out IMAGE to the file named FILENAME, as a PBM image.
 * IMAGE is a matrix of dtype whose size is indicated by ROWS and COLS.
 */
int WL_WritePbmFile(char *filename, dtype *data, 
		    int rows, int cols)
{
  FILE *fp;
  int c, r;
  unsigned char mask, byte;

  /* Open the file */
  if ((fp = fopen(filename, "wb")) == NULL) {
      WL_SetErrorMsg("WL_WritePbmFile : Unable to open file for writing.");
      return WL_ERROR;
  }

  /* write out magic number for PBM files */
  fprintf(fp, "P4\n");

  /* write out header info */
  fprintf(fp, "%d %d\n", cols, rows);

  /* write out data */
  for (r = 0; r < rows; r++) {
    c = 0;
    while (c < cols) {
      byte = 0;
      for (mask = 0x80; mask != 0; mask >>= 1) {
	if (*data++ != 0) byte |= mask;
	if (++c >= cols) break;
      }
      putc(byte, fp);
    }
  }
  fclose(fp);
  return WL_OK;
}

/* WL_WritePgmFile
 *
 * Write out IMAGE to the file named FILENAME, as a PGM image.
 * IMAGE is a matrix of dtype whose size is indicated by ROWS and COLS.
 */
int WL_WritePgmFile(char *filename, dtype *data, 
		    int rows, int cols)
{
  FILE *fp;
  size_t size;

  /* Open the file */
  if ((fp = fopen(filename, "wb")) == NULL) {
      WL_SetErrorMsg("WL_WritePgmFile : Unable to open file for writing.");
      return WL_ERROR;
  }

  /* write out magic number for PGM files */
  fprintf(fp, "P5\n");

  /* write out header info */
  fprintf(fp, "%d %d\n", cols, rows);
  fprintf(fp, "255\n");

  /* write out data */
  size = rows * cols;
  while (size-- > 0) {
    unsigned char c;
    c = (unsigned char) (*data + 0.5);
    if (*data > 255) c = 255;
    else if (*data < 0) c = 0;
    data++;
    fputc(c, fp);
  }
  fclose(fp);
  return WL_OK;
}

/* WL_WritePpmFile
 *
 * Write out IMAGE to the file named FILENAME, as a PPM image.
 * IMAGE is a volume of dtype whose size is 3 x ROWS x COLS.
 */
int WL_WritePpmFile(char *filename, dtype ***data, 
		    int rows, int cols)
{
  FILE *fp;
  int r, c, p;

  /* Open the file */
  if ((fp = fopen(filename, "wb")) == NULL) {
      WL_SetErrorMsg("WL_WritePpmFile : Unable to open file for writing.");
      return WL_ERROR;
  }

  /* write out magic number for PPM files */
  fprintf(fp, "P6\n");

  /* write out header info */
  fprintf(fp, "%d %d\n", cols, rows);
  fprintf(fp, "255\n");

  /* write out data */
  for (r = 0; r < rows; r++)
    for (c = 0; c < cols; c++)
      for (p = 0; p < 3; p++) {
	int x = (int) (data[p][r][c] + 0.5);
	unsigned char c;
	if (x > 255) x = 255;
	else if (x < 0) x = 0;
	c = (unsigned char) x;
	fputc(c, fp);
      }

  fclose(fp);
  return WL_OK;
}

/* WL_Write16uDataFile
 *
 * Writes the contents of array DATA to a file in 16 bit unsigned integer
 * format.
 */
int WL_Write16uDataFile (char *fname, int n, dtype *data)
{
  FILE *out;

  /* open the data file */
  if ((out = fopen(fname, "wb")) == NULL){
      WL_SetErrorMsg("WL_Write16uDataFile: unable to open file for writing.");
      return WL_ERROR;
  }

  while (n-- > 0) {
    dtype msb, lsb, temp;
    temp = floor(*data + 0.5);
    msb = floor(*data / 256);
    lsb = *data - msb * 256;
    if (*data <= 0) {msb = lsb = 0;}
    putc((unsigned char)msb, out);
    putc((unsigned char)lsb, out);
    data++;
  }
  
  fclose(out);
  return WL_OK;
}

/***************************************************************************
 *
 *   TEMPORARY MATRIX UTILITIES
 *
 ***************************************************************************
 *
 * A temporary matrix is a two-dimensional array that is not a full
 * WL_Matrix object.  They are slightly more lightweight than WL_Matrix.
 */


/* WL_FreeTempMatrix
 *
 * Frees up the memory used by the temporary MATRIX.
 */
int WL_FreeTempMatrix(dtype **matrix)
{
    free((char *)matrix[0]);
    free((char *)matrix);
    return WL_OK;
}
  
/* WL_MakeTempMatrix
 *
 * Creates a temporary matrix of size ROWS x COLS.
 */
int WL_MakeTempMatrix(int rows, int cols, dtype ***matrix)
{
    dtype *array;
    dtype **indices;
    int i;

    array = (dtype *) calloc(rows*cols, sizeof(dtype));
    indices = (dtype **) calloc(rows, sizeof(dtype *));
    if ((array == NULL) || (indices == NULL)){
        WL_SetErrorMsg("WL_MakeTempMatrix: unable to malloc matrix"); 
        return WL_ERROR;
    }

    /* insert pointers to the beginning of each row into "indices" array */
    for (i = 0; i < rows; i++, array += cols) indices[i] = array;
    *matrix = indices;

    return WL_OK;
}
