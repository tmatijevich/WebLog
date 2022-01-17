/*******************************************************************************
 * File:      WebLog\Time.c
 * Author:    Tyler Matijevich
 * Created:   2022-01-10
********************************************************************************
 * Description: Format time/date string and sortable time data from record
                timestamp
*******************************************************************************/

#include <AsDefault.h>

/* Format time/date string */
void setTimestamp(unsigned long sec, unsigned long nsec, char *str) {
	DTStructure dtStruct;
	
	DT_TO_DTStructure(sec, (unsigned long)&dtStruct);
	
	IecPadNumber(dtStruct.year % 10000, str, 4, 0);
	str[4] = '-';
	IecPadNumber(dtStruct.month % 100, str + 5, 2, 0);
	str[7] = '-';
	IecPadNumber(dtStruct.day % 100, str + 8, 2, 0);
	str[10] = ' ';
	str[11] = '/';
	str[12] = ' ';
	IecPadNumber(dtStruct.hour % 100, str + 13, 2, 0);
	str[15] = ':';
	IecPadNumber(dtStruct.minute % 100, str + 16, 2, 0);
	str[18] = ':';
	IecPadNumber(dtStruct.second % 100, str + 19, 2, 0);
	str[21] = '.';
	
	dtStruct.millisec = (nsec % 1000000000) / 1000000;
	dtStruct.microsec = (nsec % 1000000) / 1000;
	IecPadNumber(dtStruct.millisec % 1000, str + 22, 3, 0);
	IecPadNumber(dtStruct.microsec % 1000, str + 25, 3, 0);
	str[28] = '\0';
}

/* Format sortable time data */
void setTimeBytes(unsigned long sec, unsigned long nsec, unsigned char *bytes) {
	const int val = 1;
	unsigned char i;
	/* 0-3 seconds 4-7 nanoseconds (big endian) */
	for(i = 0; i < 4; i++) { /* Access each byte of unsigned long, if little endian then rotate bytes */
		bytes[i] = *(((unsigned char *)&sec) + (*((unsigned char*)&val) ? 3 - i : i));
		bytes[4 + i] = *(((unsigned char *)&nsec) + (*((unsigned char*)&val) ? 3 - i : i));
	}
}
