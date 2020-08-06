// gaussj.h
//*******************************************************************
//   file: gaussj.h
//
// author: Adam Russell
//
//   mods: 
//
// The single function gaussj() is ...
//
//	 Based on the function presented in Numerical Recipes in C, pages 39-40.
//
//	 Linear equation solution by Gauss-Jordan elmination.  a[0..n-1][0..n-1]
//   is the input matrix.  b[0..n-1][0..m-1] is the input containing the m
//   right-hand side vectors.  On output, a is replaced by its matrix inverse,
//   and b is replaced by the corresponding set of solution vectors.
//
//*******************************************************************

bool gaussj(float **a, int n, float **b, int m);
