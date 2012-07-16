#include "Utils.h"

#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;
using namespace cv;

void createImagePyramid(IplImage *srcImage, int level){

	if(srcImage->height < 20)
		return;

	IplImage *nextLevel = cvCreateImage(cvSize(srcImage->width/2, srcImage->height/2), srcImage->depth, srcImage->nChannels);
	cvPyrDown(srcImage, nextLevel,CV_GAUSSIAN_5x5);
	level++;

	stringstream ss;
	char s[3];
    ss << level;
    ss >> s;

	char baseName[12] = "Pyramid";

	strcat(baseName, s);

	cvNamedWindow(baseName, 1 );  
    cvShowImage(baseName, nextLevel);
	
	createImagePyramid(nextLevel, level);
}


int main(void){
   
	//Mat src1 = imread("images/Edinburgh/g1/P1020296.bmp");

	IplImage * src1= cvLoadImage("images/Edinburgh/g1/P1020295.bmp");

    //IplImage* gray_plane1 = cvCreateImage(cvGetSize(src1),8, 1);
	//IplImage* gray_plane2 = cvCreateImage(cvGetSize(src2),8, 1);

	//IplImage* mtb1 = cvCreateImage(cvGetSize(src1),8, 1);
	//IplImage* mtb2 = cvCreateImage(cvGetSize(src2),8, 1);

	//IplImage* eb1 = cvCreateImage(cvGetSize(src1),8, 1);
	//IplImage* eb2 = cvCreateImage(cvGetSize(src1),8, 1);

	IplImage * result = cvCreateImage(cvGetSize(src1), src1->depth, src1->nChannels);
	cvSet(result, CV_RGB(255, 255, 255), NULL);

	int xOffset = 50;
	int yOffset = -100;

	shiftBitMap(src1, xOffset, yOffset, result);
		

	//Rect roi_src(0, 0, src1.cols - xOffset, src1.rows - yOffset);
	/*
	srcRectROI.x = 0;
	srcRectROI.y = 0;
	srcRectROI.width = src1->width - xOffset;
	srcRectROI.height = src1->height - yOffset;

	*/

	//Rect roi_dst(xOffset, yOffset, src1.cols, src1.rows);
	/*
	resultRectROI.x = xOffset;
	resultRectROI.y = yOffset;
	resultRectROI.width = src1->width - xOffset;
	resultRectROI.height = src1->height - yOffset;
	*/
	//Mat img_src_roi = src1(roi_src);
    //Mat img_dst_roi = src1(roi_dst);

   //imshow("img_src_roi", img_src_roi);
   //imshow("img_dst_roi", img_dst_roi);
   //cvWaitKey();
   //img_src_roi.copyTo(img_dst_roi);
   
   //imshow("img_src_roi", img_src_roi);
   //imshow("img_dst_roi", img_dst_roi);
  
	cvSaveImage("images/Edinburgh/g1/new/P1020295.bmp", result);
	
	cvNamedWindow( "Source", 1 ); 
    cvShowImage("Source", src1);
	/*
	cvSetImageROI(src1, srcRectROI);
	cvSetImageROI(result, resultRectROI);

	result = cvCloneImage(src1);
	*/

	
	

    //cvCvtColor(src1,gray_plane1,CV_BGR2GRAY);  
	//cvCvtColor(src2,gray_plane2,CV_BGR2GRAY);

	//computeBitMaps(gray_plane1, mtb1, eb1);
	//computeBitMaps(gray_plane2, mtb2, eb2);

	//xorBitMap(mtb1, mtb2, result);

    //cvNamedWindow( "GraySource", 1 ); 
    //cvShowImage("GraySource",gray_plane); 
	
	cvNamedWindow( "ROIClone", 1 ); 
    cvShowImage("ROIClone",result);

	/*
    cvNamedWindow( "MedianThresholdImage", 1 );  
    cvShowImage( "MedianThresholdImage", mtb );  
	cvNamedWindow( "ExclusionImage", 1 );  
    cvShowImage( "ExclusionImage", eb );
	*/

    cvWaitKey(0);

	cvDestroyWindow( "Source" );
	cvDestroyWindow( "ROIClone" );
    cvReleaseImage( &src1 );
    cvReleaseImage( &result ); 

	return 0;
}  