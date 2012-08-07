#include "Utils/RegistrationUtil.h"
#include "Utils/Utils.h"

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
void shrinkImage2(const IplImage* sourceImage, IplImage* &nextLevel){
	cvPyrDown(sourceImage, nextLevel,CV_GAUSSIAN_5x5);
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
 * Implementation of Function getExpShift(const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[])
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

		shrinkImage2(srcImage1, sm1Img1);
		shrinkImage2(srcImage2, sm1Img2);

		getExpShift(sm1Img1, sm1Img2, shiftBits - 1, curShift);

		cvReleaseImage(&sm1Img1);
		cvReleaseImage(&sm1Img2);

		shiftRet[0] = curShift[0] * 2;
		shiftRet[1] = curShift[1] * 2;
	} else {
		curShift[0] = curShift[1] = 0;

		createBitmaps(srcImage1, tb1, eb1);
		createBitmaps(srcImage2, tb2, eb2);
		/*
		cvNamedWindow( "TB1", 1 ); 
		cvShowImage("TB1", tb1);
		cvWaitKey(0);

		cvNamedWindow( "EB1", 1 ); 
		cvShowImage("EB1", eb1);
		cvWaitKey(0);
		*/
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
				/*
				cvNamedWindow( "ShiftedEB2", 1 ); 
				cvShowImage("ShiftedEB2", shiftedEB2);
				cvWaitKey(0);
				*/
				xorBitMap(tb1, shiftedTB2, diffBMP);
				
				/*
				cvNamedWindow( "tb1", 1 ); 
				cvShowImage("tb1", tb1);
				cvWaitKey(0);

				cvNamedWindow( "shiftedTB2", 1 ); 
				cvShowImage("shiftedTB2", shiftedTB2);
				cvWaitKey(0);

				cvNamedWindow( "diffBMP_XOR", 1 ); 
				cvShowImage("diffBMP_XOR", diffBMP);
				cvWaitKey(0);

				*/
				
				andBitMap(diffBMP, eb1, diffBMP);

				/*
				cvNamedWindow( "eb1", 1 ); 
				cvShowImage("eb1", eb1);
				cvWaitKey(0);
				
				cvNamedWindow( "diffBMP_AND", 1 ); 
				cvShowImage("diffBMP_AND", diffBMP);
				cvWaitKey(0);
				*/
				
				andBitMap(diffBMP, shiftedEB2, diffBMP);

				/*
				cvNamedWindow( "shiftedEB2", 1 ); 
				cvShowImage("shiftedEB2", shiftedEB2);
				cvWaitKey(0);
				
				cvNamedWindow( "diffBMP_AND2", 1 ); 
				cvShowImage("diffBMP_AND2", diffBMP);
				cvWaitKey(0);
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