//*******************************************************************
//   file: guassj.cxx
//
// author: 
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include <math.h>
#include <iostream>

using namespace std;

static inline void swapf(float &a, float &b)
{
	float temp = a;
	a = b;
	b = temp;
}

//*******************************************************************
// gaussj()
//
// Based on the function presented in Numerical Recipes in C, pages 39-40.
//
// Linear equation solution by Gauss-Jordan elmination.  a[0..n-1][0..n-1]
// is the input matrix.  b[0..n-1][0..m-1] is the input containing the m
// right-hand side vectors.  On output, a is replaced by its matrix inverse,
// and b is replaced by the corresponding set of solution vectors.
//
bool gaussj(float **a, int n, float **b, int m)
{
	int i,icol,irow,j,k,l,ll;
	float big,dum,pivinv;

	// These arrays are used for bookkeeping on the pivoting
	int
		*indxc = new int[n],
		*indxr = new int[n],
		*ipiv = new int[n];

	for (j = 0; j < n; j++)
		ipiv[j] = 0;
	
	// Main loop over the columns to be reduced
	for (i = 0; i < n; i++) {
		big = 0.0;

		// Outer loop of the search for a pivot element
		for (j = 0; j < n; j++)
			if (ipiv[j] != 1)
				for (k = 0; k < n; k++) {
					if (ipiv[k] == 0) {
						if (fabs(a[j][k]) >= big) {
							big = (float) fabs(a[j][k]);
							irow = j;
							icol = k;
						}
					} else if (ipiv[k] > 1) {
						delete[] ipiv;
						delete[] indxr;
						delete[] indxc;
						cout << "gaussj: Singular Matrix-1" << endl;
						return false;
					}
				}

		++(ipiv[icol]);

		// We now have the pivot element, so we interchange rows,
		// if needed, to put the pivot element on the diagonal.
		// The columns are not physically interchanged, only
		// relabeled: indxc[i], the column of the ith pivot element,
		// is the ith column that is reduced, while indxr[i] is the row
		// in which that pivot element was originally located.  If indxr[i]
		// != indxc[i] there is an implied column interchange.  With
		// this form of bookkeeping, the solution b's will end up in
		// the correct order, and the inverse matrix will be scrambled
		// by columns.
		if (irow != icol) {
			for (l = 0; l < n; l++)
				swapf(a[irow][l], a[icol][l]);
			
			for (l = 0; l < m; l++)
				swapf(b[irow][l], b[icol][l]);
		}

		// We are now ready to divide the pivot row by the pivot element,
		// located at irow and icol
		indxr[i]=irow;
		indxc[i]=icol;
		
		if (a[icol][icol] == 0.0) {
			delete[] ipiv;
			delete[] indxr;
			delete[] indxc;

			cout << "gaussj: Singular Matrix-2" << endl;
			return false;
		}
		
		pivinv= 1.0f / a[icol][icol];
		a[icol][icol]=1.0;
		
		for (l = 0; l < n; l++)
			a[icol][l] *= pivinv;

		for (l = 0; l < m; l++)
			b[icol][l] *= pivinv;
		
		// Now, we reduce the rows...
		for (ll = 0; ll < n; ll++)
			// ... except for the pivot one, of course.
			if (ll != icol) {
				dum=a[ll][icol];
				a[ll][icol]=0.0;
				
				for (l = 0; l < n; l++)
					a[ll][l] -= a[icol][l] * dum;
				
				for (l = 0; l < m; l++)
					b[ll][l] -= b[icol][l] * dum;
			}
	}

	// This is the end of the main loop over columns of reduction.
	// It only remains to unscramble the solution in view of the
	// column interchanges.  We do this by interchanging pairs
	// of columns in the reverse order that the permutation was
	// built up.
	for (l = n - 1; l >= 0; l--) {
		if (indxr[l] != indxc[l])
			for (k = 0; k < n; k++)
				swapf(a[k][indxr[l]],a[k][indxc[l]]);
	}
	
	delete[] ipiv;
	delete[] indxr;
	delete[] indxc;

	return true;
}
