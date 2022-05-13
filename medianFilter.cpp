/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Median Filter C++ OpenCL Code - EEE4120F HPES Group 10 YODA Project
// Author: Michael Millard (MLLMIC055)
// Partners: Theodore Psillos (PSLTHE001), David Da Costa (DCSDAV001)
// Supervisor: Christopher Hill
// Date: 10/05/2022
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include<stdio.h>
#include<CL/cl.h>
#include<iostream>
#include<fstream>
#include<string>
#include<cmath>
#include<tuple>

using namespace std;

// Main Function
int main(void)
{
	// Parameters needed
	int filterSize = 2; 						// Median filter size (eg. Filter size of 1 corresponds to a 3x3 matrix of surrounding pixels)
	int squareSide = (2*filterSize + 1);		// One side of the square matrix (eg. A side is 3 if the matrix is 3x3)
	int squareSize = squareSide*squareSide;		// The number of elements in the square matrix (eg. 3x3 = 9 elements)

	// Image matrix parameters
	int height; 		// Number of rows in the actual image
	int width;			// Number of columns in the actual image
	int image_size;		// Number of total elements in the actual image

	// Read in the summed RGB pixel values from text file produced by Python code
	fstream textFile;
	string image_data;

	textFile.open("pixel_list.txt",ios::in); 		// pixel_list.txt is the name of the text file
   	if (textFile.is_open()){ 						// Check if the file opened correctly
	   	string line_data; 							// Create a string for each line
		int i = 0;
      	while(getline(textFile, line_data)){ 		// Read in the data one line at a time
		  	if (i == 0){
			  	width = stoi(line_data); 			// The first line is only the height
				i++;
			}
			else if (i == 1){
				height = stoi(line_data); 			// The second line is only the width
				i++;
			}
			else {
         		image_data += line_data; 			// The rest of the lines contain summed RGB pixel info
			}
      	}
      	textFile.close(); 							// Close the file
   	}
	
	image_size = height*width; // Total number of pixel elements

	int pixel_array[image_size]; 		// Create a 1D summed RGB pixel array
	string delimiter = " "; 			// Delimiter used to seperate pixels in the Python script was whitespace
	string item; 						// Individual summed RGB pixel values to be added to the pixel array
	
	// Integer variables needed to populate array
	int next_delimiter_index = 0;
	int x = 0;
	int y = 0;
	int image_length = image_data.length();

	// While loop used to populate pixel array
	while (x < image_length){
		next_delimiter_index = image_data.find(delimiter);					// Location of the next instance of whitespace
		item = image_data.substr(0, next_delimiter_index);					// This pixel item is everything up until that whitespace
		image_data = image_data.substr(next_delimiter_index + 1, - 1); 		// Make image_data a substring of itself for the next iteration
		x += item.length() + 1; 											// Increment x to be at the start of the new truncated image_data string
		pixel_array[y] = stoi(item);										// Add the current pixel item to the pixel array at the current index y
		y++;																// Increment the index variable y
	}
	 
	 
	// Initialize the buffers needed:
	// size_buffer is for the median filter size, height_buffer is for the image height, width_buffer is for the image width,
	// matrix_buffer is for the array of summed RGB pixel values (entire 1D array), output_buffer is for the output median filtered pixel array
	cl_mem size_buffer, width_buffer, height_buffer, matrix_buffer, output_buffer;
	
    
	cl_uint platformCount; // Number of platforms installed on ythe device
	cl_platform_id *platforms;
	clGetPlatformIDs(5, NULL, &platformCount); // Sets platformCount to the number of platforms

	// All platforms
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, NULL); // List of platforms
    

	cl_platform_id platform = platforms[0]; // Choosing the platform to use

	// Outputs the information of the chosen platform
	char* Info = (char*)malloc(0x1000*sizeof(char));
	clGetPlatformInfo(platform, CL_PLATFORM_NAME      , 0x1000, Info, 0);
	printf("Name      : %s\n", Info);
	clGetPlatformInfo(platform, CL_PLATFORM_VENDOR    , 0x1000, Info, 0);
	printf("Vendor    : %s\n", Info);
	clGetPlatformInfo(platform, CL_PLATFORM_VERSION   , 0x1000, Info, 0);
	printf("Version   : %s\n", Info);
	clGetPlatformInfo(platform, CL_PLATFORM_PROFILE   , 0x1000, Info, 0);
	printf("Profile   : %s\n", Info);
	
	// Device ID:
	cl_device_id device;
	cl_int err;
	
	// If there is a GPU available, choose it. Otherwise choose the CPU as the target device
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if(err == CL_DEVICE_NOT_FOUND) {
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
	}

	// Device ID:
	printf("Device ID = %i\n",err); 


	// Create the context
	cl_context context; 
	context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

	FILE *program_handle;
	program_handle = fopen("OpenCL/Kernel.cl", "r");

	size_t program_size;
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);

	char *program_buffer;
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);
   	
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&program_buffer, &program_size, NULL); 
	cl_int err3= clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

	// Program ID:
	printf("program ID = %i\n", err3); 

	// Select the median filter kernel to run
	cl_kernel kernel = clCreateKernel(program, "medianFilter", &err);

	// Create the queue
	cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, NULL);

	//------------------------------------------------------------------------

	// Chosen size parameters: 
	// Global size (number of WIs), one WI per pixel position (each computes one median filtering operation)
	// Local size (number of WIs per WG): The GPU is an Nvidia GeForce GT 1030 which has 384 CUDA cores
	//		Therefore: Ideally we want to send 1 WG to each core, meaning 384 WGs would be the ideal number, which means the maximum number number of 
	//		WIs per WG we can achieve is global_size/384. Since the number of required WIs won't always be divisible by 384 (and it is a requirement
	//		that the number of WIs be perfectly divisible by the number of WIs per WG), we will subtract 1 from 384 until the global size is perfectly
	//		divisible by the local size. This will give us the highest possible number of WIs per WG and hence achieve the highest possible speed up.

	int gpu_core_count = 384; 							// For the Nvidia GeForce GT 1030
	size_t global_size = image_size; 					// Total number of WIs
	while (global_size%gpu_core_count != 0){			// While loop to find the optimal WI per WG number
		gpu_core_count--;
	}
	size_t local_size = global_size/gpu_core_count;		// Number of WIs per WG
	cl_int num_groups = global_size/local_size; 		// Number of WGs needed
	
	// Initialize the output median filtered pixel array
	int output[global_size];
	
	// Creating the buffers:
	size_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &filterSize, &err);
	width_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &width, &err);
	height_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int), &height, &err);
	matrix_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY| CL_MEM_COPY_HOST_PTR, global_size*sizeof(int), &pixel_array, &err);
	output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, global_size*sizeof(int), output, &err);

	// Creating the kernel arguments:
	// Note, this includes the local kernel square matrix parameter (6th kernal argument) used for creating the surrounding NxN matrix to find the median pixel from
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &size_buffer);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &width_buffer);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &height_buffer);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &matrix_buffer);
	clSetKernelArg(kernel, 4, sizeof(cl_mem), &output_buffer);
	clSetKernelArg(kernel, 5, squareSize*sizeof(int), NULL);

	// Enqueue the kernel:
	cl_int err4 = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL); 
	
	// Kernel check:
	printf("\nKernel check: %i \n",err4); 

	// Read from the output buffer:
	err = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, sizeof(output), output, 0, NULL, NULL);
	
	
	// Stop the program until the queue is empty
	clFinish(queue);
	

	// Added line of code to show info on size, work-items and work-groups
	printf("\nSize of Matrices: %ix%i\nNumber of work-items used: %i\nNumber of work-groups used: %i\nNumber of work items per work-group: %i\n", 
			squareSide, squareSide, int(global_size), int(num_groups), int(local_size));

	// Write the contents of the output median filtered pixel value array to a text file to be read in by a Python script that displays the image
	string pixel_stream = "";
	for (int h = 0; h < global_size; h++){
		pixel_stream += to_string(output[h]) + " ";
	}
	ofstream output_pixels;
  	output_pixels.open ("pixels_out_list.txt"); 	// Call the text file pixels_out_list
  	output_pixels << pixel_stream;
  	output_pixels.close();
	return 0;
	
	// Deallocate resources	
	clReleaseKernel(kernel);
	clReleaseMemObject(size_buffer);
	clReleaseMemObject(width_buffer);
	clReleaseMemObject(height_buffer);
	clReleaseMemObject(matrix_buffer);
	clReleaseMemObject(output_buffer);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);

	return 0;
}