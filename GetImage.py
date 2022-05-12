# -*- coding: utf-8 -*-
"""
Created on Mon May  9 21:00:28 2022

@author: david
"""
from PIL import Image 
from numpy import* 
import numpy as np
import PIL
import random
import cv2
     






#Function which can be used to add noise to an image in the form of salt and pepper noise

def add_noise(img):
    #obtain image dimensions
    row , col = img.shape
    #set number of white pixels to a random number (salt)
    number_of_pixels = random.randint(100, 400)
    
    for i in range(number_of_pixels):
       
        #set random pixels to white
        y_coord=random.randint(0, row - 1)
        x_coord=random.randint(0, col - 1)
        img[y_coord][x_coord] = 255
         
    for i in range(number_of_pixels):
       
        #set random pixels to black
        y_coord=random.randint(0, row - 1)
        x_coord=random.randint(0, col - 1)
        img[y_coord][x_coord] = 0
         
    return img
 



#open original image and create array
orig_img_arr = np.asarray(PIL.Image.open('Leah.jpg').convert('L'), dtype = np.uint8)

#set dimensions based on original image
width,height =orig_img_arr.shape
size = width*height
noisy_img = add_noise(orig_img_arr)

#create 1D array of image
noisy_1D = orig_img_arr.reshape(size,1)

#save 1D array to a text file called image_array
np.savetxt("Noisy_Image.txt", noisy_1D,fmt="%s")

#open the text file containing median filtered image
with open('Filtered_Image.txt') as f:
     pixels = f.readlines()

#create 2D array of size width,height containing pixel values 
filtered_image_2D = np.array(pixels,dtype=(np.uint8)).reshape(width,height)

#Display median filtered image
img = Image.fromarray(filtered_image_2D)
img.show()
