#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;
using namespace cv;

enum pixelState
{
	TP,
	FP,
	FN
};

/**
 *Sobel Function 
 */
Mat sobel(Mat grayscaleInput,uchar seuil){
	//fastNlMeansDenoising(grayscaleInput, grayscaleInput, 8, 50, 10);	
	GaussianBlur(grayscaleInput, grayscaleInput, Size(7, 5), 0, 0, BORDER_DEFAULT);

	Mat output(grayscaleInput.size(), grayscaleInput.type());
	Mat gradx(grayscaleInput.size(), grayscaleInput.type());
	Mat grady(grayscaleInput.size(), grayscaleInput.type());

	Sobel(grayscaleInput, gradx, -1, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(grayscaleInput, grady, -1, 0, 1, 3, 1, 0, BORDER_DEFAULT);


	for(int i = 0; i < grayscaleInput.rows; i++){
		for(int j = 0; j < grayscaleInput.cols; j++){
			output.at<uchar>(i,j) = (uchar) round( sqrt(pow(gradx.at<uchar>(i,j), 2) + pow(grady.at<uchar>(i,j), 2)));
		}
	}

	threshold(output, output, seuil, 255, THRESH_BINARY);

	return output;
}


/**
 *laplacian Function 
 */
Mat laplacian(Mat grayscaleInput, uchar seuil){
	fastNlMeansDenoising(grayscaleInput, grayscaleInput, 8, 50, 10);	
	GaussianBlur(grayscaleInput, grayscaleInput, Size(7, 5), 0, 0, BORDER_DEFAULT);

	Mat output(grayscaleInput.size(), grayscaleInput.type());

	Mat abs_dst;

	Laplacian( grayscaleInput, output, -1, 3, 1, 0, BORDER_DEFAULT );
	convertScaleAbs( output, output );
	threshold(output, output, seuil, 255, THRESH_BINARY);

	return output;
}

/**
 *Grey scale Function 
 */
Mat seuilDeGris(Mat grayscaleInput, uchar seuil){
	Mat output(grayscaleInput.size(), grayscaleInput.type());
	threshold(grayscaleInput, output, seuil, 255, THRESH_BINARY_INV);	
	return output;
}


/**
 *Detection evaluation function Function 
 */
void evaluation( Mat userDetectionMat, Mat realDetectionMat, double& p_, double& r_)
{


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
			  
	p_ = p;
	r_ = r;
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		cout << "Utilisation : ./bestResult image_entree image_sortie image_resultat_attendue" << std::endl;
		return -1;
	}

	int seuil = 110; // seuil à fixer nous même
	const char *input = argv[1];
	const char *output = argv[2];
	const char *resultatAttendue = argv[3];



	Mat originalPic = imread(input, CV_LOAD_IMAGE_COLOR);
	Mat resultatAttenduePic = imread(resultatAttendue, CV_LOAD_IMAGE_GRAYSCALE);
	Mat grayscaleInput = imread(input, CV_LOAD_IMAGE_GRAYSCALE);
	
	Mat tmp(grayscaleInput.size(), grayscaleInput.type());
	Mat result(grayscaleInput.size(), grayscaleInput.type());
	Mat seuilgris(grayscaleInput.size(), grayscaleInput.type());
	Mat sobel_(grayscaleInput.size(), grayscaleInput.type());
	Mat laplacian_(grayscaleInput.size(), grayscaleInput.type());


	double r, p, dist = 10, distTmp = 0;
	
	if (!grayscaleInput.data || !originalPic.data || !resultatAttenduePic.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}


	for(int i = 0; i <= 255; i++){
		tmp = seuilDeGris(grayscaleInput, i);
		evaluation(tmp,resultatAttenduePic, r, p);
		std::cout << "Seuil : " << i << std::endl << "P : " << p << std::endl
		<< "R : " << r << std::endl << std::endl;
		distTmp = sqrt(pow(1-r,2) + pow(1-p, 2));
		if(distTmp < dist){
			dist = distTmp;
			seuilgris = tmp;
		}
	}
	
	imshow("seuilgris  Image", seuilgris);
	
	dist = 10;
	
	for(int i = 0; i <= 255; i++){
		tmp = sobel(grayscaleInput, i);
		evaluation(tmp,resultatAttenduePic, r, p);
		std::cout << "Seuil : " << i << std::endl << "P : " << p << std::endl
		<< "R : " << r << std::endl << std::endl;

		distTmp = sqrt(pow(1-r,2) + pow(1-p, 2));
		if(distTmp < dist){
			dist = distTmp;
			sobel_ = tmp;
		}
	}
	imshow("sobel  Image", sobel_);
	
	dist = 10;
	
	for(int i = 0; i <= 255; i++){
		tmp = laplacian(grayscaleInput, i);
		evaluation(tmp,resultatAttenduePic, r, p);
		std::cout << "Seuil : " << i << std::endl << "P : " << p << std::endl
		<< "R : " << r << std::endl << std::endl;

		distTmp = sqrt(pow(1-r,2) + pow(1-p, 2));
		if(distTmp < dist){
			dist = distTmp;
			laplacian_ = tmp;
		}
	}
	imshow("laplacian  Image", laplacian_);

	
	for(int i = 0; i < grayscaleInput.rows; i++){
		for(int j = 0; j < grayscaleInput.cols; j++){
			if(seuilgris.at<uchar>(i,j) == sobel_.at<uchar>(i,j) && sobel_.at<uchar>(i,j) == laplacian_.at<uchar>(i,j)){
				result.at<uchar>(i,j) = seuilgris.at<uchar>(i,j);
			}
		}
	}

	imshow("Result  Image", result);

	waitKey(0);
	return 0;
}
