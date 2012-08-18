#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>

/*******************************************
 * defines
 *******************************************/
#define BLOCKSIZE 8
#define SEARCHRANGE 16
#define PYRAMIDLEVEL 1

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
	double distance;
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


/** Function isQueueEmpty(BlockMotionVectorPtr headPtr)
 *		To judge if the queue is empty
 *	Return:
 *		if ture, return 1, otherwise, 0
 */
int isMotionVectorQueueEmpty(BlockMotionVectorPtr headPtr){
	return headPtr == NULL;
}

/** Function enqueueMotionVectors(BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr, int x, int y, int shiftX, int shiftY)
 *		To insert an new node to the end of the queue
 *	Parameters:
 *		headPtr: address of head node of the queue
 *		tailPtr: address of head node of the queue
 *		x, y: position information of the block, in current frame
 *		shiftX, shiftY: shift information of the block, which is named motion vector
 */
void enqueueMotionVectors(BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr,
						  int x, int y, int shiftX, int shiftY){

	BlockMotionVectorPtr newVectorPtr;
	newVectorPtr = (BlockMotionVectorPtr)malloc(sizeof(BlockMotionVector));

	if(newVectorPtr != NULL){
		newVectorPtr->x = x;
		newVectorPtr->y = y;
		newVectorPtr->shiftX = shiftX;
		newVectorPtr->shiftY = shiftY;
		newVectorPtr->nextPtr = NULL;

		if(isMotionVectorQueueEmpty(*headPtr)){
			*headPtr = newVectorPtr;
		} else {
			(*tailPtr)->nextPtr = newVectorPtr;
		}

		*tailPtr = newVectorPtr;
	} else{
		fprintf(stderr, "Error: New Motion vector is not inserted, Out of memory?\n");
		exit(-1);
	}

}

/** Function isDistanceFrequencyQueueEmpty(DistanceFrequencyNodePtr headPtr)
 *		To judge if the queue is empty
 *	Return:
 *		if ture, return 1, otherwise, 0
 */
int isDistanceFrequencyQueueEmpty(DistanceFrequencyNodePtr headPtr){
	return headPtr == NULL;
}


void enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, double distance){

	if(isDistanceFrequencyQueueEmpty(*headPtr)){
	//The first node to enter the queue
		DistanceFrequencyNodePtr newDistancePtr;
		newDistancePtr = (DistanceFrequencyNodePtr)malloc(sizeof(DistanceFrequencyNode));
		
		if(newDistancePtr != NULL){
			newDistancePtr->distance = distance;
			newDistancePtr->frequency = 1;
			newDistancePtr->nextPtr = NULL;

			*headPtr = newDistancePtr;
			
			*tailPtr = newDistancePtr;
		} else{
			fprintf(stderr, "Error: New Distance Frequency is not inserted, Out of memory?\n");
			exit(-1);
		}
	} else{
	//Not the first to enter the queue, compare from head, then consider insertion
		DistanceFrequencyNodePtr tempPtr;
		tempPtr = (DistanceFrequencyNodePtr)malloc(sizeof(DistanceFrequencyNode));
		
		if(tempPtr != NULL){
			tempPtr = *headPtr;
		} else{
			fprintf(stderr, "Error: New Temp Distance Frequency is not inserted, Out of memory?\n");
			exit(-1);
		}

		int isDistanceInQueue = 0;

		while(tempPtr != NULL){
			if(distance == tempPtr->distance){
				tempPtr->frequency++;
				isDistanceInQueue = 1;
				break;
			} else {
				tempPtr = tempPtr->nextPtr;
			}
		}

		if(!isDistanceInQueue){
			DistanceFrequencyNodePtr newDistancePtr;
			newDistancePtr = (DistanceFrequencyNodePtr)malloc(sizeof(DistanceFrequencyNode));
			
			if(newDistancePtr != NULL){
				newDistancePtr->distance = distance;
				newDistancePtr->frequency = 1;
				newDistancePtr->nextPtr = NULL;

				(*tailPtr)->nextPtr = newDistancePtr;
				
				*tailPtr = newDistancePtr;
			} else{
				fprintf(stderr, "Error: New Distance Frequency is not inserted, Out of memory?\n");
				exit(-1);
			}
		}

		//free(tempPtr);
	}
}

/** Function square(int a)
 *		To compute square value of integer a
 *	Return:
 *		double value of a^2
 */
double square(int a)
{
	return a * a;
}

/** Function getPixel(const IplImage* image, int x, int y)
 *		returns value between 0 and 255 of pixel at image position (x,y)
 *	Return:
 *		grayvalue of pixel(x, y)
 *	Used for testing in segmentation
 */
unsigned char getPixel(const IplImage* image, int x, int y){
  return ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels];
}

/** Function getColorPixel(const IplImage* image, int x, int y, int colorChannel)
 *		returns color value between 0 and 255 of pixel at image position (x,y), channel: 0 - blue, 1 - green, 2 - red
 *	Return:
 *		color value as per channel of pixel(x, y)
 */
unsigned char getColorPixel(const IplImage* image, int x, int y, int colorChannel){
  return ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels + colorChannel];
}

/** Function setPixel(IplImage* image, int x, int y, unsigned char value)
 *		sets pixel at image position (x,y), according to grayvalue
 */
void setPixel(IplImage* image, int x, int y, unsigned char value){
  ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels] = value;
}

/** Function setColorPixel(IplImage* image, int x, int y, unsigned char value, int colorChannel)
 *		sets pixel at image position (x,y), according to color channel and values, channel: 0 - blue, 1 - green, 2 - red
 */
void setColorPixel(IplImage* image, int x, int y, unsigned char value, int colorChannel){
  ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels + colorChannel] = value;
}

/** Function allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
 *		it allocates the requested image
 *		unless that image is already non-NULL.  It always leaves a non-NULL image as-is even
 *		if that image's size, depth, and/or channels are different than the request.
 */
void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
{
	if ( *img != NULL )	return;

	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		fprintf(stderr, "Error: Couldn't allocate image.  Out of memory?\n");
		exit(-1);
	}
}


/** Function shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel)
 *		Shrink image to form a image pyramid according to the level that defined at macro
 */
void shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel){
	if(downwardLevel == 0) return;

	IplImage * tempImage = NULL;

	allocateOnDemand(&tempImage, cvSize(srcImage->width / 2, srcImage->height / 2), srcImage->depth, srcImage->nChannels);

	cvPyrDown(srcImage, tempImage, CV_GAUSSIAN_5x5);

	if(downwardLevel == 1){	
		//allocateOnDemand(dstImage, cvGetSize(tempImage), tempImage->depth, tempImage->nChannels);
		*dstImage = cvCloneImage(tempImage);
		cvReleaseImage(&tempImage);
	} else{
		downwardLevel--;
		shrinkImage(tempImage, dstImage, downwardLevel);
	}
}

/** Function getBlockShift(IplImage * referenceFrame, IplImage * currentFrame, BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr)
 *		Main function of block matching algorithm that are used in motion estimation, block size is preprocessored, as well as search range
 *		The final motion vector is stored as a node which is then insert to a queue
 *  Parameters:
 *		*referenceFrame: reference frame pointer, which points to the reference image, the previous one.
 *		*currentFrame: current frame pointer, which points to the second image where motion has occured in contrast to the reference frame
 *		*headPtr: head node address of the queue
 *		*tailPtr: tail node address of the queue
 */
void getBlockShift(IplImage * referenceFrame, IplImage * currentFrame, BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr){
	int imageWidth = referenceFrame->width;
	int imageHeight = referenceFrame->height;

	int imageIndexX, imageIndexY;
	int rangeShiftX, rangeShiftY;
	int blockIndexX, blockIndexY;

	double smallestSquareSum = 10000.0000;
	int smallestShifts[2];

	double squareSumMatrix[SEARCHRANGE * SEARCHRANGE] = {10000.0000};
	int matrixIndex = 0;


	//calculation loop, adjust here for pixelwise or blockwise
	for(imageIndexX = 0; imageIndexX < imageWidth - BLOCKSIZE; imageIndexX+=BLOCKSIZE)
		for(imageIndexY = 0; imageIndexY < imageHeight - BLOCKSIZE; imageIndexY+=BLOCKSIZE){

			//smallestSquareSum = 10000.0000;
			smallestShifts[0] = 0;
			smallestShifts[1] = 0;

			
			int isFirstSumInRange = 1;

			for(rangeShiftX = -SEARCHRANGE; rangeShiftX < SEARCHRANGE; rangeShiftX++)
				for(rangeShiftY = -SEARCHRANGE; rangeShiftY < SEARCHRANGE; rangeShiftY++){

					
					int isThePositionOutOfRange = 0;
					double sum = 0;
					
					//double temp;
					//int isOutOfRange;

					for(blockIndexX = imageIndexX; blockIndexX < imageIndexX + BLOCKSIZE; blockIndexX++)
						for(blockIndexY = imageIndexY; blockIndexY < imageIndexY + BLOCKSIZE; blockIndexY++){
							
							//Control the calculated point inside the image range
							if(blockIndexX < imageWidth && blockIndexY < imageHeight && (blockIndexX + rangeShiftX) >= 0 && (blockIndexX + rangeShiftX) < imageWidth && (blockIndexY + rangeShiftY) >= 0 && (blockIndexY + rangeShiftY) < imageHeight){
								int grayDiff = getPixel(currentFrame, blockIndexX, blockIndexY) - getPixel(referenceFrame, blockIndexX + rangeShiftX, blockIndexY + rangeShiftY);
								sum += grayDiff * grayDiff;
							} else{
								isThePositionOutOfRange = 1;
							}
						}

					//squareSumMatrix[matrixIndex] = sum;
					//matrixIndex++;
					if(!isThePositionOutOfRange){
						if(isFirstSumInRange){
							smallestSquareSum = sum;
							smallestShifts[0] = rangeShiftX;
							smallestShifts[1] = rangeShiftY;
							isFirstSumInRange = 0;
						} else{
							if(sum <= smallestSquareSum){
								smallestSquareSum = sum;
								smallestShifts[0] = rangeShiftX;
								smallestShifts[1] = rangeShiftY;
							}
						}
					}
					
				}
				enqueueMotionVectors(headPtr, tailPtr, imageIndexX, imageIndexY, smallestShifts[0], smallestShifts[1]);
				
		}
}

/** Function shiftedDistance(int shiftedX, int shiftedY)
 *		Compute the distance according to the coordinates (x, y)
 *  Return:
 *		A double value that denote distance
 */
double shiftedDistance(int shiftedX, int shiftedY){
	return sqrt(double(square(shiftedX) + square(shiftedY)));
}

/** Function fillBlock(IplImage ** image, int beginPositionX, int beginPositionY, int colorValue)
 *		Assistant function for segmentation, which is used for test the block matching algorithm
 *		Fill gray value to a block according to the position in image
 */
void fillBlock(IplImage ** image, int beginPositionX, int beginPositionY, int colorValue){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); j++)
			setPixel(*image, i, j, colorValue);
}

/** Function segmentImage(IplImage ** newImage, BlockMotionVectorPtr motionVectorQueueNode, double threshold)
 *		Segment image to two values(0 and 255) to see the first - stage effect of the block matching algorithm 
 */
void segmentImage(IplImage ** newImage, BlockMotionVectorPtr motionVectorQueueNode, double threshold){

	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			if(shiftedDistance(motionVectorQueueNode->shiftX, motionVectorQueueNode->shiftY) >= threshold){
				fillBlock(newImage, motionVectorQueueNode->x * (pow(2.0, PYRAMIDLEVEL)), motionVectorQueueNode->y * (pow(2.0, PYRAMIDLEVEL)), 255);
			} else{
				fillBlock(newImage, motionVectorQueueNode->x * (pow(2.0, PYRAMIDLEVEL)), motionVectorQueueNode->y * (pow(2.0, PYRAMIDLEVEL)), 0);
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}
	}
}

/*
void printMotionVectors(BlockMotionVectorPtr motionVectorQueueNode){
	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			if(motionVectorQueueNode->shiftX != 0 || motionVectorQueueNode->shiftY !=0){
				printf("p: %d, %d;  (%d, %d)\n", motionVectorQueueNode->x, motionVectorQueueNode->y, motionVectorQueueNode->shiftX != 0, motionVectorQueueNode->shiftY);
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}
	}

}
*/


/** Function computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode)
 *		First version: to calculate the mean value of all distance, which is the simplest method to find a distance threshold
 */
double computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode){
	long count = 0;
	double sum = 0;

	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			
			if(motionVectorQueueNode->shiftX != 0 || motionVectorQueueNode->shiftY !=0){
				count++;
				sum += sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY));
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}

		return sum / count;
	}
}

/** Function computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode)
 *		First version: to calculate the mean value of all distance, which is the simplest method to find a distance threshold
 */
double weightedMeanDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode, DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr){
	long count = 0;
	double sum = 0;

	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			if(motionVectorQueueNode->shiftX != 0 || motionVectorQueueNode->shiftY != 0){	
				enqueueDistanceFrequency(headPtr, tailPtr, sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY)));
			
				count++;	
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}

		DistanceFrequencyNodePtr tempPtr;
		tempPtr = (DistanceFrequencyNodePtr)malloc(sizeof(DistanceFrequencyNode));

		if(tempPtr != NULL){
			tempPtr = *headPtr;
		} else{
			fprintf(stderr, "Error: New Temp Distance Frequency is not inserted, Out of memory?\n");
			exit(-1);
		}

		while(tempPtr != NULL){

			sum += tempPtr->distance * tempPtr->frequency;

			tempPtr = tempPtr->nextPtr;
		}

		//free(tempPtr);

		return sum / count;
	}
}



/** Function fillBackGroundBlock(IplImage ** newFrame, IplImage * refFrame, IplImage * curFrame, int beginPositionX, int beginPositionY)
 *		Assistant function for creation of the new image
 *		Pixels in background for is calculated as an average of two frames
 */
void fillBackGroundBlock(IplImage ** newFrame, IplImage * refFrame, IplImage * curFrame, int beginPositionX, int beginPositionY){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); j++){
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 0) + getColorPixel(refFrame,i, j, 0)) / 2, 0);
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 1) + getColorPixel(refFrame,i, j, 1)) / 2, 1);
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 2) + getColorPixel(refFrame,i, j, 2)) / 2, 2);
		}		
}

/** Function fillMotionBlock(IplImage ** newFrame, IplImage * curFrame, int beginPositionX, int beginPositionY, int shiftX, int shiftY)
 *		Assistant function for creation of the new image
 *		Pixels in motion area is copy from current frame, with shifted half value by motion vector.
 */
void fillMotionBlock(IplImage ** newFrame, IplImage * curFrame, int beginPositionX, int beginPositionY, int shiftX, int shiftY){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); j++){

			if(i - shiftX >= 0 && j - shiftY >= 0 && i + shiftX < curFrame->width && j + shiftY < curFrame->height){		
				setColorPixel(*newFrame, i, j, getColorPixel(curFrame, i - shiftX / 2, j - shiftY / 2, 0), 0);
				setColorPixel(*newFrame, i, j, getColorPixel(curFrame, i - shiftX / 2, j - shiftY / 2, 1), 1);
				setColorPixel(*newFrame, i, j, getColorPixel(curFrame, i - shiftX / 2, j - shiftY / 2, 2), 2);
			}
		}		
}
 
/** Function createNewFrame(IplImage ** newFrame, IplImage *refFrame, IplImage *curFrame, BlockMotionVectorPtr motionVectorQueueNode, double threshold)
 *		Main function of creating the new image between frames 
 */
void createNewFrame(IplImage ** newFrame, IplImage *refFrame, IplImage *curFrame, BlockMotionVectorPtr motionVectorQueueNode, double threshold){
	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			if(shiftedDistance(motionVectorQueueNode->shiftX, motionVectorQueueNode->shiftY) >= threshold){
				fillMotionBlock(newFrame, curFrame, motionVectorQueueNode->x * (pow(2.0, PYRAMIDLEVEL)), motionVectorQueueNode->y * (pow(2.0, PYRAMIDLEVEL)), motionVectorQueueNode->shiftX * (pow(2.0, PYRAMIDLEVEL)), motionVectorQueueNode->shiftY * (pow(2.0, PYRAMIDLEVEL)));

			} else{
				fillBackGroundBlock(newFrame, refFrame, curFrame, motionVectorQueueNode->x * (pow(2.0, PYRAMIDLEVEL)), motionVectorQueueNode->y * (pow(2.0, PYRAMIDLEVEL)));
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}
	}
}


/****************************************************************************************************************************
 * Main Function
 ****************************************************************************************************************************/
int main(void){

	//Read images
	IplImage * refFrame = cvLoadImage("images/test/P1030703.jpg");
	IplImage * curFrame = cvLoadImage("images/test/P1030704.jpg");

	BlockMotionVectorPtr headMotionVectorPtr = NULL;
	BlockMotionVectorPtr tailMotionVectorPtr = NULL;

	DistanceFrequencyNodePtr headDistanceFrequencyPtr = NULL;
	DistanceFrequencyNodePtr tailDistanceFrequencyPtr = NULL;

	/*
	cvNamedWindow("test", 1); 
	cvShowImage("test", refFrame);
	cvWaitKey(0);
	*/
	//Covert to grayscale images
	
	IplImage * refGrayPlane = NULL, * curGrayPlane = NULL, *segmentedImage = NULL,
			 * shrunkrefPlane = NULL, * shrunkcurPlane = NULL, * newFrame = NULL;

	allocateOnDemand(&newFrame, cvGetSize(curFrame), curFrame->depth, curFrame->nChannels);

	/*

	testFillMotionBlock(&newFrame, curFrame, 0, 0, 200, 200);

	cvNamedWindow("newFrame", 1); 
	cvShowImage("newFrame", newFrame);
	cvWaitKey(0);
	*/
	// Test set pixels
	/*
	for(int i = 0; i < curFrame->width; i++)
		for(int j = 0; j < curFrame->height; j++){
			setColorPixel(newImage, i, j, (getColorPixel(curFrame,i, j, 0) + getColorPixel(refFrame,i, j, 0)) / 2, 0);
			setColorPixel(newImage, i, j, (getColorPixel(curFrame,i, j, 1) + getColorPixel(refFrame,i, j, 1)) / 2, 1);
			setColorPixel(newImage, i, j, (getColorPixel(curFrame,i, j, 2) + getColorPixel(refFrame,i, j, 2)) / 2, 2);
		}

	cvNamedWindow("newFrame", 1); 
	cvShowImage("newFrame", newImage);
	cvWaitKey(0);
	*/


	allocateOnDemand(&refGrayPlane, cvGetSize(refFrame), IPL_DEPTH_8U, 1);
	cvCvtColor(refFrame, refGrayPlane, CV_BGR2GRAY);

	allocateOnDemand(&curGrayPlane, cvGetSize(curFrame), IPL_DEPTH_8U, 1);
	cvCvtColor(curFrame, curGrayPlane, CV_BGR2GRAY);

	allocateOnDemand(&segmentedImage, cvGetSize(curFrame), IPL_DEPTH_8U, 1);

	shrinkImage(refGrayPlane, &shrunkrefPlane, PYRAMIDLEVEL);
	shrinkImage(curGrayPlane, &shrunkcurPlane, PYRAMIDLEVEL);
	/*
	shrinkImage(refGrayPlane, &shrunkImage, 2);
	cvNamedWindow("ShunkImage", 1); 
	cvShowImage("ShunkImage", shrunkImage);
	cvWaitKey(0);
	*/

	//Without pyramid
	//getBlockShift(refGrayPlane, curGrayPlane, &headMotionVectorPtr, &tailMotionVectorPtr);

	//Do with pyramid to speed up
	getBlockShift(shrunkrefPlane, shrunkcurPlane, &headMotionVectorPtr, &tailMotionVectorPtr);

	
	//printMotionVectors(headMotionVectorPtr);

	//Print out threshold
	double distanceThreshold = computeDistanceThreshold(headMotionVectorPtr);
	double weightedDistanceThreshold = weightedMeanDistanceThreshold(headMotionVectorPtr, &headDistanceFrequencyPtr, &tailDistanceFrequencyPtr);
	printf("Mean Threshold: %f\n", distanceThreshold);
	printf("Weighted Mean Threshold: %f\n", weightedDistanceThreshold);

	segmentImage(&segmentedImage, headMotionVectorPtr, distanceThreshold);
	createNewFrame(&newFrame, refFrame, curFrame, headMotionVectorPtr, distanceThreshold);

	cvSaveImage("images/test/new.jpg", newFrame);

	cvNamedWindow("Segmented", 1); 
	cvShowImage("Segmented", segmentedImage);
	//cvWaitKey(0);

	cvNamedWindow("NewFrame", 1); 
	cvShowImage("NewFrame", newFrame);
	cvWaitKey(0);

	//Release memories
	cvReleaseImage(&refFrame);
	cvReleaseImage(&curFrame);
	cvReleaseImage(&refGrayPlane);
	cvReleaseImage(&curGrayPlane);
	cvReleaseImage(&shrunkrefPlane);
	cvReleaseImage(&shrunkcurPlane);
	cvReleaseImage(&segmentedImage);
	cvReleaseImage(&newFrame);


	return 0;
}