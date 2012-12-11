#Project description:
This code provides an algorithm to read a block of data from a block I/O device. While the data the code was tested on was not a block device, the algorithms should be easily extendable to real devices.

##Code description:
This code reads data from an infile (special.data) and first reads in an integer followed by a string. The string is then de-Ceaser Cypher'd using the integer from 'code' and printed out. The readspecial.c file contains a read_struct function that reads in a word form the infile into the struct defined in struct.h as well as the length of the word. The main() function then prints the world, terminating when the length of the returned word is 0. This means that your input file should have appropriate use of grammar and should not have multiple 0 bytes in a row.

##Build instructions:
This code does not have a makefile because it is uncessary. Compile using: gcc readspecial.c

##Execution instructions:
The executable is run without parameters and no input is needed. The input filename must be special.data, or you must rename it in the main() function.

##Reflection on project process: 
This project threw me for a loop because I did not understand the data types as well as I needed to. It took me a long time to figure out how to correctly use fread to read in and store the data I wanted (I understood more after looking at the example code in class but still had some trouble because I had done it incorrectly the first time).
One thing that would have helped before hand was knowing that you did not cypher the \0 character (which was good, but I wasn't sure before hand).
The most interesting thing I learned was how to use fread correctly and thereby better understanding how variable types and pointers work and how to pass them correctly. This is definitely confusing and I never formally learned in-depth how this works (we only touched on it in CS II). I found this the most interesting part because I now better understand how memory works and how storage / pointers work in general. This should be a big help in the future.
One thing I still don't understand is why I am seg faulting in my main() function when I declare any int at all. It just plain doesn't work. I have no clue why.
are four bytes after the code that I just skipped to make it word. What are these? What are they for?
I have already talked about my biggest problem -- getting fread to work correctly -- and how I solved it above.
