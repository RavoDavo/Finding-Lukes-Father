# Automating the compilation of the medianFilter.cpp program and running it 10 times for
# each median filter size (sizes 1 [3x3], 2 [5x5], 3 [7x7]).

import os

filter = 0
file_name = "medianFilter"
command1 = "g++ " + file_name + ".cpp -o " + file_name + ".out -lOpenCL"
# Run the compile command for medianFilter.cpp
os.system(command1)

# First for loop sets the median filter size
for i in range(3):
    filter = filter + 1
    # Write the command below to the text file timings.txt
    os.system("echo \"Filter Size: " + str(filter) + "\" >> timings.txt")
    # Second for loop runs the program multipe times for cache warming and accuracy (same filter size)
    for j in range(10):
        command2 = "./" + file_name + ".out " + str(filter) + " >> timings.txt"
        # Run the command to execute the output file and set the median filter size for that run
        os.system(command2)
        # Fancy print statements to follow in the terminal output
        print("Filter Size: {0}, Run Number: {1} ... Completed.\n".format(str(filter), str(j)))

# Finito
print("All done, goodbye.")


