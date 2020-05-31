//-------------------------------------------
// Insert Sort - O(n^2)
// For each element in the array, look backwards to find the
// elements position and insert.
//-------------------------------------------
void insert_sort(int *array, int length)
{
    int i;
    int current_item, current_index;
    
    for (i = 1; i < length; i++)
    {
        current_item = *(array + i);
        current_index = i - 1;

        while (current_index >= 0 && *(array + current_index) > current_item)
        {
            // Shift lower index elements right until the correct position is found
            *(array + current_index + 1) = *(array + current_index);
            current_index--;
        }
        // Insert the current item
        *(array + current_index + 1) = current_item;
    }
}
