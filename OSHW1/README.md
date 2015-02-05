 Introduction to System Calls Assignment #1
 =======
 Steven Tang</br>
 steven.tang@colorado.edu

 
 ###Functionality

 The program creates a system call that adds two integer values and 
 stores them into a pointer variable in the userspace.
 
 ###Modified files

	arch/x86/kernel/simple_add.c
	arch/x86/kernel/Makefile
	arch/x86/syscalls/syscall_64.tbl
	include/linux/syscalls.h

 
 ###Unit Test Procedure

 In the home directory there is a file called test.c
 1. Compile and execute test.c:
	gcc -o test test.c 
	./test
 2. To view the log, execute this command:
	"cat /var/log/syslog"
	or
	"dmesg"
