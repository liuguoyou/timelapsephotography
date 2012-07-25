/**
 * cpp File includes the main function
 */
#include "Utils/Utils.h"
#include "Utils/RegistrationUtil.h"

#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <io.h>
#include <string.h>
 
using namespace std;
using namespace cv;

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

/**
 * Main Function
 */
int main(int argc, char* argv[]){
	string directory = "images/test";
	
	//vectore to store source file names
	vector<string> srcFileNames;
	vector<string> fileNames;
	vector<string> dstFileNames;

	//Store file names in the vector
    getFileNames(directory, srcFileNames, fileNames);

	//iterator iter = new

	for(int i = 0; i < srcFileNames.size(); i++){
		char newName[100];
		//strcat(newName, "aligned");
	
		strcpy(newName, directory.c_str());
		strcat(newName, "/aligned_");
		strcat(newName, fileNames[i].c_str());

		dstFileNames.push_back(newName);
	}

	//Show the unregistrated images
    //showImages(srcFileNames, "Unaligned images");

	int offsets[2];
	int shiftBits = 1;

	IplImage  * srcImage1 = cvLoadImage(srcFileNames[0].c_str(),1);
	cvSaveImage(dstFileNames[0].c_str(), srcImage1, 0);

	for (vector<string>::const_iterator iter = srcFileNames.begin( ) + 1; iter != srcFileNames.end( ); iter++){
		int index = 1;

		IplImage  * srcImage2 = cvLoadImage(srcFileNames[index].c_str(),1);

		IplImage  * shiftedImage = cvCreateImage(cvGetSize(srcImage2), srcImage2->depth, srcImage2->nChannels);
		cvSet(shiftedImage, CV_RGB(255, 255, 255), NULL);
	
		getExpShift(srcImage1, srcImage2, shiftBits, offsets);

 		shiftBitMap(srcImage2, *offsets, *(offsets + 1), shiftedImage);

		/*
		cvNamedWindow( "Shifted", 1 ); 
		cvShowImage("Shifted", shiftedImage);		
		cvWaitKey(0);
		*/

		cvSaveImage(dstFileNames[index].c_str(), shiftedImage, 0);
		index++;
	}

	//const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[2]
	
	showImages(srcFileNames, "Unaligned images");
	showImages(dstFileNames, "Aligned images");
	waitKey(0);

    return 0;
}