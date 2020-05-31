//-------------------------------------------
// Quicksort - O(n^2) but average case complexity is O(n*logn)
// Pick a pivot and swap elements so that all elements less than the pivot
// are positioned to the left of the pivot and all elements greater than
// the pivot are to the right (so now the pivot is correctly positioned).
// Recurse on smaller arrays.
//
// In the worst case one of the subarrays has size 1, so the ith call
// iterates over (n-i) elements.
// Thus the complexity is n + (n-1) + ... + 1 = n(n+1)/2 = 0(n^2)
// However in the average case each recursion calls quicksort twice, on
// arrays of ~half the previous size, plus some work to partition
// T(n) = cn + 2T(n/2)
// See mergesort definition to show T(n) in O(n*logn)
//-------------------------------------------
void _quicksort(int *array, int lo, int hi);
void quicksort(int *array, int length)
{    
    _quicksort(array, 0, length - 1);
}

//-------------------------------------------
//
//-------------------------------------------
int _partition(int *array, int lo, int hi)
{   
    int i,j,pivot,tmp;
    
    pivot = *(array + ((hi + lo) / 2));
    i = lo - 1;
    j = hi + 1;
    while (true)
    {
        // Increase until we find an element to swap
        do
        {
            i++;
        } while (*(array + i) < pivot);
        
        // Decrease until we find an element to swap
        do
        {
            j--;
        } while (*(array + j) > pivot);
        
        if (i >= j) 
        {
            // We're done
            return j;
        }
        
        // Swap
        tmp = *(array + i);
        *(array + i) = *(array + j);
        *(array + j) = tmp;
    }
}

//-------------------------------------------
//
//-------------------------------------------
void _quicksort(int *array, int lo, int hi)
{
    int pivot;
    
    if (lo < hi)
    {        
        pivot = _partition(array, lo, hi);
        _quicksort(array, lo, pivot);
        _quicksort(array, pivot + 1, hi);
    }
}