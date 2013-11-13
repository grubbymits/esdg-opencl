#include <stdio.h>

int main()
{
	int i = 0;
	int step = 0;
	int x = 0;

	for(i = 0; i < 100000000; i++)
	{
		if(!(i & 255))
		{
			step = i;
		}
		x = (step -1) << 2;
	}

	printf("x = %d", x);
	return 0;

}
