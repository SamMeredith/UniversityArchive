//-------------------------------------------
// Binary Seach - O(log(n))
// Return the index of value_to_find in array
// Requires array to be sorted
//-------------------------------------------
int binary_search(int *array, int length, int value_to_find)
{
    int high_val, index = -1, low_val, mid;
    
    high_val = length - 1;
    low_val = 0;
    
    while (low_val <= high_val)
    {
        mid = (low_val + high_val) / 2;
        if (*(array + mid) == value_to_find)
        {
            index = mid;
            break;
        }
        
        if (*(array + mid) < value_to_find)
        {
            low_val = mid + 1;
        }
        else
        {
            high_val = mid - 1;
        }
    }
    
    return index;
}