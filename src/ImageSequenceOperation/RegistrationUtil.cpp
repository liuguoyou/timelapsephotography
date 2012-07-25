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
			if(abs(getPixel(gray_plane, i, j) - medianValue) <= 4)
				setPixel(eb, i, j, 0);
			else
				setPixel(eb, i, j, 255);

}

/**
 * Implementation of Function convertIplImageToBitmap(IplImage* pIpl, BYTE* &pBmp, DWORD& size)
 */
/*
void convertIplImageToBitmap(IplImage* pIpl, BYTE* &pBmp, DWORD& size){
	BITMAPFILEHEADER bfh = {0};
    DWORD dwImageSize = 0;
    DWORD dwBytesRead = 0;
    
    int w = pIpl->width;
    int l = ((w * 24 +31) & ~31) /8;
    int h = pIpl->height;
    dwImageSize    = l * h;
    
	//Definition of file class
    bfh.bfType        = (WORD)'M' << 8 | 'B';
	//Definition of header file size
    bfh.bfOffBits    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	//File size
    bfh.bfSize        = bfh.bfOffBits + dwImageSize;
    
    BITMAPINFOHEADER  bih = {0};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = pIpl->width;
    bih.biHeight = pIpl->height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage    = 0;
    bih.biXPelsPerMeter    = 0;
    bih.biYPelsPerMeter    = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;
    
    size = bfh.bfSize;
    pBmp = new BYTE[bfh.bfSize+1];
    
    memset(pBmp, 0, bfh.bfSize + 1);
    memcpy(pBmp, &bfh, sizeof(BITMAPFILEHEADER));
    memcpy(pBmp+sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));
    BYTE* p = pBmp+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    memcpy(p, (BYTE*)pIpl->imageData, dwImageSize);

}
*/

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