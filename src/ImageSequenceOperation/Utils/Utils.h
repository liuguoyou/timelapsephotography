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

/** Function getPixel(IplImage* image, int x, int y)
 *		returns gray value between 0 and 255 of pixel at image position (x,y)
 *	Parameters:
 *		image: source image to be computed
 *		x, y: position
 */
unsigned char getPixel(const IplImage* image, int x, int y);

/** Function setPixel(IplImage* image, int x, int y, unsigned char value)
 *		sets pixel at image position (x,y)
 *	Parameters:
 *		image: source image to be set pixels
 *		x, y: position
 *		value: pixel value, 0 - black, 255 - white
 */
//sets pixel at image position (x,y)
void setPixel(IplImage* image, int x, int y, unsigned char value);