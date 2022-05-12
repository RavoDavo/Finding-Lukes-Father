

//Import relevant libraries
#include <math.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
using namespace std;

vector<int> seq_medFilter( vector<int>  image , int rows, int cols , int filter)
/*
This function median filters a 1D greyscale image array with a median filter window size 3X3

Input Arguments:
    - vector<int> Image
        This is a 1D vector which contains the original image greyscale value converted to 1D using row major conversion
        i.e.
        [
        [1, 2, 3],
        [4, 5, 6], -------> [1, 2, 3, 4, 5, 6, 7, 8, 9]
        [7, 8, 9]
        ]

        Row major conversion from 2D array to 1D array conversion:
            
            Image size = RowsXColomns
            2D_Array[R,C] ==> 1D_Array[R*Colomns + C]

        Inverse row major conversion from 1D array to 2D array:

            1D_Array[i] ==> 2D_Array[i/Colomns , i%Colomns]

    - int width
        Integer value of the image's total width i.e. colomns
    - int height
        Integer value of the image's total height i.e. rows
    - int filter
        Integer holding the size of the filter window 
        i.e. a filter size 1 takes all immediate neghours (3X3 window)
             a filter size 2 takes closest 2 negbours i.e. (5X5 window)
             
Output:
    - vector<int> output
        A 1D vecor holding the filtered images greyscale values 

Filtering process:
    The relevant neigbourhood of pixels is placed in a tempory vector.
    The vector is then sorted
    The middle value corresponding to the median value is then placed in the output array at the corresponding pixel's index
*/
    {   
        //Set size of output vector to image size
        vector<int> output( rows * cols );
        
        int filter_window = (2*filter+1)*(2*filter+1); 
        for (int i = 0; i<rows*cols; ++i)
       
        {
            //Set up temporary vector used to find the median value of all neighbouring pixels
            vector<int> temp(filter_window );

            //Row major conversion
            int current_row = i/cols;
            int current_col = i%cols;
          
            //Assess for edge cases 
            if (current_col < filter || current_col > cols-filter || current_row < filter || current_row > rows - filter)
            {    
                //Cannot filter edge cases, set output pixel = input pixel 
                output[i] = image[i];      
            }
            else 
            
            //-------Interestingly the program works without edge case assesment and provides a better output therefore edge cases are ignored-------
            {

                
                //Set temporary row, colomn used to access neighbouring pixels
                int new_row = current_row - filter;
                int new_col = current_col - filter;

                //iterate through neighbouring pixels
                for( int x=0 ; x< filter_window  ; x++)
                {        
                    
                    //populate temp 1D array with inverse row major values 
                    temp[x] = image[new_row*cols + new_col];
                    new_col++;

                    //Increment row and decrement colomn by 3 if colomn value goes beyond neighbourhood
                    if (new_col == current_col+filter+1)
                    {
                        new_col=current_col-filter;
                        new_row++;
                       
                    }       
                }   
                //Sort the tem vector and set the output to the median pixel 
                int N = temp.size(); 
                sort(temp.begin(), temp.end());
                output[i] = temp[N/2];
                temp.clear();
                
            }

        }

    return output;
    }
int main()
{
    //set dimensions of image
    int height = 383;
    int width = 260;
    int size = width*height;
 
    //read the input txt file containing greyscale image pixels 
    ifstream file { "Noisy_Image.txt" };
    if (!file.is_open()) return -1;
    
    vector<int> yoda( size);

    //set the vector values to pixel values
    for (int i{}; i != size; i++) file >> yoda[i];
    
    vector<int> filtered_img( size );

    //input the vector of pixel values to median filter
    filtered_img = seq_medFilter( yoda ,height, width , 1);
    
    //output median filtered vector to txt file
    ofstream myfile ("Filtered_Image.txt");
    if (myfile.is_open())
    {
        for(int count = 0; count < size; count ++)  myfile << filtered_img[count]<<"\n";
                  
        myfile.close();
    }
  else cout << "Unable to open file";
  return 0;

}



