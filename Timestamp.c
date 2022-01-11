
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

void setTimestamp(unsigned long sec, unsigned long nsec, char *str) {
	DTStructure dtStruct;
	
	DT_TO_DTStructure(sec, (unsigned long)&dtStruct);
	
	IecPadNumber(dtStruct.year % 10000, str, 4, 0);
	str[4] = '-';
	IecPadNumber(dtStruct.month % 100, str + 5, 2, 0);
	str[7] = '-';
	IecPadNumber(dtStruct.day % 100, str + 8, 2, 0);
	str[10] = ' ';
	IecPadNumber(dtStruct.hour % 100, str + 11, 2, 0);
	str[13] = ':';
	IecPadNumber(dtStruct.minute % 100, str + 14, 2, 0);
	str[16] = ':';
	IecPadNumber(dtStruct.second % 100, str + 17, 2, 0);
	str[19] = ',';
	
	dtStruct.millisec = (nsec % 1000000000) / 1000000;
	dtStruct.microsec = (nsec % 1000000) / 1000;
	IecPadNumber(dtStruct.millisec % 1000, str + 20, 3, 0);
	IecPadNumber(dtStruct.microsec % 1000, str + 23, 3, 0);
	str[26] = '\0';
}

void setTimeBytes(unsigned long sec, unsigned long nsec, unsigned char *bytes) {
	const int val = 1;
	if(*((unsigned char*)&val)) {
		*(bytes + 0) = *(((unsigned char *)&sec) + 3);
		*(bytes + 1) = *(((unsigned char *)&sec) + 2);
		*(bytes + 2) = *(((unsigned char *)&sec) + 1);
		*(bytes + 3) = *(((unsigned char *)&sec) + 0);
		*(bytes + 4) = *(((unsigned char *)&nsec) + 3);
		*(bytes + 5) = *(((unsigned char *)&nsec) + 2);
		*(bytes + 6) = *(((unsigned char *)&nsec) + 1);
		*(bytes + 7) = *(((unsigned char *)&nsec) + 0);
	}
	else {
		*(bytes + 0) = *(((unsigned char *)&sec) + 0);
		*(bytes + 1) = *(((unsigned char *)&sec) + 1);
		*(bytes + 2) = *(((unsigned char *)&sec) + 2);
		*(bytes + 3) = *(((unsigned char *)&sec) + 3);
		*(bytes + 4) = *(((unsigned char *)&nsec) + 0);
		*(bytes + 5) = *(((unsigned char *)&nsec) + 1);
		*(bytes + 6) = *(((unsigned char *)&nsec) + 2);
		*(bytes + 7) = *(((unsigned char *)&nsec) + 3);
	}
}