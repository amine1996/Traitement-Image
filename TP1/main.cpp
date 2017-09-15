#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		cout << "Utilisation : ./seuilgris seuil image_entree image_binaire_sortie" << std::endl;
		return -1;
	}

	const int seuil = atoi(argv[1]);
	const char *input = argv[2];
	const char *output = argv[3];

	Mat grayscaleInput = imread(input, CV_LOAD_IMAGE_GRAYSCALE);
	Mat binaryOutput(grayscaleInput.size(), grayscaleInput.type());

	if (!grayscaleInput.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	threshold(grayscaleInput, binaryOutput, seuil, 255, THRESH_BINARY);

	imshow("Display window", binaryOutput);
	waitKey(0);

	return 0;
}
