#include <math.h>

//-------------------------------------------
// Radix sort O(kn)
// Where k may depend on n, depending on the nature of the array to be sorted.
// If the array contains numbers 1 to n in some order, then the numeber of
// significant digits is log(n) so radix sort is O(n*log(n))
// However if the array contains many repititions but only has entries up to 
// some n0 << n, then k = log(n0) is a constant and the runtime is O(n)
//-------------------------------------------
void radix_sort(int *array, int length)
{
    int array_bound = 0, exp, exp_max, i;
    int *array_sorted = (int*)malloc(length*sizeof(int));
    
    // Find the largest value in the array
    for (i = 0; i < length; i++)
    {
        if (*(array + i) > array_bound)
        {
            array_bound = *(array + i);
        }
    }
    exp_max = log10((double)array_bound) + 1;
    
    for (exp = 0; exp < exp_max; exp++)
    {
        int buckets[10] = {0};
        int digit = pow(10, exp);
        
        for (i = 0; i < length; i++)
        {
            // Count the number of each digit
            buckets[*(array + i )/ digit % 10]++;
        }
        
        for (i = 1; i < 10; i++)
        {
            // Accumulate
            buckets[i] += buckets[i - 1];
        }
        
        for (i = length - 1; i >= 0; i--)
        {
            buckets[*(array + i) / digit % 10]--;
            array_sorted[buckets[*(array + i) / digit % 10]] = *(array + i);
        }
        
        for (i = 0; i < length; i++)
        {
            *(array + i) = array_sorted[i];
        }
    }
}