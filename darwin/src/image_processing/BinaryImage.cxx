//*******************************************************************
//   file: BinaryImage.cxx
//
// author: Scott Hale
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and additional comments
//
//*******************************************************************

#include "BinaryImage.h"
#include "conversions.h"

using namespace std;

/*
 * Find the largest feature (blob) that is black in the image
 * 
 * @return a pointer to a Feature struct containing the feature/blob and its area
 */
Feature* BinaryImage::getLargestFeature(){
	Feature *largest=NULL, *current=NULL;
	BinaryImage imgCpy(this);
	for (int r=10; r<mRows-10; r++) {
		if (r % 100 == 0)
			printf("."); //***1.96a - progress feedback for user
		for (int c=10; c<mCols-10; c++) {
			if (imgCpy.mData[r*imgCpy.mCols + c].getIntensity()==0) {
				current=find_blob(imgCpy,r,c);
				//BinaryImage notMask((current->mask));
				//notMask.doNot();
				//imgCpy.doAnd(notMask);
				if (largest==NULL || current->area > largest->area) {
					delete largest;
					largest=current;
				} else {
					delete current;
				}
			}
		}
	}
	//printf("\n");
	return largest;
}

/*
 * Find all black pixels that touch (seedrow,seedcol).
 * (seedrow,seedcol) should be black, otherwise an empty image and an area of 0 are returned.
 * 
 * @param srcImg The image in which to look for contigous black pixels
 * 		NOTE: srcImg is changed. Do not pass *this
 * @param seedrow the row of the (row,col) pair at which to start
 * @param seedcol the column of the (row,col) pair at which to start
 * @return a pointer to a Feature containing an BinaryImage of all contigous black pixels touching (seedrow,seedcol)
 */
/*private*/ Feature* BinaryImage::find_blob(BinaryImage &srcImg, int seedrow, int seedcol) {
	//Based on code from deburekr
    stack<Point> nbr_stack;
    int i, cr, cc; // removed done & j
    int l,r,t,b;  /*left, right, top , bottom neighbor */
    int area;
    unsigned map_val = 0;

    BinaryImage *map = new BinaryImage (mRows, mCols);
    for (i=0; i<mRows*mCols; i++) {
		map->mData[i].setIntensity(255);
	}

    nbr_stack.push(Point(seedrow, seedcol));
    area = 0;
    while (!nbr_stack.empty()){
      Point pt = nbr_stack.top(); //get top element (does not remove)
       cr=pt.getRow();
       cc=pt.getCol();
       nbr_stack.pop(); //remove element from stack
       if (srcImg.mData[cr*srcImg.mCols + cc].getIntensity()!=255){  /* may already have been pushed & processed */
         srcImg.mData[cr*srcImg.mCols + cc].setIntensity(255);  /* set to white */
          map->mData[map->mCols * cr + cc].setIntensity(map_val);
          area++; 
          r = cc+1;
          l = cc-1;
          t = cr-1;
          b = cr+1;
          /* process 4 neighbors in R,B,L,T (ESWN) order */
          if (r<mCols){ /* border? */
             if (srcImg.mData[srcImg.mCols * cr + r].getIntensity()!=255){
                 nbr_stack.push(Point(cr, r));
             } 
          }
          if (b<mRows){
             if (srcImg.mData[srcImg.mCols * b + cc].getIntensity()!=255){
                 nbr_stack.push(Point(b, cc));
             } 
          }
          if (l>=0){
             if (srcImg.mData[srcImg.mCols * cr + l].getIntensity()!=255){
                 nbr_stack.push(Point(cr, l));
             } 
          }
          if (t>=0){
             if (srcImg.mData[srcImg.mCols * t + cc].getIntensity()!=255){
                 nbr_stack.push(Point(t, cc));
             } 
          }
       }
    } /* end while */
    //printf("Area = %d\n", area);
    Feature *ftr = new Feature;
    ftr->mask=map;
    ftr->area=area;
	return ftr;
}

//Binary Operators ! & | ^

/*
 * Make all white pixels (e.g. those not 0) black,
 * and all black pixels white.
 */
void BinaryImage::doNot() {
	for (int r=0; r<mRows; r++) {
		for (int c=0; c<mCols; c++) {
			mData[r*mCols+c].setIntensity((mData[r*mCols+c].getIntensity()==0)? 255 : 0);
		}
	}
}

/*
 * See BindaryImage::doNot()
 */
void BinaryImage::operator!() {
	doNot();
}

/*
 * Change each pixels in this BinaryImage to white if the coresponding pixel in img 
 * is not black.
 * 
 * @param img the image with which to execute the operation. img is not affected
 */
void BinaryImage::doAnd(BinaryImage img) {
	if (mRows!=img.mRows || mCols!=img.mCols) {
		return;
	}
	for (int r=0; r<mRows; r++) {
		for (int c=0; c<mCols; c++) {
			if (! (mData[r*mCols+c].getIntensity()==0 && img.mData[r*img.mCols + c].getIntensity()==0)) {
				mData[r*mCols+c].setIntensity(255);
			}
		}
	}
}
/*
 * See BinaryImage::doAnd(BinaryImage)
 */
void BinaryImage::operator&(BinaryImage img) {
	doAnd(img);
}

/*
 * Change each pixel in this BinaryImage to black if the coresponding pixel in img 
 * is black.
 * 
 * @param img the image with which to execute the operation. img is not affected
 */
void BinaryImage::doOr(BinaryImage img) {
	if (mRows!=img.mRows || mCols!=img.mCols) {
		return;
	}
	for (int r=0; r<mRows; r++) {
		for (int c=0; c<mCols; c++) {
			if (/*mData[r][c]==0 ||*/ img.mData[r*img.mCols+c].getIntensity()==0) {
				mData[r*mCols+c].setIntensity(0);
			}
		}
	}
}
/*
 * See BinaryImage::doOr(BinaryImage)
 */
void BinaryImage::operator|(BinaryImage img) {
	doOr(img);
}

/*
 * The resulting image is black only where one (but not both) images was black.
 * 
 * @param img the image with which to execute the operation. img is not affected
 */
void BinaryImage::doXor(BinaryImage img) {
	if (mRows!=img.mRows || mCols!=img.mCols) {
		return;
	}
	for (int r=0; r<mRows; r++) {
		for (int c=0; c<mCols; c++) {
			if (mData[r*mCols+c].getIntensity()==0 && img.mData[r*img.mCols + c].getIntensity()==0) {
				mData[r*mCols+c].setIntensity(255);
			} else if (img.mData[r*img.mCols+c].getIntensity()==0) {
				mData[r*mCols+c].setIntensity(0);
			}
		}
	}
}
/*
 * See BinaryImage::doXor(BinaryImage)
 */
void BinaryImage::operator^(BinaryImage img) {
	doXor(img);
}

//Morphological Operators
/** Algorithms doErode and doDialate from Graphics Gems (Ed. Paul S. Heckbert)**/

/* 
 * Take a thresholded image 0 or 255, as input, and erode 
 * (shrink black regions, grow white regions).
 * 
 * @param coeff The miniumum number of white neighbors (out of 8) to be considered
 * 	an edge pixel and thus changed to white. Use 0 for classical erosion.
 * @return the number of pixels affected (changed from black to white)
 */
int BinaryImage::doErode(int coeff) {

  /*int rows, cols;*/
  int r, c;
  //int i;
  int pc = 0;           /* pass count */
  int count = 1;        /*deleted pixel count */
  int p, q;             /* neighborhood maps of adjacent cells */
  unsigned char *qb;    /* neighborhood maps of previous scanline */
  //int m;                /* Deletion direction mask */
 

 
  qb = (unsigned char *) malloc (mCols * sizeof (unsigned char));
  if (qb == NULL) {
      printf ("Error in memory allocation\n");
      exit (-1);
  }
  qb[mCols-1] = 0;
 
  /* Scan image while deleting the pixels */
 
        count = 0;
 
            /* Build initial previous scan buffer */
 
            p = (mData[0]).getIntensity() != 0;
 
            for (c = 0; c<mCols-1; c++) 
                qb[c] = p = ((p<<1)&0006) | (mData[c+1].getIntensity() != 0);
 
            /* Scan image for pixel deletion candidates */
 
            for (r=0; r<(mRows-1); r++) {
                q = qb[0];
                p = ((q<<3)&0110) | (mData[(r+1)*mCols].getIntensity() != 0);
 
                for (c=0; c<(mCols-1); c++) {
                     q = qb[c];
                     p = ((p<<1) & 0666) | ((q<<3) & 0110) | 
                         (mData[(r+1) * mCols + c+1].getIntensity() != 0);
 
                     qb[c] = p;
 
                    /* if p[r][c] == 0 and more than COEFF nbrs are white */
                    if (erode[p] > coeff){
                           count++;
                           mData[r * mCols + c].setIntensity(255);
                    }
                }
 
                /* Process right edge pixel */
 
                p = (p<<1)&0666;
        if (erode[p] > coeff){
                   count++;
                   mData[r * mCols + mCols-1].setIntensity(255);
                }
            }
 
            /* Process bottom scan line */
 
 
            for (c=0; c<mCols-1; c++) {//Different from book
                   q = qb[c];
                   p = ((p<<1) & 0666) | ((q<<3) & 0110); 
 
           if (erode[p] > coeff){
                      count++;
                      mData[(mRows-1)*mCols + c].setIntensity(255);
                   }
            }
 
/*printf ("erode: pass %d, %d black pixels deleted\n", pc, count);*/
   /*}*/
 
    free (qb);
    return(count);
}	

/* 
 * Take a thresholded image 0 or 255, as input, dilate
 * (shrink white regions, grow black regions).
 * 
 * @param coeff The miniumum number of black neighbors (out of 8) to be considered
 * 	an edge pixel and thus changed to black. Use 0 for classical dialation.
 * @return the number of pixels affected (changed from white to black)
 */
int BinaryImage::doDialate(int coeff) {

	
  int r, c;
  //int i;
  int pc = 0;		/* pass count */
  int count = 1; 	/*deleted pixel count */
  int p, q;		/* neighborhood maps of adjacent cells */
  unsigned char *qb;	/* neighborhood maps of previous scanline */
  //int m;		/* Deletion direction mask */


  qb = (unsigned char *) malloc (mCols * sizeof (unsigned char));
  if (qb == NULL) {
      printf ("Error in memory allocation\n");
      exit (-1);
  }
  qb[mCols-1] = 0;

  /* Scan image while deleting the pixels */

  /*while (count && (pc<6)) {*/
//	pc++;
	count = 0;

	    /* Build initial previous scan buffer */

	    p = (mData[0]).getIntensity() != 0;

	    for (c = 0; c<mCols-1; c++) 
			qb[c] = p = ((p<<1)&0006) | (mData[c+1].getIntensity() != 0);

	    /* Scan image for pixel deletion candidates */

	    for (r=0; r<(mRows-1); r++) {
			q = qb[0];
			p = ((q<<3)&0110) | (mData[(r+1)*mCols].getIntensity() != 0);

			for (c=0; c<(mCols-1); c++) {
			     q = qb[c];
			     p = ((p<<1) & 0666) | ((q<<3) & 0110) | 
				 (mData[(r+1)*mCols + c+1].getIntensity() != 0);
	
			     qb[c] = p;
	
			    /* if p[r][c] == 1 and more than COEFF nbrs are black */
			    if (dilate[p] > coeff && r!=0 && c!=0){
				   count++;
				   mData[r*mCols + c].setIntensity(0);
			    }
			}

			/* Process right edge pixel */
	
			/*p = (p<<1)&0666;
	        if (dilate[p] > coeff){
			   count++;
			   mData[r * mCols + mCols-1].setIntensity(0);
			}*/
	    }

	    /* Process bottom scan line */


	    /*for (c=0; c<mCols-1; c++) {
		   q = qb[c];
		   p = ((p<<1) & 0666) | ((q<<3) & 0110); 

           if (dilate[p] > coeff){
		      count++;
		      mData[(mRows-1) * mCols + c].setIntensity(0);
		   }
	    }*/

 	/*printf ("dilate: pass %d, %d white pixels deleted\n", pc, count);*/
   /*}*/

    free (qb);
    return(count);
}

/** 103AT SAH
 * getPixularity
 * some explanation here.
 * a metric indicating how pixely the image is
 *
 * @return a double representing the average metric
 */
double BinaryImage::getPixularity() {
	int sum = 0;
	int mask;
	

	//TODO: INVESTIGATE AND FIX THIS WHOLE THING. Is it all wrong????
	for (int r=1; r<mRows-1; r++) {
		for (int c=1; c<mCols-1; c++) {
			//ignore edge/border pixels (for now)
		/*	mask=
			    ((mData[(r-1)*mCols + c-1])|127)&
				((mData[(r-1)*mCols + c])|191)&
				((mData[(r-1)*mCols + c+1])|223)&
				((mData[(r)*mCols + c-1])|239)&
				//!!!!OPPS!!!!!!!!!!!! ADD mData[r][c]
				((mData[(r)*mCols + c+1])|247)&
				((mData[(r+1)*mCols + c-1])|251)&
				((mData[(r+1)*mCols + c])|253)&
				((mData[(r+1)*mCols + c+1])|254);*/

			mask = 0 |
				(mData[(r-1)*mCols + c-1].getIntensity()&256)|
				(mData[(r-1)*mCols + c].getIntensity()&128)|
				(mData[(r-1)*mCols + c+1].getIntensity()&64)|
				(mData[(r)*mCols + c-1].getIntensity()&32)|
				(mData[(r)*mCols + c].getIntensity()&16)|
				(mData[(r)*mCols + c+1].getIntensity()&8)|
				(mData[(r+1)*mCols + c-1].getIntensity()&4)|
				(mData[(r+1)*mCols + c].getIntensity()&2)|
				(mData[(r+1)*mCols + c+1].getIntensity()&1);

			sum+=pixularity[mask];

		}
	}
	
	return (double)sum / (mRows*mCols);
}
