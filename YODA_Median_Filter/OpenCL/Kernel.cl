///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Median Filter Kernel - EEE4120F HPES Group 10 YODA Project
// Author: Michael Millard (MLLMIC055)
// Partners: Theodore Psillos (PSLTHE001), David Da Costa (DCSDAV001)
// Supervisor: Christopher Hill
// Date: 10/05/2022
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Kernel parameters: 
// Median Filter Size, Width of Image, Height of Image, Image Pixels (Greyscale Pixels) in 1D Array, Output Array, Square Matrix of Pixels to Find Median From (Local)
__kernel void medianFilter(__global int* filterSize, __global int* width, __global int* height, __global int* pixel_array, __global int* output, __local int* size_matrix){
	

	// Work-item and work groups numbers
	int workItemNum = get_global_id(0); //WI ID
	int workGroupNum = get_group_id(0); //WG ID
	int localGroupID = get_local_id(0); //Local WI ID within WG ID

	// Variables Needed
	int filter_size = *filterSize;					// Size of the median filter (eg. size of 1 = 3x3 matrix of pixels)
	int square_side = (2*filter_size + 1);			// One side of the square matrix (eg. 3 if the matrix is 3x3)
	int square_size = square_side*square_side; 		// Number of elements in the square matrix
	int matrix_width = *width;						// Width of the actual image
	int matrix_height = *height;					// Height of the actual image
	int matrix_size = matrix_height*matrix_width;	// Number of elements in the actual image
	uint global_addr = workItemNum;					// Set global address equal to the ID of this WI

	//Row major conversion (current row and column based on global address)
    int current_row = global_addr/matrix_width;
    int current_col = global_addr%matrix_width;

	// If the pixel is on the border of the actual image, send it straight to output (don't median filter)
	// First check is if it is in the top row(s), second check is if it is in the bottom row(s), third check is if it 
	// is in left border column(s), fourth check is if it is in the right border column(s) [(s) when filter size > 1]
    if (current_row < filter_size || current_row > (matrix_height - filter_size - 1) || current_col < filter_size || current_col > (matrix_width - filter_size - 1)) {    
        output[global_addr] = pixel_array[global_addr];      
    }

	// Otherwise, perform median filtering at this pixel position
	else {
		// Private square matrix variable for each WI
		int* square_matrix[sizeof(size_matrix)];
		// First, populate the square matrix that holds the greyscale pixel values surrounding the pixel at this global address
		// eg. If the median filter size = 1, then populate a 3x3 matrix (made into a 1D matrix)
		for (int i = 0; i < square_side; i++){
			for (int j = 0; j < square_side; j++){
				int temp_row = current_row - filter_size + i;
				int temp_col = current_col - filter_size + j;
				square_matrix[square_side*i+j] = pixel_array[temp_row*matrix_width + temp_col];
			}
		}	

		// Now that the NxN array has been populated with the surrounding pixels, sort them into ascending order based on their greyscale values
		int temp_sum = 0; // Temp variable used for swapping elements where necessary
		for (int m = 0; m < square_size - 1; m++){
			for (int n = m+1; n < square_size; n++){
				if (square_matrix[m] > square_matrix[n]){
					temp_sum = square_matrix[m];
					square_matrix[m] = square_matrix[n];
					square_matrix[n] = temp_sum;
				}
				temp_sum = 0;
			}
		}

		// Since the square matrix (in 1D) has been sorted in ascending order, the median greyscale pixel value will be the element in the middle of this array
		int median = square_matrix[square_size/2]; 	// OpenCL floors the integer division (eg. 9/2 = 4, so it will return index 4 which is the middle of a 9 element array)
		output[global_addr] = median;				// Set the global array at the index of this global address to the median greyscale pixel value
	}
}
