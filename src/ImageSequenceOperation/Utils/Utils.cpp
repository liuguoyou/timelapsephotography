/**
 *Implementation of functions defined in Utils.h
 */

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
 * Implementation of Function 
 *		showImages(vector<string>& files, const string windowName)
 */
void showImages(vector<string>& files, const string windowName){

	int index = 0;
	//alpha and beta denote the transparency of images
	double alpha = 0.5; 
	double beta;

	Mat src1, src2, dst;

	vector<string>::iterator iter;

	for(iter = files.begin() + 1; iter != files.end(); iter++){
		// Read image ( same size, same type )
		src2 = imread(files[index + 1]);

		if(index == 0){
			src1 = imread(files[index]);	
		} else{
			src1 = dst;
		}
		// File check
		if( !src1.data ) { 
			printf("Error loading src1!\n");
			return;
		}
		if( !src2.data ) { 
			printf("Error loading src2!\n");
			return;
		}

		// Create Windows
		namedWindow(windowName, 1);

		beta = ( 1.0 - alpha );

		addWeighted(src1, alpha, src2, beta, 0.0, dst);
	
		//Print to check peocess
		cout << "File " << ++index <<" is emerged to destnation"<< endl;

	}
	imshow(windowName, dst);

	//waitKey(0);
}


//returns value between 0 and 255 of pixel at image position (x,y)
unsigned char getPixel(const IplImage* image, int x, int y){
  return ((unsigned char*)(image->imageData + image->widthStep*y))[x*image->nChannels];
}

//sets pixel at image position (x,y)
void setPixel(IplImage* image, int x, int y, unsigned char value){
  ((unsigned char*)(image->imageData + image->widthStep*y))[x*image->nChannels]=value;
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