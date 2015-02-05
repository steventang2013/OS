#include <unistd.h>
#include <stdio.h>

int main(){
	int id;
	int result;
	id = syscall(318, 4, 5, &result);
	printf("should print out zero: %d\n", id);
	return 0;
}
