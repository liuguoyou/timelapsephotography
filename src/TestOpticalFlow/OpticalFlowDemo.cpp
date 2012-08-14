#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>

static const double pi = 3.14159265358979323846;

inline static double square(int a)
{
	return a * a;
}

/* This is just an inline that allocates images.  I did this to reduce clutter in the
 * actual computer vision algorithmic code.  Basically it allocates the requested image
 * unless that image is already non-NULL.  It always leaves a non-NULL image as-is even
 * if that image's size, depth, and/or channels are different than the request.
 */
inline static void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
{
	if ( *img != NULL )	return;

	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		fprintf(stderr, "Error: Couldn't allocate image.  Out of memory?\n");
		exit(-1);
	}
}


int main(void){

	
		static IplImage  *convertedOneChannel1 = NULL, *src1Copy = NULL,
					     *convertedOneChannel2 = NULL, *src2Copy = NULL,
				        *eigImage = NULL, *tempImage = NULL, *pyramid1 = NULL, *pyramid2 = NULL;

		IplImage *srcImage1 = cvLoadImage("images/test/P1030701.jpg");
		IplImage *srcImage2 = cvLoadImage("images/test/P1030702.jpg");

		
		/*
		cvNamedWindow( "Source1", 1 ); 
		cvShowImage("Source1", srcImage1);

		cvNamedWindow( "Source1CPY", 1 ); 
		cvShowImage("Source1CPY", src1Copy);

		cvNamedWindow( "Source2", 1 ); 
		cvShowImage("Source2", srcImage2);

		cvNamedWindow( "Source2CPY", 1 ); 
		cvShowImage("Source2CPY", src2Copy);

		cvWaitKey(0);
		*/

		CvSize imageSize;

		imageSize.height = srcImage1->height;
		imageSize.width = srcImage1->width;

		/* Allocate another image if not already allocated.
		 * Image has ONE channel of color (ie: monochrome) with 8-bit "color" depth.
		 * This is the image format OpenCV algorithms actually operate on (mostly).
		 */
		allocateOnDemand( &convertedOneChannel1, imageSize, IPL_DEPTH_8U, 1 );
		/* Convert whatever the AVI image format is into OpenCV's preferred format.
		 * AND flip the image vertically.  Flip is a shameless hack.  OpenCV reads
		 * in AVIs upside-down by default.  (No comment :-))
		 */
		cvConvertImage(srcImage1, convertedOneChannel1, CV_CVTIMG_FLIP);

		/* We'll make a full color backup of this frame so that we can draw on it.
		 * (It's not the best idea to draw on the static memory space of cvQueryFrame().)
		 */
		//allocateOnDemand( &convertedSrc1, imageSize, IPL_DEPTH_8U, 3 );
		//cvConvertImage(srcImage1, convertedSrc1, 1);
		
		//Simlar operations for image2
		allocateOnDemand( &convertedOneChannel2, imageSize, IPL_DEPTH_8U, 1 );
		cvConvertImage(srcImage2, convertedOneChannel2, CV_CVTIMG_FLIP);

		/* Shi and Tomasi Feature Tracking! */

		/* Preparation: Allocate the necessary storage. */
		allocateOnDemand( &eigImage, imageSize, IPL_DEPTH_32F, 1 );
		allocateOnDemand( &tempImage, imageSize, IPL_DEPTH_32F, 1 );

		/* Preparation: This array will contain the features found in frame 1. */
		CvPoint2D32f image1Features[800];

		/* Preparation: BEFORE the function call this variable is the array size
		 * (or the maximum number of features to find).  AFTER the function call
		 * this variable is the number of features actually found.
		 */
		int numberOfFeatures;
		
		/* I'm hardcoding this at 800.  But you should make this a #define so that you can
		 * change the number of features you use for an accuracy/speed tradeoff analysis.
		 */
		numberOfFeatures = 800;

		/* Actually run the Shi and Tomasi algorithm!!
		 * "frame1_1C" is the input image.
		 * "eig_image" and "temp_image" are just workspace for the algorithm.
		 * The first ".01" specifies the minimum quality of the features (based on the eigenvalues).
		 * The second ".01" specifies the minimum Euclidean distance between features.
		 * "NULL" means use the entire input image.  You could point to a part of the image.
		 * WHEN THE ALGORITHM RETURNS:
		 * "frame1_features" will contain the feature points.
		 * "number_of_features" will be set to a value <= 800 indicating the number of feature points found.
		 */
		cvGoodFeaturesToTrack(convertedOneChannel1, eigImage, tempImage, image1Features, &numberOfFeatures, .01, .01, NULL);



		//-------------------------------------------------------------------------------------------------------
		/* Pyramidal Lucas Kanade Optical Flow! */

		/* This array will contain the locations of the points from frame 1 in frame 2. */
		CvPoint2D32f image2Features[800];

		/* The i-th element of this array will be non-zero if and only if the i-th feature of
		 * frame 1 was found in frame 2.
		 */
		char opticalFlowFoundFeature[800];

		/* The i-th element of this array is the error in the optical flow for the i-th feature
		 * of frame1 as found in frame 2.  If the i-th feature was not found (see the array above)
		 * I think the i-th entry in this array is undefined.
		 */
		float opticalFlowFeatureError[800];

		/* This is the window size to use to avoid the aperture problem (see slide "Optical Flow: Overview"). */
		CvSize opticalFlowWindow = cvSize(3,3);
		
		/* This termination criteria tells the algorithm to stop when it has either done 20 iterations or when
		 * epsilon is better than .3.  You can play with these parameters for speed vs. accuracy but these values
		 * work pretty well in many situations.
		 */
		CvTermCriteria opticalFlowTerminationCriteria
			= cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 );

		/* This is some workspace for the algorithm.
		 * (The algorithm actually carves the image into pyramids of different resolutions.)
		 */
		allocateOnDemand( &pyramid1, imageSize, IPL_DEPTH_8U, 1 );
		allocateOnDemand( &pyramid2, imageSize, IPL_DEPTH_8U, 1 );

		/* Actually run Pyramidal Lucas Kanade Optical Flow!!
		 * "frame1_1C" is the first frame with the known features.
		 * "frame2_1C" is the second frame where we want to find the first frame's features.
		 * "pyramid1" and "pyramid2" are workspace for the algorithm.
		 * "frame1_features" are the features from the first frame.
		 * "frame2_features" is the (outputted) locations of those features in the second frame.
		 * "number_of_features" is the number of features in the frame1_features array.
		 * "optical_flow_window" is the size of the window to use to avoid the aperture problem.
		 * "5" is the maximum number of pyramids to use.  0 would be just one level.
		 * "optical_flow_found_feature" is as described above (non-zero iff feature found by the flow).
		 * "optical_flow_feature_error" is as described above (error in the flow for this feature).
		 * "optical_flow_termination_criteria" is as described above (how long the algorithm should look).
		 * "0" means disable enhancements.  (For example, the second array isn't pre-initialized with guesses.)
		 */
		cvCalcOpticalFlowPyrLK(convertedOneChannel1, convertedOneChannel2, pyramid1, pyramid2, image1Features, image2Features, numberOfFeatures, opticalFlowWindow, 5, opticalFlowFoundFeature, opticalFlowFeatureError, opticalFlowTerminationCriteria, 0 );


		//Make copies for drawing
		allocateOnDemand( &src1Copy, imageSize, srcImage1->depth, srcImage1->nChannels );
		allocateOnDemand( &src2Copy, imageSize, srcImage2->depth, srcImage2->nChannels );

		cvCopy(srcImage1, src1Copy, NULL);
		cvCopy(srcImage2, src2Copy, NULL);


		/* For fun (and debugging :)), let's draw the flow field. */
		for(int i = 0; i < numberOfFeatures; i++){
			/* If Pyramidal Lucas Kanade didn't really find the feature, skip it. */
			if ( opticalFlowFoundFeature[i] == 0 )	continue;

			CvPoint img1FeaturePoint, img2FeaturePoint;

			img1FeaturePoint.x = (int) image1Features[i].x;
			img1FeaturePoint.y = (int) image1Features[i].y;
			img2FeaturePoint.x = (int) image2Features[i].x;
			img2FeaturePoint.y = (int) image2Features[i].y;


			cvCircle( src1Copy, img1FeaturePoint ,1 , CV_RGB(255,0,0),1, 8, 3 );
			cvCircle( src2Copy, img2FeaturePoint ,1 , CV_RGB(255,0,0),1, 8, 3 );

		}
		/* Now display the image we drew on.  Recall that "Optical Flow" is the name of
		 * the window we created above.
		 */
		cvSaveImage("images/test/OpticalFlowResult1.bmp", src1Copy);
		cvSaveImage("images/test/OpticalFlowResult2.bmp", src2Copy);

		cvNamedWindow( "FeaturePointsOnImage1", 1 );
		cvShowImage("FeaturePointsOnImage1", src1Copy);

		cvNamedWindow( "FeaturePointsOnImage2", 1 );
		cvShowImage("FeaturePointsOnImage2", src2Copy);
		/* And wait for the user to press a key (so the user has time to look at the image).
		 * If the argument is 0 then it waits forever otherwise it waits that number of milliseconds.
		 * The return value is the key the user pressed.
		 */
		cvWaitKey(0);


		/*
		cvNamedWindow( "Source1", 1 ); 
		cvShowImage("Source1", srcImage1);

		cvNamedWindow( "Source2", 1 ); 
		cvShowImage("Source2", srcImage2);

		cvWaitKey(0);
		
		*/
		return 0;

	

}