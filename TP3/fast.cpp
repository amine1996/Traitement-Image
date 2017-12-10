#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>

using namespace std;
using namespace cv;

//Size of the patch (1 is 3x3, 2 is 5x5 ...)
#define SIZE_PATCH 2

//Max threshold to accept a match
#define SEUIL 2500

//Difference between best minimum and second best minimum
#define DIFFERENCE_MIN 500

//Number of needed contiguous pixels
#define NB_CONTIGUOUS 9

//Represents a corner point
struct cornerPoint
{
	//Coordinates of the corner point
	Point point;

	//Reference to the best matched cornerPoint
	cornerPoint *match;

	//Mean of the patch for this cornerPoint
	int meanPatch;

	//Score of the best match
	int matchedScore;
};

vector<cornerPoint> getCorners(Mat grayscalePic)
{
	//Delta for a 3x3 circle around the point of interest
	const vector<int> vecDeltaX = {0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1};
	const vector<int> vecDeltaY = {3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1, 0, 1, 2, 3};
	vector<cornerPoint> vecCorners;

	for (int row = 0; row < grayscalePic.rows; row++)
	{
		for (int col = 0; col < grayscalePic.cols; col++)
		{
			//Value of the point of interest
			uchar valCenterPixel = grayscalePic.at<uchar>(row, col);

			//Value of the delta of the compared pixel from the point of interest
			int deltaX;
			int deltaY;

			//Delta to be brighter/darker than the point of interest
			int deltaVal = 50;

			//Number of contiguous brighter pixels
			int nbContiguousBrighter = 0;

			//Number of contiguous darker pixels
			int nbContiguousDarker = 0;

			//Looking for a circle and a half around the point of interest
			for (unsigned int k = 0; k < vecDeltaX.size() + NB_CONTIGUOUS - 1; k++)
			{
				//Position of the compared pixel
				deltaX = vecDeltaX.at(k % vecDeltaX.size());
				deltaY = vecDeltaY.at(k % vecDeltaX.size());

				//If the pixel is outside the image
				if (row + deltaX < 0 || row + deltaX >= grayscalePic.rows)
					continue;

				if (col + deltaY < 0 || col + deltaY >= grayscalePic.cols)
					continue;

				//Value of the compared pixel
				uchar valDeltaPixel = grayscalePic.at<uchar>(row + deltaX, col + deltaY);

				//If brighter increment the counter and set darker pixels counter to 0
				if (valCenterPixel + deltaVal > valDeltaPixel)
				{
					nbContiguousBrighter++;
					nbContiguousDarker = 0;
				}

				//If darker increment the counter and set brighter pixels counter to 0
				if (valCenterPixel - deltaVal < valDeltaPixel)
				{
					nbContiguousDarker++;
					nbContiguousBrighter = 0;
				}

				//Save the pixel as a corner
				if (nbContiguousBrighter >= NB_CONTIGUOUS || nbContiguousDarker >= NB_CONTIGUOUS)
				{
					cornerPoint cp = {Point(col, row), nullptr, 0, INT_MAX};
					vecCorners.push_back(cp);

					break;
				}
			}
		}
	}

	return vecCorners;
}

Mat drawCorners(Mat pic, vector<cornerPoint> corners)
{
	//Clone image to draw on it
	Mat picWithCorners = pic.clone();

	for (unsigned int i = 0; i < corners.size(); i++)
	{
		Point corner = corners.at(i).point;

		//Draw a small circle on a corner
		circle(picWithCorners, corner, 1, Scalar(0, 0, 255), 1, 8, 0);
	}
	return picWithCorners;
}

void setPatchMeans(vector<cornerPoint> *myCorners, Mat pic)
{
	for (unsigned int i = 0; i < myCorners->size(); i++)
	{
		//Get current corner point
		Point pixel = myCorners->at(i).point;

		int meanPatch = 0;
		int nbPixel = 0;
		for (int k = -SIZE_PATCH; k <= SIZE_PATCH; k++)
		{
			if (pixel.x + k >= pic.cols || pixel.x + k < 0)
				continue;

			for (int m = -SIZE_PATCH; m <= SIZE_PATCH; m++)
			{
				if (pixel.y + m >= pic.rows || pixel.y + m < 0)
					continue;

				//To correctly calculate the mean
				++nbPixel;

				meanPatch += pic.at<uchar>(pixel.y + k, pixel.x + m);
			}
		}

		//Set the mean of the patch in the cornerPoint
		myCorners->at(i).meanPatch = (uchar)meanPatch / nbPixel;
	}
}

void setMatchs(vector<cornerPoint> *firstCorners, vector<cornerPoint> *secondCorners, Mat firstGrayscalePic, Mat secondGrayscalePic)
{
	for (unsigned int i = 0; i < firstCorners->size(); i++)
	{
		Point comparedPixel = firstCorners->at(i).point;

		int bestPixelIndex = -1;
		long int min = INT_MAX;
		long int min2 = INT_MAX;

		for (unsigned int j = 0; j < secondCorners->size(); j++)
		{
			Point pixelToCompare = secondCorners->at(j).point;

			//Looking for a 3x3 pixel around each corner
			long int SSD = 0;

			//3x3 area around both pixels
			for (int m = -SIZE_PATCH; m <= SIZE_PATCH; m++)
			{
				//To not get out of the image
				if (comparedPixel.y + m >= firstGrayscalePic.rows || comparedPixel.y + m < 0)
					continue;

				if (pixelToCompare.y + m >= secondGrayscalePic.rows || pixelToCompare.y + m < 0)
					continue;

				for (int k = -SIZE_PATCH; k <= SIZE_PATCH; k++)
				{
					//To not get out of the image
					if (comparedPixel.x + k >= firstGrayscalePic.cols || comparedPixel.x + k < 0)
						continue;

					if (pixelToCompare.x + k >= secondGrayscalePic.cols || pixelToCompare.x + k < 0)
						continue;

					//Values of each pixels of the patch
					uchar val1 = firstGrayscalePic.at<uchar>(comparedPixel.y + m, comparedPixel.x + k) - firstCorners->at(i).meanPatch;
					uchar val2 = secondGrayscalePic.at<uchar>(pixelToCompare.y + m, pixelToCompare.x + k) - secondCorners->at(j).meanPatch;

					//Add to SSD
					SSD += (val1 - val2) * (val1 - val2);
				}

				//Set minimum and second best minimum and compare to the SEUIL value to verify if best match
				if (SSD < min && SSD < firstCorners->at(i).matchedScore && SSD < SEUIL)
				{
					min2 = min;
					min = SSD;
					bestPixelIndex = j;
				}
			}
		}

		//Set the best match in the cornerPoint structure and the matched score
		if (min < firstCorners->at(i).matchedScore && bestPixelIndex != -1 && min < SEUIL && (min2 - min) > DIFFERENCE_MIN)
		{
			firstCorners->at(i).matchedScore = min;
			firstCorners->at(i).match = &secondCorners->at(bestPixelIndex);
		}
	}
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		cout << "Utilisation : ./fast pic1 pic2 output" << endl;
		return -1;
	}

	//Read pictures given in parameter
	Mat firstOriginalPic = imread(argv[1], CV_LOAD_IMAGE_COLOR);
	Mat secondOriginalPic = imread(argv[2], CV_LOAD_IMAGE_COLOR);

	//Create grayscale pictures from images given in parameter
	Mat firstGrayscalePic = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	Mat secondGrayscalePic = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

	//Check for non existent images
	if (!firstOriginalPic.data || !secondGrayscalePic.data) // Check for invalid input
	{
		cout << "Could not open or find one of the given image" << endl;
		return -1;
	}

	//Get corners for both pictures
	vector<cornerPoint> cornersFirstPic = getCorners(firstGrayscalePic);
	vector<cornerPoint> cornersSecondPic = getCorners(secondGrayscalePic);

	//Draw corners on two pictures
	Mat firstPicWithCorners = drawCorners(firstOriginalPic, cornersFirstPic);
	Mat secondPicWithCorners = drawCorners(secondOriginalPic, cornersSecondPic);

	//Create image with two pics side to side
	Mat bothOriginalPic(firstOriginalPic.rows + secondOriginalPic.rows, firstOriginalPic.cols + secondOriginalPic.cols, firstOriginalPic.type());
	hconcat(firstOriginalPic, secondOriginalPic, bothOriginalPic);

	//Create an image on which lines will be drawn
	Mat joinedCorners;
	joinedCorners = bothOriginalPic.clone();

	//Set means for patch around corners
	setPatchMeans(&cornersFirstPic, firstGrayscalePic);
	setPatchMeans(&cornersSecondPic, secondGrayscalePic);

	//Set matchs in the cornerPoint vectors
	setMatchs(&cornersFirstPic, &cornersSecondPic, firstGrayscalePic, secondGrayscalePic);
	setMatchs(&cornersSecondPic, &cornersFirstPic, secondGrayscalePic, firstGrayscalePic);

	vector<Point> matchFirstPic;
	vector<Point> matchSecondPic;

	//Married matching
	for (unsigned int i = 0; i < cornersFirstPic.size(); i++)
	{
		cornerPoint *corner = cornersFirstPic.at(i).match;

		if (corner == nullptr)
			continue;

		if (corner->match == nullptr)
			continue;

		//If the match is mutual
		if (cornersFirstPic.at(i).point == corner->match->point)
		{
			line(joinedCorners, Point(cornersFirstPic.at(i).point.x, cornersFirstPic.at(i).point.y), Point(corner->point.x + secondOriginalPic.cols, corner->point.y), Scalar(0, 0, 255), 1);

			matchFirstPic.push_back(cornersFirstPic.at(i).point);
			matchSecondPic.push_back(corner->point);
		}
	}

	imshow("Picture", joinedCorners);
	waitKey(0);

	return 0;
}
