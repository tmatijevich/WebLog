
#include <bur/plctypes.h>
#include <string.h>
#include <stdbool.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

void _INIT ProgramInit(void)
{

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
}

void _EXIT ProgramExit(void)
{

}
