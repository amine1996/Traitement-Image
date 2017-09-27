#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;
using namespace cv;



Mat sobel(Mat grayscaleInput,uchar seuil){
	//fastNlMeansDenoising(grayscaleInput, grayscaleInput, 8, 50, 10);	
	GaussianBlur(grayscaleInput, grayscaleInput, Size(7, 5), 0, 0, BORDER_DEFAULT);

	Mat output(grayscaleInput.size(), grayscaleInput.type());
	Mat gradx(grayscaleInput.size(), grayscaleInput.type());
	Mat grady(grayscaleInput.size(), grayscaleInput.type());

	Sobel(grayscaleInput, gradx, -1, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(grayscaleInput, grady, -1, 0, 1, 3, 1, 0, BORDER_DEFAULT);


	for(int i = 0; i < grayscaleInput.rows; i++){
		for(int j = 0; j < grayscaleInput.cols; j++){
			output.at<uchar>(i,j) = (uchar) round( sqrt(pow(gradx.at<uchar>(i,j), 2) + pow(grady.at<uchar>(i,j), 2)));
		}
	}

	threshold(output, output, seuil, 255, THRESH_BINARY);

	return output;
}

Mat laplacian(Mat grayscaleInput, uchar seuil){
	fastNlMeansDenoising(grayscaleInput, grayscaleInput, 8, 50, 10);	
	GaussianBlur(grayscaleInput, grayscaleInput, Size(7, 5), 0, 0, BORDER_DEFAULT);

	Mat output(grayscaleInput.size(), grayscaleInput.type());

	Mat abs_dst;

	Laplacian( grayscaleInput, output, -1, 3, 1, 0, BORDER_DEFAULT );
	convertScaleAbs( output, output );
	threshold(output, output, seuil, 255, THRESH_BINARY);

	return output;
}

Mat seuilDeGris(Mat grayscaleInput, uchar seuil){
	Mat output(grayscaleInput.size(), grayscaleInput.type());
	threshold(grayscaleInput, output, seuil, 255, THRESH_BINARY_INV);	
	return output;
}





int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "Utilisation : ./bestResult image_entree image_sortie" << std::endl;
		return -1;
	}

	int seuil = 110; // seuil à fixer nous même
	const char *input = argv[1];
	const char *output = argv[2];


	Mat originalPic = imread(input, CV_LOAD_IMAGE_COLOR);
	Mat grayscaleInput = imread(input, CV_LOAD_IMAGE_GRAYSCALE);
	Mat result(grayscaleInput.size(), grayscaleInput.type());
	Mat seuilgris(grayscaleInput.size(), grayscaleInput.type());
	Mat sobel_(grayscaleInput.size(), grayscaleInput.type());
	Mat laplacian_(grayscaleInput.size(), grayscaleInput.type());

	
	if (!grayscaleInput.data || !originalPic.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}


	seuilgris = seuilDeGris(grayscaleInput, 110);
	sobel_ = sobel(grayscaleInput, 80);
	laplacian_ = laplacian(grayscaleInput, 10);


	imshow("seuilgris  Image", seuilgris);
	imshow("Sobel  Image", sobel_);
	imshow("laplacian  Image", laplacian_);
	
	for(int i = 0; i < grayscaleInput.rows; i++){
		for(int j = 0; j < grayscaleInput.cols; j++){
			if(seuilgris.at<uchar>(i,j) == sobel_.at<uchar>(i,j) && sobel_.at<uchar>(i,j) == laplacian_.at<uchar>(i,j)){
				result.at<uchar>(i,j) = seuilgris.at<uchar>(i,j);
			}
		}
	}

	imshow("Result  Image", result);

	waitKey(0);
	return 0;
}
