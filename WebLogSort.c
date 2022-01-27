/*******************************************************************************
 * File:      WebLog\WebLogSort.c
 * Author:    Tyler Matijevich
 * Created:   2022-01-07
********************************************************************************
 * Description: Sort all records by time using LSB radix sort
*******************************************************************************/

#include <limits.h>
#include <string.h>

/* Sort n elements of k bytes using LSB radix sort */
void radix_sort(unsigned char *in[], unsigned short idx[], unsigned char *sort[], unsigned short sortIdx[], unsigned short n, unsigned char k, unsigned char descending) {
	
	/* 
	 * in:         Array of n pointers each pointing to array of k bytes (descructive)
	 * idx:        Unsorted indices 0..n-1 (descructive)
	 * sort:       Memory allocated with size equal to in
	 * sortIdx:    Sorted indices 0 through n-1
	 * n:          Number of elements (in, idx, sort, sortIdx)
	 * k:          Number of bytes pointed to by *in[]
	 * descending: Ascending or descending
	 */
	
	/* Declare local variables */
	long i; 											/* Loop 0..n-1 elements */
	short offset = k - 1; 								/* Offset 0..k-1 bytes, initialize to LSB */
	unsigned short count[UCHAR_MAX + 1]; 				/* Count 0..255 byte values */
	unsigned char **pSwap; 								/* Swap pointers to pointer */
	unsigned char *pByteSwap; 							/* Swap pointers to byte */
	unsigned short idxSwap; 							/* Swap indices */
	
	while(offset >= 0) {
		/* Count every byte value */
		memset(count, 0, sizeof(count)); 				/* Reset counts */
		for(i = 0; i < n; i++) 
			count[*(in[i] + offset)]++; 				/* Access element, offset address, access byte */
		
		/* Prefix sum */
		for(i = 1; i <= UCHAR_MAX; i++)
			count[i] += count[i - 1];
		
		/* Order *sort[] */
		for(i = n - 1; i >= 0; i--) {
			sort[--count[*(in[i] + offset)]] = in[i]; 	/* Assign MSB */
			sortIdx[count[*(in[i] + offset)]] = idx[i];
		}
		
		offset--;
		
		/* Swap */
		if(offset >= 0) {
			pSwap = in;
			in = sort;
			sort = pSwap;
			memcpy(idx, sortIdx, sizeof(idx[0]) * n);
		}
	}
	
	/* Descending */
	if(descending) {
		for(i = 0; i < n / 2; i++) {
			pByteSwap = sort[i];
			idxSwap = sortIdx[i];
			
			sort[i] = sort[n - 1 - i];
			sortIdx[i] = sortIdx[n - 1 - i];
			
			sort[n - 1 - i] = pByteSwap;
			sortIdx[n - 1 - i] = idxSwap;
		}
	}
	
} /* Definition */
