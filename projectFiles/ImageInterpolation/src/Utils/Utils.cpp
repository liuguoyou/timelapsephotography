#include "Utils.h"

/** 
 * Implementation of Function 
 *		getFileNames(string path, vector<string>& files)
 */
void getFileNames(string path, vector<string>& entireNames, vector<string>& fileNames){
	//File handle
    long   hFile   =   0;

    //File information
    struct _finddata_t fileinfo;
    string p;

    if((hFile = _findfirst(p.assign(path).append("/*").c_str(),&fileinfo)) != -1){
        do{
            //If it is a directory, go in iteration, otherwise, add to list
            if((fileinfo.attrib & _A_SUBDIR)){
                if(strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name,"..") != 0)
                    getFileNames( p.assign(path).append("/").append(fileinfo.name), entireNames, fileNames);
            }
            else{
                entireNames.push_back(p.assign(path).append("/").append(fileinfo.name));
				fileNames.push_back(fileinfo.name);
            }
        }   while(_findnext(hFile, &fileinfo) == 0);

        _findclose(hFile);
    }
}

/** 
 * Implementation of Function isQueueEmpty(BlockMotionVectorPtr headPtr)
 */
int isMotionVectorQueueEmpty(BlockMotionVectorPtr headPtr){
	return headPtr == NULL;
}

/** 
 * Implementation of Function enqueueMotionVectors(BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr, int x, int y, int shiftX, int shiftY)
 */
void enqueueMotionVectors(BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr,
						  int x, int y, int shiftX, int shiftY){

	BlockMotionVectorPtr newVectorPtr;
	newVectorPtr = (BlockMotionVectorPtr)malloc(sizeof(BlockMotionVector));

	if(newVectorPtr != NULL){
		newVectorPtr->x = x;
		newVectorPtr->y = y;
		newVectorPtr->shiftX = shiftX;
		newVectorPtr->shiftY = shiftY;
		newVectorPtr->nextPtr = NULL;

		if(isMotionVectorQueueEmpty(*headPtr)){
			*headPtr = newVectorPtr;
		} else {
			(*tailPtr)->nextPtr = newVectorPtr;
		}

		*tailPtr = newVectorPtr;
	} else{
		fprintf(stderr, "Error: New Motion vector is not inserted, Out of memory?\n");
		exit(-1);
	}

}

/** 
 * Implementation of Function isDistanceFrequencyQueueEmpty(DistanceFrequencyNodePtr headPtr)
 */
int isDistanceFrequencyQueueEmpty(DistanceFrequencyNodePtr headPtr){
	return headPtr == NULL;
}

/**
 * Implementation of Function enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, double distance)
 */
void enqueueDistanceFrequency(DistanceFrequencyNodePtr *headPtr, DistanceFrequencyNodePtr *tailPtr, double distance){

	if(isDistanceFrequencyQueueEmpty(*headPtr)){
	//The first node to enter the queue
		DistanceFrequencyNodePtr newDistancePtr;
		newDistancePtr = (DistanceFrequencyNodePtr)malloc(sizeof(DistanceFrequencyNode));
		
		if(newDistancePtr != NULL){
			newDistancePtr->distance = distance;
			newDistancePtr->frequency = 1;
			newDistancePtr->nextPtr = NULL;

			*headPtr = newDistancePtr;
			
			*tailPtr = newDistancePtr;
		} else{
			fprintf(stderr, "Error: New Distance Frequency is not inserted, Out of memory?\n");
			exit(-1);
		}
	} else{
	//Not the first to enter the queue, compare from head, then consider insertion
		DistanceFrequencyNodePtr tempPtr;
		tempPtr = (DistanceFrequencyNodePtr)malloc(sizeof(DistanceFrequencyNode));
		
		if(tempPtr != NULL){
			tempPtr = *headPtr;
		} else{
			fprintf(stderr, "Error: New Temp Distance Frequency is not inserted, Out of memory?\n");
			exit(-1);
		}

		int isDistanceInQueue = 0;

		while(tempPtr != NULL){
			if(distance == tempPtr->distance){
				tempPtr->frequency++;
				isDistanceInQueue = 1;
				break;
			} else {
				tempPtr = tempPtr->nextPtr;
			}
		}

		if(!isDistanceInQueue){
			DistanceFrequencyNodePtr newDistancePtr;
			newDistancePtr = (DistanceFrequencyNodePtr)malloc(sizeof(DistanceFrequencyNode));
			
			if(newDistancePtr != NULL){
				newDistancePtr->distance = distance;
				newDistancePtr->frequency = 1;
				newDistancePtr->nextPtr = NULL;

				(*tailPtr)->nextPtr = newDistancePtr;
				
				*tailPtr = newDistancePtr;
			} else{
				fprintf(stderr, "Error: New Distance Frequency is not inserted, Out of memory?\n");
				exit(-1);
			}
		}

		//free(tempPtr);
	}
}

/** 
 * Implementation of Function square(int a)
 */
double square(int a)
{
	return a * a;
}

/** 
 * Implementation of Function shiftedDistance(int shiftedX, int shiftedY)
 */
double shiftedDistance(int shiftedX, int shiftedY){
	return sqrt(double(square(shiftedX) + square(shiftedY)));
}

/** 
 * Implementation of Function getPixel(const IplImage* image, int x, int y)
 */
unsigned char getPixel(const IplImage* image, int x, int y){
  return ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels];
}

/** 
 * Implementation of Function getColorPixel(const IplImage* image, int x, int y, int colorChannel)
 */
unsigned char getColorPixel(const IplImage* image, int x, int y, int colorChannel){
  return ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels + colorChannel];
}

/** 
 * Implementation of Function setPixel(IplImage* image, int x, int y, unsigned char value)
 */
void setPixel(IplImage* image, int x, int y, unsigned char value){
  ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels] = value;
}

/** 
 * Implementation of Function setColorPixel(IplImage* image, int x, int y, unsigned char value, int colorChannel)
 */
void setColorPixel(IplImage* image, int x, int y, unsigned char value, int colorChannel){
  ((unsigned char*)(image->imageData + image->widthStep * y))[x * image->nChannels + colorChannel] = value;
}

/** 
 * Implementation of Function allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
 */
void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
{
	if ( *img != NULL )	return;

	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		fprintf(stderr, "Error: Couldn't allocate image.  Out of memory?\n");
		exit(-1);
	}
}