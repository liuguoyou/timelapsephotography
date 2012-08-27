#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <iostream>
#include <io.h>
#include <string.h>

using namespace std;
using namespace cv;

/*******************************************
 * defines
 *******************************************/
#define BLOCKSIZE 8
#define SEARCHRANGE 16
#define PYRAMIDLEVEL 4
#define INTERPOLATION_RANGE 0.8

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


/*******************************************
 * typedef
 *******************************************/
typedef struct blockMotionVector BlockMotionVector;
typedef BlockMotionVector *BlockMotionVectorPtr;

typedef struct distanceFrequencyNode DistanceFrequencyNode;
typedef DistanceFrequencyNode *DistanceFrequencyNodePtr;


/** Function getFileNames(string path, vector<string>& files)
 *		To search in a directory, including all files, typically image files in this case, and store their names in a vector
 *	Parameters:
 *		path: the directory path to be searched
 *		files: the vector that used to store file names with directory path
 */
void getFileNames(string path, vector<string>& entireNames, vector<string>& fileNames);

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

/** Function enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, double distance);
 *		To insert an new node to the end of the queue
 *	Parameters:
 *		headPtr: address of head node of the queue
 *		tailPtr: address of head node of the queue
 *		distance: the distance computed from motion vector
 */
void enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int shiftX, int shiftY);

/** Function square(int a)
 *		To compute square value of integer a
 *	Return:
 *		double value of a^2
 */
double square(int a);

/** Function shiftedDistance(int shiftedX, int shiftedY)
 *		Compute the distance according to the coordinates (x, y)
 *  Return:
 *		A double value that denote distance
 */
double shiftedDistance(int shiftedX, int shiftedY);

/** Function getPixel(const IplImage* image, int x, int y)
 *		returns value between 0 and 255 of pixel at image position (x,y)
 *	Return:
 *		grayvalue of pixel(x, y)
 *	Used for testing in segmentation
 */
unsigned char getPixel(const IplImage* image, int x, int y);

/** Function getColorPixel(const IplImage* image, int x, int y, int colorChannel)
 *		returns color value between 0 and 255 of pixel at image position (x,y), channel: 0 - blue, 1 - green, 2 - red
 *	Return:
 *		color value as per channel of pixel(x, y)
 */
unsigned char getColorPixel(const IplImage* image, int x, int y, int colorChannel);

/** Function setPixel(IplImage* image, int x, int y, unsigned char value)
 *		sets pixel at image position (x,y), according to grayvalue
 */
void setPixel(IplImage* image, int x, int y, unsigned char value);

/** Function setColorPixel(IplImage* image, int x, int y, unsigned char value, int colorChannel)
 *		sets pixel at image position (x,y), according to color channel and values, channel: 0 - blue, 1 - green, 2 - red
 */
void setColorPixel(IplImage* image, int x, int y, unsigned char value, int colorChannel);

/** Function allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
 *		it allocates the requested image
 *		unless that image is already non-NULL.  It always leaves a non-NULL image as-is even
 *		if that image's size, depth, and/or channels are different than the request.
 */
void allocateOnDemand(IplImage **img, CvSize size, int depth, int channels);

#endif