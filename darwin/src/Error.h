//*******************************************************************
//   file: Error.h
//
// author: Adam Russell
//
//   mods: 
//
//*******************************************************************

#ifndef ERROR_HH
#define ERROR_HH

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

class Error
{
public:
	Error() : mErrorMsg("Undefined error.")
	{ }

	Error(std::string s) : mErrorMsg(s)
	{ }
	
	virtual ~Error() { }

#ifdef DEBUG
	virtual void print() const
	{
		std::cout << "ERROR: " << mErrorMsg << std::endl;
	}
#endif

	virtual std::string errorString() const {
		return mErrorMsg;
	}

protected:
	std::string mErrorMsg;
};

class EmptyArgumentError : public Error {
	public:
		EmptyArgumentError() : Error("Function call with empty argument.")
		{ }
		EmptyArgumentError(std::string s) : Error("Call to " + s + " with empty argument.")
		{ }
};

class InvalidArgumentError : public Error {
	public:
		InvalidArgumentError() : Error("Function call with invalid argument.")
		{ }

		InvalidArgumentError(std::string s) : Error("Function call to " + s + " with an invalid argument.")
		{ }
};

class OutputError : public Error {
	public:
		OutputError() : Error("Error writing output.")
		{ }

		OutputError(std::string s) : Error("Error writing output in " + s)
		{ }
};

class InputError : public Error {
	public:
		InputError() : Error("Error reading input.")
		{ }

		InputError(std::string s) : Error("Error reading input in " + s)
		{ }
};

class BoundsError : public Error {
	public:
		BoundsError() : Error("Error in bounds.")
		{ }

		BoundsError(std::string s)
			: Error("Bounds error in " + s)
		{ }
};

#endif
