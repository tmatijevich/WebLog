
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#include <string.h>
#include <stdbool.h>

void MSB_radixSort(unsigned char *arr[], unsigned int n, const unsigned char k, unsigned char *sort[]);

void _INIT ProgramInit(void)
{
	unsortedList[0] = 65468;
	unsortedList[1] = 21635;
	unsortedList[2] = 560;
	unsortedList[3] = 231;
	unsortedList[4] = 56165;
	unsortedList[5] = 5610;
	unsortedList[6] = 11023;
	unsortedList[7] = 5156;
	unsortedList[8] = 53032;
	unsortedList[9] = 41312;
}

void _CYCLIC ProgramCyclic(void)
{
	if(test) {
		test = false;
		
		strcpy(fbGetIdent.Name, "$arlogsys");
		fbGetIdent.Execute 	= true;
		ArEventLogGetIdent(&fbGetIdent);
		
		fbGetLatestRecordID.Ident = fbGetIdent.Ident;
		fbGetLatestRecordID.Execute = true;
		ArEventLogGetLatestRecordID(&fbGetLatestRecordID);
		sampleRecord.ID = fbGetLatestRecordID.RecordID;
		
		fbReadRecord.Ident 		= fbGetIdent.Ident;
		fbReadRecord.RecordID 	= sampleRecord.ID;
		fbReadRecord.Execute 	= true;
		ArEventLogRead(&fbReadRecord);
		sampleRecord.EventID = fbReadRecord.EventID;
		
		utcConversion.DT1 = fbReadRecord.TimeStamp.sec;
		utcConversion.pDTStructure = (unsigned long)&dateTimeStructure;
		utcConversion.enable = true;
		UtcDT_TO_LocalDTStructure(&utcConversion);
		dateTimeStructure.millisec = (fbReadRecord.TimeStamp.nsec % 1000000000) / 1000000;
		dateTimeStructure.microsec = (fbReadRecord.TimeStamp.nsec % 1000000) / 1000;
		
		/* YYYY-MM-DD HH:MM:SS,UUUUUU */
		IecPadNumber(dateTimeStructure.year % 10000, sampleRecord.Timestamp, 4, 0);
		sampleRecord.Timestamp[4] = '-';
		IecPadNumber(dateTimeStructure.month % 100, sampleRecord.Timestamp + 5, 2, 0);
		sampleRecord.Timestamp[7] = '-';
		IecPadNumber(dateTimeStructure.day % 100, sampleRecord.Timestamp + 8, 2, 0);
		sampleRecord.Timestamp[10] = ' ';
		IecPadNumber(dateTimeStructure.hour % 100, sampleRecord.Timestamp + 11, 2, 0);
		sampleRecord.Timestamp[13] = ':';
		IecPadNumber(dateTimeStructure.minute % 100, sampleRecord.Timestamp + 14, 2, 0);
		sampleRecord.Timestamp[16] = ':';
		IecPadNumber(dateTimeStructure.second % 100, sampleRecord.Timestamp + 17, 2, 0);
		sampleRecord.Timestamp[19] = ',';
		IecPadNumber(dateTimeStructure.millisec % 1000, sampleRecord.Timestamp + 20, 3, 0);
		IecPadNumber(dateTimeStructure.microsec % 1000, sampleRecord.Timestamp + 23, 3, 0);
		sampleRecord.Timestamp[26] = '\0';
		
		memcpy(sampleRecord.TimeBytes, &fbReadRecord.TimeStamp.sec, 4);
		memcpy(sampleRecord.TimeBytes + 4, &fbReadRecord.TimeStamp.nsec, 4);
		sampleRecord.TimeBytes[0] ^= 0x80; /* Flip the first sign bit */
		
		fbReadAddnData.Ident 		= fbGetIdent.Ident;
		fbReadAddnData.RecordID 	= sampleRecord.ID;
		fbReadAddnData.AddData 		= (unsigned long)sampleRecord.AdditionalData;
		fbReadAddnData.BytesToRead 	= sizeof(sampleRecord.AdditionalData);
		fbReadAddnData.Execute 		= true;
		ArEventLogReadAddData(&fbReadAddnData);
		
		fbReadObjectID.Ident 		= fbGetIdent.Ident;
		fbReadObjectID.RecordID 	= sampleRecord.ID;
		fbReadObjectID.Execute 		= true;
		ArEventLogReadObjectID(&fbReadObjectID);
		strncpy(sampleRecord.ObjectID, fbReadObjectID.ObjectID, 36);
		sampleRecord.ObjectID[36] = '\0';
	}
	
	// Wait for refresh request
	
	// Refresh latest x entries for y logbooks
	
	// Wait for refresh to complete
	
	// Sort x*y entries to get latest x entries of all logbooks
	if(runSort) {
		runSort = false;
		//memcpy(unsortedData, unsortedList, sizeof(unsortedData));
		for(i = 0; i < 10; i++) {
			unsortedAdr = (unsigned char *)&unsortedList[i];
			unsortedData[i][1] = *unsortedAdr;
			unsortedData[i][0] = *(unsortedAdr + 1);
			inputArray[i] = (unsigned long)&unsortedData[i][0];
			outputArray[i] = (unsigned long)&sortedData[i][0];
		}
		MSB_radixSort((unsigned char**)inputArray, 10, 2, (unsigned char**)outputArray);
		for(i = 0; i < 10; i++) {
			sortedData[i][0] = *((unsigned char *)outputArray[i] + 1);
			sortedData[i][1] = *((unsigned char *)outputArray[i]);
//			for(j = 0; j < 2; j++)
//				sortedData[i][j] = *((unsigned char*)outputArray[i] + j);
		}
		memcpy(sortedList, sortedData, sizeof(sortedList));
	}
	
}

void _EXIT ProgramExit(void)
{

}

