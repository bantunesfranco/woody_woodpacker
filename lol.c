#include <unistd.h>

int main(void)
{
	write(1, "Hello, World!\n", 14);
}