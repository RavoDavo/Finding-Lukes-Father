import os
filter = 0
file_name = "sobelGoldenStandard.cpp"
image_name = ["han_solo.pgm", "Leia.pgm", "Yoda.pgm", "darth_vader.pgm", "anakin.pgm", "Luke.pgm"]
os.system("g++ -Wall -o out " +file_name)

for img in image_name:
    for i in range(15):
        os.system("echo \"Image: "+img+"\" >>timing_edge_goldenMeasure.txt") 
        os.system( "./out "+img+" 100 35 >>timings_edge_goldenMeasure.txt")