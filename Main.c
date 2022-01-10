
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#include <string.h>
#include <stdbool.h>

#define RECORD_MAX 20
#define LOGBOOK_MAX 10
#define STRLEN_LOGBOOK 36

/* Function prototypes */
//void MSB_radixSort(unsigned char *arr[], unsigned int n, const unsigned char k, unsigned char *sort[]);

/* Global variables */
char logbookID[LOGBOOK_MAX][11];
char logbookName[LOGBOOK_MAX][STRLEN_LOGBOOK];

void _INIT ProgramInit(void)
{
	strcpy(logbookID[0], "$arlogsys"); 		strcpy(logbookName[0], "System");
	strcpy(logbookID[1], "$fieldbus"); 		strcpy(logbookName[1], "Fieldbus");
	strcpy(logbookID[2], "$arlogconn"); 	strcpy(logbookName[2], "Connectivity");
	strcpy(logbookID[3], "$textsys"); 		strcpy(logbookName[3], "Text System");
	strcpy(logbookID[4], "$accsec"); 		strcpy(logbookName[4], "Access & Security");
	strcpy(logbookID[5], "$visu"); 			strcpy(logbookName[5], "Visualization");
	strcpy(logbookID[6], "$firewall"); 		strcpy(logbookName[6], "Firewall");
	strcpy(logbookID[7], "$versinfo"); 		strcpy(logbookName[7], "Version Info");
	strcpy(logbookID[8], "$diag"); 			strcpy(logbookName[8], "Diagnostics");
	strcpy(logbookID[9], "$arlogusr"); 		strcpy(logbookName[9], "User");
}

void _CYCLIC ProgramCyclic(void)
{
	/* Declare local variables */
	static WebLoggerRecordType record[LOGBOOK_MAX][RECORD_MAX]; 			/* x records for all y logbooks */
	static unsigned char prevRefresh; 										/* Detect rising edge of refresh command */
	static unsigned char state;
	const int val = 1;
	const unsigned char littleEndian = *((unsigned char*)&val);
	static long utcToLocalOffset;
	static DTGetTime_typ fbGetLocalTime;
	static UtcDTGetTime_typ fbGetUtcTime;
	
	static ArEventLogGetIdent_typ fbGetIdent; 								/* Synchronous execution */
	static ArEventLogGetLatestRecordID_typ fbGetLatestRecord;
	static ArEventLogGetPreviousRecordID_typ fbGetPreviousRecord;
	static ArEventLogRead_typ fbReadRecord;
	static ArEventLogReadDescription_typ fbReadDescription[LOGBOOK_MAX]; 	/* Asynchronous execution */
	static ArEventLogReadAddData_typ fbReadAsciiData;
	static ArEventLogReadObjectID_typ fbReadObjectName;
	static unsigned char lstate[LOGBOOK_MAX], li, ri; 						/* Logbook and record index */
	
	switch(state) {
		case 0:
			fbGetLocalTime.enable = true;
			DTGetTime(&fbGetLocalTime);
			
			fbGetUtcTime.enable = true;
			UtcDTGetTime(&fbGetUtcTime);
			
			if(fbGetLocalTime.status == ERR_OK && fbGetUtcTime.status == ERR_OK) {
				utcToLocalOffset = ((long)(fbGetLocalTime.DT1 - fbGetUtcTime.DT1) / 3600L) * 3600L;
				state = 10;
			}
			else
				state = 255;
				
			break;
			
		case 10:
			break;
		case 20:
		case 30:
		case 255:
			break;
	}
	
	for(li = 0; li < WEBLOG_LOGBOOK_MAX; li++) {
		switch(lstate[li]) {
			case 0:
				if(refresh && !prevRefresh && state == 10)
					lstate[li] = 10;
				else
					break;
				
			case 10:
				
				
			case 20:
			case 30:
			case 255:
				break;
		}
	}
	prevRefresh = refresh;
	
//	if(test) {
//		test = false;
//		
//		strcpy(fbGetIdent.Name, "$arlogsys");
//		fbGetIdent.Execute 	= true;
//		ArEventLogGetIdent(&fbGetIdent);
//		
//		fbGetLatestRecordID.Ident = fbGetIdent.Ident;
//		fbGetLatestRecordID.Execute = true;
//		ArEventLogGetLatestRecordID(&fbGetLatestRecordID);
//		sampleRecord.ID = fbGetLatestRecordID.RecordID;
//		
//		fbReadRecord.Ident 		= fbGetIdent.Ident;
//		fbReadRecord.RecordID 	= sampleRecord.ID;
//		fbReadRecord.Execute 	= true;
//		ArEventLogRead(&fbReadRecord);
//		sampleRecord.EventID = fbReadRecord.EventID;
//		
//		utcConversion.DT1 = fbReadRecord.TimeStamp.sec;
//		utcConversion.pDTStructure = (unsigned long)&dateTimeStructure;
//		utcConversion.enable = true;
//		UtcDT_TO_LocalDTStructure(&utcConversion);
//		dateTimeStructure.millisec = (fbReadRecord.TimeStamp.nsec % 1000000000) / 1000000;
//		dateTimeStructure.microsec = (fbReadRecord.TimeStamp.nsec % 1000000) / 1000;
//		
//		/* YYYY-MM-DD HH:MM:SS,UUUUUU */
//		IecPadNumber(dateTimeStructure.year % 10000, sampleRecord.Timestamp, 4, 0);
//		sampleRecord.Timestamp[4] = '-';
//		IecPadNumber(dateTimeStructure.month % 100, sampleRecord.Timestamp + 5, 2, 0);
//		sampleRecord.Timestamp[7] = '-';
//		IecPadNumber(dateTimeStructure.day % 100, sampleRecord.Timestamp + 8, 2, 0);
//		sampleRecord.Timestamp[10] = ' ';
//		IecPadNumber(dateTimeStructure.hour % 100, sampleRecord.Timestamp + 11, 2, 0);
//		sampleRecord.Timestamp[13] = ':';
//		IecPadNumber(dateTimeStructure.minute % 100, sampleRecord.Timestamp + 14, 2, 0);
//		sampleRecord.Timestamp[16] = ':';
//		IecPadNumber(dateTimeStructure.second % 100, sampleRecord.Timestamp + 17, 2, 0);
//		sampleRecord.Timestamp[19] = ',';
//		IecPadNumber(dateTimeStructure.millisec % 1000, sampleRecord.Timestamp + 20, 3, 0);
//		IecPadNumber(dateTimeStructure.microsec % 1000, sampleRecord.Timestamp + 23, 3, 0);
//		sampleRecord.Timestamp[26] = '\0';
//		
//		memcpy(sampleRecord.TimeBytes, &fbReadRecord.TimeStamp.sec, 4);
//		memcpy(sampleRecord.TimeBytes + 4, &fbReadRecord.TimeStamp.nsec, 4);
//		sampleRecord.TimeBytes[0] ^= 0x80; /* Flip the first sign bit */
//		
//		fbReadAddnData.Ident 		= fbGetIdent.Ident;
//		fbReadAddnData.RecordID 	= sampleRecord.ID;
//		fbReadAddnData.AddData 		= (unsigned long)sampleRecord.AdditionalData;
//		fbReadAddnData.BytesToRead 	= sizeof(sampleRecord.AdditionalData);
//		fbReadAddnData.Execute 		= true;
//		ArEventLogReadAddData(&fbReadAddnData);
//		
//		fbReadObjectID.Ident 		= fbGetIdent.Ident;
//		fbReadObjectID.RecordID 	= sampleRecord.ID;
//		fbReadObjectID.Execute 		= true;
//		ArEventLogReadObjectID(&fbReadObjectID);
//		strncpy(sampleRecord.ObjectID, fbReadObjectID.ObjectID, 36);
//		sampleRecord.ObjectID[36] = '\0';
//	}
//	
//	// Wait for refresh request or advance/return requests
//	
//	// Refresh latest x entries for y logbooks
//	
//	// Wait for refresh to complete
//	
//	// Sort x*y entries to get latest x entries of all logbooks
//	if(runSort) {
//		runSort = false;
//		//memcpy(unsortedData, unsortedList, sizeof(unsortedData));
//		for(i = 0; i < 10; i++) {
//			unsortedAdr = (unsigned char *)&unsortedList[i];
//			unsortedData[i][1] = *unsortedAdr;
//			unsortedData[i][0] = *(unsortedAdr + 1);
//			inputArray[i] = (unsigned long)&unsortedData[i][0];
//			outputArray[i] = (unsigned long)&sortedData[i][0];
//		}
//		MSB_radixSort((unsigned char**)inputArray, 10, 2, (unsigned char**)outputArray);
//		for(i = 0; i < 10; i++) {
//			sortedData[i][0] = *((unsigned char *)outputArray[i] + 1);
//			sortedData[i][1] = *((unsigned char *)outputArray[i]);
////			for(j = 0; j < 2; j++)
////				sortedData[i][j] = *((unsigned char*)outputArray[i] + j);
//		}
//		memcpy(sortedList, sortedData, sizeof(sortedList));
//	}
	
}

void _EXIT ProgramExit(void)
{

}

