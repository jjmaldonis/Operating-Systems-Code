#Project description
The goal of this project is the create my own 'counter' device that will exist at /dev/counter. I have modified the /dev/hello driver (code in /usr/src/drivers/hello) to make my counter device. This code teaches us how to implement something new in a driver. You learn about how things are passed in drivers and a bit about how they work otherwise.

##Code description 
This code prints out an integer, starting with zero, incrementing each time you run the driver.

##Build instructions 
Copy /usr/src over into a directory of your choice so that you don't over write anything important. Then make a new directory called 'counter' in whichever directory you chose, followed by /src/drivers/, I chose ~). Now take the files here on github and copy them into ~/src/drivers/counter. To build do the following commands in the /driver/counter directory: 'make && make install'.

##Execution instructions
In order to use the driver do the following commands (in su mode): 'service up /usr/sbin/counter -dev /dev/counter && cat /dev/counter'. To shut down the driver do: 'service down counter'.
More information on this below.

##Reflection
The most interesting thing I learned was that you can take even a driver and modify it to produce something new that you want to do. I know that it is often fairly easy to take a non-OS program and modify it to do what you want, but to be able to do this even with things as complex as the operating system is cool. And it really wasn't that hard. It was also good to learn how you pass strings and other variables you want to print out (such as ints) using address passing. One thing I don't understand even after completing the project is exactly how you communicate with external devices such as the keyboard using a driver, this is something I would like to do. One reason this is tough for me to grasp is that I am not entirely sure how a keyboard's input is passed to the driver. This project was more about how you use information once you already have it. My biggest problem was definitely not understanding how you were supposed to pass the int using address passing. The work-around I did for this was converting the int into a string using a simple function. Then I just passed the string like they did before. One thing I would tell someone new to this project would be to pay extremely close attention how strings are interpreted when you pass the address of the string. Your explanation of this is really what made the project click for me.


###Below are some steps to repeat this exercise.
Edit the following file and add the stuff it tells you to in the site linked to us by DC.
edit /etc/system.conf
reboot

To make this work you run the following lines from the terminal:

mknod /dev/counter c 17 0 (You only need to do this once ever to make the node)

service up /usr/sbin/counter -dev /dev/counter (This starts the dev up)

cat /dev/counter (Run this as many times as you want. It runs the device.)

service refresh counter (This resets the device.)

cat /dev/counter (Same as above)

service down counter (This shuts down the device.)

Shorthand: make && make install && service up /usr/sbin/counter -dev /dev/counter && cat /dev/counter

If you get the error Request 0x700 to RS failed: resource busy (error 16) you need to reboot. This will likely hapen after you have edited the system.conf file.

Some notes: 

	unsigned long ex64lo(u64_t i) extracts the low 32 bits of a 64 bit number.

	SYS_SAFECOPYTO: Copy data from one's own address space, to a granted memory area of another process. The caller must have CPF_WRITE access to the grant.

		int sys_safecopyto(endpoint_t dest, cp_grant_id_t grant, vir_bytes grant_offset, vir_bytes my_address, size_t bytes);

	typedef unsigned int vir_bytes;       /* virtual addresses and lengths in bytes */ 

	cp_grant_id_t is an int or a long, I'm not sure which.

	endpoint_t is an int.

	Here is the link to the website DC gave us: http://wiki.minix3.org/en/DevelopersGuide/DriverProgramming
