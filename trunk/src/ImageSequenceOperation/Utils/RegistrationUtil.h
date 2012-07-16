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
 
using namespace std;
using namespace cv;

/**
 * Function createBitmaps(const IplImage *img, BYTE *tb, BYTE *eb)
 *		Allocate and compute the threshold bitmap tb and the exclusion bitmap eb for the image img.
 * Parameters:
 *		img: the original image to be processed
 *		tb: a bitmap storing the threshold bitmap, converted from IplImage
 *		eb: exclusion bitmap converted from IplImage
 */
void createBitmaps(const IplImage* img, IplImage* &mtb, IplImage* &eb);

/**
 * Function convertIplImageToBitmap(IplImage* pIpl, BYTE* &pBmp, DWORD& size)
 *		Convert ImlImage file format to bitmap (BMP) format
 * Parameters:
 *		pIpl: the original image to be processed
 *		pBmp: a bitmap storing converted data
 *		eb: exclusion bitmap converted from IplImage
 */
void convertIplImageToBitmap(IplImage* pIpl, BYTE* &pBmp, DWORD& size);

/**
 * Function shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel)
 *		Shink an image by factor of 2, a quater of the original size
 * Parameters:
 *		sourceImage: the original image to be processed
 *		nextLevel: the shrunk image whose size is a quarter of the source image
 */
void shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel);