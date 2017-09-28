#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace cv;

int evaluation(const char *expectedResultImagePath, const char *userImagePath)
{
    enum pixelState
    {
        TP,
        FP,
        FN
    };

    const char *userDetection = expectedResultImagePath;
    const char *realDetection = userImagePath;

    Mat userDetectionMat = imread(userImagePath, CV_LOAD_IMAGE_GRAYSCALE);
    Mat realDetectionMat = imread(expectedResultImagePath, CV_LOAD_IMAGE_GRAYSCALE);

    if (!userDetectionMat.data || !realDetectionMat.data) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    //Faster using the data pointers
    unsigned char *userDetectionDataPtr = (unsigned char *)(userDetectionMat.data);
    unsigned char *realDetectionDataPtr = (unsigned char *)(realDetectionMat.data);

    pixelState result = FN;

    int userPixel, realPixel;
    double truePositive = 0, falsePositive = 0, falseNegative = 0;

    bool found = false;
    for (int i = 0; i < userDetectionMat.cols; i++)
    {
        for (int j = 0; j < userDetectionMat.rows; j++)
        {
            //Pixel from the user matrix
            userPixel = userDetectionDataPtr[userDetectionMat.step * j + i];
            //Pixel from the expected result matrix
            realPixel = realDetectionDataPtr[realDetectionMat.step * j + i];

            //State of the observed pixel
            pixelState result = FN;

            //Only truePositive
            if (userPixel == 255 && realPixel == 255)
            {
                result = TP;
            }
            //Ignore
            else if (userPixel == 0 && realPixel == 0)
            {
                continue;
            }
            else
            {
                //Here userPixel is 255 so it can either be truePositive or falsePositive
                if (userPixel == 255)
                {
                    for (int k = 0; k < 9; k++)
                    {
                        int rowPadding = (k % 3) - 1;
                        int colPadding = (k / 3) - 1;

                        //std::cout << j << "+" << rowPadding << " x " << i << "+" << colPadding << std::endl;
                        if (colPadding == -1 && i == 0 || colPadding == 1 && i == userDetectionMat.rows - 1)
                            continue;

                        if (rowPadding == -1 && j == 0 || rowPadding == 1 && j == userDetectionMat.cols - 1)
                            continue;

                        realPixel = realDetectionDataPtr[realDetectionMat.step * (j + rowPadding) + (i + colPadding)];

                        if (realPixel == 255)
                        {
                            result = TP;
                            break;
                        }
                        else
                        {
                            result = FP;
                        }
                    }
                }

                //Here realPixel can't be 255 so it can either be truePositive or falseNegative
                else if (realPixel == 255)
                {
                    for (int k = 0; k < 9; k++)
                    {
                        int rowPadding = (k % 3) - 1;
                        int colPadding = (k / 3) - 1;

                        //std::cout << j << "+" << rowPadding << " x " << i << "+" << colPadding << std::endl;
                        if (colPadding == -1 && i == 0 || colPadding == 1 && i == userDetectionMat.rows - 1)
                            continue;

                        if (rowPadding == -1 && j == 0 || rowPadding == 1 && j == userDetectionMat.cols - 1)
                            continue;

                        userPixel = userDetectionDataPtr[userDetectionMat.step * (j + rowPadding) + (i + colPadding)];

                        if (userPixel == 255)
                        {
                            result = TP;
                            break;
                        }
                        else
                        {
                            result = FN;
                        }
                    }
                }
            }

            if (result == TP)
                truePositive += 1;
            else if (result == FP)
                falsePositive += 1;
            else if (result == FN)
                falseNegative += 1;
        }
    }

    double p = 0;
    double r = 0;

    if (truePositive != 0)
    {
        p = truePositive / (truePositive + falsePositive);
        r = truePositive / (truePositive + falseNegative);
    }

    std::cout <<  "P : " << p << std::endl
    << "R : " << r << std::endl << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cout << "Utilisation : ./evaluation image_binaire_verite_terrain image_binaire_sortie" << std::endl;
        return -1;
    }

    evaluation(argv[1], argv[2]);
    return 0;
}
