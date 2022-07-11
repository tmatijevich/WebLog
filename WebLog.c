/*******************************************************************************
 * File:      WebLog\WebLog.c
 * Author:    Tyler Matijevich
 * Created:   2022-01-05
********************************************************************************
 * Description: Use ArEventLog to read, collect, and sort logbook records for
   display on web page
*******************************************************************************/

#include "WebLog.h"

/* Variable declaration */
_LOCAL unsigned char refresh, refreshed, down, up, done, valid, previousRefresh, previousDown, previousUp, previousValid;
_LOCAL long l, r, s, d, d0, dl, dr, searchCount, searchOffset, displayCount, displayOffset;
_LOCAL unsigned char state;
_LOCAL struct weblog_display_typ display[WEBLOG_RECORD_MAX];
unsigned short unsortedIndices[WEBLOG_SORT_MAX], sortedIndices[WEBLOG_SORT_MAX];
unsigned char *unsortedBytes[WEBLOG_SORT_MAX], *sortedBytes[WEBLOG_SORT_MAX];
_LOCAL enum webLogCommandEnum command, previousCommand;
_LOCAL struct webLogBookType book[WEBLOG_LOGBOOK_MAX];
_LOCAL struct webLogRecordSearchType record[WEBLOG_LOGBOOK_MAX][WEBLOG_RECORD_MAX];
unsigned char zeroTime[WEBLOG_BYTE_MAX];

/* Function block instances */
ArEventLogGetIdent_typ fbGetIdent;
ArEventLogGetLatestRecordID_typ fbGetLatestRecord;
ArEventLogGetPreviousRecordID_typ fbGetPreviousRecord;
ArEventLogRead_typ fbReadRecord;
ArEventLogReadErrorNumber_typ fbReadErrorNumber;
ArEventLogReadDescription_typ fbReadDescription;
ArEventLogReadAddData_typ fbReadAsciiData;
ArEventLogReadObjectID_typ fbReadObjectName;

/* Initialization routine */
void _INIT program_init(void) {
	
	/* Initialize (system) logbook identifiers and their associated names */
	strcpy(book[0].name, "$arlogsys"); 	strcpy(book[0].description, "System");
	strcpy(book[1].name, "$arlogusr"); 	strcpy(book[1].description, "User");
//	strcpy(book[1].name, "$fieldbus"); 	strcpy(book[1].description, "Fieldbus");
//	strcpy(book[2].name, "$arlogconn"); 	strcpy(book[2].description, "Connectivity");
//	strcpy(book[3].name, "$textsys"); 	strcpy(book[3].description, "Text System");
//	strcpy(book[4].name, "$accsec"); 	strcpy(book[4].description, "Access & Security");
//	strcpy(book[5].name, "$visu"); 		strcpy(book[5].description, "Visualization");
//	strcpy(book[6].name, "$firewall"); 	strcpy(book[6].description, "Firewall");
//	strcpy(book[7].name, "$versinfo"); 	strcpy(book[7].description, "Version Info");
//	strcpy(book[8].name, "$diag"); 		strcpy(book[8].description, "Diagnostics");
//	strcpy(book[9].name, "$arlogusr"); 	strcpy(book[9].description, "User");
	
	/* Get idents of all logbooks */
	for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
		strcpy(fbGetIdent.Name, book[l].name); /* Destination size is 257 */
		fbGetIdent.Execute = true;
		ArEventLogGetIdent(&fbGetIdent);
		book[l].ident = fbGetIdent.Ident; /* Zero if error (arEVENTLOG_ERR_LOGBOOK_NOT_FOUND) */
		fbGetIdent.Execute = false;
		ArEventLogGetIdent(&fbGetIdent);
	}
	
	refreshed = false;
	displayOffset = 0;
	displayCount = 0;
	memset(zeroTime, 0, sizeof(zeroTime));
}

/* Cyclic routine */
void _CYCLIC program_cyclic(void)
{	
	/* Accept user command */
	if(state == 0) {
		if(refresh && !previousRefresh)
			command = WEBLOG_COMMAND_REFRESH;
		else if(down && !previousDown && refreshed)
			command = WEBLOG_COMMAND_DOWN;
		else if(up && !previousUp && refreshed)
			command = WEBLOG_COMMAND_UP;
		else
			command = WEBLOG_COMMAND_NONE; /* Zero */
	}
	else
		command = WEBLOG_COMMAND_NONE; /* Zero */
	
	/* Prepare Search Parameters */
	switch(command) {
		case WEBLOG_COMMAND_REFRESH:
			for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
				book[l].search.skip = false;
				book[l].search.startID = 0;
				book[l].search.nextID = 0;
				book[l].search.stopID = 0;
			}
			break;
			
		case WEBLOG_COMMAND_DOWN:
			/* Search past oldest displayed record */
			
			if(previousCommand == WEBLOG_COMMAND_REFRESH || previousCommand == WEBLOG_COMMAND_DOWN) {
				/* Newest searched records were displayed */
				
				for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
					/* 1. Empty search */
					if(book[l].search.newestID == 0) {
						book[l].search.skip = true;
					}
					
					/* 2. Nothing displayed */
					else if(book[l].display.newestID == 0) {
						book[l].search.skip = false;
						book[l].search.startID = book[l].search.newestID;
						book[l].search.stopID = 0;
					}
					
					/* 3. Oldest displayed record ID is 1 */
					else if(book[l].display.oldestID == 1) {
						book[l].search.skip = true;
					}
					
					/* 4. Start search after the oldest displayed */
					else {
						book[l].search.skip = false;
						book[l].search.startID = 0;
						book[l].search.nextID = book[l].display.oldestID;
						book[l].search.stopID = 0;
					}
					
				} /* Loop logbooks */
			} /* Continue down */
			
			else { /* Previously up */
				/* Oldest searched records were displayed */
				
				for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
					/* 1. Empty search */
					if(book[l].search.newestID == 0) {
						if(book[l].latestID) {
							book[l].search.skip = false;
							book[l].search.startID = book[l].latestID;
							book[l].search.stopID = 0;
						}
						else {
							book[l].search.skip = true;
						}
					}
					
					/* 2. Oldest searched record ID is 1 */
					else if(book[l].search.oldestID == 1) {
						book[l].search.skip = true;
					}
					
					/* 3. Nothing displayed */
					else if(book[l].display.newestID == 0) {
						book[l].search.skip = false;
						book[l].search.startID = 0;
						book[l].search.nextID = book[l].search.oldestID;
						book[l].search.stopID = 0;
					}
					
					/* 4. Start search after the oldest searched record */
					else {
						book[l].search.skip = false;
						book[l].search.startID = 0;
						book[l].search.nextID = book[l].search.oldestID;
						book[l].search.stopID = 0;
					}
					
				} /* Loop logbooks */
			} /* Up then down */
			
			break;
			
		case WEBLOG_COMMAND_UP:
			/* Search above newest displayed record */
			
			if(previousCommand == WEBLOG_COMMAND_REFRESH || previousCommand == WEBLOG_COMMAND_DOWN) {
				/* Newest searched records were displayed */
				
				for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
					/* 1. Empty search */
					if(book[l].search.newestID == 0) {
						if(book[l].latestID) {
							book[l].search.skip = false;
							book[l].search.startID = MIN(WEBLOG_RECORD_MAX, book[l].latestID);
							book[l].search.stopID = 0;
						}
						else {
							book[l].search.skip = true;
						}
					}
					
					/* 2. Newest searched record ID is latest */
					else if(book[l].search.newestID == book[l].latestID) {
						book[l].search.skip = true;
					}
					
					/* 3. Nothing displayed */
					else if(book[l].display.newestID == 0) {
						book[l].search.skip = false;
						book[l].search.startID = MIN(book[l].search.newestID + WEBLOG_RECORD_MAX, book[l].latestID);
						book[l].search.stopID = book[l].search.newestID;
					}
					
					/* 4. Search above the newest searched record */
					else {
						book[l].search.skip = false;
						book[l].search.startID = MIN(book[l].search.newestID + WEBLOG_RECORD_MAX, book[l].latestID);
						book[l].search.stopID = book[l].search.newestID;
					}
				
				} /* Loop logbooks */
			} /* Down then up */
			
			else { /* Previously up */
				/* Oldest searched records were displayed */
				
				for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
					/* 1. Empty search */
					if(book[l].search.newestID == 0) {
						book[l].search.skip = true;
					}
					
					/* 2. Nothing displayed */
					else if(book[l].display.newestID == 0) {
						book[l].search.skip = false;
						book[l].search.startID = book[l].search.newestID;
						book[l].search.stopID = book[l].search.oldestID - 1;
					}
					
					/* 3. Newest record displayed is latest */
					else if(book[l].display.newestID == book[l].latestID) {
						book[l].search.skip = true;
					}
					
					/* 4. Search above the newest displayed record */
					else {
						book[l].search.skip = false;
						book[l].search.startID = MIN(book[l].display.newestID + WEBLOG_RECORD_MAX, book[l].latestID);
						book[l].search.stopID = book[l].display.newestID;
					}
					
				} /* Loop logbooks */
			} /* Continue up */
			
			break;
		default:
			/* Do nothing */
			break;
	}
	
	/* Perform Search */
	if(command) { /* Only search on command */
		memset(&record, 0, sizeof(record));
		searchCount = 0;
		valid = false;
		
		for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
			/* Clear status */
			book[l].search.count = 0;
			book[l].search.newestID = 0;
			book[l].search.oldestID = 0;
			
			/* Check requirements */
			if(book[l].ident == 0 || book[l].search.skip) continue;
			
			/* Begin reading records */
			for(r = 0; r < WEBLOG_RECORD_MAX; r++) {
				/* Check stop condition */
				if(r && book[l].search.stopID) {
					if(record[l][r - 1].ID - 1 == book[l].search.stopID) break;
				}
				
				/* Get record ID */
				if(r == 0 && book[l].search.startID)
					/* Search parameter specifies a start ID */
					record[l][r].ID = book[l].search.startID;
					
				else if(r == 0 && book[l].search.nextID == 0) {
					/* Now start ID or next ID search parameter */
					fbGetLatestRecord.Ident = book[l].ident;
					fbGetLatestRecord.Execute = true;
					ArEventLogGetLatestRecordID(&fbGetLatestRecord);
					if(fbGetLatestRecord.StatusID == ERR_OK) {
						record[l][r].ID = fbGetLatestRecord.RecordID;
						book[l].latestID = fbGetLatestRecord.RecordID;
					}
					fbGetLatestRecord.Execute = false;
					ArEventLogGetLatestRecordID(&fbGetLatestRecord);
				}
				else {
					/* Search parameter specifies a next ID or subsequent record */
					fbGetPreviousRecord.Ident = book[l].ident;
					if(r == 0) fbGetPreviousRecord.RecordID = book[l].search.nextID;
					else fbGetPreviousRecord.RecordID = record[l][r - 1].ID;
					fbGetPreviousRecord.Execute = true;
					ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
					
					if(fbGetPreviousRecord.StatusID == ERR_OK) 
						record[l][r].ID = fbGetPreviousRecord.PrevRecordID;
					
					fbGetPreviousRecord.Execute = false;
					ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
				}
				
				/* Verify ID */
				if(record[l][r].ID == 0) break;
				
				/* Read timestamp */
				fbReadRecord.Ident = book[l].ident;
				fbReadRecord.RecordID = record[l][r].ID;
				fbReadRecord.Execute = true;
				ArEventLogRead(&fbReadRecord);
					
				if(fbReadRecord.StatusID == ERR_OK || fbReadRecord.StatusID == arEVENTLOG_WRN_NO_EVENTID)
					formatTimestamp(fbReadRecord.TimeStamp.sec, fbReadRecord.TimeStamp.nsec, record[l][r].time);
				
				fbReadRecord.Execute = false;
				ArEventLogRead(&fbReadRecord);
				
				/* Verify timestamp */
				if(memcmp(zeroTime, record[l][r].time, sizeof(zeroTime)) == 0) break;
				
				/* Update search status */
				book[l].search.count++;
				searchCount++;
				valid = true;
				refreshed = true;
				if(r == 0) book[l].search.newestID = record[l][r].ID;
				book[l].search.oldestID = record[l][r].ID;
				
			} /* Loop records */
		} /* Loop logbooks */
		
		/* Sort through searched records */
		for(s = 0; s < WEBLOG_SORT_MAX; s++) {
			sortedBytes[s] = unsortedBytes[s] = record[s / WEBLOG_RECORD_MAX][(WEBLOG_RECORD_MAX - 1) - s % WEBLOG_RECORD_MAX].time;
			sortedIndices[s] = unsortedIndices[s] = (s / WEBLOG_RECORD_MAX) * WEBLOG_RECORD_MAX + (WEBLOG_RECORD_MAX - 1) - s % WEBLOG_RECORD_MAX;
		}
		radixSort(unsortedBytes, unsortedIndices, sortedBytes, sortedIndices, WEBLOG_SORT_MAX, WEBLOG_BYTE_MAX, 1);
		
		/* Get the oldest WEBLOG_RECORD_MAX records when seeking up */
		if(command == WEBLOG_COMMAND_UP) {
			/* Find the first invalid record then look RECORD_MAX up */
			for(s = 0; s < WEBLOG_SORT_MAX; s++) {
				dl = sortedIndices[s] / WEBLOG_RECORD_MAX;
				dr = sortedIndices[s] % WEBLOG_RECORD_MAX;
				if(memcmp(zeroTime, record[dl][dr].time, sizeof(zeroTime)) == 0) {
					searchOffset = MAX(s - (long)WEBLOG_RECORD_MAX, 0);
					break;
				}
			}
		}
		else searchOffset = 0;
		
	} /* User command */
	
	/* Read Records for Display */
	for(d = d0; d < WEBLOG_RECORD_MAX; d++) {
		switch(state) {
			/* Client request */
			case 0:
				if(command) {
					for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
						book[l].display.count = 0;
						book[l].display.newestID = 0;
						book[l].display.oldestID = 0;
					}
					
					if(!valid) {
						state = 201;
						break;
					}
					
					if(command == WEBLOG_COMMAND_REFRESH)
						displayOffset = 0;
					
					else if(command == WEBLOG_COMMAND_DOWN) {
						if(previousValid) displayOffset += WEBLOG_RECORD_MAX;
						else displayOffset = 0;
					}
						
					else if(command == WEBLOG_COMMAND_UP) {
						if(previousValid) displayOffset -= WEBLOG_RECORD_MAX;
						else displayOffset += displayCount - WEBLOG_RECORD_MAX;
						if(displayOffset < 0) displayOffset = 0;
					}
					
					displayCount = 0;
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
				dl = sortedIndices[searchOffset + d] / WEBLOG_RECORD_MAX;
				dr = sortedIndices[searchOffset + d] % WEBLOG_RECORD_MAX;
				
				/* Verify record */
				if(memcmp(zeroTime, record[dl][dr].time, sizeof(zeroTime)) == 0) {
					display[d].severity = 10; /* Give invalid severity to avoid displaying "Success" */
					if(d >= WEBLOG_RECORD_MAX - 1) {
						state = 201;
						break;
					}
					continue; /* Skip this record because it is invalid */
				}
				displayCount++;
				
				strncpy(display[d].logbook, book[dl].description, WEBLOG_STRLEN_LOGBOOK);
				display[d].logbook[WEBLOG_STRLEN_LOGBOOK] = '\0';
				
				display[d].ID = record[dl][dr].ID; /* Copy record ID */
				
				/* EventID + Severity +_Facility + Code or ErrorNumber + Severity */
				fbReadRecord.Ident = book[dl].ident;
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
					fbReadErrorNumber.Ident = book[dl].ident;
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
				display[d].sec = fbReadRecord.TimeStamp.sec;
				display[d].nsec = fbReadRecord.TimeStamp.nsec;
				
				fbReadRecord.Execute = false;
				ArEventLogRead(&fbReadRecord);
				
				state = 20; /* Finished */
				/* Do not break case */
				
			/* Description */
			case 20:
				fbReadDescription.Ident = book[dl].ident;
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
				replaceCharacter(display[d].description, '"', ' ');
				state = 30;
				/* Do not break case */
			
			/* Ascii data and object ID */
			case 30:
				fbReadAsciiData.Ident = book[dl].ident;
				fbReadAsciiData.RecordID = display[d].ID;
				fbReadAsciiData.AddData = (unsigned long)display[d].asciiData;
				fbReadAsciiData.BytesToRead = sizeof(display[d].asciiData) - 1;
				fbReadAsciiData.Execute = true;
				ArEventLogReadAddData(&fbReadAsciiData);
				fbReadAsciiData.Execute = false;
				ArEventLogReadAddData(&fbReadAsciiData);
				replaceCharacter(display[d].asciiData, '"', ' ');
				
				fbReadObjectName.Ident = book[dl].ident;
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
				book[dl].display.count++;
				if(book[dl].display.newestID == 0) book[dl].display.newestID = display[d].ID;
				book[dl].display.oldestID = display[d].ID;
				
				if(d < WEBLOG_RECORD_MAX - 1) {
					state = 10;
					continue; /* Next record index */
				}
				state = 201;
				/* Do not break case */
				
			/* Done */
			case 201:
				done = true;
				if(!refresh && !down && !up) {
					done = false;
					state = 0;
				}
				break;
				
			case 255:
				break;
		}
		break; /* Break record loop */
	}
	
	previousRefresh = refresh;
	previousDown = down;
	previousUp = up;
	previousValid = valid;
	
	if(command) previousCommand = command; /* Update previous command only on a rising edge */
	
}

/* Format sortable time data */
void formatTimestamp(unsigned long sec, unsigned long nsec, unsigned char *bytes) {
	const int val = 1;
	unsigned char i;
	/* 0-3 seconds 4-7 nanoseconds (big endian) */
	for(i = 0; i < 4; i++) { /* Access each byte of unsigned long, if little endian then rotate bytes */
		bytes[i] = *(((unsigned char *)&sec) + (*((unsigned char*)&val) ? 3 - i : i));
		bytes[4 + i] = *(((unsigned char *)&nsec) + (*((unsigned char*)&val) ? 3 - i : i));
	}
}

/* Replace quotes which are invalid in JSON value */
void replaceCharacter(char *source, char find, char replace) {
	unsigned char i = 0;
	while(source[i] && i < UCHAR_MAX) {
		if(source[i] == find) source[i] = replace;
		i++;
	}
}
