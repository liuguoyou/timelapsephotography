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
	string directory = "images/car";
	
	//vectore to store source file names
	vector<string> srcFileNames;
	vector<string> fileNames;
	vector<string> dstFileNames;

	//Store file names in the vector
    getFileNames(directory, srcFileNames, fileNames);

	ImageInfoNodePtr headImageInfoNodePtr = NULL;
	ImageInfoNodePtr tailImageInfoNodePtr = NULL;

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
	long referencePoint = findReferencePoint(&headImageInfoNodePtr, &tailImageInfoNodePtr, srcFileNames);
	adjustShiftsToReferenceImage(&headImageInfoNodePtr, &tailImageInfoNodePtr, referencePoint);
	shiftImageSequence(headImageInfoNodePtr, srcFileNames, dstFileNames);
		
    return 0;
}