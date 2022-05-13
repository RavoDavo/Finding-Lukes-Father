/****************************************************************************************************************** 
* EEE4120F - YODA Project Edge Detection
* Description: Sobel.cpp file that implements the parallelisation of the Sobel Edge Detection Filter using OpenCL
* Author: Theodore Psillos (PSLTHE001)
* Patners: Michael Millard (MLLMIC055) & David Da Costa (DCSDAV001)
* Supervisor: Chris Hill
* Last Date Modified: 13/05/2022
*
* Commands needed to run this file:
*	g++ sobel.cpp -o sobel.out -lOpenCL ----> compiles file and creates an executable file
*	./sobel.out chess.pgm 100 35  ----> runs the executable file on the chess image for a high threshold of 100 and
*										a signam value of 1
*******************************************************************************************************************/

#include<stdio.h>
#include<CL/cl.h>
#include<iostream>
#include<fstream>
#include<string>
#include<cmath>
#include <tuple>

using namespace std;

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

	// Storing header information and copying into the new ouput images
	infile  >> buffer >> width >> height >> intensity;
	img_mag << buffer << endl << width << " " << height << endl << intensity << endl;
	img_hi  << buffer << endl << width << " " << height << endl << intensity << endl;
	img_lo  << buffer << endl << width << " " << height << endl << intensity << endl;
	img_x   << buffer << endl << width << " " << height << endl << intensity << endl;
	img_y   << buffer << endl << width << " " << height << endl << intensity << endl;

	// These matrices will hold the integer values of the input image
	int Size = width * height;
	int pic[Size];

	// Reading in the input image
	for (int i = 0; i < Size; i++){
		pic[i] = (int)infile.get();
	}

	// setting up the OpenCL
	clock_t start, end;  //Timers to for execution timing & performance
	 
	//Initialize Buffers, memory space the allows for communication between the host and the target device
	cl_mem width_buffer, height_buffer, input_buffer, xConv_buffer, yConv_buffer, size_buffer, magOutput_buffer;

	//Get the platform you want to use
	cl_uint platformCount; //keeps track of the number of platforms you have installed on your device
	cl_platform_id *platforms;
	// get platform count
	clGetPlatformIDs(5, NULL, &platformCount); //sets platformCount to the number of platforms

	// get all platforms
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, NULL); //saves a list of platforms in the platforms variable
    
	//Select the platform you would like to use in this program (change index to do this). If you would like to see all available platforms run platform.cpp.
	cl_platform_id platform = platforms[0]; 
	
	//Outputs the information of the chosen platform
	char* Info = (char*)malloc(0x1000*sizeof(char));
	clGetPlatformInfo(platform, CL_PLATFORM_NAME      , 0x1000, Info, 0);
	printf("Name      : %s\n", Info);
	clGetPlatformInfo(platform, CL_PLATFORM_VENDOR    , 0x1000, Info, 0);
	printf("Vendor    : %s\n", Info);
	clGetPlatformInfo(platform, CL_PLATFORM_VERSION   , 0x1000, Info, 0);
	printf("Version   : %s\n", Info);
	clGetPlatformInfo(platform, CL_PLATFORM_PROFILE   , 0x1000, Info, 0);
	printf("Profile   : %s\n", Info);

	// get device ID must first get platform
	cl_device_id device; //this is your deviceID
	cl_int err, err1, err2;
	
	// Access a device
	//The if statement checks to see if the chosen platform uses a GPU, if not it setups the device using the CPU
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if(err == CL_DEVICE_NOT_FOUND) {
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
	}
	printf("Device ID = %i\n",err);

	// creates a context that allows devices to send and receive kernels and transfer data
	cl_context context; //This is your contextID, the line below must just run
	context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

	// get details about the kernel.cl file in order to create it (read the kernel.cl file and place it in a buffer)
	//read file in	
	FILE *program_handle;
	program_handle = fopen("OpenCL/Kernel.cl", "r");

	//get program size
	size_t program_size;//, log_size;
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);

	//sort buffer out
	char *program_buffer;//, *program_log;
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);
  
	// create program from source because the kernel is in a separate file 'kernel.cl', therefore the compiler must run twice once on main and once on kernel
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&program_buffer, &program_size, NULL); //this compiles the kernels code

	// build the program, this compiles the source code from above for the devices that the code has to run on (ie GPU or CPU)
	cl_int err3= clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	printf("program ID = %i\n", err3);

	// creates the kernel, this creates a kernel from one of the functions in the cl_program you just built
	// select the kernel you are running
	cl_kernel kernel = clCreateKernel(program, "sobelEdgeDetection", &err);
	
	// create command queue to the target device. This is the queue that the kernels get dispatched too, to get the the desired device.
	cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, NULL);

	// create data buffers for memory management between the host and the target device
	size_t global_size = Size; //total number of work items
	size_t local_size = height; //Size of each work group
	cl_int num_groups = global_size/local_size; //number of work groups needed
	int magOutput[global_size];
	int xConv[global_size];
	int yConv[global_size];
   
	//Buffer (memory block) that both the host and target device can access 
	width_buffer = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int), &width, &err);
	height_buffer = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int), &height, &err);
	input_buffer = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,global_size*sizeof(int), &pic, &err);
	xConv_buffer = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,global_size*sizeof(int), &xConv, &err);
	yConv_buffer = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,global_size*sizeof(int), &yConv, &err);
	size_buffer = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(int), &Size, &err);
	magOutput_buffer = clCreateBuffer(context,CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,global_size*sizeof(int), &magOutput, &err);

	// create the arguments for the kernel (link these to the buffers set above, using the pointers for the respective buffers)
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &width_buffer);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &height_buffer);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &input_buffer);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &xConv_buffer);
	clSetKernelArg(kernel, 4, sizeof(cl_mem), &yConv_buffer);
	clSetKernelArg(kernel, 5, sizeof(cl_mem), &size_buffer);
	clSetKernelArg(kernel, 6, sizeof(cl_mem), &magOutput_buffer);
	
	//enqueue kernel, deploys the kernels and determines the number of work-items that should be generated to execute the kernel (global_size) and the number of work-items in each work-group (local_size).
	cl_int err4 = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL); 
	
	printf("\nKernel check: %i \n",err4);

	// Allows the host to read from the buffer object 
	err = clEnqueueReadBuffer(queue, xConv_buffer, CL_TRUE, 0, sizeof(xConv), xConv, 0, NULL, NULL);
	err1 = clEnqueueReadBuffer(queue, yConv_buffer, CL_TRUE, 0, sizeof(yConv), yConv, 0, NULL, NULL);
	err2 = clEnqueueReadBuffer(queue, magOutput_buffer, CL_TRUE, 0, sizeof(magOutput), magOutput, 0, NULL, NULL);
	//This command stops the program here until everything in the queue has been run
	clFinish(queue);

	// Once OpenCL has been used finish off the processing by normalising the magOutput array
	// Make sure all the x,y and output magnitude values are between 0-255
	int maxVal = 0; 
	int maxx = 0; 
	int maxy = 0;

	for (int j = 0; j < Size; j++)
	{
		if (xConv[j] > maxx)
		{ 
			maxx = xConv[j];
		}

		if (yConv[j] > maxy)
		{ 
			maxy = yConv[j];
		}

		if (magOutput[j] > maxy)
		{ 
			maxVal = magOutput[j];
		}
	}	

	int tempx;
	// Make sure all the magnitude values are between 0-255
	for (int z = 0; z < Size; z++)
	{
		xConv[z] = xConv[z] * 255 / maxx;
		yConv[z] = yConv[z] * 255 / maxy;
		magOutput[z] = magOutput[z] * 255 / maxVal;
	}	

	/*for (int z = 0; z < Size; z++)
	{
		printf("Input image: %i \n", magOutput[z]); 
	}*/

	printf("\nMaxx: %i \n",maxx); 
	printf("Maxy: %i \n",maxy);
	printf("MaxVal: %i \n",maxVal);  

	// Make sure to cast back to char before outputting
	// Also to avoid any wonky results, get rid of any decimals by casting to int first
	for (int i = 0; i < Size; i++){
		// Output the x image
		img_x << (char)(xConv[i]);

		// Output the y image
		img_y << (char)(yConv[i]);

		// Output the magnitude image
		img_mag << (char)(magOutput[i]);

		// Ouput the low threshold image
		if (magOutput[i] > lo){
			img_lo << (char)255;
		}

		else{
			img_lo << (char)0;
		}
		// Ouput the high threshold image
		if (magOutput[i] > hi){
			img_hi << (char)255;
		}

		else{
			img_hi << (char)0;
		}
	}
	
	// Deallocate all the OpenCL resources			
	clReleaseKernel(kernel);
	clReleaseMemObject(width_buffer);
	clReleaseMemObject(height_buffer);
	clReleaseMemObject(input_buffer);
	clReleaseMemObject(xConv_buffer);
	clReleaseMemObject(yConv_buffer);
	clReleaseMemObject(size_buffer);
	clReleaseMemObject(magOutput_buffer);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	return 0;;
}
