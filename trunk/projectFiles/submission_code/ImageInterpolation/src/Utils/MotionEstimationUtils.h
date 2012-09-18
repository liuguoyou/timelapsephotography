#include "Utils.h"
/** Function shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel)
 *		Shrink image to form a image pyramid according to the level that defined at macro
 */
void shrinkImage(const IplImage * srcImage, IplImage ** dstImage, int downwardLevel);

/** Function getBlockShift(IplImage * referenceFrame, IplImage * currentFrame, BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr)
 *		Main function of block matching algorithm that are used in motion estimation, block size is preprocessored, as well as search range
 *		The final motion vector is stored as a node which is then insert to a queue
 *  Parameters:
 *		*referenceFrame: reference frame pointer, which points to the reference image, the previous one.
 *		*currentFrame: current frame pointer, which points to the second image where motion has occured in contrast to the reference frame
 *		*headPtr: head node address of the queue
 *		*tailPtr: tail node address of the queue
 */
void getBlockShift(IplImage * referenceFrame, IplImage * currentFrame, BlockMotionVectorPtr *headPtr, BlockMotionVectorPtr *tailPtr);

/** Function fillBlock(IplImage ** image, int beginPositionX, int beginPositionY, int colorValue)
 *		Assistant function for segmentation, which is used for test the block matching algorithm
 *		Fill gray value to a block according to the position in image
 */
void fillBlock(IplImage ** image, int beginPositionX, int beginPositionY, int colorValue);

/** Function segmentImage(IplImage ** newImage, BlockMotionVectorPtr motionVectorQueueNode, double threshold)
 *		Segment image to two values(0 and 255) to see the first - stage effect of the block matching algorithm 
 */
void segmentImage(IplImage ** newImage, BlockMotionVectorPtr motionVectorQueueNode, double threshold);

/** Function computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode)
 *		First version: to calculate the mean value of all distance, which is the simplest method to find a distance threshold
 */
double computeDistanceThreshold(BlockMotionVectorPtr motionVectorQueueNode);


/** Function fillBackGroundBlock(IplImage ** newFrame, IplImage * refFrame, IplImage * curFrame, int beginPositionX, int beginPositionY)
 *		Assistant function for creation of the new image
 *		Pixels in background for is calculated as an average of two frames
 */
void fillBackGroundBlock(IplImage ** newFrame, IplImage * refFrame, IplImage * curFrame, int beginPositionX, int beginPositionY);

/** Function fillMotionBlock(IplImage ** newFrame, IplImage * curFrame, int beginPositionX, int beginPositionY, int shiftX, int shiftY)
 *		Assistant function for creation of the new image
 *		Pixels in motion area is copy from current frame, with shifted half value by motion vector.
 */
void fillMotionBlock(IplImage ** newFrame, IplImage * curFrame, int beginPositionX, int beginPositionY, int shiftX, int shiftY);
 
/** Function createNewFrame(IplImage ** newFrame, IplImage *refFrame, IplImage *curFrame, BlockMotionVectorPtr motionVectorQueueNode, double threshold)
 *		Main function of creating the new image between frames 
 */
void createNewFrame(IplImage ** newFrame, IplImage *refFrame, IplImage *curFrame, BlockMotionVectorPtr motionVectorQueueNode, double threshold);

/** Function void fillBlankPixels(IplImage ** sourceImage)
 *		Fill in the blank pixels after interpolation
 */
void fillBlankPixels(IplImage * sourceImage, IplImage * interFrame, IplImage ** dstImage);

/** Function void adjustMotionBlock(BlockMotionVectorPtr *motionVectorQueueNode)
 *		Find the point that needs interpolation and then create a new image
 */
void adjustMotionBlock(BlockMotionVectorPtr motionVectorQueueNode, double distanceThreshold, int motions[]);

/** Function void interpolateInSequence(char * directory, vector<string>& fileNames, vector<string>& srcFileNames, double *thresholds, int quantity)
 *		Find the point that needs interpolation and then create a new image
 */
void interpolateInSequence(char * directory, vector<string>& fileNames, vector<string>& srcFileNames, double *thresholds, int quantity);