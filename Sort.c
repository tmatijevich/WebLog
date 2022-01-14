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

void radixSort(unsigned char *arr[], unsigned short a[], unsigned short n, unsigned char k, unsigned char *sort[], unsigned short s[]) {
	
	/* Declare local variables */
	short i, j = k - 1; /* Must exceed bounds of unsigned char */
	unsigned short count[UCHAR_MAX + 1];
	unsigned char **temp, *swapPChar;
	unsigned short swapShort;
	
	while(j >= 0) {
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
			s[count[*(arr[i] + j)]] = a[i]; /* Store indices as well as addresses */
		}
		
		j--; /* Update byte */
		
		/* Swap output and input */
		if(j >= 0) {
			temp = arr;
			arr = sort;
			sort = temp;
			for(i = 0; i < n; i++) a[i] = s[i]; /* Swap indecies */
		}
	}
	
	/* Reverse sorted output */
	for(i = 0; i < n/2; i++) {
		swapShort = s[i];
		swapPChar = sort[i];
		
		s[i] = s[n - 1 - i];
		sort[i] = sort[n - 1 - i];
		
		s[n - 1 - i] = swapShort;
		sort[n - 1 - i] = swapPChar;
	}
	
} /* Function definition */
