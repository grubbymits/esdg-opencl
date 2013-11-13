#include <stdio.h>


int main()
{
	int i;
	int a = 0;
	int b = 0; 

	for(i = 0; i < 100000000; i++)
	{
		if(i < 99000000)
		{
			a = 1;
		}
		else
		{
			a = a + b;
		}
		
		b = a;

		if( i < 99000000)
		{
			b++;
		}
		else
		{
			b--;
		}
	}

	printf("a = %d\n", a);
	printf("b = %d\n", b);
	printf("i = %d\n", i);
	return 0;
}
