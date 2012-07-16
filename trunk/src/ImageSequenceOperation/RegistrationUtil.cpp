#include "Utils/RegistrationUtil.h"
#include "Utils/Utils.h"


/**
 * Implementation of Function createBitmaps(const IplImage *img, BYTE *tb, BYTE *eb)
 */
void createBitmaps(const IplImage *srcImage, IplImage* &mtb, IplImage* &eb){

	IplImage* gray_plane = cvCreateImage(cvGetSize(srcImage),8,1);

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

/**
 * Implementation of Function void shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel)
 */
void shrinkImage2(IplImage* sourceImage, IplImage* &nextLevel){
	cvPyrDown(sourceImage, nextLevel,CV_GAUSSIAN_5x5);
}