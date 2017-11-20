#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>

using namespace std;
using namespace cv;

/**
 *Sobel Function 
 */
Mat sobel(Mat grayscaleInput, bool gaussian)
{
	Mat grayscaleCopy = grayscaleInput.clone();

	if (gaussian)
		GaussianBlur(grayscaleCopy, grayscaleCopy, Size(9, 9), 0, 0);

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
		cout << "Utilisation : ./fast image_entree image_sortie" << endl;
		return -1;
	}

	const char *input = argv[1];
	const char *output = argv[2];

	Mat originalPic = imread(input, CV_LOAD_IMAGE_COLOR);
	Mat grayscaleInput = imread(input, CV_LOAD_IMAGE_GRAYSCALE);
	Mat sobel_(grayscaleInput.size(), grayscaleInput.type());

	if (!grayscaleInput.data || !originalPic.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}

	vector<int> vecDeltaX = {0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1};
	vector<int> vecDeltaY = {3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1, 0, 1, 2, 3};

	vector<Point2d> vecCorners;

	for (int row = 0; row < grayscaleInput.rows; row++)
	{
		for (int col = 0; col < grayscaleInput.cols; col++)
		{
			uchar valCenterPixel = grayscaleInput.at<uchar>(col, row);

			int deltaX;
			int deltaY;
			int deltaVal = 15;

			int nbContiguousBrighter = 0;
			int nbContiguousDarker = 0;
			int neededContiguous = 9;

			bool foundCorner = false;

			for (int k = 0; k < vecDeltaX.size() + 8; k++)
			{
				deltaX = vecDeltaX.at(k % vecDeltaX.size());
				deltaY = vecDeltaY.at(k % vecDeltaX.size());

				if (row + deltaX < 0 || row + deltaX > grayscaleInput.rows)
					continue;

				if (col + deltaY < 0 || col + deltaY > grayscaleInput.cols)
					continue;

				uchar valDeltaPixel = grayscaleInput.at<uchar>(col + deltaY, row + deltaX);

				if (valCenterPixel + deltaVal > valDeltaPixel)
				{
					nbContiguousBrighter++;
					nbContiguousDarker = 0;
				}

				if (valCenterPixel - deltaVal < valDeltaPixel)
				{
					nbContiguousDarker++;
					nbContiguousBrighter = 0;
				}

				if (nbContiguousBrighter >= neededContiguous || nbContiguousDarker >= neededContiguous)
				{
					foundCorner = true;
					vecCorners.push_back(Point2d(row + deltaX, col + deltaY));
					circle(originalPic, Point(row + deltaX, col + deltaY), 5, Scalar(0, 0, 255), 1, 8, 0);
					break;
				}
			}
		}
	}

	imshow("Center", originalPic);
	waitKey(0);

	return 0;
}
