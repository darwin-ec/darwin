/* Header file for the Core Data Structures in the base WL System.
  
   Mike Hilton, 20 April 1995
   Modified by Fausto Espinal, 18 Jan. 1997
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WLCORE_HEADER
#define WLCORE_HEADER

#define WL_OK            0
#define WL_ERROR         1

typedef double dtype;   /* the basic data type for WL */
#define DTYPE_PRINT_FORMAT "%.17g"
#define DTYPE_READ_FORMAT "%lf"

#define WL_TYPESTRING_LENGTH 5 /* length of the type name stored with object */
#define WL_SCRATCH_LENGTH 128  /* length of scratch string */
#ifndef PI
#define PI 3.141592653589793
#endif

#ifndef MAX
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

/* WL_PropList
   Property list.  An associative data structure for storing information
   in the form of {Key, Value} pairs.
*/
typedef struct PropList {
  char *key;
  char *value;
  struct PropList *next;
} WL_PropList;


/* WL_Struct
   All of the WL data structures must include WL_Struct as its first
   field.  This structure contains information for identifying the
   type of an object at runtime.
*/
typedef struct {
  char *type;           /* string naming the type of structure */
  char *name;           /* command name associated with object */
  WL_PropList *plist;   /* property list of object */
} WL_Struct;


/* WL_StructList
   Basic building block for linked lists of WL_Struct's.
*/
typedef struct WL_StructList {
  WL_Struct *object;  
  struct WL_StructList *next;   
} WL_StructList;


/* WL_Filter */
typedef struct {
  WL_Struct info;
  int length;
  int offset;
  double *coefs;
} WL_Filter;

/* WL_SubbandFilter */
typedef struct {
  WL_Struct info;
  WL_Filter filters[4];
} WL_SubbandFilter;

/* WL_Matrix */
typedef struct {
  WL_Struct info;
  int rows;
  int cols;
  dtype **data;
} WL_Matrix;


/* WL_Vector
   1-D Vector data type
*/
typedef struct {
  WL_Struct info;
  int length;      /* length of the vector */
  dtype *data;     /* data array */
} WL_Vector;


/* WL_Volume */
typedef struct {
  WL_Struct info;
  int rows;
  int cols;
  int depth;
  dtype ***data;
} WL_Volume;


/* wlcError.c */
void    WL_SetErrorMsg(char *);
char   *WL_GetErrorMsg();

/* wlcSBFilter.c */
int WL_SBFilterLoad (char *fileName, char *filterName, WL_SubbandFilter **newFilter);

/* wlcNumerical.c */
void  WL_cosft1(dtype *, int);
void  WL_cosft2(dtype *, int, int);
void  WL_four1(dtype *data, unsigned long nn, int isign);
float WL_gasdev(long *);
float WL_ran1(long *);
void  WL_ksone(dtype *data, int n, double (*func)(double), double *d, 
	      double *prob);
void  WL_kstwo(dtype *data1, int n1, dtype *data2, int n2, double *d,
 	      double *prob);
void  WL_realft(dtype *data, unsigned long n, int isign);


/* wlcUtils.c  */
int WL_DtypeCmp(const void *a, const void *b);
int WL_isDyadic(int x);
int WL_log2(int x);
int WL_pow2(int x);
void WL_Swap(dtype *a, dtype *b);    /* Added by Fausto Espinal */
int WL_ReadAsciiDataFile(char *filename, int *rows, 
			 int *cols, dtype **data);
int WL_Read16uDataFile(char *fname, int *n, dtype **data);
int WL_ReadPbmFile(char *file, int *rows, int *cols,
		   dtype **data);
int WL_ReadPgmFile(char *file, int *rows, int *cols,
		   dtype **data);
int WL_ReadPpmFile(char *file, int *rows, int *cols,
		   dtype **data);
void WL_StripFilename(char *infile, char *outfile);
int WL_WriteAsciiDataFile(char *filename, dtype *data,
		int rows, int cols);
int WL_WriteGnuplotDataFile(char *filename, dtype *data,
		int rows, int cols);
int WL_WritePbmFile(char *filename, dtype *data, 
		    int rows, int cols);
int WL_WritePgmFile(char *filename, dtype *data, 
		    int rows, int cols);
int WL_WritePpmFile(char *filename, dtype ***data, 
		    int rows, int cols);
int WL_Write16uDataFile (char *fname, int n, dtype *data);

int WL_FreeTempMatrix(dtype **matrix);

int WL_MakeTempMatrix(int rows, int cols, dtype ***matrix);

/* wlcPWavelet.c */
int WL_FwtMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass);
int WL_IwtMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass);
int WL_FwtVolume(dtype ***source, dtype ***dest, 
		 int depth, int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass);
int WL_IwtVolume(dtype ***source, dtype ***dest, 
		 int depth, int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass);
int WL_FwtVector(dtype *source, dtype *dest, int length,
		 int levels, WL_Filter *lowpass, WL_Filter *hipass);
int WL_IwtVector(dtype *source, dtype *dest, int length,
		 int levels, WL_Filter *lowpass, WL_Filter *hipass);
int WL_BoundingBox(int rows, int cols, int level, 
		    int detail, int *left, int *right, int *top, int *bottom);
int WL_ExtractDetail(dtype **matrix, int rows, int cols, 
		     int level, int detail, dtype ***out);

/* wlcSWavelet.c */
int WL_FswtVolume(dtype ***source, dtype ***dest, 
		 int depth, int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass);
int WL_IswtVolume(dtype ***source, dtype ***dest, 
		 int depth, int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass);
int WL_FswtMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass);
int WL_IswtMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass);
int WL_FswtVector(dtype *source, dtype *dest, int length,
		 int levels, WL_Filter *lowpass, WL_Filter *hipass);
int WL_IswtVector(dtype *source, dtype *dest, int length,
		 int levels, WL_Filter *lowpass, WL_Filter *hipass);

/* wlcPackets.c */
int WL_FswptMatrix(dtype **source, dtype **dest,int rows, int cols, 
                   int levels, WL_Filter *lowpass, WL_Filter *hipass);
int WL_IswptMatrix(dtype **source, dtype **dest,int rows, int cols, 
                   int levels, WL_Filter *lowpass, WL_Filter *hipass);
int WL_PacketBounding(int rows, int cols, char *addr,
                      int *left, int *right, int *top, int *bottom);

/* wlcMem.c */
void **WL_Alloc2Dmem(int r, int c, int size_elem);
void **WL_Calloc2Dmem(int r, int c, int size_elem);
void WL_Free2Dmem(void *memarg);
void ***WL_Alloc3Dmem(int d1, int d2, int d3, int size_elem);
void ***WL_Calloc3Dmem(int d1, int d2, int d3, int size_elem);
void WL_Free3Dmem(void *memarg);
dtype **WL_Extract2D(dtype **data,int sR, int sC, int numR, int numC);
void WL_Put2D(dtype **data,dtype **putD,int sR, int sC, int numR, int numC);
dtype ***WL_Extract3D(dtype ***data,int sD, int sR, int sC, int numD, 
                        int numR, int numC);

/* wlRWavelet.c */
int WL_FrwtVector(dtype *source, dtype **dest, int length, 
		  int levels, WL_Filter *lowpass, WL_Filter *hipass);
int WL_FrwtMatrix(
	dtype ** source,
	dtype *** dest,
	int rows,
	int cols,
	int levels,
	WL_Filter *lowpass,
	WL_Filter * hipass
);

int WL_IrwtMatrix(dtype **source, dtype *dest, int length,
		  int levels, WL_Filter *lowpass, WL_Filter *hipass);

#endif     /* WLCORE_HEADER */

#ifdef __cplusplus
}
#endif
