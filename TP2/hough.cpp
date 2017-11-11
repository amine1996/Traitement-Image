#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <algorithm>
#include <time.h>

using namespace std;
using namespace cv;

#define NB_CIRCLES 5

#define ROW_STEP 1
#define COL_STEP 1
#define RAD_STEP 1

#define RAD_MIN 7
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

	//If the 4th argument is -g, apply a gaussian blur
	if (argc == 4)
	{
		string arg = argv[3];
		if (arg == "-g")
		{
			gaussian = true;
		}
	}

	clock_t start = clock();

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

	sobel_ = sobel(grayscaleInput, gaussian);

	imshow("Sobel", sobel_);

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

	float acc[rowSize][colSize][radSize];

	//Filling accumulator with 0
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

	//Voting for circles
	for (int row = 0; row < sobel_.rows; row++)
	{
		for (int col = 0; col < sobel_.cols; col++)
		{
			if (sobel_.at<uchar>(row, col) == 255)
			{
				//For all circles size within radMin and radMax
				for (int rad = radMin; rad <= radMax; rad += radStep)
				{
					//For all angles
					for (int angle = 0; angle < 360; angle++)
					{
						//Center of the circle
						a = ceil(col - rad * cos(angle * M_PI / 180)) / colStep;
						b = ceil(row - rad * sin(angle * M_PI / 180)) / rowStep;

						//If within the accumulator size increment
						if (a >= 0 && b >= 0 && a < rowSize && b < colSize)
						{
							tmprad = (rad - radMin) / radStep;
							acc[a][b][tmprad] += 1;
						}
					}
				}
			}
		}
	}

	float max = 0;
	bool isMax = true;

	vector<localMax> localMaxs;

	//cubeSize is the size for the field of search of local maximas (1 is 3x3x3)
	int cubeSize = 1;
	for (int r = 0; r < rowSize; r++)
	{
		for (int c = 0; c < colSize; c++)
		{
			for (int rad = 0; rad < radSize; rad++)
			{
				//If there is vote for a circle
				if (acc[r][c][rad] != 0)
				{
					//Number of votes for the circle
					max = acc[r][c][rad];

					//By default is a local max
					isMax = true;

					//Looking for circles of size rad within the cube
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

								//If a circle has more votes
								if (acc[i][j][k] > max)
								{
									isMax = false;
									break;
								}
							}
						}
					}

					//If the circle is a local max, clean its neighbors
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

						//Adding the local max to a vector
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

	//Sorting the local maxs by votes
	sort(localMaxs.begin(), localMaxs.end());

	int nbCircles = NB_CIRCLES;
	if (localMaxs.size() < nbCircles)
		nbCircles = localMaxs.size() - 1;

	//Geting the nbCircles circles with most votes
	vector<localMax> bestMax(localMaxs.end() - nbCircles, localMaxs.end());

	for (int i = 0; i < bestMax.size(); i++)
	{
		float newRad = ((bestMax.at(i).rad + (radMin - 1)) * radStep);
		int newRow = ((bestMax.at(i).r + (rowMin - 1)) * rowStep);
		int newCol = ((bestMax.at(i).c + (colMin - 1)) * colStep);

		//Draw a circle
		circle(originalPic, Point(newRow, newCol), newRad, Scalar(0, 0, 255), 1, LINE_8, 0);
	}

	clock_t end = clock();

	imshow("Circles", originalPic);

	std::cout << "Execution time : " << (((float)(end - start)) / CLOCKS_PER_SEC) * 1000 << "ms" << std::endl;
	waitKey(0);
	return 0;
}

//Trouver le bon rapport entre la taille du cercle et l'importance du vote
//Enlever le bruit correctement avec opencv
//coins2.png trop grande, il faudrait réduire la taille de l'image, chercher les cercles et placer les cercles au bon endroit
//Flou fonctionnant pour toutes les images
//Ne pas garder les cercles dans la meme zone