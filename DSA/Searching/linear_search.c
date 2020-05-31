//-------------------------------------------
// Linear Seach - O(n)
// Return the index of value_to_find in array
//-------------------------------------------
int linear_search(int *array, int length, int value_to_find)
{
    int i, index = -1;
    
    for (i = 0; i < length; i++)
    {
        if (*(array + i) == value_to_find)
        {
            index = i;
            break;
        }
    }
    
    return index;
}