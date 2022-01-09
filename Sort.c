
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#include <limits.h>
#include <string.h>

void MSB_radixSort(unsigned char *arr[], unsigned int n, const unsigned char k, unsigned char *sort[]) {
	
	/* Declare local variables */
	short i, j = 0; /* Must exceed bounds of unsigned char */
	unsigned short count[UCHAR_MAX + 1];
	unsigned char **temp;
	
	while(j < k) {
		/* Clear count, then count the byte values */
		memset(count, 0, sizeof(count));
		for(i = 0; i < n; i++) 
			count[*(arr[i] + j)]++;
		
		/* Compute the prefix sum of byte counts */
		for(i = 1; i <= UCHAR_MAX; i++)
			count[i] += count[i-1];
		
		/* Use prefix sum to re-order sort */
		for(i = n-1; i >= 0; i--) 
			sort[count[*(arr[i] + j)] - 1] = arr[i] + j;
		
		/* Swap output and input */
		if(j < k || k % 2) {
			temp = arr;
			arr = sort;
			sort = temp;
		}
		j++; /* Update byte */
	} 
	
} /* Function definition */
