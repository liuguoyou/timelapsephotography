#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>

#define BLOCKSIZE 8
#define SEARCHRANGE 16
#define BLOCKNUM 32400
#define MAXDISTANCETHRESHOLD 12.0000

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
  return ((unsigned char*)(image->imageData + image->widthStep*y))[x*image->nChannels];
}


//sets pixel at image position (x,y)
void setPixel(IplImage* image, int x, int y, unsigned char value){
  ((unsigned char*)(image->imageData + image->widthStep*y))[x*image->nChannels]=value;
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

	for(imageIndexX = 0; imageIndexX < imageWidth - BLOCKSIZE; imageIndexX += BLOCKSIZE)
		for(imageIndexY = 0; imageIndexY < imageHeight - BLOCKSIZE; imageIndexY += BLOCKSIZE){

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
	return sqrt(double(shiftedX * shiftedX + shiftedY * shiftedY));
}

void fillBlock(IplImage ** image, int beginPositionX, int beginPositionY, int colorValue){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE; i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE; j++)
			setPixel(*image, i, j, colorValue);
}


void segmentImage(IplImage ** newImage, BlockMotionVectorPtr motionVectorQueueNode){

	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			if(shiftedDistance(motionVectorQueueNode->shiftX, motionVectorQueueNode->shiftY) >= MAXDISTANCETHRESHOLD){
				fillBlock(newImage, motionVectorQueueNode->x, motionVectorQueueNode->y, 255);
			} else{
				fillBlock(newImage, motionVectorQueueNode->x, motionVectorQueueNode->y, 0);
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}
	}
}


int main(void){

	//Read images
	IplImage * refFrame = cvLoadImage("images/test/P1030701.jpg");
	IplImage * curFrame = cvLoadImage("images/test/P1030702.jpg");

	BlockMotionVectorPtr headMotionVectorPtr = NULL;
	BlockMotionVectorPtr tailMotionVectorPtr = NULL;
	
	/*
	cvNamedWindow("test", 1); 
	cvShowImage("test", refFrame);
	cvWaitKey(0);
	*/
	//Covert to grayscale images
	
	IplImage * refGrayPlane = NULL, * curGrayPlane = NULL, *segmentedImage = NULL;

	allocateOnDemand(&refGrayPlane, cvGetSize(refFrame), IPL_DEPTH_8U, 1);
	cvCvtColor(refFrame, refGrayPlane, CV_BGR2GRAY);

	allocateOnDemand(&curGrayPlane, cvGetSize(curFrame), IPL_DEPTH_8U, 1);
	cvCvtColor(curFrame, curGrayPlane, CV_BGR2GRAY);

	allocateOnDemand(&segmentedImage, cvGetSize(curFrame), IPL_DEPTH_8U, 1);
	//cvCvtColor(curFrame, curGrayPlane, CV_BGR2GRAY);


	getBlockShift(refGrayPlane, curGrayPlane, &headMotionVectorPtr, &tailMotionVectorPtr);

	segmentImage(&segmentedImage, headMotionVectorPtr);

	cvNamedWindow("Segmented", 1); 
	cvShowImage("Segmented", segmentedImage);
	cvWaitKey(0);

	//Release memories
	cvReleaseImage(&refFrame);
	cvReleaseImage(&curFrame);
	cvReleaseImage(&refGrayPlane);
	cvReleaseImage(&curGrayPlane);
	cvReleaseImage(&segmentedImage);

	return 0;
}