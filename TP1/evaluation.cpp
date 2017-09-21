#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cout << "Utilisation : ./evaluation image_binaire_verite_terrain image_binaire_sortie" << std::endl;
        return -1;
    }

    const char *userDetection = argv[1];
    const char *realDetection = argv[2];

    Mat userDetectionMat = imread(userDetection, CV_LOAD_IMAGE_GRAYSCALE);
    Mat realDetectionMat = imread(realDetection, CV_LOAD_IMAGE_GRAYSCALE);

    if (!userDetectionMat.data || !realDetectionMat.data) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    //Faster using the data pointers
    unsigned char *userDetectionDataPtr = (unsigned char *)(userDetectionMat.data);
    unsigned char *realDetectionDataPtr = (unsigned char *)(realDetectionMat.data);

    int userPixel, realPixel;
    double truePositive = 0, falsePositive = 0, falseNegative = 0;

    for (int i = 0; i < userDetectionMat.cols; i++)
    {
        for (int j = 0; j < userDetectionMat.rows; j++)
        {
            userPixel = userDetectionDataPtr[userDetectionMat.step * j + i];
            realPixel = realDetectionDataPtr[realDetectionMat.step * j + i];

            //Belong to the fissure
            if (realPixel == 255)
            {
                if (userPixel == 255)
                    truePositive += 1;
                else
                    falseNegative += 1;
            }

            //Here realPixel can't be 255 so it's a false positive
            else if (userPixel == 255)
            {
                falsePositive += 1;
            }
        }
    }

    double p = 0;
    double r = 0;

    if (truePositive != 0)
    {
        p = truePositive / (truePositive + falsePositive);
        r = truePositive / (truePositive + falseNegative);
    }

    std::cout << p << std::endl
              << r << std::endl;

    return 0;
}
