//*******************************************************************
//   file: Matrix.h
//
// author: Adam Russell
//
//   mods:
//
// This is a modified version of code given in
// Marshall P. Cline's "C++ FAQ Lite" document,
// http://marshall-cline.att.net/cpp-faq-lite/
//
//*******************************************************************

#ifndef MATRIX_HH
#define MATRIX_HH

#include <stdio.h>
#include "../Error.h"

template <class T>
class Matrix
{
public:
	Matrix(unsigned numRows, unsigned numCols);
	class BadSize : public Error {
		public: BadSize() : Error("Bad matrix size.") { }
	};
	
	Matrix();

	virtual ~Matrix();
	Matrix(const Matrix<T>& m);
	Matrix<T>& operator= (const Matrix<T>& m);

	T& operator() (unsigned row, unsigned col);
	const T& operator() (unsigned row, unsigned col) const;

	class BoundsViolation : public Error {
		public: BoundsViolation() : Error("Attempt to access element outside matrix bounds.") { }
			BoundsViolation(unsigned row, unsigned col)
				: Error()
			{
				char buffer[200];

				sprintf(buffer, "Attempt to access element outside matrix bounds at"
					" row: %d col: %d", row, col);
				mErrorMsg = buffer;
			}
	};

	unsigned getNumRows() const;
	unsigned getNumCols() const;

	virtual bool isInitialized() const;

protected:
	T* mData;
	unsigned mRows, mCols;
	bool mInitialized;
};

template <class T>
inline Matrix<T>::Matrix(unsigned numRows, unsigned numCols)
	: mData (new T[numRows * numCols]),
	  mRows (numRows),
	  mCols (numCols),
	  mInitialized (true)
{
	if (numRows == 0 || numCols == 0) throw BadSize ();
}

template <class T>
inline Matrix<T>::Matrix()
	: mData (NULL),
	  mRows (0),
	  mCols (0),
	  mInitialized (false)
{ }

template <class T>
inline Matrix<T>::Matrix(const Matrix<T>& m)
{
	if (!m.isInitialized ()) {
		mInitialized = false;
		mRows = mCols = 0;
		mData = NULL;
	} else {
		mRows = m.getNumRows ();
		mCols = m.getNumCols ();
		mData = new T[mRows * mCols];

		memcpy (mData, &m(0,0), sizeof(T) * mRows * mCols);
		
		mInitialized = true;
	}
}

template <class T>
inline Matrix<T>::~Matrix()
{
	if (mInitialized) delete[] mData;
}

template <class T>
inline T& Matrix<T>::operator() (unsigned row, unsigned col)
{
	if (row >= mRows || col >= mCols) throw BoundsViolation (row, col);

	return mData[row*mCols + col];
}

template <class T>
inline const T& Matrix<T>::operator() (unsigned row, unsigned col) const
{
	if (row >= mRows || col >= mCols) throw BoundsViolation (row, col);

	return mData[row*mCols + col];
}

template <class T>
Matrix<T>& Matrix<T>::operator= (const Matrix<T>& m)
{
	if (this == &m) return *this; // handle self assignment
	
	if (!m.isInitialized ()) {
		mInitialized = false;
		mRows = mCols = 0;
		mData = NULL;
	} else {
		if (!mInitialized || mRows != m.getNumRows() || mCols != m.getNumCols()) {
				mRows = m.getNumRows ();
				mCols = m.getNumCols ();
				
				if (mInitialized)
					delete[] mData;
		
				mData = new T[mRows * mCols];
		}

		memcpy(mData, &m(0,0), sizeof(T) * mRows * mCols);
		
		mInitialized = true;
	}

	return *this;
}

template <class T>
inline unsigned Matrix<T>::getNumRows() const { return mRows; }

template <class T>
inline unsigned Matrix<T>::getNumCols() const { return mCols; }

template <class T>
inline bool Matrix<T>::isInitialized() const { return mInitialized; }

#endif
