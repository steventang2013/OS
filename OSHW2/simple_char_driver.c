#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <asm/uaccess.h>
#define BUFFER_SIZE 1024

static char device_buffer[BUFFER_SIZE];
int counter = 0;
int counter1 = 0;

ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	int bytes; 
    int bytes_to_read; 
    int bytes_read;
    bytes = BUFFER_SIZE - *offset;
    if(bytes > length) 
        bytes_to_read = length;
    else
        bytes_to_read = bytes;
    if(bytes_to_read == 0)
        printk(KERN_ALERT "Reached end of the device\n");
    bytes_read = bytes_to_read - copy_to_user(buffer, device_buffer + *offset, bytes_to_read);
    printk(KERN_ALERT "Device has been read - %d bytes\n",bytes_read);
    *offset += bytes_read;
	/* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
	/*  length is the length of the userspace buffer*/
	/*  current position of the opened file*/
	/* copy_to_user function. source is device_buffer (the buffer defined at the start of the code) and destination is the userspace buffer *buffer */
	return bytes_read;
}

ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
    int bytes; 
    int bytes_to_write; 
    int bytes_written;
    bytes = BUFFER_SIZE - *offset;
    /*if the maximum number of bytes allowed is greater than your string length,
     * set bytes_to_write as string length. Else, maxbytes to prevent overwriting buffer*/
    if(bytes > length) 
        bytes_to_write = length;
    else
        bytes_to_write = bytes;
    bytes_written = bytes_to_write - copy_from_user(device_buffer + *offset, buffer, bytes_to_write);
    printk(KERN_ALERT "device has been written - %d bytes\n",bytes_written);
    *offset += bytes_written;
    return bytes_written;
	/* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
	/*  length is the length of the userspace buffer (string that you wrote)*/
	/*  current position of the opened file*/
	/* copy_from_user function. destination is device_buffer (the buffer defined at the start of the code) and source is the userspace 		buffer *buffer */
}


int simple_char_driver_open (struct inode *pinode, struct file *pfile)
{
	counter += 1;
	printk(KERN_ALERT "file is opened %d times\n", counter);
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
	return 0;
}


int simple_char_driver_close (struct inode *pinode, struct file *pfile)
{
	counter1 += 1;
	printk(KERN_ALERT "file is closed %d times\n", counter1);
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
	return 0;
}

struct file_operations simple_char_driver_file_operations = {

	.owner   = THIS_MODULE,
	.read = simple_char_driver_read,
	.write = simple_char_driver_write,
	.release = simple_char_driver_close,
	.open = simple_char_driver_open,
	/* add the function pointers to point to the corresponding file operations. look at the file fs.h in the linux souce code*/
};

static int simple_char_driver_init(void)
{
	printk(KERN_ALERT "Init function called\n");
	
	register_chrdev(240, "simple_char_driver", &simple_char_driver_file_operations);
	/* print to the log file that the init function is called.*/
	/* register the device */
	return 0;
}

static int simple_char_driver_exit(void)
{
	printk(KERN_ALERT "Exit function called\n");
	unregister_chrdev(240, "simple_char_driver");
	/* print to the log file that the exit function is called.*/
	/* unregister  the device using the register_chrdev() function. */
	return 0;
}

/* add module_init and module_exit to point to the corresponding init and exit function*/
module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);
