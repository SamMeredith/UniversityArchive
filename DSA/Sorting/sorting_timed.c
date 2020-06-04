#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "bubble_sort.c"
#include "insert_sort.c"
#include "merge_sort.c"
#include "quicksort.c"
#include "radix_sort.c"
#include "heap_sort.c"

typedef void (*sort_function_t)(int *, int);

void time_sort(int *array, int length, sort_function_t algorithm, char *name);

//-------------------------------------------
// Check if an array is sorted
//-------------------------------------------
bool is_sorted(int *array, int length)
{
    int i;
    
    for (i = 0; i < length - 1; i++) {
        if (*(array + i) > *(array + i + 1)) {
            return false;
        }
    }
    
    return true;
}

//-------------------------------------------
// Sorting Functions
// Expects a file input.txt containing 2 line pairs consisting of
// 1. A single digit for the number of elements
// 2. A comma separated list of elements
//-------------------------------------------
int main(int argc, char *argv[])
{
    FILE *fp;
    int i, num_items;
    
    fp = fopen("sort_input.txt", "r");
    
    if (fp)
    {
        printf("Sorting function performance comparison.");
        
        while(fscanf(fp, "%d\n", &num_items) != EOF)
        {
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
            
            printf("\r\n\r\nFound %d elements", num_items);
            
            time_sort(array, num_items, &bubble_sort, "Bubble sort");
            time_sort(array, num_items, &insert_sort, "Insert sort");
            time_sort(array, num_items, &mergesort, "Merge sort");
            time_sort(array, num_items, &quicksort, "quicksort");
            time_sort(array, num_items, &radix_sort, "Radix sort");
            time_sort(array, num_items, &heap_sort, "Heap sort");
        }
    }
    
    fclose(fp);
    
    return 0;
}

//-------------------------------------------
// Time a sorting algorithm
//-------------------------------------------
void time_sort(int *array, int length, sort_function_t algorithm, char *name)
{
    int *array_to_sort = (int*)malloc(length*sizeof(int));
    memcpy(array_to_sort, array, length*sizeof(int));
    
    clock_t begin = clock();
    (*algorithm)(array_to_sort, length);
    clock_t end = clock();
    
    if (is_sorted(array_to_sort, length))
    {
        printf("\r\n%s succeeds in %f seconds.", name, (double)(end - begin) / CLOCKS_PER_SEC);
    }
    else
    {
        printf("\r\n%s fails.", name);
    }
}