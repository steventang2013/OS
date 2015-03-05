#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(){

	char a;
	char buff[1024] = {'\0'};
	int test = 1;
	while (test){
		printf("Please input a command (r,w,e,etc): \n");
		scanf(" %c", &a);
		if (a == 'w'){
			int fp = open("/dev/simple_char_driver", O_RDWR);
			printf("Enter data you want to write to the device: \n");
			scanf(" %[^\n]s", buff);
			printf("This is the length of the string written: %d\n",strlen(buff));
			write(fp, buff, strlen(buff));
			close(fp);
		}
		else if (a == 'r'){
			int fp = open("/dev/simple_char_driver", O_RDWR);
			read(fp, buff, sizeof(buff));
			printf("Data read from the device: %s\n", buff);
			close(fp);
		}
		else if (a == 'e'){
			test = 0;
		}
	}
	return 0;
}
