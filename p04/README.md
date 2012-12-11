#Project description 
This code re-implements line buffering without using setvbuff with the _IOLBF setting.

##Code description
This code takes a string entered in the main function into the function bprintf which then buffers the string(s), printing the buffer when the buffer is full or when a new line is reached.

##Build instructions 
bprinf.c can be compiled using gcc.

##Execution instructions 
./a.out or however you named it in gcc -o

##Reflection
The in class portion of this project was the most interesting. It's always fun to learn about new implementation and buffering is cool. Although now that I've edited it I like how I used all the string manipulation. It's kinda cool.
I don't understand / do not know if there is another way of implementing this without using global variables (or at least without using the current position of the buffer array as a global variable). Otherwise I understand it all.
The largest problem I faced was getting the string manipulation correct. It took me a minute to remember that I did not need to use & in the first argument of fwrite because it's already a pointer.
I would tell the next person how you want it implemented, with string like I did or with ints like I did before. I could have done it with just one int but I thought I'd show you I could do it without any, and I think this is a unique and interesting way of implementing this project.
