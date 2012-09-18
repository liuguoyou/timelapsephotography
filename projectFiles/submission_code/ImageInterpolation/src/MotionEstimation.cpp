#include "Utils\Utils.h"
#include "Utils\MotionEstimationUtils.h"

/****************************************************************************************************************************
 * Main Function
 ****************************************************************************************************************************/
int main(void){

	char * directory = "images/car1";
	
	//vectore to store source file names
	vector<string> srcFileNames;
	vector<string> fileNames;
	//vector<string> dstFileNames;

	//Store file names in the vector
    getFileNames(directory, srcFileNames, fileNames);

	int quantity = (int) srcFileNames.size() - 1;

	//printf("%d", quantity);
	double *thresholds;

	thresholds = (double *) malloc(sizeof(double) *quantity);

	long index = 0;

	for (vector<string>::const_iterator iter = srcFileNames.begin( ); iter < srcFileNames.end( ) - 1; iter++){

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

		//Print out threshold
		//double distanceThreshold = computeDistanceThreshold(headMotionVectorPtr);

		*(thresholds + index)= computeDistanceThreshold(headMotionVectorPtr);

		/* For segmentation show
		IplImage * segImg = NULL;
		allocateOnDemand(&segImg, cvGetSize(curFrame), curFrame->depth, curFrame->nChannels);

		segmentImage(&segImg, headMotionVectorPtr, *(thresholds + index));
		cvSaveImage("images/midResults/segImage.jpg", segImg);
		*/

		printf("threshold[%ld]: %f\n", index, *(thresholds + index));

		index++;

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

	interpolateInSequence(directory, fileNames, srcFileNames, thresholds, quantity);

	free(thresholds);

	return 0;
}