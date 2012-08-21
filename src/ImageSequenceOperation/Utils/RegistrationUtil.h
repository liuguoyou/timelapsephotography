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
#define PYRAMID_LEVEL 5
#define MATCH_THRESHOLD 0.9
 
using namespace std;
using namespace cv;


struct imageInfoNode{
	int imageIndex;
	long matchedImageNumbers;
	int shiftX;
	int shiftY;
	struct imageInfoNode * previousPtr;
	struct imageInfoNode * nextPtr;
};

typedef struct imageInfoNode ImageInfoNode;
typedef ImageInfoNode * ImageInfoNodePtr;


/** Function isImageInfoQueueEmpty(BlockMotionVectorPtr headPtr)
 *		To judge if the queue is empty
 *	Return:
 *		if ture, return 1, otherwise, 0
 */
int isImageInfoQueueEmpty(ImageInfoNodePtr headPtr);

/** Function enqueueImageInfo(ImageInfoNodePtr *headPtr, ImageInfoNodePtr * tailPtr, int imageIndex, long matchedImageNumbers, int shiftX, int shiftY);
 *		Insert a node to the tail of the queue, which is double linked with previous and next address that are linked to neighbours
 */
void enqueueImageInfo(ImageInfoNodePtr *headPtr, ImageInfoNodePtr * tailPtr, int imageIndex, long matchedImageNumbers, int shiftX, int shiftY);

/**
 * Function createGrayPlane(const IplImage *srcImage, IplImage* &grayPlane)
 *		Allocate and compute the gray plane of the source image.
 * Parameters:
 *		srcImage: the original image to be processed
 *		grayPlane: image storing the gray plane
 */
void createGrayPlane(const IplImage *srcImage, IplImage* &grayPlane);

/**
 * Function isImagesMatch(const IplImage *srcGrayImage1, const IplImage *srcGrayImage2)
 *		Substract two images and judge weather two images are matched by finding the percentage of difference images
 * Parameters:
 *		srcGrayImage1: the first original gray image to be processed
 *		srcGrayImage2: the first original gray image to be processed
 */
int isImagesMatch(const IplImage *srcGrayImage1, const IplImage *srcGrayImage2);

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
 * Function shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel)
 *		Shink an image by factor of 2, a quater of the original size
 * Parameters:
 *		sourceImage: the original image to be processed
 *		nextLevel: the shrunk image whose size is a quarter of the source image
 */
void shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel);

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
 * Function  andBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result)
 *		Compute the AND value result of two images.
 * Parameters:
 *		sourceImage 1, 2: the original images to be processed
 *		result: image storing the AND result
 */
void andBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result);

/**
 * Function  totalOneInBitMap(const IplImage* srcImage)
 *		Compute total ones in an images
 * Parameters:
 *		sourceImage: the original images to be processed
 * Return:
 *		Numbers of one in an image.
 */
int totalOneInBitMap(const IplImage* srcImage);

/** 
 *
 */

/**
 * Function  getExpShift(const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[])
 *		find shift in X and Y directions, for an image, with one reference, shiftBits is specified to control the pyramid level
 * Parameters:
 *		srcImage1: the reference image
 *		srcImage2: the image to be computed
 *		shiftBits: an integer to control pyramid level
 *		shiftRet[]: array contains two integers that denote shifts in X and Y directions
 */
void getExpShift(const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[]);

/**
 * Function findReferencePoint(vector<string> srcFileNames)
 *		Try to find the best registration point inorder to do tha fastest registration
 * Parameters:
 *		srcFileNames: the vector that stores file names in the folder which contains all files in image sequence
 * Return:
 *		reference number, an integer that denotes the image index which will be select as reference.
 */
int findReferencePoint(vector<string> srcFileNames);