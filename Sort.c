
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#include <limits.h>
#include <string.h>

void MSB_radixSort(unsigned char *arr[], unsigned int n, unsigned char k, unsigned char *sort[]) {
	
	/* Declare local variables */
	short i; /* Must exceed bounds of unsigned char */
	unsigned short count[UCHAR_MAX + 1];
	
	/* Clear count, then count the byte values */
	memset(count, 0, sizeof(count));
	for(i = 0; i < n; i++) 
		count[*arr[i]]++;
	
	/* Compute the prefix sum of byte counts */
	for(i = 1; i <= UCHAR_MAX; i++)
		count[i] += count[i-1];
	
	/* Use prefix sum to re-order sort */
	for(i = n-1; i >= 0; i--) 
		sort[count[*arr[i]] - 1] = arr[i];
	
} /* Function definition */
