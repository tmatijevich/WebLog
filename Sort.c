/*******************************************************************************
 * File:      WebLog\Sort.c
 * Author:    Tyler Matijevich
 * Created:   2022-01-07
********************************************************************************
 * Description: Sort all records by time using a MSB (most significant bit) 
 radix-counting sort algorithm
*******************************************************************************/

#define _REPLACE_CONST
#include <AsDefault.h>
#include <limits.h>
#include <string.h>

//extern unsigned long ai[WEBLOG_SORT_MAX], si[WEBLOG_SORT_MAX];

void radixSort(unsigned char *arr[], unsigned int n, unsigned char k, unsigned char *sort[]) {
	
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
		for(i = n-1; i >= 0; i--) {
			sort[--count[*(arr[i] + j)]] = arr[i]; /* Do not offset by j, offset is always applied to arr */
			si[count[*(arr[i] + j)]] = ai[i]; /* Store indices as well as addresses */
		}
		
		/* Swap output and input */
		if(j < k || k % 2) {
			temp = arr;
			arr = sort;
			sort = temp;
			/* Swap indecies */
			for(i = 0; i < n; i++) 
				ai[i] = si[i];
		}
		j++; /* Update byte */
	}
	
} /* Function definition */
