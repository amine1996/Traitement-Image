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

	int seuil = 110; // seuil à fixer nous même
	const char *input = argv[1];
	const char *output = argv[2];

	if (argc > 3)
	{
		seuil = atoi(argv[3]);
	}

	Mat originalPic = imread(input, CV_LOAD_IMAGE_COLOR);
	Mat result;
	//bilateralFilter(originalPic, result, 9, 50, 50);

	GaussianBlur(originalPic, originalPic, Size(9, 1), 0, 0);
	fastNlMeansDenoisingColored(originalPic, result, 10, 10, 13, 21);

	Mat grayscaleInput;
	cvtColor(result, grayscaleInput, CV_BGR2GRAY);

	if (!grayscaleInput.data || !originalPic.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	Mat binaryOutput(grayscaleInput.size(), grayscaleInput.type());
	Mat gradx(grayscaleInput.size(), grayscaleInput.type());
	Mat grady(grayscaleInput.size(), grayscaleInput.type());

	Sobel(grayscaleInput, gradx, -1, 1, 0, 3, 5, 0, BORDER_DEFAULT);
	Sobel(grayscaleInput, grady, -1, 0, 1, 3, 5, 0, BORDER_DEFAULT);

	//imshow("gradx Image", gradx);
	//imshow("grady Image", grady);

	//morphologyEx(gradx, gradx, MORPH_GRADIENT, Mat());
	//morphologyEx(grady, grady, MORPH_TOPHAT, Mat());

	for (int i = 0; i < originalPic.cols; i++)
	{
		for (int j = 0; j < originalPic.rows; j++)
		{
			binaryOutput.at<uchar>(j, i) = (uchar)round(sqrt(pow(gradx.at<uchar>(j, i), 2) + pow(grady.at<uchar>(j, i), 2)));
		}
	}

	//morphologyEx(binaryOutput, binaryOutput, MORPH_GRADIENT, Mat());
	threshold(binaryOutput, binaryOutput, seuil, 255, THRESH_BINARY);

	//imshow("Original Image", originalPic);
	imshow("Sobel  Image", binaryOutput);

	imwrite(output, binaryOutput);

	waitKey(0);
	return 0;
}
