# 4. Saves and displays the resulting median filtered image

from PIL import Image 
import numpy as np
import cv2

# Read in the original pixel list to get the height and width parameters
with open("pixel_list.txt", 'r') as whFile:
    line = whFile.readline()
    width = int(line)
    while (line):
        line = whFile.readline()
        height = int(line)
        break       # Break the loop once h and w have been found

# Open the text file containing the median filtered image pixels
with open('pixels_out_list_par.txt') as f: # For sequential, text file is: 'pixels_out_list_seq.txt'
     pixels = f.readlines()

# Create a 2D array of dimensions (height, width) containing the pixel values 
filtered_image_2D = np.array(pixels, dtype=(np.uint8)).reshape(height, width)

# Save and display the median filtered image
img = Image.fromarray(filtered_image_2D)
cv2.imwrite("Output_Images/median_filtered_luke.jpg", filtered_image_2D) # Save the image as a jpg
img.show() # Display the image

# Checking the differences in pixel values returned by the sequential and parallel algorithms
# File comparison method (checks for errors in each line of output file by comparing to input file)
def file_comparison(seqFile, parFile):
    seq_file = open(seqFile, "r")  
    par_file = open(parFile, "r")  
    
    line_error_count = 0

    # The substring [:-1] is used to remove the newline character ('\n')
    seq_line = seq_file.readline()[:-1]
    par_line = par_file.readline()[:-1]

    if (seq_line != par_line):
        line_error_count += 1

    while (seq_line):
        seq_line = seq_file.readline()[:-1]
        par_line = par_file.readline()[:-1]
        if (seq_line != par_line):
            line_error_count += 1

    seq_file.close()                                       
    par_file.close()

    return line_error_count

error_count = file_comparison("pixels_out_list_seq.txt", "pixels_out_list_par.txt")
print("Errors found between the parallel and sequental median filter files: {} errors.".format(error_count))
print("Therefore, the algorithms were " + str(100*(1 - error_count/(width*height))) + "%" + " consistent with each other.")