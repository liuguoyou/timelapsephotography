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
//void convertIplImageToBitmap(IplImage* pIpl, BYTE* &pBmp, DWORD& size);

/**
 * Function shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel)
 *		Shink an image by factor of 2, a quater of the original size
 * Parameters:
 *		sourceImage: the original image to be processed
 *		nextLevel: the shrunk image whose size is a quarter of the source image
 */
void shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel);

/**
 * Function shiftBitMap(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result)
 *		Shift an image to four directions, by four different cases in x and y offsets combinations
 * Parameters:
 *		sourceImage: the original image to be processed
 *		xOffset, yOffset: offsets that defines the shifted image, positive values define right and downward directions, and negative values denote left and upward directions
 *		result: Shifted image, cropped, with white margins.
 */
void shiftBitMap(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result);

/**
 * Function  xorBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result)
 *		Compute the XOR value result of two images.
 * Parameters:
 *		sourceImage 1, 2: the original images to be processed
 *		result: image storing the XOR result
 */
void xorBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result);

/**
 * Function  totalOneInBitMap(const IplImage* srcImage)
 *		Compute total ones in an images
 * Parameters:
 *		sourceImage: the original images to be processed
 * Return:
 *		Numbers of one in an image.
 */
int totalOneInBitMap(const IplImage* srcImage);