/**
 * This header file defines functions that works as public utils, listed as follows:
 *		* getFileNames(string path, vector<string>& files)
 *		* showImages(vector<string>& files, const string windowName)
 */

#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <io.h>
#include <string.h>
 
using namespace std;
using namespace cv;

/** Function getFileNames(string path, vector<string>& files)
 *		To search in a directory, including all files, typically image files in this case, and store their names in a vector
 *	Parameters:
 *		path: the directory path to be searched
 *		files: the vector that used to store file names with directory path
 */
void getFileNames(string path, vector<string>& files);

/** Function showImages(vector<string>& files, const string windowName)
 *		With file names stored in the vector, overlap them each to show in a name - specified window.
 *	Parameters:
 *		files: the vector that used to store file names with directory path
 *		windowName: specified name of the window
 */
void showImages(vector<string>& files, const string windowName);
