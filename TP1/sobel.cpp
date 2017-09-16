#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "Utilisation : ./sobel image_entree image_binaire_sortie" << std::endl;
		return -1;
	}

	const int seuil = 120; // seuil à fixer nous même
	const char *input = argv[1];
	const char *output = argv[2];

	Mat orignalPic = imread(input, CV_LOAD_IMAGE_COLOR);
	Mat grayscaleInput = imread(input, CV_LOAD_IMAGE_GRAYSCALE);
	Mat binaryOutput(grayscaleInput.size(), grayscaleInput.type());
	Mat gradx(grayscaleInput.size(), grayscaleInput.type());
	Mat grady(grayscaleInput.size(), grayscaleInput.type());

	if (!grayscaleInput.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	Sobel( grayscaleInput, gradx, -1, 1, 0, 3, 1, 0, BORDER_DEFAULT );
	Sobel( grayscaleInput, grady, -1, 0, 1, 3, 1, 0, BORDER_DEFAULT );

	imshow("gradx Image", gradx);
	imshow("grady Image", grady);
		
	pow(gradx, 2, gradx);
	pow(grady, 2, grady);
	
	add( gradx, grady, binaryOutput );

	/*sqrt(binaryOutput, binaryOutput); /// bug ici core dump pow je sais plus quoi mais c'est bien ici 

	imshow("Sobel Image", binaryOutput);

	threshold(binaryOutput, binaryOutput, seuil, 255, THRESH_BINARY);
	//adaptiveThreshold(grayscaleInput, binaryOutput, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 31, 70);

	imshow("Original Image", orignalPic);
	imshow("Binary Image", binaryOutput);

	imwrite( output, binaryOutput );*/


	waitKey(0);
	return 0;
}
