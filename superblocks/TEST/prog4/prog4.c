#include <stdio.h>


int main()
{
	int sum = 0;
	int sum2 = 0;
	int i = 0;
	int j = 0;
	int i1 = 0;
	int i2 = 0;
	int i3 = 0;
	int j1 = 0;

	for(i = 0; i < 100; i++)
	{
		if(i < 91)
		{	
			for(i1 = 0; i1 < 100 < i1++)
			{
				
				if(i < 90)
				{
					for(i2 = 0; i2 < 100 < i2++)
					{
						if(i < 89)
						{	
							for(i3 = 0; i3 < 100; i3++)
							{
								if(i < 88)
								{
									sum++;
								}
								else
								{
									sum = sum + 2;
								}
							}
						}
						else
						{
							sum = sum + 2;
						}	
					}
				}
				else
				{
					sum = sum + 2;
				}
				
			}
		}
		else
		{
			if(i < 100)
			{
				if(i < 99)
				{
					if(i < 98)
					{
						sum--;
					}
				}	
			}
		}
		if(i < 91)
		{
			for(j = 0; j < 100; j++)
			{
				if(j < 81)
				{
					if(j < 80)
					{
						for(j1 = 0; j1 < 100; j1++)	
						{
							if(j < 79)
							{
								sum2++;
							}
							else
							{
								sum2 = sum2 + 2;
							}
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
