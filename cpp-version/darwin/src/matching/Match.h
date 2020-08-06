//*******************************************************************
//   file: Match.h
//
// author: Adam Russell
//
//   mods: J H Stewman (8/2/2005)
//         -- reformatting of code and addition of comment blocks
//         -- new constants to specify method of match
//         J H Stewman (9/2/2005)
//         -- add category matching code
//
//*******************************************************************

#ifndef MATCH_H
#define MATCH_H

#include "../Chain.h"
#include "../Database.h"
#include "../DatabaseFin.h"
#include "../FloatContour.h"
#include "MatchResults.h"

// new defined constants (8/2/05) to specify method of alignment / mapping

// fundamental registration method to be used

#define ORIGINAL_3_POINT            10
#define TRIM_FIXED_PERCENT          20
#define TRIM_OPTIMAL                30
#define TRIM_OPTIMAL_TOTAL          40
#define TRIM_OPTIMAL_TIP            41
#define TRIM_OPTIMAL_IN_OUT         42
#define TRIM_OPTIMAL_IN_OUT_TIP     43
#define TRIM_OPTIMAL_AREA	        45
#define LEADING_EDGE_ANGLE_METHOD   50
#define SIGSHIFT                    60

// additional values

#define MSQERR_ONLY                 1
#define APPLY_UNUSED_PT_PENALTY     2

// Outline segment(s) to be used in registration

#define ALL_POINTS                  100
#define LEAD_TO_TIP_ONLY            200
#define LEAD_TO_NOTCH_ONLY          300
#define LEAD_THEN_TRAIL             400

// 1.85 constant to pass when parameter for midpoint between begin and end
// of error calculation limits is to be ignored -- this is required so that the
// prototypes of all error finding functions are the same -- so the error 
// function can be passed into the mapping function as a function pointer

#define IGNORE_MID_POSIT            0

// Big note about this (sorta lousy) class:
// In somewhat bad form, this class always returns a pointer
// to its MatchResults member to make the whole matching process
// faster (returning a pointer to a new object could be very slow
// since the MatchResults class could hold many MB of data).
// Because of this, it can't delete its member MatchResults object
// when cleaning up.  So, it always assumes that the MatchResults
// has been fetched, and will be cleaned up externally.

// 005CM mseInfo struct added
// Addendum to the comments above: The float/double return parameters from 
// the private MSE functions have been changed to return a *mseInfo* struct
// (declared below)to facilitate the passing of the two Contours being 
// matched back to the MatchResultsWindow

/* 1.4 - replaced with class below
typedef struct {
  double error;
  FloatContour *c1,*c2;
  int b1,t1,e1,b2,t2,e2; // 1.1 - indices for shifted begin, tip & end points on contours
} mseInfo;
*/

// 1.4 - new class instead of struct, this way all members intialized
class mseInfo 
{
	public :

		// members

		double error;

		FloatContour *c1, *c2;
		
		int b1, t1, e1, b2, t2, e2;
		
		// constructor and destructor

		mseInfo() 
		:	error(10000.0),
			c1(NULL),
			c2(NULL),
			b1(0),t1(0),e1(0),b2(0),t2(0),e2(0)
		{ }

		~mseInfo()
		{ }
};

// can this work as a forward reference without including the header from here?
// YES it does (at least for MSVC++)
class MatchingDialog;

class Match
{
	public:
		// Makes a new DatabaseFin object from *unknownFin,
		// but ONLY MAKES A POINTER COPY OF THE DATABASE.
		Match(
			DatabaseFin<ColorImage> *unknownFin,
			Database *db,
			Options *o);      // 054

		~Match();

		// Compares the unknown fin against all those in the
		// database, and returns results.
		//MatchResults* matchFin(); // 1.85 removed
		
		// A variant on the previous function.  This one compares
		// the unknown fin to a single fin in the database at a time.
		// getMatchResults() must be called after this function to
		// retrieve the results of the comparison process.
		// 
		// RETURN:
		// 	float - percentage of the database completed.  When
		// 		done, returns 1.0
		float matchSingleFin(int registrationMethod, int regSegmentsUsed,
		                     bool categoryToMatch[],
							 bool useFullFinError, //***055ER
							 bool useAbsoluteOffsets); //***1.3

		int find_tip_pos(point_t PosPoint, FloatContour *c2); //***005CM

		MatchResults* getMatchResults();

		void setDisplay(MatchingDialog *mDialog);

	private:
		DatabaseFin<ColorImage> *mUnknownFin;
		Database *mDatabase;
		int mCurrentFin;
		MatchResults *mMatchResults;

		MatchingDialog *mMatchingDialog; // 043MA

		Options *mOptions; // 054

		int
			mUnknownTipPosition,
			mUnknownNotchPosition,
			mUnknownBeginLE,
			mUnknownEndLE,
			mUnknownEndTE;

		point_t
			mUnknownTipPositionPoint,
			mUnknownNotchPositionPoint,
			mUnknownBeginLEPoint,
			mUnknownEndLEPoint,
			mUnknownEndTEPoint;

		mseInfo findErrorBetweenFins( // 005CM
				DatabaseFin<ColorImage> *dbFin,
				float &timeTaken);

		double meanSquaredErrorBetweenChains(
				const Chain *chain1,
				int index1,
				const Chain *chain2,
				int index2);

		mseInfo meanSquaredErrorBetweenOutlines( // 005CM
				FloatContour *c1,
				int start1,
				int end1,
				FloatContour *c2, // 005CM
				int start2,
				int end2);

		//----------- new stuff JHS ----------

		// 1.85 - removed
		//mseInfo findErrorBetweenFinsJHS( 
		//		DatabaseFin<ColorImage> *dbFin,
		//		float &timeTaken,
		//		int regSegmentsUsed,
		//		bool useFullFinError); // 055ER

		// 1.85 - new member function pointer
		double (Match::*errorBetweenOutlines)(FloatContour*,int,int,int,FloatContour*,int,int,int);

		double meanSquaredErrorBetweenOutlineSegments( 
				FloatContour *c1, // mapped unknown fin 
				int start1,
				int mid1, // 1.85 not used here but makes prototype same as area based approach
				int end1,
				FloatContour *c2, // envenly spaced database fin
				int start2,
				int mid2, // 1.85 not used here but makes prototype same as area based approach
				int end2);

		double areaBasedErrorBetweenOutlineSegments_OLD( // 1.85 - new, calls external function
				FloatContour *c1, // mapped unknown fin 
				int begin1,
				int end1,
				FloatContour *c2, // envenly spaced database fin // 0005CM
				int begin2,
				int end2);

		double areaBasedErrorBetweenOutlineSegments( // 1.85 - new, calls external function
				FloatContour *c1, // mapped unknown fin 
				int begin1,
				int mid1,
				int end1,
				FloatContour *c2, // envenly spaced database fin // 0005CM
				int begin2,
				int mid2,
				int end2);

		double meanSquaredErrorBetweenOutlineSegmentsNew( 
				FloatContour *c1, // mapped unknown fin 
				int begin1,
				int end1,
				FloatContour *c2, // envenly spaced database fin // 0005CM
				int begin2,
				int end2);

		double meanSquaredErrorBetweenOutlineSegmentsMedial( 
				FloatContour *c1, // mapped unknown fin 
				int begin1,
				int end1,
				FloatContour *c2, // envenly spaced database fin // 0005CM
				int begin2,
				int end2);

		mseInfo findErrorBetweenFinsOptimal(
				DatabaseFin<ColorImage> *dbFin,
				float &timeTaken,
				//int regSegmentsUsed,
				bool moveTip, // 1.1
				bool moveEndsInAndOut, // 1.1
				bool useFullFinError); // 055ER

		// 1.75 - newest method of computing error

		double areaBasedErrorBetweenOutlineSegments( 
				FloatContour *c1, // mapped unknown fin 
				int begin1,
				int end1,
				FloatContour *c2, // envenly spaced database fin
				int begin2,
				int end2);

		//-------------- reconstruction of original methods JHS -----

		mseInfo findErrorBetweenFins_Original3Point(
				DatabaseFin<ColorImage> *dbFin,
				float &timeTaken);

		mseInfo meanSquaredErrorBetweenOutlines_Original( // 005CM
				FloatContour *c1,
				int tip1,
				FloatContour *c2, // 0005CM
				int tip2);
};

#endif
