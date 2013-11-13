#include <stdio.h>


int main()
{
	int sum = 0;
	int sum2 = 0;
	int i = 0;
	int j = 0;
	for(i = 0; i < 1000000; i++)
	{
		if(i < 810000)
		{
			sum++;	
		}
		else
		{
			sum--;
		}
		for(j = 0; j < 1000; j++)
		{
			if(j < 810)
			{
				sum2++;
			}
			else
			{
				sum2--;
			}
		}
	}
	
	printf("sum = %d", sum);
	printf("sum2 = %d", sum2);
	return 0;
	
}
