# 1. START HERE
# Reads in the original image, converts it to greyscale and adds noise to it

from PIL import Image 
import numpy as np
import random
from math import ceil
     
# Function which can be used to add noise to an image in the form of salt and pepper noise

def add_noise(img, size):
    # Obtain image dimensions
    row , col = img.shape
    # Add a random number - between 10% and 15% of the image size - of black and white (salt and pepper) pixels to the image
    number_of_pixels = random.randint(ceil(0.10*size), ceil(0.15*size))
    
    for i in range(number_of_pixels):
       
        # Set random pixels to white
        y_coord=random.randint(0, row - 1)
        x_coord=random.randint(0, col - 1)
        img[y_coord][x_coord] = 255
         
    for i in range(number_of_pixels):
       
        # Set random pixels to black
        y_coord=random.randint(0, row - 1)
        x_coord=random.randint(0, col - 1)
        img[y_coord][x_coord] = 0
         
    return img
 

# Open original image and create array
orig_img_arr = np.array(Image.open('Input_Images/luke_skywalker.jpg').convert('L'), dtype = np.uint8)

# Set the dimensions of the noisy image to be the same as those of the original image
height, width = orig_img_arr.shape
size = width*height
noisy_img = add_noise(orig_img_arr, size)
noisy_img = Image.fromarray(noisy_img)
noisy_img.save("Noisy_Images/noisy_luke_skywalker.jpg")
