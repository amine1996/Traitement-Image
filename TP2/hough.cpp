#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;
using namespace cv;

/**
 *Sobel Function 
 */
Mat sobel(Mat grayscaleInput, bool gaussian)
{
	Mat grayscaleCopy = grayscaleInput.clone();

	if (gaussian)
	{
		//Denoising the image before applying the sobel filter
		fastNlMeansDenoising(grayscaleCopy, grayscaleCopy, 3, 7, 10);
		//Denoising using a gaussian blur
		GaussianBlur(grayscaleCopy, grayscaleCopy, Size(7, 5), 0, 0, BORDER_DEFAULT);
	}

	Mat output(grayscaleCopy.size(), grayscaleCopy.type());
	Mat gradx(grayscaleCopy.size(), grayscaleCopy.type());
	Mat grady(grayscaleCopy.size(), grayscaleCopy.type());

	Sobel(grayscaleCopy, gradx, -1, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(grayscaleCopy, grady, -1, 0, 1, 3, 1, 0, BORDER_DEFAULT);

	double max = 0;
	for (int i = 0; i < grayscaleCopy.rows; i++)
	{
		for (int j = 0; j < grayscaleCopy.cols; j++)
		{
			uchar value = (uchar)round(sqrt(pow(gradx.at<uchar>(i, j), 2) + pow(grady.at<uchar>(i, j), 2)));
			output.at<uchar>(i, j) = value;
			if (max < value)
				max = value;
		}
	}

	max /= 4;
	threshold(output, output, max, 255, THRESH_BINARY);

	return output;
}

/**
 *Grey scale Function 
 */
Mat seuilDeGris(Mat grayscaleInput, uchar seuil)
{
	Mat output(grayscaleInput.size(), grayscaleInput.type());
	threshold(grayscaleInput, output, seuil, 255, THRESH_BINARY_INV);
	return output;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "Utilisation : ./hough image_entree image_sortie (-g)" << std::endl;
		return -1;
	}

	const char *input = argv[1];
	const char *output = argv[2];

	bool gaussian = false;
	if (argc == 4)
	{
		if (argv[3] == "-g")
		{
			gaussian = true;
		}
	}

	Mat originalPic = imread(input, CV_LOAD_IMAGE_COLOR);
	Mat grayscaleInput = imread(input, CV_LOAD_IMAGE_GRAYSCALE);

	Mat tmp(grayscaleInput.size(), grayscaleInput.type());
	Mat result(grayscaleInput.size(), grayscaleInput.type());
	Mat seuilgris(grayscaleInput.size(), grayscaleInput.type());
	Mat sobel_(grayscaleInput.size(), grayscaleInput.type());

	if (!grayscaleInput.data || !originalPic.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	sobel_ = sobel(grayscaleInput, gaussian);

	imshow("titre", sobel_);

	float rowStep = 1;
	float colStep = 1;
	float radStep = 1;

	int radMin = 5;
	int radMax = 100;

	int rowSize = ceil(sobel_.rows / rowStep);
	int colSize = ceil(sobel_.cols / colStep);
	int radSize = ceil((radMin - radMax) / radStep);

	float[rowSize][colSize][radSize] acc;

	for (int i = 0; i < rowSize; i++)
	{
		for (int j = 0; j < colSize; j++)
		{
			for (int k = 0; k < radSize; k++)
			{
				acc[i][j][k] = 0;
			}
		}
	}

	for (int i = 0; i < sobel_.rows; i++)
	{
		for (int j = 0; j < sobel_.cols; j++)
		{
			if (sobel_.at<uchar>(i, j) == 255)
			{
				for (int rad = radMin; rad < radMax; rad += radStep)
				{
					for (int angle = 0; angle < 360; angle++)
					{
						//https://en.wikipedia.org/wiki/Circle_Hough_Transform
					}
				}
			}
		}
	}
	//Test and selection of the best threshold

	imwrite(output, result);

	waitKey(0);
	return 0;
}
