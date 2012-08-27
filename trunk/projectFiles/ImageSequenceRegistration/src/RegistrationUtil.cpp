#include "Utils/RegistrationUtil.h"
#include "Utils/Utils.h"

/** 
 * Implementation of Function isMotionVectorQueueEmpty(BlockMotionVectorPtr headPtr)
 */
int isMotionVectorQueueEmpty(BlockMotionVectorPtr headPtr){
	return headPtr == NULL;
}

/** 
 * Implementation of Function enqueueMotionVectors(BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr, int x, int y, int shiftX, int shiftY)
 */
void enqueueMotionVectors(BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr, int x, int y, int shiftX, int shiftY){

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

/** 
 * Implementation of Function isDistanceFrequencyQueueEmpty(DistanceFrequencyNodePtr headPtr)		
 */
int isDistanceFrequencyQueueEmpty(DistanceFrequencyNodePtr headPtr){
	return headPtr == NULL;
}

/** 
 * Implementation of Function enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int sX, int sY)
 */
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


/** 
 * Implementation of Function isQueueEmpty(BlockMotionVectorPtr headPtr)
 */
int isImageInfoQueueEmpty(ImageInfoNodePtr headPtr){
	return headPtr == NULL;
}

void enqueueImageInfo(ImageInfoNodePtr *headPtr, ImageInfoNodePtr * tailPtr, long imageIndex, long matchedImageNumbers, int shiftX, int shiftY){
	ImageInfoNodePtr newInfoPtr;
	newInfoPtr = (ImageInfoNodePtr)malloc(sizeof(ImageInfoNode));

	if(newInfoPtr != NULL){
		newInfoPtr->imageIndex = imageIndex;
		newInfoPtr->matchedImageNumbers = matchedImageNumbers;
		newInfoPtr->shiftX = shiftX;
		newInfoPtr->shiftY = shiftY;
		newInfoPtr->nextPtr = NULL;

		if(isImageInfoQueueEmpty(*headPtr)){
			*headPtr = newInfoPtr;
			newInfoPtr->previousPtr = NULL;
		} else {
			(*tailPtr)->nextPtr = newInfoPtr;
			newInfoPtr->previousPtr = *tailPtr;
		}

		*tailPtr = newInfoPtr;
	} else{
		fprintf(stderr, "Error: New Image Info vector is not inserted, Out of memory?\n");
		exit(-1);
	}
}

/**
 * Implementation of Function createGrayPlane(const IplImage *srcImage, IplImage* &grayPlane)
 */
void createGrayPlane(const IplImage *srcImage, IplImage ** grayPlane){
	//allocateOnDemand(grayPlane, cvGetSize(srcImage), srcImage->depth, 1);
	cvCvtColor(srcImage, grayPlane, CV_BGR2GRAY);
}

/**
 * Implementation of Function differImages(const IplImage *srcGrayImage1, const IplImage *srcGrayImage2)
 */
int isImagesMatch(const IplImage *srcGrayImage1, const IplImage *srcGrayImage2){
	long matchedPixelNumber = 0;

	IplImage *result = NULL;
	allocateOnDemand(&result, cvGetSize(srcGrayImage1), srcGrayImage1->depth, 1);

	cvXor(srcGrayImage1, srcGrayImage2, result, 0);

	for(int i = 0; i < result->width; i++)
		for(int j = 0; j < result->height; j++)
			printf("%d\n", getPixel(result, i, j));


	cvNamedWindow( "XORresult", 1 ); 
	cvShowImage("XORresult", result);		
	cvWaitKey(0);

	return matchedPixelNumber / (srcGrayImage1->width * srcGrayImage1->height) >= MATCH_THRESHOLD;
}

/**
 * Implementation of Function void shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel)
 */
void shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel){
	if(downwardLevel == 0) {
		*dstImage = cvCloneImage(srcImage);
	} else{

		IplImage * tempImage = NULL;

		allocateOnDemand(&tempImage, cvSize(srcImage->width / 2, srcImage->height / 2), srcImage->depth, srcImage->nChannels);

		cvPyrDown(srcImage, tempImage, CV_GAUSSIAN_5x5);

		downwardLevel--;
		shrinkImage(tempImage, dstImage, downwardLevel);
	}
}

/**
 * Implementation of Function shiftImage(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result)
 */
void shiftImage(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result){
	int i, j;

	if(xOffset == 0 && yOffset == 0){
		result = cvCloneImage(srcImage);
		return;
	}

	if(xOffset >= 0 && yOffset >= 0){
		for(i = 0; i < srcImage->height - yOffset; i++)
			for(j = 0; j < srcImage->width - xOffset; j++)
				cvSet2D(result, i + yOffset, j + xOffset, cvGet2D(srcImage, i, j));
	}

	if(xOffset >= 0 && yOffset < 0){
		for(i = abs(yOffset); i < srcImage->height; i++)
			for(j = 0; j < srcImage->width - xOffset; j++)
				cvSet2D(result, i - abs(yOffset), j + xOffset, cvGet2D(srcImage, i, j));		
	}

	if(xOffset < 0 && yOffset >= 0){
		for(i = 0; i < srcImage->height - yOffset; i++)
			for(j = abs(xOffset); j < srcImage->width; j++)
				cvSet2D(result, i + yOffset, j - abs(xOffset), cvGet2D(srcImage, i, j));			
	}

	if(xOffset < 0 && yOffset < 0){
		for(i = abs(yOffset); i < srcImage->height; i++)
			for(j = abs(xOffset); j < srcImage->width; j++)
				cvSet2D(result, i - abs(yOffset), j - abs(xOffset), cvGet2D(srcImage, i, j));
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

	double squareSumMatrix[SEARCH_RANGE * SEARCH_RANGE] = {10000.0000};
	int matrixIndex = 0;


	//calculation loop, adjust here for pixelwise or blockwise
	for(imageIndexX = 0; imageIndexX < imageWidth - BLOCK_SIZE; imageIndexX+=BLOCK_SIZE)
		for(imageIndexY = 0; imageIndexY < imageHeight - BLOCK_SIZE; imageIndexY+=BLOCK_SIZE){

			//smallestSquareSum = 10000.0000;
			smallestShifts[0] = 0;
			smallestShifts[1] = 0;

			int isFirstSumInRange = 1;

			for(rangeShiftX = -SEARCH_RANGE; rangeShiftX < SEARCH_RANGE; rangeShiftX++)
				for(rangeShiftY = -SEARCH_RANGE; rangeShiftY < SEARCH_RANGE; rangeShiftY++){

					
					int isThePositionOutOfRange = 0;
					double sum = 0;
					
					//double temp;
					//int isOutOfRange;

					for(blockIndexX = imageIndexX; blockIndexX < imageIndexX + BLOCK_SIZE; blockIndexX++)
						for(blockIndexY = imageIndexY; blockIndexY < imageIndexY + BLOCK_SIZE; blockIndexY++){
							
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
 * Implementation of Function getImageShift(BlockMotionVectorPtr motionVectorQueueNode, DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int shiftCo[])
 */
void getImageShift(BlockMotionVectorPtr motionVectorQueueNode, DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int shiftCo[]){
	
	double sum = 0;

	double distanceThreshold = computeDistanceThreshold(motionVectorQueueNode);

	printf("%f\n", distanceThreshold);

	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){

			if(sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY)) < distanceThreshold){
				enqueueDistanceFrequency(headPtr, tailPtr, motionVectorQueueNode->shiftX, motionVectorQueueNode->shiftY);
			
			}
					
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}

		computeBackgroundMotion(*headPtr, shiftCo);

		//free(tempPtr);
		

		*shiftCo = (*shiftCo) * pow(2.0, PYRAMID_LEVEL);
		*(shiftCo + 1) = *(shiftCo + 1) * pow(2.0, PYRAMID_LEVEL);
	}
}

/**
 * Implementation of Function findReferencePoint(ImageInfoNodePtr *headImageInfoNodePtr, ImageInfoNodePtr *tailImageInfoNodePtr, vector<string>& srcFileNames)
 */
int findReferencePoint(ImageInfoNodePtr *headImageInfoNodePtr, ImageInfoNodePtr *tailImageInfoNodePtr, vector<string>& srcFileNames){
	long referencePoint = 0, index = 1, refIndex = 0, matchNumbers = 0;
	int groupChanged = 0;
	
	vector<string>::iterator iter;

	while(index < (long) srcFileNames.size()){

		IplImage *src1 = NULL, *src2 = NULL, 
			 *grayPlane1 = NULL, *grayPlane2 = NULL, 
			 *shunkedGrayPlane1 = NULL, *shunkedGrayPlane2 = NULL;

		BlockMotionVectorPtr headMotionVectorPtr = NULL;
		BlockMotionVectorPtr tailMotionVectorPtr = NULL;

		DistanceFrequencyNodePtr headDistanceFrequencyPtr = NULL;
		DistanceFrequencyNodePtr tailDistanceFrequencyPtr = NULL;

		if(groupChanged)
			matchNumbers = groupChanged = 0;
		// Read image ( same size, same type )
		src1 = cvLoadImage(srcFileNames[refIndex].c_str(), 1);
		src2 = cvLoadImage(srcFileNames[index].c_str(), 1);

		// File check
		if(src1 ==  NULL) { 
			fprintf(stderr, "Error: Error loading src1!\n");
			exit(-1);
		}
		if(src2 ==  NULL) { 
			fprintf(stderr, "Error: Error loading src2!\n");
			exit(-1);
		}

		//create gray planes
		allocateOnDemand(&grayPlane1, cvGetSize(src1), src1->depth, 1);
		allocateOnDemand(&grayPlane2, cvGetSize(src2), src2->depth, 1);
		cvCvtColor(src1, grayPlane1, CV_BGR2GRAY);
		cvCvtColor(src2, grayPlane2, CV_BGR2GRAY);

		//shrink images according to pyramid level
		shrinkImage(grayPlane1, &shunkedGrayPlane1, PYRAMID_LEVEL);
		shrinkImage(grayPlane2, &shunkedGrayPlane2, PYRAMID_LEVEL);

		getBlockShift(shunkedGrayPlane1, shunkedGrayPlane2, &headMotionVectorPtr, &tailMotionVectorPtr);

		int shiftArray[2] = {0};

		getImageShift(headMotionVectorPtr, &headDistanceFrequencyPtr, &tailDistanceFrequencyPtr, shiftArray);

		if(shiftArray[0] == 0 && shiftArray[1] == 0){
			//Two images match, move to next image, continue comparison
			matchNumbers++;
			index++;
		} else{
			//COMPUTE MOTION VECTOR, ENQUEUE
			

			enqueueImageInfo(headImageInfoNodePtr, tailImageInfoNodePtr, refIndex, matchNumbers, shiftArray[0], shiftArray[1]);

			//if(index == (long) srcFileNames.size())

			refIndex = index;
			index++;
			groupChanged = 1;
		}


		free(headMotionVectorPtr);
		free(tailMotionVectorPtr);
		free(headDistanceFrequencyPtr);
		free(tailDistanceFrequencyPtr);

		cvReleaseImage(&src1);
		cvReleaseImage(&src2);
		cvReleaseImage(&grayPlane1);
		cvReleaseImage(&grayPlane2);
		cvReleaseImage(&shunkedGrayPlane1);
		cvReleaseImage(&shunkedGrayPlane2);
	}

	enqueueImageInfo(headImageInfoNodePtr, tailImageInfoNodePtr, refIndex, matchNumbers, 0, 0);

	ImageInfoNodePtr tempPtr = NULL;
	tempPtr = (ImageInfoNodePtr)malloc(sizeof(ImageInfoNode));

	if(tempPtr != NULL){
		tempPtr = *headImageInfoNodePtr;
		int largestMatchNumbers = tempPtr->matchedImageNumbers;
		tempPtr = tempPtr->nextPtr;

		while(tempPtr != NULL){
			if(tempPtr->matchedImageNumbers > largestMatchNumbers){
				largestMatchNumbers = tempPtr->matchedImageNumbers;
				referencePoint = tempPtr->imageIndex;
			}


			tempPtr = tempPtr->nextPtr;
		}
	} else {
		fprintf(stderr, "Error: New Image Info vector is not created, Out of memory?\n");
		exit(-1);
	}

	free(tempPtr);

	return referencePoint;
}

/**
 * Implementation of Function adjustShiftsToReferenceImage(ImageInfoNodePtr * imageInfoNodePtr, ImageInfoNodePtr * tailImageInfoNode, ImageInfoNodePtr * newHead, ImageInfoNodePtr * newTail, long referencePoint)
 */
void adjustShiftsToReferenceImage(ImageInfoNodePtr * imageInfoNodePtr, ImageInfoNodePtr * tailImageInfoNode, long referencePoint){
	ImageInfoNodePtr referenceNode = NULL;
	referenceNode = (ImageInfoNodePtr)malloc(sizeof(ImageInfoNode));

	ImageInfoNodePtr imageInfoNode = NULL;
	imageInfoNode = (ImageInfoNodePtr)malloc(sizeof(ImageInfoNode));

	if(referenceNode == NULL || imageInfoNode == NULL){
		fprintf(stderr, "Error: New Image Info vector is not created, Out of memory?\n");
		exit(-1);
	}

	imageInfoNode = *imageInfoNodePtr;

	int nextShiftX = 0;
	int nextShiftY = 0;

	//find reference node
	while(imageInfoNode !=NULL){
		if(imageInfoNode->imageIndex == referencePoint){
			referenceNode = imageInfoNode;
			nextShiftX = imageInfoNode->shiftX;
			nextShiftY = imageInfoNode->shiftY;	
			break;
		}
		imageInfoNode = imageInfoNode->nextPtr;
	}

	imageInfoNode = *tailImageInfoNode;

	while(imageInfoNode->imageIndex > referencePoint){

		imageInfoNode->shiftX = imageInfoNode->previousPtr->shiftX;
		imageInfoNode->shiftY = imageInfoNode->previousPtr->shiftY;

		imageInfoNode = imageInfoNode->previousPtr;
	}
	
	referenceNode->shiftX = referenceNode->shiftY = 0;

	imageInfoNode = referenceNode->nextPtr;

	//Adjust nodes after reference node
	while(imageInfoNode != NULL){

		imageInfoNode->shiftX += imageInfoNode->previousPtr->shiftX;
		imageInfoNode->shiftY += imageInfoNode->previousPtr->shiftY;

		imageInfoNode = imageInfoNode->nextPtr;		
	}

	imageInfoNode = referenceNode->previousPtr;

	//Adjust nodes before reference node
	while(imageInfoNode != NULL){
		imageInfoNode->shiftX = imageInfoNode->nextPtr->shiftX - imageInfoNode->shiftX;
		imageInfoNode->shiftY = imageInfoNode->nextPtr->shiftY - imageInfoNode->shiftY;

		imageInfoNode = imageInfoNode->previousPtr;
	}

}

/**
 * Implementation of Function shiftImageSequence(ImageInfoNodePtr imageInfoNode, vector<string>& srcFileNames, vector<string>& dstFileNames)
 */
void shiftImageSequence(ImageInfoNodePtr imageInfoNode, vector<string>& srcFileNames, vector<string>& dstFileNames){
	long matchedNumber = 0;
	
	long index = 0;

	//for (vector<string>::const_iterator iter = srcFileNames.begin( ) + 1; iter != srcFileNames.end( ); iter++){

	while(imageInfoNode != NULL){
		
		IplImage  * srcImage = cvLoadImage(srcFileNames[index].c_str(),1);

		IplImage  * shiftedImage = cvCreateImage(cvGetSize(srcImage), srcImage->depth, srcImage->nChannels);
		cvSet(shiftedImage, CV_RGB(255, 255, 255), NULL);
	
		shiftImage(srcImage, imageInfoNode->shiftX, imageInfoNode->shiftY, shiftedImage);
		printf("Image: %d, aligned at (%d, %d)!\n", index,  imageInfoNode->shiftX, imageInfoNode->shiftY);

		cvSaveImage(dstFileNames[index].c_str(), shiftedImage, 0);
		index++;
		
		if(imageInfoNode->matchedImageNumbers == 0){
			imageInfoNode = imageInfoNode->nextPtr;
		} else{
			matchedNumber++;

			if(matchedNumber > imageInfoNode->matchedImageNumbers){
				matchedNumber = 0;
				imageInfoNode = imageInfoNode->nextPtr;
			}
		
		}

		/*
		cvNamedWindow( "Shifted", 1 ); 
		cvShowImage("Shifted", shiftedImage);		
		cvWaitKey(0);
		*/
		
	}
}


/** 
 * Implementation of Function computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode)
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

/** 
 * Implementation of Function computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode)
 */
void computeBackgroundMotion(DistanceFrequencyNodePtr distanceFrequencyQueueNode, int shifts[]){
	long maxFrequency = distanceFrequencyQueueNode->frequency;
	int shiftX = distanceFrequencyQueueNode->shiftX;
	int shiftY = distanceFrequencyQueueNode->shiftY;

	if(distanceFrequencyQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(distanceFrequencyQueueNode != NULL){

			if(distanceFrequencyQueueNode->frequency > maxFrequency){
				shiftX = distanceFrequencyQueueNode->shiftX;
	 			shiftY = distanceFrequencyQueueNode->shiftY;
			} else if(distanceFrequencyQueueNode->frequency == maxFrequency){
				shiftX = (shiftX + distanceFrequencyQueueNode->shiftX) / 2;
				shiftY = (shiftY + distanceFrequencyQueueNode->shiftY) / 2;
			}
			
			distanceFrequencyQueueNode = distanceFrequencyQueueNode->nextPtr;
		}

		*(shifts) = (int)shiftX;
		*(shifts + 1) = (int)shiftY;

	}
}