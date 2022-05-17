import os
filter = 0
file_name = "sobel.cpp"
image_name = ["Yoda.pgm", "darth_vader.pgm", "anakin.pgm", "Luke.pgm", "han_solo.pgm", "Leia.pgm"]
os.system("g++ -Wall -o out " +file_name+" -lOpenCL")

for img in image_name:
    for i in range(15):
        os.system("echo \"Image: "+img+"\" >>timing_edge.txt") 
        os.system( "./out "+img+" 100 35 >>timings_edge.txt")