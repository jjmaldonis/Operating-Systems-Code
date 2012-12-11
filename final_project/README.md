# Project description

Use a file as a virtual hard drive to implement filesystem functionality. Include commands: ls, mkdir, cd, cat (to display the contents of a file), echo (to print to screen or to a file using .>.), and rm. I have also implemented a cwd command and a .printbm. (aka .bitmap.) command that prints the bitmap of the filesystem. Lastly, you can type .exit. to exit the filesystem interaction interface.
 
# Filesystem Implementation

## Filesystem Design Information

The most important thing to note when debugging / looking at my code is how I set up my filesystem. This is partly done in formatfs.c / .h. First, I set the base block size to 512 bytes (you can change this) for both files and directories. This is to keep things simple. Secondly, a bitmap is stored at the beginning of the filesystem with each bit (8 in a byte) representing a block. A 0 means the block is unused, a 1 means it is in use. You can view the bitmap by typing .bitmap. or .printbm. into the interactive interface. Next, I have stored my root directory immediately after the bitmap. In the root directorys header (not in order) is its size (number of children * 2), creation date (i.e. creation date of the filesystem), a parent address set to -1, its successor address (if it gets larger than the base block size, otherwise set to 0), its type ("d" for directory in this case, as opposed to "f" for file), and its name ("root"). This block header applies to ALL files and directories to keep things standard. The total header size is 20 bytes and is outlined below in order.
Name (8 bytes)
Parent address (2 bytes)
Timestamp in seconds (4 bytes)
Size (2 bytes)
Successor address (2 bytes)
Type (1 byte)
This makes a total of 19 bytes, and thus one byte is skipped before writing to the file/directory, making a total of 20 bytes in each header.
Knowing this information is essential to understanding how I fseek and fread/fwrite within the filesystem.

You can change the filesystem size or block size in formatfs.h.

In addition, you should note that each command has its own .c file (for the most part, and with a couple extras). These are #included in fs.c. There is also a fild_child.c file/function to reduce code duplication.

Now, on to the commands.

## Commands and the other .c files

The ls command outputs a size, creation date, type (file or directory), and name. The size that is outputted for a directory is NOT the total size of all its files. Rather, it reflects the number of children it has. Each child is stored as two bytes, so a directory with 5 children will have size 10. This is more intuitive to the user operating the filesystem because they can then easy calculate how many blocks a directory is using (you can do this by [(size+1)/(BLOCKSIZE-20) +1]). The size of a file is simply the number of characters that was written to the file by echo. Th ls command works by iterating through the directory and printing out the names of each child with the other information. Unfortunately, in order to get this information you must fseek to the childs location and then fseek back to the cwd to get the next child. This could be fixed by storing more information for each child in the cwd but this would take up a TON of space and would make things vastly more complicated when it came to directory implementation.

The mkdir command makes a directory in the cwd and sets the block headers as specified above. It also increments the size of the parent directory by two and gives it this child (written as an address). If the directory is full then an error is output.

The cd command changes directories. You can use .. and . as arguments or leave out all arguments to go to the root directory. Its a simple function.

The echo command can take two arguments (separated by a >) or just one. If only one argument is given echo will print the argument to the terminal. If two arguments are give echo will print the argument to a file (give by argument 2), which can later be read using cat. If a file is specified and the length of argument 1 is longer than BLOCKSIZE-20, then more blocks are allocated until the entire argument 1 is written to the file system. Headers for the first block are initialized as detailed above, but sub sequential blocks dont require as much information, although 20 bytes are still used as the header to keep things standard (its a slight waste of space, but not much). Also, note that the size of the entire string is stored in the first blocks "size" space; it does not max out at BLOCKSIZE-20 with sub sequential writings to the successor blocks. This is a large function because you have to take in many considerations and do multiple error checks to make sure the user isnt doing something they shouldnt / dont want to be doing.

The cat command takes one argument, a file name. It will then print the contents of said file. If the file is larger than one block it will continue to read until the end. This is controlled using the size of a file (as specified in the first block). Its fairly simple.

The cwd command just prints the name of the current working directory.

The printbm / bitmap (you can type either) command prints the bitmap of the filesystem.

The find_child.c file looks for a child with the specified name (looking in successor blocks if necessary) and returns that child.s address if it is found or returns 0 if it is not. This was made to reduce duplication of code.

The fs.c file is implemented in the following way: A filename is passed which corresponds to the filesystem, and this name is sub sequentially passed to each command. The filesystem is then opened up (with r+) in each command and closed at the end of it. Each command file is #included on lines 24 to 31 so they arent all written within this file (it would be huge). Some bitmap size ints are specified for both of our conveniences and to make error checking easier at the beginning of main().

The Makefile was unchanged and uses gcc. 

## Comments

Each of my files contains duplicated code to get the name, size, successor address, parent address, etc. of different files / directories. I could have written a class / struct to store this information using an easy function but I used this method so that 1) I could fseek less when I didn.t need all the information and 2) to make it clear exactly what was going on and what information I was going to use. Also, you sometimes need to read/write in different ways so it was useful to be able to write this code exactly as you wanted and not have it as a general function because then you would have to fseek even more.

## Error Checking / Limitations

You cant echo to a directory.
You cant make a directory with the same name as one already specified in the cwd.
A cat command on a directory will return an error.
Names are limited to 8 characters
Directory sizes are limited to (BLOCKSIZE-20)/2 files/directories (246 if your block size is 512).
Im sure Im forgetting some.

## Known Problems

Right now if you start changing around the sizes of your filesystem and blocksize you could have issues at the end of the bitmap / with the last blocks in the filesystem. However, if you leave these values as they are then any error I came across or thought of I fixed, so I hope there are not too many.

# Reflection

I learned a ton in this project. First, I now understand so much more about filesystems and how they are implemented. It.s not a crazy difficult thing, but it isnt easy either. I can see optimizing it being extremely difficult, however. I also partly learned how I would implement a data base. I had multiple errors where I would forget to close a file and then I re-open it only to find that my data hadnt been changed because I hadnt closed the file yet in my other function. This was the biggest problem I had in my recursive functions within the commands. I now will be able to better implement a data base and larger projects in general. I probably spent around 40+ hours of work on this project and when I started I wasnt sure how to pseudo code everything. I now have a better understanding of how I would approach pseudo coding in a big project in the future.

I really enjoyed doing this project, especially implementing the mkdir, ls, cd, rm, and cat commands as well as dealing with the formatfs.c and fs.c files. I learned a significant amount as I wrote these functions about how I wanted to implement my design. I had more trouble with the echo functionality but it was still good to have to implement files larger than one block.

I did not like having to implement large files, but it was probably beneficial. There were a lot of errors that sprang up that I hadnt expected immediately but now that I understand what I need to do I should be able to do something similar again fairly easily.

The most confusing part for me was realizing that you dont have to open the file for writing. You can still edit the file using fwrite even though you are in read mode. I still dont really understand how this all works but I have a better grasp of it now.

I would definitely provide people in the future with the information you discussed in class about how to implement filesystems in general. That was a huge help. Also, letting them know that you can use fwrite in r+ mode might be a great help. Having you explain how all the reading/writing modes work would have been great.
