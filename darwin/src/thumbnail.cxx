//*******************************************************************
//   file: thumbnail.cxx
//
// author: J H Stewman 5/11/2007
//
//*******************************************************************

#include "thumbnail.h"
#include <stdio.h>
#include <string.h>

void free_thumbnail(gpointer data)
{
	if (NULL == data)
		return;
	
	char **image = (char **) data;

	int width, height, colors, rows;

	sscanf(image[0],"%d %d %d", &width, &height, &colors);

	rows = height + colors + 1;

	for (int r = 0; r < rows; r++)
		delete [] image[r];
	delete image;

	data = NULL;
}

char** copy_thumbnail(char **data)
{
	int width, height, colors, rows;

	sscanf(data[0],"%d %d %d", &width, &height, &colors);

	rows = height + colors + 1;

	char **thumbCopy = new char* [rows];
	for (int ti = 0; ti < rows; ti++)
	{
		thumbCopy[ti] = new char [strlen(data[ti]) + 1];
		strcpy(thumbCopy[ti], data[ti]);
	} 

	return thumbCopy;
}