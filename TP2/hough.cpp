#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

#define NB_CIRCLES 4

#define ROW_STEP 1
#define COL_STEP 1
#define RAD_STEP 1

#define RAD_MIN 10
#define RAD_MAX 100

/**
 *Sobel Function 
 */

struct localMax
{
	int r;
	int c;
	int rad;
	float val;

	bool operator<(localMax b)
	{
		return val < b.val;
	}

	bool operator>(localMax b)
	{
		return val > b.val;
	}

	bool operator==(localMax b)
	{
		return val == b.val;
	}

	bool operator>=(localMax b)
	{
		return val >= b.val;
	}

	bool operator<=(localMax b)
	{
		return val <= b.val;
	}
};

Mat sobel(Mat grayscaleInput, bool gaussian)
{
	Mat grayscaleCopy = grayscaleInput.clone();

	if (gaussian)
	{
		fastNlMeansDenoising(grayscaleCopy, grayscaleCopy, 3, 7, 50);
		GaussianBlur(grayscaleCopy, grayscaleCopy, Size(5, 5), 0, 0, BORDER_DEFAULT);
	}

	imshow("lol", grayscaleCopy);

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
		cout << "Utilisation : ./hough image_entree image_sortie (-g)" << endl;
		return -1;
	}

	const char *input = argv[1];
	const char *output = argv[2];

	bool gaussian = false;
	if (argc == 4)
	{
		string arg = argv[3];
		if (arg == "-g")
		{
			gaussian = true;
		}
	}

	/*
	int r = 31;
	int c = 31;
	int rad = 14;
	int cubeSize = 1;
	int rowSize = 100;
	int colSize = 100;
	int radSize = 100;

	for (int i = r - 1; i <= r + 1; i++)
	{
		if (i < 0)
			continue;

		if (i >= rowSize)
			break;

		for (int j = c - cubeSize; j <= c + cubeSize; j++)
		{
			if (j < 0)
				continue;

			if (j >= colSize)
				break;

			for (int k = rad - cubeSize; k <= rad + cubeSize; k++)
			{
				if (k < 0)
					continue;

				if (k >= radSize)
					break;

				/*if (i != r && j != c && k != rad)
					acc[i][j][k] = 0;
				std::cout << "i=" << i << " j=" << j << " k=" << k << endl;
			}
		}
	}*/

	Mat originalPic = imread(input, CV_LOAD_IMAGE_COLOR);
	Mat grayscaleInput = imread(input, CV_LOAD_IMAGE_GRAYSCALE);

	Mat tmp(grayscaleInput.size(), grayscaleInput.type());
	Mat result(grayscaleInput.size(), grayscaleInput.type());
	Mat seuilgris(grayscaleInput.size(), grayscaleInput.type());
	Mat sobel_(grayscaleInput.size(), grayscaleInput.type());

	if (!grayscaleInput.data || !originalPic.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return -1;
	}
	cout << "Enter the number of splits: 1" << endl;
	sobel_ = sobel(grayscaleInput, gaussian);

	imshow("titre", sobel_);
	//erode(sobel_, sobel_, Mat());
	//imshow("titre2", sobel_);
	//waitKey(0);

	float rowMin = 1;
	float rowMax = sobel_.rows;
	float rowStep = ROW_STEP;

	float colMin = 1;
	float colMax = sobel_.cols;
	float colStep = COL_STEP;

	float radMin = RAD_MIN;
	float radMax = RAD_MAX;
	float radStep = RAD_STEP;

	int const rowSize = ceil((rowMax - (rowMin - 1)) / rowStep);
	int const colSize = ceil((colMax - (colMin - 1)) / colStep);
	int const radSize = ceil((radMax - (radMin - 1)) / radStep);
	cout << "rowSize = " << rowSize << " colSize = " << colSize << " radSize = " << radSize << endl;

	float acc[rowSize][colSize][radSize];

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

	int a = 0, b = 0;
	int tmprad = 0;

	for (int row = 0; row < sobel_.rows; row++)
	{
		for (int col = 0; col < sobel_.cols; col++)
		{
			if (sobel_.at<uchar>(row, col) == 255)
			{
				for (int rad = radMin; rad <= radMax; rad += radStep)
				{
					for (int angle = 0; angle < 360; angle++)
					{
						//https://en.wikipedia.org/wiki/Circle_Hough_Transform
						a = ceil(col - rad * cos(angle * M_PI / 180)) / colStep;
						b = ceil(row - rad * sin(angle * M_PI / 180)) / rowStep;
						if (a >= 0 && b >= 0 && a < rowSize && b < colSize)
						{
							//std::cout << "a=" << a << " b=" << b << endl;
							tmprad = (rad - radMin) / radStep;
							acc[a][b][tmprad] += 1.0;
						}
					}
				}
			}
		}
	}

	float max = 0;
	bool isMax = true;

	vector<localMax> localMaxs;
	int cubeSize = 2;
	for (int r = 0; r < rowSize; r++)
	{
		for (int c = 0; c < colSize; c++)
		{
			for (int rad = 0; rad < radSize; rad++)
			{
				if (acc[r][c][rad] != 0)
				{
					max = acc[r][c][rad];
					isMax = true;

					for (int i = r - cubeSize; i <= r + cubeSize && isMax; i++)
					{
						if (i < 0)
							continue;

						if (i >= rowSize)
							break;

						for (int j = c - cubeSize; j <= c + cubeSize && isMax; j++)
						{
							if (j < 0)
								continue;

							if (j >= colSize)
								break;

							for (int k = rad - cubeSize; k <= rad + cubeSize && isMax; k++)
							{
								if (k < 0)
									continue;

								if (k >= radSize)
									break;

								if (acc[i][j][k] > max)
								{
									isMax = false;
									break;
								}
							}
						}
					}

					if (isMax)
					{
						for (int i = r - cubeSize; i <= r + cubeSize; i++)
						{
							if (i < 0)
								continue;

							if (i >= rowSize)
								break;

							for (int j = c - cubeSize; j <= c + cubeSize; j++)
							{
								if (j < 0)
									continue;

								if (j >= colSize)
									break;

								for (int k = rad - cubeSize; k <= rad + cubeSize; k++)
								{

									if (i == r && j == c && k == rad)
										continue;

									if (k < 0)
										continue;

									if (k >= radSize)
										break;

									acc[i][j][k] = 0;
								}
							}
						}

						localMax tmpMax;
						tmpMax.r = r;
						tmpMax.c = c;
						tmpMax.rad = rad;
						tmpMax.val = max;
						localMaxs.push_back(tmpMax);
					}
				}
			}
		}
	}

	sort(localMaxs.begin(), localMaxs.end());

	int nbCircles = NB_CIRCLES;
	if (localMaxs.size() < nbCircles)
		nbCircles = localMaxs.size() - 1;

	vector<localMax> bestMax(localMaxs.end() - nbCircles, localMaxs.end());

	for (int i = 0; i < bestMax.size(); i++)
	{

		float newRad = ((bestMax.at(i).rad + (radMin - 1)) * radStep);
		int newRow = ((bestMax.at(i).r + (rowMin - 1)) * rowStep);
		int newCol = ((bestMax.at(i).c + (colMin - 1)) * colStep);
		cout << bestMax.at(i).r << " " << bestMax.at(i).c << " " << bestMax.at(i).rad << " " << bestMax.at(i).val << endl;
		circle(originalPic, Point(newRow, newCol), newRad, Scalar(0, 0, 255), 1, LINE_8, 0);
	}

	imshow("salut", originalPic);

	cout << "FIN" << endl;

	waitKey(0);
	return 0;
}
