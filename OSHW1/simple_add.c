#include <linux/kernel.h>
#include <linux/linkage.h>

asmlinkage long sys_simple_add(int num1, int num2, int* result) {
	printk(KERN_ALERT "Value for num1 is %d\n", num1);
	printk(KERN_ALERT "Value for num2 is %d\n", num2);
	*result = num1 + num2;
	printk(KERN_ALERT "Result of sum is %d\n",*result);
	return 0;
}
