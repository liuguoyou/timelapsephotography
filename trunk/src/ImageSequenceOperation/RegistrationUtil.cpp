#include "Utils/RegistrationUtil.h"
#include "Utils/Utils.h"


/** 
 * Implementation of Function isQueueEmpty(BlockMotionVectorPtr headPtr)
 */
int isImageInfoQueueEmpty(ImageInfoNodePtr headPtr){
	return headPtr == NULL;
}

void enqueueImageInfo(ImageInfoNodePtr *headPtr, ImageInfoNodePtr * tailPtr, int imageIndex, long matchedImageNumbers, int shiftX, int shiftY){
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
		fprintf(stderr, "Error: New Motion vector is not inserted, Out of memory?\n");
		exit(-1);
	}
}

/**
 * Implementation of Function createGrayPlane(const IplImage *srcImage, IplImage* &grayPlane)
 */
void createGrayPlane(const IplImage *srcImage, IplImage ** grayPlane){
	allocateOnDemand(grayPlane, cvGetSize(srcImage), srcImage->depth, 1);
	cvCvtColor(srcImage, grayPlane, CV_BGR2GRAY);
}

/**
 * Implementation of Function differImages(const IplImage *srcGrayImage1, const IplImage *srcGrayImage2)
 */
int isImagesMatch(const IplImage *srcGrayImage1, const IplImage *srcGrayImage2){
	int matchedPixelNumber = 0;

	for(int i = 0; i < srcGrayImage1->width; i++)
		for(int j = 0; j < srcGrayImage1->height; j++)
			if(getPixel(srcGrayImage1, i, j) - getPixel(srcGrayImage2, i, j) == 0)
				matchedPixelNumber++;

	return matchedPixelNumber / (srcGrayImage1->width * srcGrayImage1->height) >= MATCH_THRESHOLD;
}

/**
 * Implementation of Function createBitmaps(const IplImage *img, BYTE *tb, BYTE *eb)
 */
void createBitmaps(const IplImage *srcImage, IplImage* &mtb, IplImage* &eb){

	IplImage* gray_plane = cvCreateImage(cvGetSize(srcImage),8,1);
	cvCvtColor(srcImage, gray_plane, CV_BGR2GRAY);  

	/*
	cvNamedWindow("gray", 1); 
	cvShowImage("gray", gray_plane);
	cvWaitKey(0);
	*/

	int i, j;

	int medianValue = 0, accumulator = 0;

	int grayValue[256] = {0};

	int width = gray_plane->width;
	int height = gray_plane->height;

	for(i = 0; i < width; i++)
		for(j = 0; j < height; j++)
			grayValue[getPixel(gray_plane, i, j)]++;

	i = 0; 
	while(accumulator < (width * height / 2))
		accumulator += grayValue[i++];

	medianValue = i;

	//Create median threshold bitmap
	for(i = 0; i < width; i++)
		for(j = 0; j < height; j++)
			if(getPixel(gray_plane, i, j) <= medianValue)
				setPixel(mtb, i, j, 0);
			else
				setPixel(mtb, i, j, 255);

	/*
	cvNamedWindow( "mtb", 1 ); 
	cvShowImage("mtb", mtb);
	cvWaitKey(0);
	*/

	//Create exclusion bitmap
	for(i = 0; i < width; i++)
		for(j = 0; j < height; j++)
			if(abs(getPixel(gray_plane, i, j) - medianValue) <= 6)
				setPixel(eb, i, j, 0);
			else
				setPixel(eb, i, j, 255);

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
 * Implementation of Function shiftBitMap(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result)
 */
void shiftBitMap(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result){
	int i, j;

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
 * Implementation of Function  xorBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result)
 */
void xorBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result){
	int i, j;

	IplImage* tempImg1 = cvCreateImage(cvGetSize(srcImage1), srcImage1->depth, srcImage1->nChannels);
	IplImage* tempImg2 = cvCreateImage(cvGetSize(srcImage2), srcImage2->depth, srcImage2->nChannels);

	for(i = 0; i < srcImage1->width; i++)
		for(j = 0; j < srcImage1->height; j++)
			if(getPixel(srcImage1, i, j) != 0)
				setPixel(tempImg1, i, j, 1);

	for(i = 0; i < srcImage2->width; i++)
		for(j = 0; j < srcImage2->height; j++)
			if(getPixel(srcImage2, i, j) != 0)
				setPixel(tempImg2, i, j, 1);

	for(i = 0; i < srcImage2->width; i++)
		for(j = 0; j < srcImage2->height; j++)
			if(getPixel(tempImg1, i, j) ^ getPixel(tempImg2, i, j))
				setPixel(result, i, j, 255);
			else
				setPixel(result, i, j, 0);
	//cvXor(tempImg1, tempImg2, result, 0);
}

/**
 * Implementation of Function andBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result)
 */
void andBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result){
	int i, j;

	IplImage* tempImg1 = cvCreateImage(cvGetSize(srcImage1), srcImage1->depth, srcImage1->nChannels);
	IplImage* tempImg2 = cvCreateImage(cvGetSize(srcImage2), srcImage2->depth, srcImage2->nChannels);

	for(i = 0; i < srcImage1->width; i++)
		for(j = 0; j < srcImage1->height; j++)
			if(getPixel(srcImage1, i, j) != 0)
				setPixel(tempImg1, i, j, 1);

	for(i = 0; i < srcImage2->width; i++)
		for(j = 0; j < srcImage2->height; j++)
			if(getPixel(srcImage2, i, j) != 0)
				setPixel(tempImg2, i, j, 1);


	for(i = 0; i < srcImage2->width; i++)
		for(j = 0; j < srcImage2->height; j++)
			if(getPixel(tempImg1, i, j) == getPixel(tempImg2, i, j))
				setPixel(result, i, j, 255);
			else
				setPixel(result, i, j, 0);
	
	//cvAnd(tempImg1, tempImg2, result, 0);
}

/**
 * Implementation of Function totalOneInBitMap(const IplImage* srcImage)
 */
int totalOneInBitMap(const IplImage* srcImage){
	int i,j;
	int totalOnes = 0;

	for(i = 0; i < srcImage->width; i++)
		for(j = 0; j < srcImage->height; j++)
			if(getPixel(srcImage, i, j) != 0)
				totalOnes++;

	return totalOnes;
}

/**
 * Implementation of Function findReferencePoint(vector<string> srcFileNames)
 */
int findReferencePoint(vector<string>& srcFileNames){
	int referencePoint = 0, index = 1, refIndex = 0;
	int matchNumbers = 0;

	vector<string>::iterator iter;

	IplImage *src1 = NULL, *src2 = NULL, 
			 *grayPlane1 = NULL, *grayPlane2 = NULL, 
			 *shunkedGrayPlane1 = NULL, *shunkedGrayPlane2 = NULL;

	for(iter = srcFileNames.begin() + 1; iter < srcFileNames.end(); iter++){
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
		createGrayPlane(src1, &grayPlane1);
		createGrayPlane(src2, &grayPlane2);

		//shrink images according to pyramid level
		shrinkImage(grayPlane1, &shunkedGrayPlane1, PYRAMID_LEVEL);
		shrinkImage(grayPlane2, &shunkedGrayPlane2, PYRAMID_LEVEL);

		if(isImagesMatch(shunkedGrayPlane1, shunkedGrayPlane2)){
			//Two images match, move to next image, continue comparison
			matchNumbers++;
			index++;
		} else{
			

			//TODO COMPUTE MOTION VECTOR, ENQUEUE

			refIndex = index;
			index++;
		}
	}
}