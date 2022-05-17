/********************************************************************************************** 
* EEE4120F - YODA Project Edge Detection
* Description: Kernel Code that implements the parallelisation of the Sobel Edge Detection Filter
* Author: Theodore Psillos (PSLTHE001)
* Patners: Michael Millard (MLLMIC055) & David Da Costa (DCSDAV001)
* Supervisor: Chris Hill
* Last Date Modified: 17/05/2022
***********************************************************************************************/

__kernel void sobelEdgeDetection(__global int* width,__global int* height, __global int* pic, __global int* xConv, __global int* yConv, __global int* Size, __global int* magOutput){
	int workItemNum = get_global_id(0); //Work item ID
	int workGroupNum = get_group_id(0); //Work group ID
	int localGroupID = get_local_id(0); //Work items ID within each work group
	
	// size refers to the total size of a matrix. So for a 3x3 size = 9
	int dim = *Size;
	int numRow = *height; // only square matrices are used and as such the sqrt of size produces the row length
	int numCol = *width; // only square matrices are used and as such the sqrt of size produces the column length

	int current_row = workItemNum/numCol; // the current row is calculated by using the current workitem number divided by the total size of the matrix
	int current_col = workItemNum % numCol; // the current column is calculated by using the current workitem number modulus by the total size of the matrix

	/*if (workItemNum == dim-1)
	{ 
		printf("\nColumn size:  %i \n",col);
		printf("Row size:  %i \n",row);
		printf("Image Size:  %i \n",dim);
	} */

	// This if statement excludes all boundary pixels from the calculation as you require the neighbouring pixel cells 
	// for this calculation
	if (current_col == 0 || current_col == numCol-1 || current_row == 0 || current_row == numRow - 1){
		xConv[workItemNum] = 0;
		yConv[workItemNum] = 0;
		magOutput[workItemNum] = 0; // do not assess the bondary pixels and just set the value of the output array to zero
		//printf("Workitemnum: %i \n", workItemNum);
	}

	else{
		/****************************************************************************************************************
		* The xConv array performs the kernal convultion of the input grey scale values with the following matrix:
		*
		*     						 [-1  0 +1]
		* X - Directional Kernel  =  [-2  0 +2]
		*     						 [-1  0 +1]
		* 
		* This scans across the X direction of the image and enhances all edges in the X-direction 
		*****************************************************************************************************************/
		xConv[workItemNum] = pic[(current_row - 1)*numCol + current_col - 1]*-1 
				 + pic[(current_row)*numCol + current_col - 1]*-2 
				 + pic[(current_row + 1)*numCol + current_col - 1]*-1 
				 + pic[(current_row - 1)*numCol + current_col]*0 
				 + pic[(current_row)*numCol + current_col]*0 
				 + pic[(current_row + 1)*numCol + current_col]*0 
				 + pic[(current_row - 1)*numCol + current_col + 1]*1 
				 + pic[(current_row)*numCol + current_col + 1]*2 
				 + pic[(current_row + 1)*numCol + current_col + 1]*1;

		/****************************************************************************************************************
		* The xConv array performs the kernal convultion of the input grey scale values with the following matrix:
		*
		*     						 [+1 +2 +1]
		* Y - Directional Kernel  =  [ 0  0  0]
		*     						 [-1 -2 -1]
		* 
		* This scans across the Y direction of the image and enhances all edges in the Y-direction 
		*****************************************************************************************************************/
		yConv[workItemNum] =  pic[(current_row - 1)*numCol + current_col - 1]*1 
				 + pic[(current_row)*numCol + current_col - 1]*0 
				 + pic[(current_row + 1)*numCol + current_col - 1]*-1 
				 + pic[(current_row - 1)*numCol + current_col]*2 
				 + pic[(current_row)*numCol + current_col]*0 
				 + pic[(current_row + 1)*numCol + current_col]*-2 
				 + pic[(current_row - 1)*numCol + current_col + 1]*1 
				 + pic[(current_row)*numCol + current_col + 1]*0 
				 + pic[(current_row + 1)*numCol + current_col + 1]*-1;

		/*****************************************************************************************************************
		* Calculates the convolution matrix of the X and Y arrays. Does so by squaring each item of the X and Y arrays,  
		* adding them and taking the square root. This is the basic magnitude formula. This is done for by each workItem
		******************************************************************************************************************/
		const float xConvf = (float)xConv[workItemNum], yConvf = (float)yConv[workItemNum];
		magOutput[workItemNum] = (int)(sqrt(xConvf*xConvf + yConvf*yConvf)+0.5f);
	}	
}