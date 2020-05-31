//-------------------------------------------
// Mergesort - O(n*log(n))
//
// To see the time complexity T(n): each recursive call runs mergesort twice more
// on arrays of size n/2, plus the work for the merge which is clearly linear
// T(n) = c*n + 2T(n/2)
// Make the guess T(n) in O(n*log(n)), that is T(n) = A*n*log(n)
// => 2T(n/2) = 2A*(n/2)*log(n/2) = A*n*log(n) - A*n
// By substitution: A*n*log(n) = c*n + A*n*log(n) - A*n
// => c*n = A*n qed.
//-------------------------------------------
void _merge(int *array, int *merged, int first, int mid, int last);
void _mergesort(int *array, int *merged, int first, int last);
void mergesort(int *array, int length)
{    
    int *merged_array = (int*)malloc(length*sizeof(int));
    
    _mergesort(array, merged_array, 0, length - 1);
}

//-------------------------------------------
//
//-------------------------------------------
void _merge(int *array, int *merged, int first, int mid, int last)
{
    int i = first, j = mid, merged_i = first;
    
    while (i <= mid - 1 && j <= last)
    {
        if (*(array + i) < *(array + j))
        {
            *(merged + merged_i++) = *(array + i++);
        }
        else
        {
            *(merged + merged_i++) = *(array + j++);
        }
    }
    
    while (i <= mid - 1)
    {
        *(merged + merged_i++) = *(array + i++);
    }
    
    while (j <= last)
    {
        *(merged + merged_i++) = *(array + j++);
    }
    
    for (i = first; i <= last; i++)
    {
        *(array + i) = *(merged + i);
    }
}

//-------------------------------------------
//
//-------------------------------------------
void _mergesort(int *array, int *merged, int first, int last)
{
    if (last - first <= 0)
        return;
    
    int mid = (last + first) / 2 + 1;
    
    _mergesort(array, merged, first, mid - 1);
    _mergesort(array, merged, mid, last);
    
    _merge(array, merged, first, mid, last);
    
    return;
}