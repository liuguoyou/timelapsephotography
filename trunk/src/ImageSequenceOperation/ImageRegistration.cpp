/**
 * cpp File includes the main function
 */
#include "Utils/Utils.h"
#include "Utils/RegistrationUtil.h"

void getExpShift(const IplImage * srcImage1, const IplImage * srcImage2, int shiftBits, int shiftRet[2]){
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

		curShift[0] *= 2;
		curShift[1] *= 2;
	} else {
		curShift[0] = curShift[1] = 0;

		createBitmaps(srcImage1, tb1, eb1);
		createBitmaps(srcImage2, tb2, eb2);

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

				xorBitMap(tb1, shiftedTB2, diffBMP);

				andBitMap(diffBMP, eb1, diffBMP);
				andBitMap(diffBMP, shiftedEB2, diffBMP);

				totalOneInBitMap(diffBMP);

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
	string directory = "images/Edinburgh/g2";
	//vectore to store source file names
	vector<string> srcFileNames;

	//Store file names in the vector
    getFileNames(directory, srcFileNames);

	//Show the unregistrated images
    showImages(srcFileNames, "Unaligned images");

    return 0;
}