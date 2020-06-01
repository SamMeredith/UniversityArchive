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
void _imergesort(int *array, int first, int last);
void mergesort(int *array, int length)
{    
    int *merged_array = (int*)malloc(length*sizeof(int));
    
    _mergesort(array, merged_array, 0, length - 1);
    
    free(merged_array);
    
    // In place merge sort - slower but interesting
    // Original paper "Practical In-Place Mergesort, Katajainen et al."
    // Taken from S.O https://stackoverflow.com/questions/2571049
    //_imergesort(array, 0, length);
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

void _wmerge(int *array, int i, int m, int j, int n, int working);
void _wsort(int *array, int lower, int upper, int working);

//-------------------------------------------
// In place merge sort needs to use some part of the existing array
// to carry out the merge step
// This is done by recursively merging half of the sub-array in each merge
//
// Note: Sorts [lower, upper) unlike _mergesort above (just because the paper did this)
//-------------------------------------------
void _imergesort(int *array, int lower, int upper)
{
    int mid, n, working;
    
    if (upper - lower <= 0)
        return;
    
    mid = lower + (upper - lower) / 2; // Nice trick to avoid int overflow
    working = lower + upper - mid; // Space within array for merge operations
    
    _wsort(array, lower, mid, working);
    // array[lower, mid) and array[mid, upper) are merged into array[working..]
    // i.e the last half of array contains sorted elements
    while (working - lower > 2)
    {
        n = working;
        working = lower + (n - lower) / 2 + 1;
        _wsort(array, working, n, lower);
        // Now the array looks like
        // || .. sorted 1/4 array B.. | .. unsorted working area .. | .. sorted 1/2 array A ..
        // Now we merge A and B into the working area. The working area is not large
        // enough for A + B but it is fine to overlap into A so long as we don't 
        // overwrite unmerged elements. This can't happen since each time an element from
        // A is merged, its memory becomes available to the 'working' area
        _wmerge(array, lower, lower + n - working, n, upper, working);
    }
    // Insert sort anything left over. This is a constant number of inserts, over the
    // sub-array only
    for (n = working; n > lower; n--)
    {
        for (mid = n; mid < upper && *(array + mid) < *(array + mid - 1); mid++)
        {
            int tmp = *(array + mid);
            *(array + mid) = *(array + mid - 1);
            *(array + mid - 1) = tmp;
        }
    }
}

//-------------------------------------------
// Merge two sorted sub-arrays array[i, m) and array[j, n) to working area array[w...]
//-------------------------------------------
void _wmerge(int *array, int i, int m, int j, int n, int working)
{
    int tmp;
    
    while (i < m && j < n)
    {
        tmp = *(array + working);
        if (*(array + i) < *(array + j))
        {
            *(array + working++) = *(array + i);
            *(array + i++) = tmp;
        }
        else
        {
            *(array + working++) = *(array + j);
            *(array + j++) = tmp;
        } 
    }
    while (i < m)
    {
        tmp = *(array + working);
        *(array + working++) = *(array + i);
        *(array + i++) = tmp;
    }
    while (j < n)
    {
        tmp = *(array + working);
        *(array + working++) = *(array + j);
        *(array + j++) = tmp;
    }
}

//-------------------------------------------
//
//-------------------------------------------
void _wsort(int *array, int lower, int upper, int working)
{
    int mid, tmp;
    
    if (upper - lower > 1)
    {
        mid = lower + (upper - lower) / 2;
        _imergesort(array, lower, mid);
        _imergesort(array, mid, upper);
        _wmerge(array, lower, mid, mid, upper, working);
        // array[lower, mid) and array[mid, upper) are merged into array[working..]
    }
    else
    {
        // Just swap the remaining indices
        while (lower < upper)
        {
            tmp = *(array + lower);
            *(array + working++) = *(array + lower);
            *(array + lower++) = tmp;
        }
    }
}
