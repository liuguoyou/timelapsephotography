/**************************************************************************
  Essential codes in the thresholding - XOR algorithm
 **************************************************************************/
/**
 * Function getExpShift(const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[])
 * 		Find the offsets in x and y direction for the image comparing to the reference 
 * Parameters:
 * 		srcImage1: the reference frame
 * 		srcImage2: the current frame
 *		shiftBits: specified imagepyramid levels
 *		shiftRet[]: array for storing x and y shifts
 */
void getExpShift(const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[]){
	int minErr;
	int curShift[2];

	IplImage *tb1 = cvCreateImage(cvGetSize(srcImage1), 8, 1);
	IplImage *tb2 = cvCreateImage(cvGetSize(srcImage2), 8, 1);
	IplImage *eb1 = cvCreateImage(cvGetSize(srcImage1), 8, 1);
	IplImage *eb2 = cvCreateImage(cvGetSize(srcImage2), 8, 1);

	int i, j;

	if(shiftBits > 0){
		IplImage *sm1Img1 = cvCreateImage(cvSize(srcImage1->width/2, srcImage1->height/2), srcImage1->depth, srcImage1->nChannels);
		IplImage *sm1Img2 = cvCreateImage(cvSize(srcImage2->width/2, srcImage2->height/2), srcImage2->depth, srcImage2->nChannels);
		//image pyramid
		shrinkImage(srcImage1, &sm1Img1);
		shrinkImage(srcImage2, &sm1Img2);
		//recursive call
		getExpShift(sm1Img1, sm1Img2, shiftBits - 1, curShift);

		cvReleaseImage(&sm1Img1);
		cvReleaseImage(&sm1Img2);
		//re - calculate positions
		shiftRet[0] = curShift[0] * 2;
		shiftRet[1] = curShift[1] * 2;
	} else {
		curShift[0] = curShift[1] = 0;
		//Segmentation, tb denotes for segmented images with a threshold, and eb stands for the exclusion image
		createBitmaps(srcImage1, tb1, eb1);
		createBitmaps(srcImage2, tb2, eb2);
		
		minErr = srcImage1->width * srcImage1->height;

		for(i = -1; i < 1; i++)
			for(j = -1; j < 1; j++) {
				int xOffset = curShift[0] + i;
				int yOffset = curShift[1] + j;

				IplImage * shiftedTB2 = cvCreateImage(cvGetSize(srcImage2), 8, 1);
				IplImage * shiftedEB2 = cvCreateImage(cvGetSize(srcImage2), 8, 1);
				IplImage * diffBMP = cvCreateImage(cvGetSize(srcImage2), 8, 1);

				int err;

				shiftBitMap(tb2, xOffset, yOffset, shiftedTB2);
				shiftBitMap(eb2, xOffset, yOffset, shiftedEB2);
				
				//Compare for the difference for the two images using an XPR operator
				xorBitMap(tb1, shiftedTB2, diffBMP);
				
				//ANDing the result with the reference for noise exclusion, using for test, results shows no differences
				/*
				andBitMap(diffBMP, eb1, diffBMP);
				andBitMap(diffBMP, shiftedEB2, diffBMP);
				*/
				err = totalOneInBitMap(diffBMP);

				if(err < minErr) {
					shiftRet[0] = xOffset;
					shiftRet[1] = yOffset;
					minErr = err;
				}
				cvReleaseImage(&shiftedTB2);
				cvReleaseImage(&shiftedEB2);
			}
			cvReleaseImage(&tb1);
			cvReleaseImage(&tb2);
			cvReleaseImage(&eb1);
			cvReleaseImage(&eb2);
	}
}

/**************************************************************************
  Essential codes in the Motion estimation methods
 **************************************************************************/


/**************************************************************************
  Essential codes in the Motion estimation methods
	Some significant public functions
 **************************************************************************/
/** Function getFileNames(string path, vector<string>& files)
 *		To search in a directory, including all files, typically image files in this case, and store their names in a vector
 *	Parameters:
 *		path: the directory path to be searched
 *		files: the vector that used to store file names with directory path
 */
void getFileNames(string path, vector<string>& entireNames, vector<string>& fileNames){
	//File handle
    long   hFile   =   0;

    //File information
    struct _finddata_t fileinfo;
    string p;

    if((hFile = _findfirst(p.assign(path).append("/*").c_str(),&fileinfo)) != -1){
        do{
            //If it is a directory, go in iteration, otherwise, add to list
            if((fileinfo.attrib & _A_SUBDIR)){
                if(strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name,"..") != 0)
                    getFileNames( p.assign(path).append("/").append(fileinfo.name), entireNames, fileNames);
            }
            else{
                entireNames.push_back(p.assign(path).append("/").append(fileinfo.name));
				fileNames.push_back(fileinfo.name);
            }
        }   while(_findnext(hFile, &fileinfo) == 0);

        _findclose(hFile);
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

	double squareSumMatrix[SEARCH_RANGE * SEARCH_RANGE] = {10000.0000};
	int matrixIndex = 0;

	//calculation loop, adjust here for pixelwise or blockwise
	for(imageIndexX = 0; imageIndexX < imageWidth - BLOCK_SIZE; imageIndexX+=BLOCK_SIZE)
		for(imageIndexY = 0; imageIndexY < imageHeight - BLOCK_SIZE; imageIndexY+=BLOCK_SIZE){

			//smallestSquareSum = 10000.0000;
			smallestShifts[0] = 0;
			smallestShifts[1] = 0;

			int isFirstSumInRange = 1;
			//Second loop, in the search range for motion blocks
			for(rangeShiftX = -SEARCH_RANGE; rangeShiftX < SEARCH_RANGE; rangeShiftX++)
				for(rangeShiftY = -SEARCH_RANGE; rangeShiftY < SEARCH_RANGE; rangeShiftY++){

					int isThePositionOutOfRange = 0;
					double sum = 0;
					//This loop is to compare pixels in motion blocks
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
					//Out - of - bound control
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

/**************************************************************************
  Essential codes in the Motion estimation methods
		Significant codes in image sequence registration
 **************************************************************************/
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
			// Computation is limited for non - zero vectors to enhance the motion distance threshold
			if(motionVectorQueueNode->shiftX != 0 || motionVectorQueueNode->shiftY !=0){
				count++;
				sum += sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY));
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}

		if(sum != 0){
			return sum / count;
		} else {
			return 0;
		}
	}
}

/**
 * Function findReferencePoint(ImageInfoNodePtr *headImageInfoNodePtr, ImageInfoNodePtr *tailImageInfoNodePtr, vector<string>& srcFileNames)
 *		Try to find the best registration point inorder to do tha fastest registration
 * Parameters:
 *		srcFileNames: the vector that stores file names in the folder which contains all files in image sequence
 * Return:
 *		reference number, an integer that denotes the image index which will be select as reference.
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
		//Find a series of motion vectors for blocks in the current frame
		getBlockShift(shunkedGrayPlane1, shunkedGrayPlane2, &headMotionVectorPtr, &tailMotionVectorPtr);

		int shiftArray[2] = {0};
		//Find the shift array that stores the x and y shift of the current frame, which will be shifted to do the registration
		getImageShift(headMotionVectorPtr, &headDistanceFrequencyPtr, &tailDistanceFrequencyPtr, shiftArray);

		if(shiftArray[0] == 0 && shiftArray[1] == 0){
			//Two images match, move to next image, continue comparison
			matchNumbers++;
			index++;
		} else{
			//COMPUTE MOTION VECTOR, ENQUEUE
			enqueueImageInfo(headImageInfoNodePtr, tailImageInfoNodePtr, refIndex, matchNumbers, shiftArray[0], shiftArray[1]);

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
	//Enqueue the nodes that stores shifting information in the image sequence
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

/** Function computeBackgroundMotion(DistanceFrequencyNodePtr distanceFrequencyQueueNode, int shifts[])
 *		Find background motion by frequency, result stored in shifts[]
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

/** Function getImageShift(BlockMotionVectorPtr motionVectorQueueNode, DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, int shiftCo[])
 *		Find the shift array that stores the x and y shift of the current frame, which will be shifted to do the registration
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

		if(distanceThreshold != 0){
			//Find background motion by frequency, result stored in shifts[]
			computeBackgroundMotion(*headPtr, shiftCo);
		} else{
			*shiftCo = 0;
			*(shiftCo + 1) = 0;
		}
		*shiftCo = (*shiftCo) * pow(2.0, PYRAMID_LEVEL);
		*(shiftCo + 1) = *(shiftCo + 1) * pow(2.0, PYRAMID_LEVEL);
	}
}

/**
 * Function shiftImageSequence(ImageInfoNodePtr imageInfoNode, vector<string>& srcFileNames, vector<string>& dstFileNames)
 *		Shift the entire sequence one by one, and save to new files
 * Parameters:
 *		imageInfoNode: the queue node that stores shift information of the image
 *		srcFileNames: the vector that includes all source image file names
 *		dstFileNames: the vector that includes all new image file names where they could be saved
 */
void shiftImageSequence(ImageInfoNodePtr imageInfoNode, vector<string>& srcFileNames, vector<string>& dstFileNames){
	long matchedNumber = 0;	
	long index = 0;

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
	}
}


/**************************************************************************
  Essential codes in the Motion estimation methods
		Essential functions for image interpolations in image sequence
 **************************************************************************/
/** Function createNewFrame(IplImage ** newFrame, IplImage *refFrame, IplImage *curFrame, BlockMotionVectorPtr motionVectorQueueNode, double threshold)
 *		Main function of creating the new image between frames 
 */
void createNewFrame(IplImage ** newFrame, IplImage *refFrame, IplImage *curFrame, BlockMotionVectorPtr motionVectorQueueNode, double threshold){	
	int motions[2];
	//select the most probable motions
	adjustMotionBlock(motionVectorQueueNode, threshold, motions);

	if(motionVectorQueueNode == NULL){
		fprintf(stderr, "Error: Motion Vector Queue is empty\n");
		exit(-1);
	} else{
		while(motionVectorQueueNode != NULL){
			if(shiftedDistance(motionVectorQueueNode->shiftX, motionVectorQueueNode->shiftY) >= threshold){
				fillMotionBlock(newFrame, curFrame, motionVectorQueueNode->x * (pow(2.0, PYRAMIDLEVEL)), motionVectorQueueNode->y * (pow(2.0, PYRAMIDLEVEL)), (*motions) * (pow(2.0, PYRAMIDLEVEL)), (*(motions + 1)) * (pow(2.0, PYRAMIDLEVEL)));
			} else{
				fillBackGroundBlock(newFrame, refFrame, curFrame, motionVectorQueueNode->x * (pow(2.0, PYRAMIDLEVEL)), motionVectorQueueNode->y * (pow(2.0, PYRAMIDLEVEL)));
			}
			motionVectorQueueNode = motionVectorQueueNode->nextPtr;
		}
	}
}

/** Function void adjustMotionBlock(BlockMotionVectorPtr *motionVectorQueueNode)
 *		Find the point that needs interpolation and then create a new image
 */
void adjustMotionBlock(BlockMotionVectorPtr motionVectorQueueNode, double distanceThreshold, int motions[]){
	DistanceFrequencyNodePtr headDistanceFrequencyPtr = NULL;
	DistanceFrequencyNodePtr tailDistanceFrequencyPtr = NULL;
	BlockMotionVectorPtr headMotionVector = motionVectorQueueNode;

	while(motionVectorQueueNode != NULL){
		if(sqrt(square(motionVectorQueueNode->shiftX) + square(motionVectorQueueNode->shiftY)) >= distanceThreshold){
			enqueueDistanceFrequency(&headDistanceFrequencyPtr, &tailDistanceFrequencyPtr, motionVectorQueueNode->shiftX, motionVectorQueueNode->shiftY);
		}
		motionVectorQueueNode = motionVectorQueueNode->nextPtr;
	}

	long largestFrequency = headDistanceFrequencyPtr->frequency;
	int likelyMotionX = headDistanceFrequencyPtr->shiftX;
	int likelyMotionY = headDistanceFrequencyPtr->shiftY;

	while(headDistanceFrequencyPtr != NULL){
		if(largestFrequency < headDistanceFrequencyPtr->frequency){
			largestFrequency = headDistanceFrequencyPtr->frequency;
			likelyMotionX = headDistanceFrequencyPtr->shiftX;
			likelyMotionY = headDistanceFrequencyPtr->shiftY;
		}
		headDistanceFrequencyPtr =  headDistanceFrequencyPtr->nextPtr;
	}

	*motions = likelyMotionX;
	*(motions + 1) = likelyMotionY;
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
			//Do interpolation for those images	(within an specified range of largest motion)

			//Read images
			IplImage * refFrame = cvLoadImage(srcFileNames[index].c_str());
			IplImage * curFrame = cvLoadImage(srcFileNames[index + 1].c_str());

			BlockMotionVectorPtr headMotionVectorPtr = NULL;
			BlockMotionVectorPtr tailMotionVectorPtr = NULL;

			IplImage * refGrayPlane = NULL, * curGrayPlane = NULL, 
						* shrunkRefPlane = NULL, * shrunkCurPlane = NULL, * interFrame = NULL, * newFrame = NULL;

			allocateOnDemand(&interFrame, cvGetSize(curFrame), curFrame->depth, curFrame->nChannels);
			allocateOnDemand(&newFrame, cvGetSize(curFrame), curFrame->depth, curFrame->nChannels);
			//create grey planes
			allocateOnDemand(&refGrayPlane, cvGetSize(refFrame), IPL_DEPTH_8U, 1);
			cvCvtColor(refFrame, refGrayPlane, CV_BGR2GRAY);
			allocateOnDemand(&curGrayPlane, cvGetSize(curFrame), IPL_DEPTH_8U, 1);
			cvCvtColor(curFrame, curGrayPlane, CV_BGR2GRAY);
			//create image pyramid
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

			printf("New frame between image %ld and %ld interpolated!\n", index, index + 1);
			//new image re - naming
			char newName[500];
			strcpy(newName, directory);
			strcat(newName, "/inter_");
			strcat(newName, fileNames[index].c_str());

			cvSaveImage(newName, newFrame);

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