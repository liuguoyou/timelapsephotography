#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>

/*******************************************
 * defines
 *******************************************/
#define BLOCKSIZE 1
#define SEARCHRANGE 5
#define PYRAMIDLEVEL 5

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


void enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int sX, int sY){

	if(isDistanceFrequencyQueueEmpty(*headPtr)){
	//The first node to enter the queue
		DistanceFrequencyNodePtr newDistancePtr;
		newDistancePtr = (DistanceFrequencyNodePtr)malloc(sizeof(DistanceFrequencyNode));
		
		if(newDistancePtr != NULL){
			newDistancePtr->shiftX = sX;
			newDistancePtr->shiftY = sY;
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
			if(sX == tempPtr->shiftX && sY == tempPtr->shiftY){
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
				newDistancePtr->shiftX = sX;
				newDistancePtr->shiftY = sY;
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



/** Function getImageShift(BlockMotionVectorPtr motionVectorQueueNode, DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int shiftCo[])
 *		Find the shift array that stores the x and y shift of the current frame, which will be shifted to do the registration
 */
void getImageShift(BlockMotionVectorPtr motionVectorQueueNode, DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int shiftCo[]){
	long count = 0;
	double sum = 0;

	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			if(motionVectorQueueNode->shiftX != 0 || motionVectorQueueNode->shiftY != 0){	
				enqueueDistanceFrequency(headPtr, tailPtr, motionVectorQueueNode->shiftX, motionVectorQueueNode->shiftY);
			
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


		double smallestDistance = sqrt(square(tempPtr->shiftX) + square(tempPtr->shiftY));
		int largestFrequency = tempPtr->frequency;
		int shiftX = tempPtr->shiftX;
		int shiftY = tempPtr->shiftY;
		tempPtr = tempPtr->nextPtr;
		

		while(tempPtr != NULL){

			if(sqrt(square(tempPtr->shiftX) + square(tempPtr->shiftY)) <= smallestDistance && tempPtr->frequency >= largestFrequency){
				smallestDistance = sqrt(square(tempPtr->shiftX) + square(tempPtr->shiftY));
				largestFrequency = tempPtr->frequency;
				shiftX = tempPtr->shiftX;
				shiftY = tempPtr->shiftY;
			}

			tempPtr = tempPtr->nextPtr;
		}

		*shiftCo = shiftX * pow(2.0, PYRAMIDLEVEL);
		*(shiftCo + 1) = shiftY * pow(2.0, PYRAMIDLEVEL);
	}
}



/****************************************************************************************************************************
 * Main Function
 ****************************************************************************************************************************/
int main(void){

	//Read images
	IplImage * refFrame = cvLoadImage("images/test/P1020295.JPG");
	IplImage * curFrame = cvLoadImage("images/test/P1020296.JPG");

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
	//double distanceThreshold = computeDistanceThreshold(headMotionVectorPtr);
	//double weightedDistanceThreshold = weightedMeanDistanceThreshold(headMotionVectorPtr, &headDistanceFrequencyPtr, &tailDistanceFrequencyPtr);
	//printf("Mean Threshold: %f\n", distanceThreshold);
	//printf("Weighted Mean Threshold: %f\n", weightedDistanceThreshold);

	int shiftArray[2] = {0};

	getImageShift(headMotionVectorPtr, &headDistanceFrequencyPtr, &tailDistanceFrequencyPtr, shiftArray);

	printf("X: %d\nY: %d\n", shiftArray[0], shiftArray[1]);

	//segmentImage(&segmentedImage, headMotionVectorPtr, distanceThreshold);
	//createNewFrame(&newFrame, refFrame, curFrame, headMotionVectorPtr, distanceThreshold);

	//cvSaveImage("images/test/new.jpg", newFrame);

	//cvNamedWindow("Segmented", 1); 
	//cvShowImage("Segmented", segmentedImage);
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