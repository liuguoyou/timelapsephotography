#include "cv.h"
#include "highgui.h"

using namespace std;


void shrinkImage(const IplImage* srcImage, IplImage* &nextLevel);

void computeBitMaps(const IplImage* srcImage, IplImage* &mtb, IplImage* &eb);

void shiftBitMap(const IplImage* srcImage, int xOffset, int yOffset, IplImage* &result);

void xorBitMap(const IplImage* srcImage1, const IplImage* srcImage2, IplImage* &result);

int totalOneInBitMap(const IplImage* srcImage);


//returns value between 0 and 255 of pixel at image position (x,y)
unsigned char getPixel(const IplImage* image, int x, int y);

//sets pixel at image position (x,y)
void setPixel(IplImage* image, int x, int y, unsigned char value);