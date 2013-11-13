#include <stdio.h>


int main()
{
	int sum = 0;
	int sum2 = 0;
	int i = 0;
	int j = 0;
	for(i = 0; i < 1000000; i++)
	{
		if(i < 910000)
		{
			if(i < 900000)
			{
				if(i < 890000)
				{
					if(i < 880000)
					{
						sum++;
					}
					else
					{
						sum = sum + 2;
					}
				}
				else
				{
					sum = sum + 2;
				}	
			}
			else
			{
				sum = sum + 2;
			}
			
		}
		else
		{
			if(i < 1000000)
			{
				if(i < 990000)
				{
					if(i < 980000)
					{
						sum--;
					}
				}	
			}
		}
		if(i < 910000)
		{
			for(j = 0; j < 1000; j++)
			{
				if(j < 810)
				{
					if(j < 800)
					{
						if(j < 790)
						{
							sum2++;
						}
						else
						{
							sum2 = sum2 + 2;
						}
					}
					else
					{
						sum2 = sum2 + 2;
					}
				}
				else
				{
					sum2--;
				}
			}
		}
	}
	
	printf("sum = %d", sum);
	printf("sum2 = %d", sum2);
	return 0;
	
}
