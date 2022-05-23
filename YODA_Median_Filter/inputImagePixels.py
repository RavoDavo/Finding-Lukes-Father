# 2. Reads in noisy image, makes 1D and writes it to a text file

#########################################################################
# EEE4120F - YODA Project Image Reader
# Author: Michael Millard (MLLMIC055) 
# Partners: David Da Costa (DCSDAV001), Theodore Psillos (PSLTHE001)
# Supervisor: Christopher Hill
# Date: 09/05/2022
#########################################################################

# Imports
from PIL import Image

# Implementation

# Set the image name variable below to the image saved in the folder
image_name = "Noisy_Image/noisy_luke_skywalker.jpg" # Include file extension in name

# Read in the image
image_in = Image.open(image_name, 'r')
image_in.show() # Display the noisy image

# Obtain image dimensions (in pixels)
image_width = image_in.size[0] # Number of row elements
image_height = image_in.size[1] # Number of column elements
image_size = image_width*image_height # Total number of elements

# Obtain the pixel values and put them into a list
pixel_list = list(image_in.getdata())

# Make a string of pixel values
pixel_string = ""
for item in pixel_list:
    pixel_string += str(item) + "\n"

# Write the with, height, and pixel values to a text file to be accessed by the C++ program
with open("pixel_list.txt", 'w') as textFile:
    textFile.write(str(image_width))
    textFile.write("\n" + str(image_height))
    textFile.write("\n" + pixel_string)