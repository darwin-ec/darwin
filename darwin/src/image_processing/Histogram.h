//*******************************************************************
//   file: Histogram.h
//
// author: Adam Russel?
//
//   mods:
//
//*******************************************************************

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "../utility.h"

/*
 * 101AT 
 * Range is used to represent a segment of a Histrogram object
 */
struct Range {
  int start;
  int tip;
  int end;
  int highestValue;
  int pixleCount;
};


template <class IMAGE_TYPE>
class Histogram{
public:
	Histogram (const IMAGE_TYPE* image);
	~Histogram ();
	unsigned findMaxPeak (void) const;
	void smoothHistogram ();
	unsigned findMinVal (void) const;
	unsigned getSize (void) const;
	unsigned getValue (unsigned position) const;
	float findRange (void) const;
	float findVariance (void) const;
	
	Range* findNextValley(int start);//101AT
private:
	unsigned *histogram;
	unsigned size;
	
	int getDirection(int index); //need by findNextValley(int start)  //101AT
};

// IMPLEMENTATION

template <class IMAGE_TYPE>
Histogram<IMAGE_TYPE>::Histogram (const IMAGE_TYPE* image)
{
	if (image == NULL) return;

	size = image->getColorDepth ();

	histogram = new unsigned[size];

	for (unsigned i = 0; i < size; i++)  //initialize the histogram
		histogram[i] = 0;

	for (unsigned r = 0; r < image->getNumRows (); r++)
		for (unsigned c = 0; c < image->getNumCols(); c++)
			histogram[(*image)(r, c).getIntensity ()]++;
}

template <class IMAGE_TYPE>
Histogram<IMAGE_TYPE>::~Histogram ()
{
	delete []histogram;
}

template <class IMAGE_TYPE>
unsigned Histogram<IMAGE_TYPE>::findMaxPeak () const
//
//Returns the number of the shade which corresponds to the highest peak
{
	unsigned test = 0;

	for (unsigned i = 1; i < size; i++) {
		if (histogram[i] > histogram[test])
			test = i;
	}
	return test;
}

template <class IMAGE_TYPE>
void Histogram<IMAGE_TYPE>::smoothHistogram ()
//Try to average out any anomolous values
{
	int
		sum,
		num_vals,
		mask_size = 7,
		offset;

	offset = mask_size / 2;

	for (unsigned i = 0; i < size; i++) {
		sum = num_vals = 0;
		for (unsigned pos = i - offset; pos < i + offset; pos++) {
			if ((pos < 0) || (pos > size))
				continue;

			sum += histogram[pos];
			num_vals++;
		}

		histogram[i] = (unsigned) round ((float) sum / num_vals);
	}
}

template <class IMAGE_TYPE>
unsigned Histogram<IMAGE_TYPE>::findMinVal (void) const
{
	unsigned min = 0;

	for (unsigned i = 0; i < size; i++)
		if (histogram[i] < histogram[min])
			min = i;

	return min;
}

template <class IMAGE_TYPE>
unsigned Histogram<IMAGE_TYPE>::getSize (void) const
{
	return size;
}

template <class IMAGE_TYPE>
unsigned Histogram<IMAGE_TYPE>::getValue (unsigned position) const
{
	return histogram[position];
}

template <class IMAGE_TYPE>
float Histogram<IMAGE_TYPE>::findRange (void) const
//
// Returns the ratio of spread values used / total number of values
//
//	Note: ignores black, or histogram[0]
//
{
	unsigned	
		lowestVal,	// lowest value in the histogram other than black
		highestVal;	// highest value in the histogram

	lowestVal = 1;

	while (lowestVal < size) {
		if (!this->histogram[lowestVal])
			break;
		lowestVal++;
	}
	
	highestVal = size - 1;

	while (highestVal > lowestVal) {
		if( !this->histogram[highestVal] )
			break;
		
		highestVal++;
	}

	return (float)( highestVal - lowestVal ) / size;
}

template <class IMAGE_TYPE>
float Histogram<IMAGE_TYPE>::findVariance (void) const
{
	float
		mean;

	double
		variance;

	if (histogram == NULL) return 0.0;
	
	mean = 0.0f;
	variance = 0.0;

	for (unsigned i = 0; i < size; i++)
		mean += histogram[i];

	mean /= size;

	for (unsigned hPos = 0; hPos < size; hPos++)
		variance += (histogram[hPos] - mean)*(histogram[hPos] - mean);

	variance /= size - 1;
	
	return (float)variance;
}

/*
 * 101AT
 * 
 * Find the next relative min in the histogram. 
 * 
 * @param start The index into the histogram [ 0-getSize() ] at which to start
 * @return a Range object representing the segment of the histrogram from start
 * 	to the next valley.
 */
template <class IMAGE_TYPE>
Range* Histogram<IMAGE_TYPE>::findNextValley(int start) {
 int tip=0, //The highest point in the peak
    end=0; //The end of the range comprising this peak

  int direction;
  int directionState=0;
  int count = 0;
  int pixleCount = 0;

  for (int i=start; i<size; i++) {
    direction = getDirection(i);
    pixleCount += histogram[i];
    
    if (directionState==0 && direction==1) {
    	//first incrase
    	directionState=1;
    } else if (directionState==1 && direction==-1) {
    	//first pleatuea / peak
    	directionState=-1;
    } else if (directionState==-1 ) {
    	if (direction==0) {//plateau
    		count++;
    	} else if (direction==1 && i-tip>10) {//started increasing again, break
    		end=i-count+(int)(count/2.0);
    		break;
    	} else {//(if direction==-1) //decreasing, reset count
	  count=0;
    	}

	/*if (count>30) {
	  //early termination
	  cout << "early termination based on many level regions" << endl;
	  end=i-count+(int)(count/2.0);
	  break;
	  }*/
	  
    }

    end=i;
    if (histogram[i] > histogram[tip]) tip=i;

    //Cases just to continue
    //Decreasing, but have never increased
    //pleteau without a decreased proceeded by an increase
  }

  Range *rng = new Range;
  rng->start=start;
  rng->end=end;
  rng->tip=tip;
  rng->highestValue=histogram[tip];
  rng->pixleCount=pixleCount;

  return rng;
}	

/*
 * 101AT
 * 
 * Look at neighbors of index to determine the general direction (slope) of the
 * 	histogram at point index.
 * 
 * @param index The position [ 0 - getSize() ] to analyize in the histogram.
 * 
 * @return -1=decreasing (e.g. negative slope)
 *          0=level		(e.g. slope==0)
 *          1=increasing	(e.g. positive slope)
 */
template <class IMAGE_TYPE>
int Histogram<IMAGE_TYPE>::getDirection(int index) {
	  int diff=15;

  int inc=0, desc=0, level=0;
  int i;//loop counter can't be in loop b/c throws redefinition error when used twice.
  int start = index-diff,
    end=index+diff;

  //check bounds
  if (start<0)	start = 0;
  if (end>size)	end = size;

  int current; // removed right & left
  int index_value=histogram[index];

  for (i=start; i<index; i++) {
    current = histogram[i];

    if (index_value < current*1.02) {//less than left neighbors
      desc++;
    } else if(index_value*1.02 > current) {//greater than left neighbors
      inc++;
    } else {
      level++;
    }
  }

  for (i=index+1; i<end; i++) {
    current = histogram[i];

    if (index_value > current*1.02) {//greater than right neighbors
      desc++;
    } else if(index_value*1.02 < current) {//less than right neighbors
      inc++;
    } else {
      level++;
    }
  } 

  if (inc > desc && inc > level) {
    return 1; //increasing
  } else if (desc > inc && desc > level) {
    return -1; //decreasing
  } else {
    return 0;//level
  }
}

#endif
