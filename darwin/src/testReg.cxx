//*******************************************************************
//   file: testReg.cxx
//
// author: Adam Russell
//
//   mods: 
//
//*******************************************************************

#include <fstream>
#include "Chain.h"
#include "feature.h"
#include "FloatContour.h"
#include "mapContour.h"
#include "testReg.h"

using namespace std;

void testRegistration(Database<ColorImage> *db)
{
	if (NULL == db)
		return;
	
	int numFins = db->size();

	DatabaseFin<ColorImage> *pivotFin = NULL, *curFin = NULL;
	Chain *pivotChain = NULL, *curChain = NULL;
	
	for (int i = 0; i < numFins; i++) {
		pivotFin = db->getItem(i);

		ofstream outPivot(pivotFin->mIDCode.c_str());
		
		//Contour *c = pivotFin->mFinContour; removed 008OL
    FloatContour *c = pivotFin->mFinOutline->getFloatContour(); //***008OL
		for (int l = 0; l < (int)c->length(); l++)
			outPivot << (*c)[l].x << " " << -(*c)[l].y << endl;
		
		outPivot.close();

		//***008OL replace all below with following
    /*
    pivotChain = new Chain(c, 3.0);

		int pivTipPosition, pivBeginLE, pivNotchPosition;
		pivTipPosition = findTip(pivotChain);
		pivBeginLE = findLECutoff(pivotChain, pivTipPosition);
		pivNotchPosition = findNotch(pivotChain, pivTipPosition);

		point_t
			pivTipPositionPoint,
			pivBeginLEPoint,
			pivNotchPositionPoint;
	
		pivTipPositionPoint = pivotChain->getSavedPoint(pivTipPosition);
		pivBeginLEPoint = pivotChain->getSavedPoint(pivBeginLE);
		pivNotchPositionPoint = pivotChain->getSavedPoint(pivNotchPosition);
		*/
		point_t
			pivTipPositionPoint,
			pivBeginLEPoint,
			pivNotchPositionPoint;
	
		pivTipPositionPoint = pivotFin->mFinOutline->getFeaturePointCoords(TIP); //***008OL
		pivBeginLEPoint = pivotFin->mFinOutline->getFeaturePointCoords(LE_BEGIN); //***008OL
		pivNotchPositionPoint = pivotFin->mFinOutline->getFeaturePointCoords(NOTCH); //***008OL

		for (int j = 0; j < numFins; j++) {
			if (j == i)
				continue;
		
			curFin = db->getItem(j);
			
      //***008OL again replace the following
      /*
			curChain = new Chain(curFin->mFinContour, 3.0);

			int curTipPosition, curBeginLE, curNotchPosition;
			curTipPosition = findTip(curChain);
			curBeginLE = findLECutoff(curChain, curTipPosition);
			curNotchPosition = findNotch(curChain, curTipPosition);

			point_t
				curTipPositionPoint,
				curBeginLEPoint,
				curNotchPositionPoint;
	
			curTipPositionPoint = curChain->getSavedPoint(curTipPosition);
			curBeginLEPoint = curChain->getSavedPoint(curBeginLE);
			curNotchPositionPoint = curChain->getSavedPoint(curNotchPosition);
      */

			point_t
				curTipPositionPoint,
				curBeginLEPoint,
				curNotchPositionPoint;
	
			curTipPositionPoint = curFin->mFinOutline->getFeaturePointCoords(TIP); //***008OL
			curBeginLEPoint = curFin->mFinOutline->getFeaturePointCoords(LE_BEGIN); //***008OL
			curNotchPositionPoint = curFin->mFinOutline->getFeaturePointCoords(NOTCH); //***008OL

			FloatContour *mappedContour = mapContour(
					//pivotFin->mFinContour, removed 008OL
          pivotFin->mFinOutline->getFloatContour(), //***008OL
					pivTipPositionPoint,
					pivBeginLEPoint,
					pivNotchPositionPoint,
					curTipPositionPoint,
					curBeginLEPoint,
					curNotchPositionPoint);

			// magic number! =)
			char fName[255];
			sprintf(fName, "%s-%s", pivotFin->mIDCode.c_str(), curFin->mIDCode.c_str());
			
			ofstream outRegged(fName);
			
			for (int k = 0; k < (int)mappedContour->length(); k++)
				outRegged << (*mappedContour)[k].x << " " << -(*mappedContour)[k].y << endl;

			outRegged.close();

			delete mappedContour;
			delete curChain;
			delete curFin;	
		}
		delete pivotChain;
		delete pivotFin;
	}
}
