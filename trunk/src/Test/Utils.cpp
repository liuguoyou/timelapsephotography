#include "Utils.h"
#include <math.h>

//returns value between 0 and 255 of pixel at image position (x,y)
unsigned char getPixel(const IplImage* image, int x, int y){
  return ((unsigned char*)(image->imageData + image->widthStep*y))[x*image->nChannels];
}

//sets pixel at image position (x,y)
void setPixel(IplImage* image, int x, int y, unsigned char value){
  ((unsigned char*)(image->imageData + image->widthStep*y))[x*image->nChannels]=value;
}

void shrinkImage(const IplImage* srcImage, IplImage* &nextLevel){
	cvPyrDown(srcImage, nextLevel,CV_GAUSSIAN_5x5);
}

void computeBitMaps(const IplImage* srcImage, IplImage* &mtb, IplImage* &eb){
	int i,j;
	int accumulator = 0;
	
	int grayValue[256] = {0};

	int width = srcImage->width;
	int height = srcImage->height;

	for(i = 0; i < width; i++)
		for(j = 0; j < height; j++)
			grayValue[getPixel(srcImage, i, j)]++;

	i = 0; 
	while(accumulator < (width * height / 2))
		accumulator += grayValue[i++];

	int medianValue = i;

	for(i = 0; i < width; i++)
		for(j = 0; j < height; j++)
			if(getPixel(srcImage, i, j) <= medianValue)
				setPixel(mtb, i, j, 0);
			else
				setPixel(mtb, i, j, 255);

	for(i = 0; i < width; i++)
		for(j = 0; j < height; j++)
			if(abs(getPixel(srcImage, i, j) - medianValue) <= 4)
				setPixel(eb, i, j, 0);
			else
				setPixel(eb, i, j, 255);

}

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
	
	cvXor(tempImg1, tempImg2, result, 0);
}

int totalOneInBitMap(const IplImage* srcImage){
	return cvCountNonZero(srcImage);
}