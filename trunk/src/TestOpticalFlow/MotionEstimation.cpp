#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>

#define BLOCKSIZE 8
#define SEARCHRANGE 16
//#define MAXDISTANCETHRESHOLD 3.75
#define PYRAMIDLEVEL 0

struct blockMotionVector{
	int x;
	int y;
	int shiftX;
	int shiftY;
	struct blockMotionVector *nextPtr;
};

typedef struct blockMotionVector BlockMotionVector;
typedef BlockMotionVector *BlockMotionVectorPtr;



int isQueueEmpty(BlockMotionVectorPtr headPtr){
	return headPtr == NULL;
}

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

		if(isQueueEmpty(*headPtr)){
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


//static const double pi = 3.14159265358979323846;

inline static double square(int a)
{
	return a * a;
}

//returns value between 0 and 255 of pixel at image position (x,y)
unsigned char getPixel(const IplImage* image, int x, int y){
  return ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels];
}

//returns color value between 0 and 255 of pixel at image position (x,y), channel: 0 - blue, 1 - green, 2 - red
unsigned char getColorPixel(const IplImage* image, int x, int y, int colorChannel){
  return ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels + colorChannel];
}


//sets pixel at image position (x,y)
void setPixel(IplImage* image, int x, int y, unsigned char value){
  ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels] = value;
}

void setColorPixel(IplImage* image, int x, int y, unsigned char value, int colorChannel){
  ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels + colorChannel] = value;
}

/* This is just an inline that allocates images.  I did this to reduce clutter in the
 * actual computer vision algorithmic code.  Basically it allocates the requested image
 * unless that image is already non-NULL.  It always leaves a non-NULL image as-is even
 * if that image's size, depth, and/or channels are different than the request.
 */
inline static void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
{
	if ( *img != NULL )	return;

	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		fprintf(stderr, "Error: Couldn't allocate image.  Out of memory?\n");
		exit(-1);
	}
}


/**
 * Implementation of Function void shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel)
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


double shiftedDistance(int shiftedX, int shiftedY){
	return sqrt(double(square(shiftedX) + square(shiftedY)));
}

void fillBlock(IplImage ** image, int beginPositionX, int beginPositionY, int colorValue){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); j++)
			setPixel(*image, i, j, colorValue);
}


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


void fillBackGroundBlock(IplImage ** newFrame, IplImage * refFrame, IplImage * curFrame, int beginPositionX, int beginPositionY){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); j++){
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 0) + getColorPixel(refFrame,i, j, 0)) / 2, 0);
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 1) + getColorPixel(refFrame,i, j, 1)) / 2, 1);
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 2) + getColorPixel(refFrame,i, j, 2)) / 2, 2);
		}		
}


void fillMotionBlock(IplImage ** newFrame, IplImage * curFrame, int beginPositionX, int beginPositionY, int shiftX, int shiftY){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); j++){

			if(i - shiftX >= 0 && j - shiftY >= 0 && i + shiftX < curFrame->width && j + shiftY < curFrame->height){		
				setColorPixel(*newFrame, i, j, getColorPixel(curFrame, i + shiftX / 2, j + shiftY / 2, 0), 0);
				setColorPixel(*newFrame, i, j, getColorPixel(curFrame, i + shiftX / 2, j + shiftY / 2, 1), 1);
				setColorPixel(*newFrame, i, j, getColorPixel(curFrame, i + shiftX / 2, j + shiftY / 2, 2), 2);
			}
		}		
}

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


int main(void){

	//Read images
	IplImage * refFrame = cvLoadImage("images/test/P1030710.jpg");
	IplImage * curFrame = cvLoadImage("images/test/P1030711.jpg");

	BlockMotionVectorPtr headMotionVectorPtr = NULL;
	BlockMotionVectorPtr tailMotionVectorPtr = NULL;
	
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
	//cvCvtColor(curFrame, curGrayPlane, CV_BGR2GRAY);

	shrinkImage(refGrayPlane, &shrunkrefPlane, PYRAMIDLEVEL);
	shrinkImage(curGrayPlane, &shrunkcurPlane, PYRAMIDLEVEL);
	/*
	shrinkImage(refGrayPlane, &shrunkImage, 2);
	cvNamedWindow("ShunkImage", 1); 
	cvShowImage("ShunkImage", shrunkImage);
	cvWaitKey(0);
	*/

	//Without pyramid
	getBlockShift(refGrayPlane, curGrayPlane, &headMotionVectorPtr, &tailMotionVectorPtr);

	//Do with pyramid to speed up
	//getBlockShift(shrunkrefPlane, shrunkcurPlane, &headMotionVectorPtr, &tailMotionVectorPtr);

	
	//printMotionVectors(headMotionVectorPtr);

	//Print out threshold
	double distanceThreshold = computeDistanceThreshold(headMotionVectorPtr);
	printf("Threshold: %f\n", distanceThreshold);

	segmentImage(&segmentedImage, headMotionVectorPtr, distanceThreshold);
	createNewFrame(&newFrame, refFrame, curFrame, headMotionVectorPtr, distanceThreshold);

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