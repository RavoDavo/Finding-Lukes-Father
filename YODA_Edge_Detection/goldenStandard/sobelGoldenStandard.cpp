/******************************************************************************************************************** 
* EEE4120F - YODA Project Edge Detection
* Description: Sobel.cpp file that implements the parallelisation of the Sobel Edge Detection Filter using OpenCL
* Author: Theodore Psillos (PSLTHE001)
* Patners: Michael Millard (MLLMIC055) & David Da Costa (DCSDAV001)
* Supervisor: Chris Hill
* Last Date Modified: 17/05/2022
*
* Original Author: Soliman Alnaizy
* Reference: https://github.com/sorazy/canny/blob/master/sobel.cpp
*
* Commands needed to run this file:
*	g++ sobelGoldenStandard.cpp -o sobelGoldenStandard.out ----> compiles file and creates an executable file
*	./sobelGoldenStandard.out anakin.pgm 100 35 ----> runs the executable file on the anakin image for a high threshold 
*													 of 100 and a low threshold value of 35
*********************************************************************************************************************/

#include <cmath>
#include <fstream>
#include <iostream>

using namespace std;

int maskx[3][3] = {{-1,0,1}, {-2,0,2}, {-1,0,1}};
int masky[3][3] = {{1,2,1}, {0,0,0}, {-1,-2,-1}};

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		cout << "Proper syntax: ./a.out <input_filename> <high_threshold> <low_threshold>" << endl;
		return 0;
	}

	// Exit program if file doesn't open
	string filename(argv[1]);
	string path = "./input_images/" + filename;
	ifstream infile(path, ios::binary);
	if (!infile.is_open())
	{
		cout << "File " << path << " not found in directory." << endl;
		return 0;
	}	

	ofstream img_mag("./output_images/sobel_mag.pgm", ios::binary);
	ofstream img_hi("./output_images/sobel_hi.pgm", ios::binary);
	ofstream img_lo("./output_images/sobel_lo.pgm", ios::binary);
	ofstream img_x("./output_images/sobel_x.pgm", ios::binary);
	ofstream img_y("./output_images/sobel_y.pgm", ios::binary);

	char buffer[1024];
	int width, height, intensity, hi = stoi(argv[2]), lo = stoi(argv[3]);
	int sumx, sumy;

	clock_t start, end;  //Timers to for execution timing & performance

	// Storing header information and copying into the new ouput images
	infile  >> buffer >> width >> height >> intensity;
	img_mag << buffer << endl << width << " " << height << endl << intensity << endl;
	img_hi  << buffer << endl << width << " " << height << endl << intensity << endl;
	img_lo  << buffer << endl << width << " " << height << endl << intensity << endl;
	img_x   << buffer << endl << width << " " << height << endl << intensity << endl;
	img_y   << buffer << endl << width << " " << height << endl << intensity << endl;

	// These matrices will hold the integer values of the input image
	double pic[height][width], x[height][width], y[height][width], mag[height][width];

	// Reading in the input image
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			pic[i][j] = (int)infile.get();

	// mr stands for maskRadius
	// This does the scanning convultion of the masks declared above, and stores
	// the values in the matrices outputx and outputy
	start = clock();
	int mr = 1;
	int maxx = 0, maxy = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			sumx = 0;
			sumy = 0;

			for (int p = -mr; p <= mr; p++)
			{
				for (int q = -mr; q <= mr; q++)
				{
					// To make sure we don't go out of bounds
					if ((i + p) < 0 || (j + q) < 0 || (i + p) >= height || (j + q) >= width)
						continue;

					sumx += pic[i+p][j+q] * maskx[p+mr][q+mr];
					sumy += pic[i+p][j+q] * masky[p+mr][q+mr];
				}
			}

			if (sumx > maxx)
				maxx = sumx;
			if (sumy > maxy)
				maxy = sumy;

			x[i][j] = sumx;
			y[i][j] = sumy;
		}
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			x[i][j] = x[i][j] / maxx * 255;
			y[i][j] = y[i][j] / maxy * 255;
		}
	}

	// Find gradient and maxval
	int maxVal = 0;
	for (int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			mag[i][j] = sqrt((x[i][j] * x[i][j]) + (y[i][j] * y[i][j]));

			if (mag[i][j] > maxVal)
				maxVal = mag[i][j];
		}
	}

	// Make sure all the magnitude values are between 0-255
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			mag[i][j] = mag[i][j] / maxVal * 255;

	// Make sure to cast back to char before outputting
	// Also to avoid any wonky results, get rid of any decimals by casting to int first
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// Output the x image
			img_x << (char)((int)x[i][j]);

			// Output the y image
			img_y << (char)((int)y[i][j]);

			// Output the magnitude image
			img_mag << (char)((int)mag[i][j]);

			// Ouput the low threshold image
			if (mag[i][j] > lo)
				img_lo << (char)255;
			else
				img_lo << (char)0;

			// Ouput the high threshold image
			if (mag[i][j] > hi)
				img_hi << (char)255;
			else
				img_hi << (char)0;
		}
	}
	
	end = clock();
	double time_ex = ((float) end - start)/CLOCKS_PER_SEC;
	printf(" %f \n", time_ex);

	return 0;
}