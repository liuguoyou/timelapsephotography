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
#define BLOCK_SIZE 1
#define SEARCH_RANGE 5
#define PYRAMID_LEVEL 5
#define MATCH_THRESHOLD 0.9
 
using namespace std;
using namespace cv;

/*******************************************
 * struct definition
 *******************************************/
struct blockMotionVector{
	int x;
	int y;
	int shiftX;
	int shiftY;
	struct blockMotionVector *nextPtr;
};

struct distanceFrequencyNode{
	int shiftX;
	int shiftY;
	long frequency;
	struct distanceFrequencyNode *nextPtr;
};

struct imageInfoNode{  
	long imageIndex;
	long matchedImageNumbers;
	int shiftX;
	int shiftY; //The shifts are next image group referenced to the current group
	struct imageInfoNode * previousPtr;
	struct imageInfoNode * nextPtr;
};


/*******************************************
 * typedef
 *******************************************/
typedef struct blockMotionVector BlockMotionVector;
typedef BlockMotionVector *BlockMotionVectorPtr;

typedef struct distanceFrequencyNode DistanceFrequencyNode;
typedef DistanceFrequencyNode *DistanceFrequencyNodePtr;

typedef struct imageInfoNode ImageInfoNode;
typedef ImageInfoNode * ImageInfoNodePtr;


/** Function isQueueEmpty(BlockMotionVectorPtr headPtr)
 *		To judge if the queue is empty
 *	Return:
 *		if ture, return 1, otherwise, 0
 */
int isMotionVectorQueueEmpty(BlockMotionVectorPtr headPtr);

/** Function enqueueMotionVectors(BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr, int x, int y, int shiftX, int shiftY)
 *		To insert an new node to the end of the queue
 *	Parameters:
 *		headPtr: address of head node of the queue
 *		tailPtr: address of head node of the queue
 *		x, y: position information of the block, in current frame
 *		shiftX, shiftY: shift information of the block, which is named motion vector
 */
void enqueueMotionVectors(BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr, int x, int y, int shiftX, int shiftY);

/** Function isDistanceFrequencyQueueEmpty(DistanceFrequencyNodePtr headPtr)
 *		To judge if the queue is empty
 *	Return:
 *		if ture, return 1, otherwise, 0
 */
int isDistanceFrequencyQueueEmpty(DistanceFrequencyNodePtr headPtr);

/** 
 * Function enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int sX, int sY);
 *		Insert a node to the tail of the queue
 */
void enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int sX, int sY);

/** Function isImageInfoQueueEmpty(BlockMotionVectorPtr headPtr)
 *		To judge if the queue is empty
 *	Return:
 *		if ture, return 1, otherwise, 0
 */
int isImageInfoQueueEmpty(ImageInfoNodePtr headPtr);

/** 
 * Function enqueueImageInfo(ImageInfoNodePtr *headPtr, ImageInfoNodePtr * tailPtr, int imageIndex, long matchedImageNumbers, int shiftX, int shiftY);
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
 * Function shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel)
 *		Shink an image by factor of 2, a quater of the original size
 * Parameters:
 *		sourceImage: the original image to be processed
 *		nextLevel: the shrunk image whose size is a quarter of the source image
 */
void shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel);

/**
 * Function shiftImage(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result)
 *		Shift an image to four directions, by four different cases in x and y offsets combinations
 * Parameters:
 *		sourceImage: the original image to be processed
 *		xOffset, yOffset: offsets that defines the shifted image, positive values define right and downward directions, and negative values denote left and upward directions
 *		result: Shifted image, cropped, with white margins.
 */
void shiftImage(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result);

/** Function getBlockShift(IplImage * referenceFrame, IplImage * currentFrame, BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr)
 *		Main function of block matching algorithm that are used in motion estimation, block size is preprocessored, as well as search range
 *		The final motion vector is stored as a node which is then insert to a queue
 *  Parameters:
 *		*referenceFrame: reference frame pointer, which points to the reference image, the previous one.
 *		*currentFrame: current frame pointer, which points to the second image where motion has occured in contrast to the reference frame
 *		*headPtr: head node address of the queue
 *		*tailPtr: tail node address of the queue
 */
void getBlockShift(IplImage * referenceFrame, IplImage * currentFrame, BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr);

/** Function getImageShift(BlockMotionVectorPtr motionVectorQueueNode, DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int shiftCo[])
 *		Find the shift array that stores the x and y shift of the current frame, which will be shifted to do the registration
 */
void getImageShift(BlockMotionVectorPtr motionVectorQueueNode, DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int shiftCo[]);

/**
 * Function findReferencePoint(ImageInfoNodePtr *headImageInfoNodePtr, ImageInfoNodePtr *tailImageInfoNodePtr, vector<string>& srcFileNames)
 *		Try to find the best registration point inorder to do tha fastest registration
 * Parameters:
 *		srcFileNames: the vector that stores file names in the folder which contains all files in image sequence
 * Return:
 *		reference number, an integer that denotes the image index which will be select as reference.
 */
int findReferencePoint(ImageInfoNodePtr *headImageInfoNodePtr, ImageInfoNodePtr *tailImageInfoNodePtr, vector<string>& srcFileNames);

/**
 * Function findReferencePoint(ImageInfoNodePtr *headImageInfoNodePtr, ImageInfoNodePtr *tailImageInfoNodePtr, vector<string>& srcFileNames)
 *		Try to find the best registration point inorder to do tha fastest registration
 * Parameters:
 *		srcFileNames: the vector that stores file names in the folder which contains all files in image sequence
 * Return:
 *		reference number, an integer that denotes the image index which will be select as reference.
 */
void adjustShiftsToReferenceImage(ImageInfoNodePtr imageInfoNode, ImageInfoNodePtr * tailImageInfoNode, long referencePoint);

/**
 * Function shiftImageSequence(ImageInfoNodePtr imageInfoNode, vector<string>& srcFileNames, vector<string>& dstFileNames)
 *		Shift the entire sequence one by one, and save to new files
 * Parameters:
 *		imageInfoNode: the queue node that stores shift information of the image
 *		srcFileNames: the vector that includes all source image file names
 *		dstFileNames: the vector that includes all new image file names where they could be saved
 */
void shiftImageSequence(ImageInfoNodePtr imageInfoNode, vector<string>& srcFileNames, vector<string>& dstFileNames);