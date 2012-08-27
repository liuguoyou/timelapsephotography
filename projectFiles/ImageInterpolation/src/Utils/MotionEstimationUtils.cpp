#include "MotionEstimationUtils.h"
/** 
 * Implementation of Function shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel)
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

/** 
 * Implementation of Function getBlockShift(IplImage * referenceFrame, IplImage * currentFrame, BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr)
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

/** 
 * Implementation of Function fillBlock(IplImage ** image, int beginPositionX, int beginPositionY, int colorValue)
 */
void fillBlock(IplImage ** image, int beginPositionX, int beginPositionY, int colorValue){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); j++)
			setPixel(*image, i, j, colorValue);
}

/** 
 * Implementation of Function segmentImage(IplImage ** newImage, BlockMotionVectorPtr motionVectorQueueNode, double threshold)
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

/** 
 * Implementation of Function computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode)
 */
double computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode){
	long count = 0;
	double sum = 0;
	//double smallest = sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY));
	//double largest = sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY));

	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			
			if(motionVectorQueueNode->shiftX != 0 || motionVectorQueueNode->shiftY !=0){
				/*
				if(sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY)) < smallest){
					smallest = sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY));
				} 

				if(sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY)) >= largest){
					largest = sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY));
				}
				*/
				count++;
				sum += sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY));
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}

		return sum / count;
	}
}

/** 
 * Implementation of Function computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode)
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



/**
 * Implementation of Function fillBackGroundBlock(IplImage ** newFrame, IplImage * refFrame, IplImage * curFrame, int beginPositionX, int beginPositionY)
 */
void fillBackGroundBlock(IplImage ** newFrame, IplImage * refFrame, IplImage * curFrame, int beginPositionX, int beginPositionY){
	for(int i = beginPositionX; i < beginPositionX + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); i++)
		for(int j = beginPositionY; j < beginPositionY + BLOCKSIZE * (pow(2.0, PYRAMIDLEVEL)); j++){
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 0) + getColorPixel(refFrame,i, j, 0)) / 2, 0);
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 1) + getColorPixel(refFrame,i, j, 1)) / 2, 1);
			setColorPixel(*newFrame, i, j, (getColorPixel(curFrame,i, j, 2) + getColorPixel(refFrame,i, j, 2)) / 2, 2);
		}		
}

/** 
 * Implementation of Function fillMotionBlock(IplImage ** newFrame, IplImage * curFrame, int beginPositionX, int beginPositionY, int shiftX, int shiftY)
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
 
/** 
 * Implementation of Function createNewFrame(IplImage ** newFrame, IplImage *refFrame, IplImage *curFrame, BlockMotionVectorPtr motionVectorQueueNode, double threshold)
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

/** 
 * Implementation of Function void fillBlankPixels(IplImage ** sourceImage)
 */
void fillBlankPixels(IplImage * sourceImage, IplImage * interFrame, IplImage ** dstImage){
	for(int i = 0; i < sourceImage->width; i++)
		for(int j = 0; j < sourceImage->height; j++)
			if(getPixel(interFrame, i, j) == 205){
				setColorPixel(*dstImage, i, j, getColorPixel(sourceImage, i, j, 0), 0);
				setColorPixel(*dstImage, i, j, getColorPixel(sourceImage, i, j, 1), 1);
				setColorPixel(*dstImage, i, j, getColorPixel(sourceImage, i, j, 2), 2);
			} else{
				setColorPixel(*dstImage, i, j, getColorPixel(interFrame, i, j, 0), 0);
				setColorPixel(*dstImage, i, j, getColorPixel(interFrame, i, j, 1), 1);
				setColorPixel(*dstImage, i, j, getColorPixel(interFrame, i, j, 2), 2);
			}
}

/** 
 * Implementation of Function isInterpolationNeeded(IplImage * referenceFrame, IplImage * currentFrame, BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr, vector<string>& srcFileNames)
 */
int isInterpolationNeeded(BlockMotionVectorPtr motionVectorQueueNode, int count){
	double distanceThreshold = computeDistanceThreshold(motionVectorQueueNode);
	return 0;
}

/** 
 * Implementation of Function interpolateInSequence(BlockMotionVectorPtr motionVectorQueueNode, double interpolateThreshold)
 */
void interpolateInSequence(char * directory, vector<string>& fileNames, vector<string>& srcFileNames, double *thresholds, int quantity){

	//Find maximum threshold
	double maxThreshold = *(thresholds);

	for(int i = 1; i < quantity; i++){
		if(*(thresholds + i) > maxThreshold){
			maxThreshold = *(thresholds + i);
		}
	}
	
	
	for(long i = 0; i < quantity; i++){
		if(*(thresholds + i) >= maxThreshold * INTERPOLATION_RANGE){
			long index = i;
			//Do interpolation for those images	

			//Read images
			IplImage * refFrame = cvLoadImage(srcFileNames[index].c_str());
			IplImage * curFrame = cvLoadImage(srcFileNames[index + 1].c_str());

			BlockMotionVectorPtr headMotionVectorPtr = NULL;
			BlockMotionVectorPtr tailMotionVectorPtr = NULL;

			//Covert to grayscale images
	
			IplImage * refGrayPlane = NULL, * curGrayPlane = NULL, 
						* shrunkRefPlane = NULL, * shrunkCurPlane = NULL, * interFrame = NULL, * newFrame = NULL;

			allocateOnDemand(&interFrame, cvGetSize(curFrame), curFrame->depth, curFrame->nChannels);
			allocateOnDemand(&newFrame, cvGetSize(curFrame), curFrame->depth, curFrame->nChannels);

			allocateOnDemand(&refGrayPlane, cvGetSize(refFrame), IPL_DEPTH_8U, 1);
			cvCvtColor(refFrame, refGrayPlane, CV_BGR2GRAY);

			allocateOnDemand(&curGrayPlane, cvGetSize(curFrame), IPL_DEPTH_8U, 1);
			cvCvtColor(curFrame, curGrayPlane, CV_BGR2GRAY);

			shrinkImage(refGrayPlane, &shrunkRefPlane, PYRAMIDLEVEL);
			shrinkImage(curGrayPlane, &shrunkCurPlane, PYRAMIDLEVEL);

			//Without pyramid
			//getBlockShift(refGrayPlane, curGrayPlane, &headMotionVectorPtr, &tailMotionVectorPtr);

			//Do with pyramid to speed up
			getBlockShift(shrunkRefPlane, shrunkCurPlane, &headMotionVectorPtr, &tailMotionVectorPtr);

			double distanceThreshold = computeDistanceThreshold(headMotionVectorPtr);
				
			//Interpolation
			createNewFrame(&interFrame, refFrame, curFrame, headMotionVectorPtr, distanceThreshold);
			fillBlankPixels(curFrame, interFrame, &newFrame);

			char newName[500];
			strcpy(newName, directory);
			strcat(newName, "/inter_");
			strcat(newName, fileNames[index].c_str());

			cvSaveImage(newName, newFrame);

			//cvNamedWindow("NewFrame", 1); 
			//cvShowImage("NewFrame", newFrame);
			//cvWaitKey(0);

			//Release memories
			cvReleaseImage(&refFrame);
			cvReleaseImage(&curFrame);
			cvReleaseImage(&refGrayPlane);
			cvReleaseImage(&curGrayPlane);
			cvReleaseImage(&shrunkRefPlane);
			cvReleaseImage(&shrunkCurPlane);
			cvReleaseImage(&interFrame);
			cvReleaseImage(&newFrame);
			
		}
	}
}