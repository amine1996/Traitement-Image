#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>






using namespace std;
using namespace cv;



int main(int argc, char** argv){



	Mat imageIn = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
	
	
	if(! imageIn.data )                              // Check for invalid input
    	{
        	cout <<  "Could not open or find the image" << std::endl ;
        	return -1;
   	}
	
       
       
	
	imshow( "Display window", imageIn );             
	waitKey(0);       	

	
	return 0;
}

