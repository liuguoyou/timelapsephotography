/**
 * This header file defines functions that used for image registration, listed as follows:
 *		* createBitmaps(const IplImage *img, BYTE *tb, BYTE *eb)
 *		* convertIplImageToBitmap(IplImage* pIpl, BYTE* &pBmp, DWORD& size)
 */

#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <io.h>
#include <string.h>

/*******************************************
 * defines
 *******************************************/
#define BLOCK_SIZE 2
#define SEARCH_RANGE 4
#define PYRAMID_LEVEL 5
#define MATCH_THRESHOLD 0.9
 
using namespace std;
using namespace cv;

/**
 * Function createBitmaps(const IplImage *img, BYTE *tb, BYTE *eb)
 */
void createBitmaps(const IplImage *srcImage, IplImage* &mtb, IplImage* &eb);

/**
 * Function void shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel)
 */
void shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel);

/**
 * Function shiftBitMap(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result)
 */
void shiftBitMap(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result);

/**
 * Function  xorBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result)
 */
void xorBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result);

/**
 * Function andBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result)
 */
void andBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result);

/**
 * Function totalOneInBitMap(const IplImage* srcImage)
 */
int totalOneInBitMap(const IplImage* srcImage);

/**
 * Function getExpShift(const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[])
 */
void getExpShift(const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[]);