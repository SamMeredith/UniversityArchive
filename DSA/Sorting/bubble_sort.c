//-------------------------------------------
// Bubble Sort - O(n^2)
// Repeatedly iterate through the array, swapping adjacent elements if they are
// out of order. Terminate when no swaps occur. Note that we don't need to
// check beyond the last swap of the previous pass, since those elements must 
// already me sorted.
//-------------------------------------------
void bubble_sort(int *array, int length)
{
    int i, n, new_n;
    
    n = length;
    do
    {     
        new_n = 0;
        for (i = 1; i < n; i++)
        {
            if (*(array + i - 1) > *(array + i))
            {
                int tmp = *(array + i);
                *(array + i) = *(array + i - 1);
                *(array + i - 1) = tmp;
                // If i is the position of the last swap,
                // then all elements beyond i must be sorted
                new_n = i;
            }
        }
        n = new_n;
    } while (n > 1);
}
