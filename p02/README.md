
#Project Description / Code Description
Edit the Minix 3 scheduler to keep track of how much CPU time each user process has had recently and change the scheduling mechanism for user processors so that the process that has had the smallest share of the CPU time (measured in quanta) runs first.

#Build Instructions
Copy the source tree to your home directory.
The files that have been modified are in the directory ~/src/servers/sched/. Any changes should be done in this directory as they need to be in this directory to have the build scripts add them to the kernel.
I modified the files: schedule.c, main.c, and schedproc.h
Once you have copied all the files correctly build by typing 'make'.

#Execution Instructions
To run you must recompile the Minix 3 OS using hdboot after recompiling /usr/src. You can then reboot the OS.
See wiki.minix3.org/en/DevelopersGuide/RebuildingSystem for more information.

#What I did to modify the original code
1) I disabled the balance_queues function so it doesnt run.

2) I made an "unsigned time_used" in schedproc.h (in the struct schedproc) 
	to count the number of quanta used for each processes. It only increments when the process runs
	out of quantum and the do_noquantum() function is fun.
	"time_used" is set to 0 in do_start_scheduling().
	
3) I made a new function called set_order() that resets the priorities to make it so that the 
	process with the least time_used runs.
	I do this by going through all the processes and finding the minimum time_used of them all.
	I then schedule_process the ones with time_used = min_time_used with priority = 1.
	I schedule_process all the other ones with priority = 2.
	This runs everytime (right before) the SCHED schedules a process to set everything to the correct priorities.
	
4) I got rid of the nice functionality and balance_question function as well (in main.c) because we dont want them to run.

5) I made a new function called print_stuff that prints the proc_nr, time_used, priority, and endpoint of all the processes in the queues.
   It runs in schedule_process once using a timer like balance_queues did and it calls itself recursively every 500 ticks so that we can see whats going on every so often. 
   Id like to use the endpoint as a comparison to the schedule_process endpoint so that we can see what is actually running. Its kinda annoying to see this all the time because it runs so fast you cant see what the heck is going on. I have two lines in schedule_process that do this but they are commented out currently.

#Reflection
The most interesting part was recompiling the OS and realizing how easy it is to implement a change. It might take a lot of trouble shooting, but it is very doable. I did not think this was so easily possible.

I still dont exactly understand how the intricacies of messages work because the code seems more complicated than the picture I have in my head.

My biggest problem was figuring out how I was going to record how much time a process had used. I resolved this by not using the time in seconds, but rather the amount of quanta (a specific number of seconds) that the process had completed. This made editing the code drastically easier and it still works very efficiently.

I would help explain what the code that is already in place in the Minix OS does to help students in the future. By going through the code line by line in a function that we wont be editing (so you dont give the project away) we will better be able to understand the structure of the implementation. We did do some of this in class and it helped significantly, but it was only after I had started the project so I had a lot of wasted time.
