#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "..\Sorting\quicksort.c"

#include "linear_search.c"
#include "binary_search.c"

typedef int (*search_function_t)(int *, int, int);

void time_search(int *array, int length, int value_to_find, search_function_t algorithm, char *name);

//-------------------------------------------
// Searching Functions
// Expects a file input.txt containing 3 line pairs consisting of
// 1. A single digit for the number of elements
// 4. A single digit for the value to find
// 2. A comma separated list of elements
//-------------------------------------------
int main(int argc, char *argv[])
{
    FILE *fp;
    int i, num_items, value_to_find;
    
    fp = fopen("search_input.txt", "r");
    
    if (fp)
    {
        printf("Searching function performance comparison.");
        
        while(fscanf(fp, "%d\n", &num_items) != EOF)
        {
            fscanf(fp, "%d\n", &value_to_find);
            
            int *array = (int*)malloc(num_items*sizeof(int));
            
            for (i = 0; i < num_items; i++)
            {
                fscanf(fp, "%d", array + i);
                if (getc(fp) != ',')
                {
                    num_items = i + 1;
                    break;
                }
            }
            
            printf("\r\n\r\nLooking for value %d in array of %d elements", value_to_find, num_items);
            
            time_search(array, num_items, value_to_find, &linear_search, "Linear search");
            time_search(array, num_items, value_to_find, &binary_search, "Binary search");
        }
    }
    
    fclose(fp);
    
    return 0;
}

//-------------------------------------------
// 
//-------------------------------------------
void time_search(int *array, int length, int value_to_find, search_function_t algorithm, char *name)
{
    int index;
    int *array_to_sort = (int*)malloc(length*sizeof(int));
    memcpy(array_to_sort, array, length*sizeof(int));
    
    // Binary search needs the array to be sorted and I'm lazy today
    quicksort(array_to_sort, length);
    
    clock_t begin = clock();
    index = (*algorithm)(array_to_sort, length, value_to_find);
    clock_t end = clock();
    
    if (index == -1)
    {
        printf("\r\n%s finds value %d is not in the array in %f seconds", name, value_to_find, (double)(end - begin) / CLOCKS_PER_SEC);
    }
    else
    {
        printf("\r\n%s finds value %d at index %d in %f seconds", name, value_to_find, index, (double)(end - begin) / CLOCKS_PER_SEC);
    }
}
