#Project Description
This project is the implementation of a simple shell. It gives you a prompt where you can enter standard commands which will be executed using execvp (or chdir for cd) and forking. This project uses the readline library to read in an input command.

#Build Instructions
You compile using 'make'.

##Execution Instructions
Execute using './myshell'.

##Reflection
Things I learned:
1) How forking works.
2) Some of the commands in C to get the username, cwd, etc.
3) How to better use the C library and understand man pages.
4) How you can parse or modify your information by changing the actual memory and not just re-assigning things
5) execve is a pain, use execvp instead. i.e. some functions are better than others, look for similar ones if you are having trouble
6) How execvp works, as well as what the PID is and why it matters.
7) How readline and strsep works.

The most interesting part was learning how forking works and execvp. It was also cool to see readline work so well - much better than gets().
It would be interesting to learn more about how the readline library works with a comparison to other intake functions - i.e. I dont really understand what all the differences are.
My biggest problem was getting the readline library to link correctly. I wish I knew beforehand where they were in the FS.
