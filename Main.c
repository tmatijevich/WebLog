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
void radixSort(unsigned char *in[], unsigned short idx[], unsigned char *sort[], unsigned short sortIdx[], unsigned short n, unsigned char k, unsigned char descending);

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
	for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
		strcpy(fbGetIdent.Name, logbook[l].ID); /* Destination size is 257 */
		fbGetIdent.Execute = true;
		ArEventLogGetIdent(&fbGetIdent);
		logbook[l].ident = fbGetIdent.Ident; /* Zero if error (arEVENTLOG_ERR_LOGBOOK_NOT_FOUND) */
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
	
	/* Search RECORD_MAX records in each LOGBOOK_MAX logbooks */
	if(refresh && !prevRefresh && state == 0) {
		memset(search, 0, sizeof(search)); /* Clear search memory */
		for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
			if(!logbook[l].ident) continue; /* Continue to next logbook */
			for(r = 0; r < WEBLOG_RECORD_MAX; r++) {
				/* Find record ID */
				if(r == 0) { /* Latest record */
					fbGetLatestRecord.Ident = logbook[l].ident;
					fbGetLatestRecord.Execute = true;
					ArEventLogGetLatestRecordID(&fbGetLatestRecord);
					if(fbGetLatestRecord.Done) 
						search[l][r].ID = fbGetLatestRecord.RecordID;
					/* Reset execution */
					fbGetLatestRecord.Execute = false;
					ArEventLogGetLatestRecordID(&fbGetLatestRecord);
				}
				else { /* Previous record */
					fbGetPreviousRecord.Ident = logbook[l].ident;
					fbGetPreviousRecord.RecordID = search[l][r - 1].ID;
					fbGetPreviousRecord.Execute = true;
					ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
					if(fbGetPreviousRecord.StatusID == ERR_OK) 
						search[l][r].ID = fbGetPreviousRecord.PrevRecordID;
					else if(fbGetPreviousRecord.StatusID == arEVENTLOG_ERR_RECORDID_INVALID) {
						fbGetPreviousRecord.Execute = false;
						ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
						break; /* End of records for this logbook */
					}
					/* Reset execution */
					fbGetPreviousRecord.Execute = false;
					ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
				}
				
				/* Find timestamp */
				fbReadRecord.Ident = logbook[l].ident;
				fbReadRecord.RecordID = search[l][r].ID;
				fbReadRecord.Execute = true;
				ArEventLogRead(&fbReadRecord);
				if(fbReadRecord.StatusID == ERR_OK || fbReadRecord.StatusID == arEVENTLOG_WRN_NO_EVENTID) {
					setTimeBytes((unsigned long)((long)fbReadRecord.TimeStamp.sec + utcToLocalOffset), fbReadRecord.TimeStamp.nsec, search[l][r].time);
					search[l][r].status |= 0x1; /* Record valid */
				}
				/* Reset execution */
				fbReadRecord.Execute = false;
				ArEventLogRead(&fbReadRecord);
				
			} /* Record loop */
		} /* Logbook loop */
		
		/* Sort through searched records */
		for(s = 0; s < WEBLOG_SORT_MAX; s++) {
			output[s] = input[s] = search[s / WEBLOG_RECORD_MAX][(WEBLOG_RECORD_MAX - 1) - s % WEBLOG_RECORD_MAX].time;
			si[s] = ai[s] = (s / WEBLOG_RECORD_MAX) * WEBLOG_RECORD_MAX + (WEBLOG_RECORD_MAX - 1) - s % WEBLOG_RECORD_MAX;
		}
		radixSort(input, ai, output, si, WEBLOG_SORT_MAX, WEBLOG_BYTE_MAX, 1);
		
	} /* Refresh command */
	
	for(d = d0; d < WEBLOG_RECORD_MAX; d++) {
		switch(state) {
			/* Client request */
			case 0:
				if(refresh && !prevRefresh) {
					memset(display, 0, sizeof(display));
					
					for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
						logbook[l].ri_a = UCHAR_MAX;
						logbook[l].ri_b = UCHAR_MAX;
					}
					
					state = 10;
					/* Do not break, proceed to next case */
				}
				else {
					d0 = 0;
					break;
				}
				
			/* EventID/ErrorNumber, Severity (Code, Facility), Timestamp */
			case 10:
				dl = si[d] / WEBLOG_RECORD_MAX;
				dr = si[d] % WEBLOG_RECORD_MAX;
				
				/* Check if valid */
				if((search[dl][dr].status & 1) != 1) {
					display[d].severity = 10; /* Give invalid severity to avoid displaying "Success" */
					if(d >= WEBLOG_RECORD_MAX - 1) {
						state = 201;
						break;
					}
					continue; /* Skip this record because it is invalid */
				}
				
				strncpy(display[d].logbook, logbook[dl].name, WEBLOG_STRLEN_LOGBOOK);
				display[d].logbook[WEBLOG_STRLEN_LOGBOOK] = '\0';
				
				display[d].ID = search[dl][dr].ID; /* Copy record ID */
				
				/* EventID + Severity +_Facility + Code or ErrorNumber + Severity */
				fbReadRecord.Ident = logbook[dl].ident;
				fbReadRecord.RecordID = display[d].ID;
				fbReadRecord.Execute = true;
				ArEventLogRead(&fbReadRecord);
				if(fbReadRecord.StatusID == ERR_OK) {
					display[d].event = fbReadRecord.EventID;
					display[d].severity = (unsigned char)((fbReadRecord.EventID >> 30) & 0x3);
					/*display[rd].facility = (unsigned short)((fbReadRecord.EventID >> 16) & 0xFFFF);*/
					/*display[rd].code = (unsigned short)((fbReadRecord.EventID) && 0xFFFF);*/
				}
				else if(fbReadRecord.StatusID == arEVENTLOG_WRN_NO_EVENTID) {
					fbReadErrorNumber.Ident = logbook[dl].ident;
					fbReadErrorNumber.RecordID = display[d].ID;
					fbReadErrorNumber.Execute = true;
					ArEventLogReadErrorNumber(&fbReadErrorNumber);
					if(fbReadErrorNumber.StatusID == ERR_OK) {
						display[d].errorNumber = fbReadErrorNumber.ErrorNumber;
						display[d].severity = fbReadErrorNumber.Severity;
					}
					fbReadErrorNumber.Execute = false;
					ArEventLogReadErrorNumber(&fbReadErrorNumber);
				}
				
				/* Timestamp */
				display[d].sec = fbReadRecord.TimeStamp.sec + utcToLocalOffset;
				display[d].nsec = fbReadRecord.TimeStamp.nsec;
				
				fbReadRecord.Execute = false;
				ArEventLogRead(&fbReadRecord);
				
				state = 20; /* Finished */
				/* Do not break case */
				
			/* Description */
			case 20:
				fbReadDescription.Ident = logbook[dl].ident;
				fbReadDescription.RecordID = display[d].ID;
				fbReadDescription.TextBuffer = (unsigned long)display[d].description;
				fbReadDescription.TextBufferSize = sizeof(display[d].description);
				fbReadDescription.Execute = true;
				ArEventLogReadDescription(&fbReadDescription);
				if(fbReadDescription.Busy) {
					d0 = d;
					break; /* Break and then return to state 20 */
				}
				fbReadDescription.Execute = false;
				ArEventLogReadDescription(&fbReadDescription);
				replaceQuotes(display[d].description);
				state = 30;
				/* Do not break case */
			
			/* Ascii data and object ID */
			case 30:
				fbReadAsciiData.Ident = logbook[dl].ident;
				fbReadAsciiData.RecordID = display[d].ID;
				fbReadAsciiData.AddData = (unsigned long)display[d].asciiData;
				fbReadAsciiData.BytesToRead = sizeof(display[d].asciiData) - 1;
				fbReadAsciiData.Execute = true;
				ArEventLogReadAddData(&fbReadAsciiData);
				fbReadAsciiData.Execute = false;
				ArEventLogReadAddData(&fbReadAsciiData);
				replaceQuotes(display[d].asciiData);
				
				fbReadObjectName.Ident = logbook[dl].ident;
				fbReadObjectName.RecordID = display[d].ID;
				fbReadObjectName.Execute = true;
				ArEventLogReadObjectID(&fbReadObjectName);
				if(fbReadObjectName.StatusID == ERR_OK) {
					strncpy(display[d].object, fbReadObjectName.ObjectID, WEBLOG_STRLEN_LOGBOOK);
					display[d].object[WEBLOG_STRLEN_LOGBOOK] = '\0';
				}
				fbReadObjectName.Execute = false;
				ArEventLogReadObjectID(&fbReadObjectName);
				
				state = 200;
				/* Do not break case */
			
			/* Record done */
			case 200:
				if(logbook[dl].ri_a == UCHAR_MAX) logbook[dl].ri_a = dr; /* Newest (timestamp) record displayed */
				logbook[dl].ri_b = dr; /* Oldest (timestamp) record displayed */
				
				if(d < WEBLOG_RECORD_MAX - 1) {
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
