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