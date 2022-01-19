/*******************************************************************************
 * File:      WebLog\Main.c
 * Author:    Tyler Matijevich
 * Created:   2022-01-05
********************************************************************************
 * Use ArEventLog to read, collect, and sort logbook records to display on 
   web page
*******************************************************************************/ 

#define _REPLACE_CONST /* Replace PLC constants with define macros instead of const storage modifier */
#include <AsDefault.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

/* Function prototypes */
void setTimeBytes(unsigned long sec, unsigned long nsec, unsigned char *bytes);
void replaceQuotes(char *str);
void radixSort(unsigned char *arr[], unsigned short a[], unsigned short n, unsigned char k, unsigned char *sort[], unsigned short s[]);

/* Program initialization routine */
void _INIT ProgramInit(void) {
	
	/* Initialize (system) logbook identifiers and their associated names */
	strcpy(logbook[0].ID, "$arlogsys"); 	strcpy(logbook[0].name, "System");
	strcpy(logbook[1].ID, "$fieldbus"); 	strcpy(logbook[1].name, "Fieldbus");
	strcpy(logbook[2].ID, "$arlogconn"); 	strcpy(logbook[2].name, "Connectivity");
	strcpy(logbook[3].ID, "$textsys"); 		strcpy(logbook[3].name, "Text System");
	strcpy(logbook[4].ID, "$accsec"); 		strcpy(logbook[4].name, "Access & Security");
	strcpy(logbook[5].ID, "$visu"); 		strcpy(logbook[5].name, "Visualization");
	strcpy(logbook[6].ID, "$firewall"); 	strcpy(logbook[6].name, "Firewall");
	strcpy(logbook[7].ID, "$versinfo"); 	strcpy(logbook[7].name, "Version Info");
	strcpy(logbook[8].ID, "$diag"); 		strcpy(logbook[8].name, "Diagnostics");
	strcpy(logbook[9].ID, "$arlogusr"); 	strcpy(logbook[9].name, "User");
	
	/* Local time change */
	fbGetLocalTime.enable = true;
	DTGetTime(&fbGetLocalTime);
	
	fbGetUtcTime.enable = true;
	UtcDTGetTime(&fbGetUtcTime);
	
	utcToLocalOffset = ((long)(fbGetLocalTime.DT1 - fbGetUtcTime.DT1) / 3600L) * 3600L;
	
	/* Get idents of all logbooks */
	for(li = 0; li < WEBLOG_LOGBOOK_MAX; li++) {
		strcpy(fbGetIdent.Name, logbook[li].ID); /* Destination size is 257 */
		fbGetIdent.Execute = true;
		ArEventLogGetIdent(&fbGetIdent);
		logbook[li].ident = fbGetIdent.Ident; /* Zero if error (arEVENTLOG_ERR_LOGBOOK_NOT_FOUND) */
		fbGetIdent.Execute = false;
		ArEventLogGetIdent(&fbGetIdent);
	}
	
}

/* Main cyclic routine */
void _CYCLIC ProgramCyclic(void)
{	
	/* Declare local variables */
	unsigned char *input[WEBLOG_SORT_MAX], *output[WEBLOG_SORT_MAX];
	//unsigned short ai[WEBLOG_SORT_MAX], si[WEBLOG_SORT_MAX];
	unsigned long i;
	
	/* Search RECORD_MAX records in each LOGBOOK_MAX logbooks */
	if(refresh && !prevRefresh) {
		memset(search, 0, sizeof(search)); /* Clear search memory */
		for(li = 0; li < WEBLOG_LOGBOOK_MAX; li++) {
			if(!logbook[li].ident) continue; /* Continue to next logbook */
			for(ri = 0; ri < WEBLOG_RECORD_MAX; ri++) {
				/* Find record ID */
				if(ri == 0) { /* Latest record */
					fbGetLatestRecord.Ident = logbook[li].ident;
					fbGetLatestRecord.Execute = true;
					ArEventLogGetLatestRecordID(&fbGetLatestRecord);
					if(fbGetLatestRecord.Done) 
						search[li][ri].ID = fbGetLatestRecord.RecordID;
					/* Reset execution */
					fbGetLatestRecord.Execute = false;
					ArEventLogGetLatestRecordID(&fbGetLatestRecord);
				}
				else { /* Previous record */
					fbGetPreviousRecord.Ident = logbook[li].ident;
					fbGetPreviousRecord.RecordID = search[li][ri - 1].ID;
					fbGetPreviousRecord.Execute = true;
					ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
					if(fbGetPreviousRecord.StatusID == ERR_OK) 
						search[li][ri].ID = fbGetPreviousRecord.PrevRecordID;
					else if(fbGetPreviousRecord.StatusID == arEVENTLOG_ERR_RECORDID_INVALID) {
						fbGetPreviousRecord.Execute = false;
						ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
						r0[li] = ri; /* Store initial record */
						break; /* End of records for this logbook */
					}
					/* Reset execution */
					fbGetPreviousRecord.Execute = false;
					ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
				}
				
				/* Find timestamp */
				fbReadRecord.Ident = logbook[li].ident;
				fbReadRecord.RecordID = search[li][ri].ID;
				fbReadRecord.Execute = true;
				ArEventLogRead(&fbReadRecord);
				if(fbReadRecord.StatusID == ERR_OK || fbReadRecord.StatusID == arEVENTLOG_WRN_NO_EVENTID) {
					setTimeBytes((unsigned long)((long)fbReadRecord.TimeStamp.sec + utcToLocalOffset), fbReadRecord.TimeStamp.nsec, search[li][ri].time);
					search[li][ri].status |= 0x1; /* Record valid */
				}
				/* Reset execution */
				fbReadRecord.Execute = false;
				ArEventLogRead(&fbReadRecord);
				
			} /* Record loop */
		} /* Logbook loop */
		
		/* Sort through searched records */
		for(i = 0; i < WEBLOG_SORT_MAX; i++) {
			output[i] = input[i] = search[i / WEBLOG_RECORD_MAX][(WEBLOG_RECORD_MAX - 1) - i % WEBLOG_RECORD_MAX].time;
			si[i] = ai[i] = (i / WEBLOG_RECORD_MAX) * WEBLOG_RECORD_MAX + (WEBLOG_RECORD_MAX - 1) - i % WEBLOG_RECORD_MAX;
		}
		radixSort(input, ai, WEBLOG_SORT_MAX, WEBLOG_BYTE_MAX, output, si);
		
	} /* Refresh command */
	
	for(rd = d0; rd < WEBLOG_RECORD_MAX; rd++) {
		switch(state) {
			/* Client request */
			case 0:
				if(refresh && !prevRefresh) {
					memset(display, 0, sizeof(display));
					state = 10;
					/* Do not break, proceed to next case */
				}
				else {
					d0 = 0;
					break;
				}
				
			/* EventID/ErrorNumber, Severity (Code, Facility), Timestamp */
			case 10:
				ld = si[rd] / WEBLOG_RECORD_MAX;
				
				/* Check if valid */
				if((search[ld][si[rd] % WEBLOG_RECORD_MAX].status & 1) != 1) {
					display[rd].severity = 10; /* Give invalid severity to avoid displaying "Success" */
					if(rd >= WEBLOG_RECORD_MAX - 1) {
						state = 201;
						break;
					}
					continue; /* Skip this record because it is invalid */
				}
				
				strncpy(display[rd].logbook, logbook[ld].name, WEBLOG_STRLEN_LOGBOOK);
				display[rd].logbook[WEBLOG_STRLEN_LOGBOOK] = '\0';
				
				display[rd].ID = search[ld][si[rd] % WEBLOG_RECORD_MAX].ID; /* Copy record ID */
				
				/* EventID + Severity +_Facility + Code or ErrorNumber + Severity */
				fbReadRecord.Ident = logbook[ld].ident;
				fbReadRecord.RecordID = display[rd].ID;
				fbReadRecord.Execute = true;
				ArEventLogRead(&fbReadRecord);
				if(fbReadRecord.StatusID == ERR_OK) {
					display[rd].event = fbReadRecord.EventID;
					display[rd].severity = (unsigned char)((fbReadRecord.EventID >> 30) & 0x3);
					/*display[rd].facility = (unsigned short)((fbReadRecord.EventID >> 16) & 0xFFFF);*/
					/*display[rd].code = (unsigned short)((fbReadRecord.EventID) && 0xFFFF);*/
				}
				else if(fbReadRecord.StatusID == arEVENTLOG_WRN_NO_EVENTID) {
					fbReadErrorNumber.Ident = logbook[ld].ident;
					fbReadErrorNumber.RecordID = display[rd].ID;
					fbReadErrorNumber.Execute = true;
					ArEventLogReadErrorNumber(&fbReadErrorNumber);
					if(fbReadErrorNumber.StatusID == ERR_OK) {
						display[rd].errorNumber = fbReadErrorNumber.ErrorNumber;
						display[rd].severity = fbReadErrorNumber.Severity;
					}
					fbReadErrorNumber.Execute = false;
					ArEventLogReadErrorNumber(&fbReadErrorNumber);
				}
				
				/* Timestamp */
				/* setTimestamp((unsigned long)((long)fbReadRecord.TimeStamp.sec + utcToLocalOffset), fbReadRecord.TimeStamp.nsec, display[rd].timestamp); */
				display[rd].sec = fbReadRecord.TimeStamp.sec;
				display[rd].nsec = fbReadRecord.TimeStamp.nsec;
				
				fbReadRecord.Execute = false;
				ArEventLogRead(&fbReadRecord);
				
				state = 20; /* Finished */
				/* Do not break case */
				
			/* Description */
			case 20:
				fbReadDescription.Ident = logbook[ld].ident;
				fbReadDescription.RecordID = display[rd].ID;
				fbReadDescription.TextBuffer = (unsigned long)display[rd].description;
				fbReadDescription.TextBufferSize = sizeof(display[rd].description);
				fbReadDescription.Execute = true;
				ArEventLogReadDescription(&fbReadDescription);
				if(fbReadDescription.Busy) {
					d0 = rd;
					break; /* Break and then return to state 20 */
				}
				fbReadDescription.Execute = false;
				ArEventLogReadDescription(&fbReadDescription);
				replaceQuotes(display[rd].description);
				state = 30;
				/* Do not break case */
			
			/* Ascii data and object ID */
			case 30:
				fbReadAsciiData.Ident = logbook[ld].ident;
				fbReadAsciiData.RecordID = display[rd].ID;
				fbReadAsciiData.AddData = (unsigned long)display[rd].asciiData;
				fbReadAsciiData.BytesToRead = sizeof(display[rd].asciiData) - 1;
				fbReadAsciiData.Execute = true;
				ArEventLogReadAddData(&fbReadAsciiData);
				fbReadAsciiData.Execute = false;
				ArEventLogReadAddData(&fbReadAsciiData);
				replaceQuotes(display[rd].asciiData);
				
				fbReadObjectName.Ident = logbook[ld].ident;
				fbReadObjectName.RecordID = display[rd].ID;
				fbReadObjectName.Execute = true;
				ArEventLogReadObjectID(&fbReadObjectName);
				if(fbReadObjectName.StatusID == ERR_OK) {
					strncpy(display[rd].object, fbReadObjectName.ObjectID, WEBLOG_STRLEN_LOGBOOK);
					display[rd].object[WEBLOG_STRLEN_LOGBOOK] = '\0';
				}
				fbReadObjectName.Execute = false;
				ArEventLogReadObjectID(&fbReadObjectName);
				
				state = 200;
				/* Do not break case */
			
			/* Record done */
			case 200:
				if(rd < WEBLOG_RECORD_MAX - 1) {
					state = 10;
					continue; /* Next record index */
				}
				state = 201;
				/* Do not break case */
				
			/* Done */
			case 201:
				done = true;
				if(!refresh) {
					done = false;
					state = 0;
				}
				break;
				
			case 255:
				break;
		}
		break; /* Break record loop */
	}
	
}

void _EXIT ProgramExit(void)
{

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


/* Replace quotes which are invalid in JSON value */
void replaceQuotes(char *str) {
	unsigned char i = 0;
	while(str[i] && i < UCHAR_MAX) {
		if(str[i] == '"') str[i] = ' ';
		i++;
	}
}
