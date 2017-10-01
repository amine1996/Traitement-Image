#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "Utilisation : ./sobel image_entree image_binaire_sortie" << std::endl;
		return -1;
	}

	//Value for threshold applied after Sobel
	int seuil = 110;
	const char *input = argv[1];
	const char *output = argv[2];

	if (argc > 3)
	{
		seuil = atoi(argv[3]);
	}

	Mat originalPic = imread(input, CV_LOAD_IMAGE_COLOR);
	Mat grayscaleInput = imread(input, CV_LOAD_IMAGE_GRAYSCALE);

	if (!grayscaleInput.data || !originalPic.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	Mat binaryOutput(grayscaleInput.size(), grayscaleInput.type());
	Mat gradx(grayscaleInput.size(), grayscaleInput.type());
	Mat grady(grayscaleInput.size(), grayscaleInput.type());

	//Horizontal gradient, derivative 1 in x
	Sobel(grayscaleInput, gradx, -1, 1, 0, 3, 2, 0, BORDER_DEFAULT);

	//Vertical gradient, derivative 1 in y
	Sobel(grayscaleInput, grady, -1, 0, 1, 3, 1, 0, BORDER_DEFAULT);

	//Calculation of the gradient on each pixel
	//C(i,j) = sqrt(A(i,j)² + B(i,j)²)
	for (int i = 0; i < originalPic.rows; i++)
	{
		for (int j = 0; j < originalPic.cols; j++)
		{
			binaryOutput.at<uchar>(i, j) = (uchar)round(sqrt(pow(gradx.at<uchar>(i, j), 2) + pow(grady.at<uchar>(i, j), 2)));
		}
	}

	imshow("Gradient Image", binaryOutput);

	threshold(binaryOutput, binaryOutput, seuil, 255, THRESH_BINARY);

	imshow("Sobel  Image", binaryOutput);

	imwrite(output, binaryOutput);

	waitKey(0);
	return 0;
}
