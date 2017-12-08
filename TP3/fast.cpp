#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>

using namespace std;
using namespace cv;

struct cornerPoint
{
	Point point;
	bool matched;
};

vector<cornerPoint> getCorners(Mat grayscalePic)
{
	//Delta for a 3x3 circle around the point of interest
	const vector<int> vecDeltaX = {0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1};
	const vector<int> vecDeltaY = {3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1, 0, 1, 2, 3};
	vector<cornerPoint> vecCorners;

	for (int col = 0; col < grayscalePic.cols; col++)
	{
		for (int row = 0; row < grayscalePic.rows; row++)
		{
			//Value of the point of interest
			uchar valCenterPixel = grayscalePic.at<uchar>(row, col);

			//Value of the delta of the compared pixel from the point of interest
			int deltaX;
			int deltaY;

			//Delta to be brighter/darker than the point of interest
			int deltaVal = 15;

			//Number of contiguous brighter pixels
			int nbContiguousBrighter = 0;

			//Number of contiguous darker pixels
			int nbContiguousDarker = 0;

			//Number of needed contiguous pixels to be a corner
			const int neededContiguous = 12;

			//Looking for a circle and a half around the point of interest
			for (unsigned int k = 0; k < vecDeltaX.size() + 8; k++)
			{
				//Position of the compared pixel
				deltaX = vecDeltaX.at(k % vecDeltaX.size());
				deltaY = vecDeltaY.at(k % vecDeltaX.size());

				//If the pixel is outside the image
				if (row + deltaX < 0 || row + deltaX > grayscalePic.rows)
					continue;

				if (col + deltaY < 0 || col + deltaY > grayscalePic.cols)
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
				if (nbContiguousBrighter >= neededContiguous || nbContiguousDarker >= neededContiguous)
				{
					cornerPoint cp = {Point(col, row), false};
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
	Mat picWithCorners = pic.clone();
	for (unsigned int i = 0; i < corners.size(); i++)
	{
		Point corner = corners.at(i).point;
		circle(picWithCorners, corner, 1, Scalar(0, 0, 255), 1, 8, 0);
	}
	return picWithCorners;
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		cout << "Utilisation : ./fast pic1 pic2 output" << endl;
		return -1;
	}

	//const char *output = argv[3];

	vector<Mat> originalPictures;
	vector<Mat> picWithCircles;
	vector<Mat> grayscalePictures;

	Mat firstOriginalPic = imread(argv[1], CV_LOAD_IMAGE_COLOR);
	Mat secondOriginalPic = imread(argv[2], CV_LOAD_IMAGE_COLOR);

	Mat firstGrayscalePic = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	Mat secondGrayscalePic = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

	for (unsigned int i = 0; i < originalPictures.size(); i++)
	{
		if (!originalPictures.at(i).data || !grayscalePictures.at(i).data) // Check for invalid input
		{
			cout << "Could not open or find one of the given image" << endl;
			return -1;
		}
	}

	vector<cornerPoint> cornersFirstPic = getCorners(firstGrayscalePic);
	vector<cornerPoint> cornersSecondPic = getCorners(secondGrayscalePic);

	Mat firstPicWithCorners = drawCorners(firstOriginalPic, cornersFirstPic);
	Mat secondPicWithCorners = drawCorners(secondOriginalPic, cornersSecondPic);

	//imshow("first", firstPicWithCorners);
	//imshow("second", secondPicWithCorners);

	//waitKey(0);

	Mat bothOriginalPic(firstOriginalPic.rows + secondOriginalPic.rows, firstOriginalPic.cols + secondOriginalPic.cols, firstOriginalPic.type());
	hconcat(firstOriginalPic, secondOriginalPic, bothOriginalPic);

	Mat joinedCorners;
	joinedCorners = bothOriginalPic.clone();

	vector<cornerPoint> salut;

	int c = 0;
	for (unsigned int i = 0; i < cornersFirstPic.size(); i++)
	{
		Point comparedPixel = cornersFirstPic.at(i).point;

		int bestPixelIndex = 0;
		long int min = INT_MAX;

		for (unsigned int j = 0; j < cornersSecondPic.size(); j++)
		{
			Point pixelToCompare = cornersSecondPic.at(j).point;

			//Looking for a 3x3 pixel around each corner
			long int SSD = 0;

			//3x3 area around both pixels
			for (int k = -1; k <= 1; k++)
			{
				if (comparedPixel.x + k >= firstGrayscalePic.rows || comparedPixel.x + k < 0)
					continue;

				if (pixelToCompare.x + k >= secondGrayscalePic.rows || pixelToCompare.x + k < 0)
					continue;

				for (int m = -1; m <= 1; m++)
				{
					if (comparedPixel.y + m >= firstGrayscalePic.cols || comparedPixel.y + m < 0)
						continue;

					if (pixelToCompare.y + m >= secondGrayscalePic.cols || pixelToCompare.y + m < 0)
						continue;

					if (k == 0 && m == 0)
						continue;

					uchar val1 = firstGrayscalePic.at<uchar>(comparedPixel.x + k, comparedPixel.y + m);
					uchar val2 = secondGrayscalePic.at<uchar>(pixelToCompare.x + k, pixelToCompare.y + m);

					SSD += (val1 - val2) * (val1 - val2);

					/*cout << "Compared pixel : " << comparedPixel.x << "+" << k << "x" << comparedPixel.y << "+" << m << "| "
						 << "PixelToCompare : " << pixelToCompare.x << "+" << k << "x" << comparedPixel.y << "+" << m << "| "
						 << "SSD : " << SSD << endl;*/
				}

				/*if (SSD == 0)
				{
					salut.push_back(cornersSecondPic.at(j));
				}*/

				if (SSD < min && !cornersSecondPic.at(j).matched)
				{
					min = SSD;
					bestPixelIndex = j;
				}
			}
		}

		//cout << salut.size() << endl;

		if (!cornersSecondPic.at(bestPixelIndex).matched)
		{
			cout << min << endl;
			/*cout << "min : " << min << endl;
			cout << "compared " << comparedPixel.x << " " << comparedPixel.y << endl;
			cout << "matched " << cornersSecondPic.at(bestPixelIndex).point.x << " " << cornersSecondPic.at(bestPixelIndex).point.y << endl;*/
			cornersSecondPic.at(bestPixelIndex).matched = true;

			Mat test;
			line(joinedCorners, Point(comparedPixel.x, comparedPixel.y), Point(cornersSecondPic.at(bestPixelIndex).point.x + secondOriginalPic.cols, cornersSecondPic.at(bestPixelIndex).point.y), Scalar(0, 0, 255), 1);
			//imshow("tamere", test);
			//waitKey(0);
		}
	}

	imshow("Picture", joinedCorners);
	waitKey(0);
	// for (unsigned int i = 0; i < originalPictures.size(); i++)
	// {
	// 	imshow("Picture", originalPictures.at(i));
	// 	waitKey(0);
	// }

	waitKey(0);

	return 0;
}
