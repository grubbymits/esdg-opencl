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

        for(i = 0; i < 1000000; i++)
        {
                if(i < 910000)
                {
                        for(i1 = 0; i1 < 10; i1++)
                        {

                                if(i < 900000)
                                {
                                        for(i2 = 0; i2 < 10; i2++)
                                        {
                                                if(i < 890000)
                                                {
                                                        for(i3 = 0; i3 < 10; i3++)
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
                        for(j = 0; j < 100; j++)
                        {
                                if(j < 81)
                                {
                                        if(j < 80)
                                        {
                                                for(j1 = 0; j1 < 10; j1++)
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

