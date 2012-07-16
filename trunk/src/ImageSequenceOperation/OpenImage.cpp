/**
 * cpp File includes the main function
 */

#include "Utils/Utils.h"

/**
 * Main Function
 */
int main(int argc, char* argv[]){
	string directory = "images/Edinburgh/g2";
	//vectore to store source file names
	vector<string> srcFileNames;

	//Store file names in the vector
    getFileNames(directory, srcFileNames);

	//Show the unregistrated images
    showImages(srcFileNames, "Unaligned images");

    return 0;
}