//*******************************************************************
//   file: pngFile.h
//
// author: J H Stewman
//
//   mods:
//
//
// The following is from the original header file (PNGFILE.H) - JHS
//
//*******************************************************************

#ifdef __cplusplus
extern "C" {
#endif

int PngLoadImage (char *pstrFileName, png_byte **ppbImageData, 
                   int *piWidth, int *piHeight, int *piChannels, png_color *pBkgColor,
				   png_textp *comment, int *num_comments);
int PngSaveImage (char *pstrFileName, png_byte *pDiData,
                   int iWidth, int iHeight, png_color BkgColor, 
				   png_textp text_ptr, int num_text);

//***1.85 - added for access to comments only
int PngLoadImageComments (char *pstrFileName, png_textp *comment, int *num_comments);

#ifdef __cplusplus
}
#endif
