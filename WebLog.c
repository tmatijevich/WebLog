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
_LOCAL unsigned char refresh, refreshed, down, up, done, valid, prevRefresh, prevDown, prevUp, prevCmd, prevValid;
_LOCAL long tableOffset;
long l, r;
_LOCAL struct weblog_logbook_typ logbook[WEBLOG_LOGBOOK_MAX];
struct weblog_recordsearch_typ search[WEBLOG_LOGBOOK_MAX][WEBLOG_RECORD_MAX];
unsigned char state, d, d0, dl, dr, displayCount;
_LOCAL struct weblog_display_typ display[WEBLOG_RECORD_MAX];
unsigned short s, displayOffset, unsortedIndices[WEBLOG_SORT_MAX], sortedIndices[WEBLOG_SORT_MAX];
unsigned char *unsortedBytes[WEBLOG_SORT_MAX], *sortedBytes[WEBLOG_SORT_MAX];
enum webLogCommandEnum command, previousCommand;
_LOCAL struct webLogBookType book[WEBLOG_LOGBOOK_MAX];

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
	strcpy(logbook[0].name, "$arlogsys"); 	strcpy(logbook[0].description, "System");
	strcpy(logbook[1].name, "$arlogusr"); 	strcpy(logbook[1].description, "User");
//	strcpy(logbook[1].name, "$fieldbus"); 	strcpy(logbook[1].description, "Fieldbus");
//	strcpy(logbook[2].name, "$arlogconn"); 	strcpy(logbook[2].description, "Connectivity");
//	strcpy(logbook[3].name, "$textsys"); 	strcpy(logbook[3].description, "Text System");
//	strcpy(logbook[4].name, "$accsec"); 	strcpy(logbook[4].description, "Access & Security");
//	strcpy(logbook[5].name, "$visu"); 		strcpy(logbook[5].description, "Visualization");
//	strcpy(logbook[6].name, "$firewall"); 	strcpy(logbook[6].description, "Firewall");
//	strcpy(logbook[7].name, "$versinfo"); 	strcpy(logbook[7].description, "Version Info");
//	strcpy(logbook[8].name, "$diag"); 		strcpy(logbook[8].description, "Diagnostics");
//	strcpy(logbook[9].name, "$arlogusr"); 	strcpy(logbook[9].description, "User");
	
	/* Get idents of all logbooks */
	for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
		strcpy(fbGetIdent.Name, logbook[l].name); /* Destination size is 257 */
		fbGetIdent.Execute = true;
		ArEventLogGetIdent(&fbGetIdent);
		logbook[l].ident = fbGetIdent.Ident; /* Zero if error (arEVENTLOG_ERR_LOGBOOK_NOT_FOUND) */
		fbGetIdent.Execute = false;
		ArEventLogGetIdent(&fbGetIdent);
	}
	
	refreshed = false;
	tableOffset = 0;
	displayCount = 0;
}

/* Cyclic routine */
void _CYCLIC program_cyclic(void)
{	
	/* Accept user command */
	if(refresh && !prevRefresh)
		command = WEBLOG_COMMAND_REFRESH;
	else if(down && !prevDown && refreshed)
		command = WEBLOG_COMMAND_DOWN;
	else if(up && !prevUp && refreshed)
		command = WEBLOG_COMMAND_UP;
	
	/* Prepare Search Parameters */
	switch(command) {
		case WEBLOG_COMMAND_REFRESH:
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
						book[l].search.startID = book[l].search.newestID;
					}
					
					/* 3. Oldest displayed record ID is 1 */
					else if(book[l].display.oldestID == 1) {
						book[l].search.skip = true;
					}
					
					/* 4. Start search after the oldest displayed */
					else {
						book[l].search.nextID = book[l].display.oldestID;
					}
					
				} /* Loop logbooks */
			} /* Continue down */
			
			else { /* Previously up */
				/* Oldest searched records were displayed */
				
				for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
					/* 1. Empty search */
					if(book[l].search.newestID == 0) {
						if(book[l].latestID) {
							book[l].search.startID = book[l].latestID;
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
						book[l].search.nextID = book[l].search.oldestID;
					}
					
					/* 4. Start search after the oldest searched record */
					else {
						book[l].search.nextID = book[l].search.oldestID;
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
							book[l].search.startID = MIN(WEBLOG_RECORD_MAX, book[l].latestID);
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
						book[l].search.startID = MIN(book[l].search.newestID + WEBLOG_RECORD_MAX, book[l].latestID);
						book[l].search.stopID = book[l].search.newestID;
					}
					
					/* 4. Search above the newest searched record */
					else {
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
						book[l].search.startID = book[l].search.newestID;
						book[l].search.stopID = book[l].search.oldestID - 1;
					}
					
					/* 3. Newest record displayed is latest */
					else if(book[l].display.newestID == book[l].latestID) {
						book[l].search.skip = true;
					}
					
					/* 4. Search above the newest displayed record */
					else {
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
	
	/* Search RECORD_MAX records in each LOGBOOK_MAX logbooks */
	if(((refresh && !prevRefresh) || (down && !prevDown && refreshed) || (up && !prevUp && refreshed)) && state == 0) {
		refreshed = true;
		valid = false; /* Invalidate until a new record is found */
		memset(search, 0, sizeof(search)); /* Clear search memory */
		
		/* 
		 * Refresh
		 *   Clear search memory
		 *   Read latest RECORD_MAX records
		 */
		if(refresh) {
			for(l = 0; l < WEBLOG_RECORD_MAX; l++) {
				/* Clear search parameters */
				logbook[l].search.skip = false;
				logbook[l].search.readID = 0;
				logbook[l].search.referenceID = 0;
				logbook[l].search.skipID = 0;
			}
			prevCmd = 0;
		}
		
		else if(down) { 
			for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
				if(prevCmd == 0 || prevCmd == 1) { /* Newest search is displayed */
					/* Search past oldest record displayed */
					if(logbook[l].search.oldestSearchID == 0) { /* Empty (end) */
						logbook[l].search.skip = true;
					}
					else if(logbook[l].search.oldestDisplayID == 0) {
						/* Re-read newest search */
						logbook[l].search.skip = false;
						logbook[l].search.skipID = 0;
						logbook[l].search.readID = logbook[l].search.newestSearchID; /* Older than what else is displayed */
						logbook[l].search.referenceID = 0;
					}
					else {
						logbook[l].search.skip = false;
						logbook[l].search.skipID = 0;
						logbook[l].search.readID = 0;
						logbook[l].search.referenceID = logbook[l].search.oldestDisplayID;
					}
				}
				else { /* Oldest search is displayed */
					if(logbook[l].search.oldestSearchID == 0) { /* Empty (beginning) */
						/* Read the latest */
						logbook[l].search.skip = false;
						logbook[l].search.skipID = 0;
						logbook[l].search.readID = logbook[l].search.latestID;
						logbook[l].search.referenceID = 0;
					}
					else {
						logbook[l].search.skip = false;
						logbook[l].search.skipID = 0;
						logbook[l].search.readID = 0;
						logbook[l].search.referenceID = logbook[l].search.oldestSearchID;
					}
				}
			}
			prevCmd = 1;
		}
		
		else if(up) {
			/* Find newer searches */
			for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
				if(prevCmd == 0 || prevCmd == 1) { 
					/* Newest searches were displayed (search above newestSearchID) */
					if(logbook[l].search.newestSearchID == logbook[l].search.latestID) {
						/* Nothing new to search */
						logbook[l].search.skip = true;
					}
					else if(logbook[l].search.newestSearchID == 0) {
						/* Empty (end) */
						logbook[l].search.skip = false;
						logbook[l].search.skipID = logbook[l].search.newestSearchID;
						logbook[l].search.readID = MIN(logbook[l].search.newestSearchID + WEBLOG_RECORD_MAX, logbook[l].search.latestID);
						logbook[l].search.referenceID = 0;
					}
					else {
						/* Search above newest search */
						logbook[l].search.skip = false;
						logbook[l].search.skipID = logbook[l].search.newestSearchID;
						logbook[l].search.readID = MIN(logbook[l].search.newestSearchID + WEBLOG_RECORD_MAX, logbook[l].search.latestID);
						logbook[l].search.referenceID = 0;
					}
				}
				else { 
					/* Oldest searches were displayed (search above newestDisplayID) */
					if(logbook[l].search.newestSearchID == 0) { 
						/* Empty (beginning) */
						logbook[l].search.skip = true;
					}
					else if(logbook[l].search.newestDisplayID == 0) {
						/* Repeat search */
						logbook[l].search.skip = false;
						logbook[l].search.skipID = logbook[l].search.oldestSearchID - 1; 
						logbook[l].search.readID = logbook[l].search.newestSearchID; 
						logbook[l].search.referenceID = 0;
					}
					else if(logbook[l].search.newestDisplayID == logbook[l].search.latestID) {
						/* Nothing new to search */
						logbook[l].search.skip = true;
					}
					else {
						/* Search above newestDisplayID */
						logbook[l].search.skip = false;
						logbook[l].search.skipID = logbook[l].search.newestDisplayID;
						logbook[l].search.readID = MIN(logbook[l].search.newestDisplayID + WEBLOG_RECORD_MAX, logbook[l].search.latestID);
						logbook[l].search.referenceID = 0;
					}
				}
			}
			prevCmd = 2;
		}
		
		for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
			logbook[l].search.newestSearchID = 0;
			logbook[l].search.oldestSearchID = 0;
			if(!logbook[l].ident || logbook[l].search.skip) continue; /* Continue to next logbook */
			for(r = 0; r < WEBLOG_RECORD_MAX; r++) {
				if(r > 0 && logbook[l].search.skipID > 0) {
					if(search[l][r - 1].ID - 1 <= logbook[l].search.skipID) {
						break; /* Break record loop */
					}
				}
				
				/* Find record ID */
				if(r == 0 && logbook[l].search.readID) {
					search[l][r].ID = logbook[l].search.readID;
				}
				else if(r == 0 && logbook[l].search.referenceID == 0) { /* Latest record */
					fbGetLatestRecord.Ident = logbook[l].ident;
					fbGetLatestRecord.Execute = true;
					ArEventLogGetLatestRecordID(&fbGetLatestRecord);
					if(fbGetLatestRecord.Done) {
						search[l][r].ID = fbGetLatestRecord.RecordID;
						logbook[l].search.latestID = fbGetLatestRecord.RecordID;
					}
					/* Reset execution */
					fbGetLatestRecord.Execute = false;
					ArEventLogGetLatestRecordID(&fbGetLatestRecord);
				}
				else { /* Previous record */
					fbGetPreviousRecord.Ident = logbook[l].ident;
					if(r == 0) fbGetPreviousRecord.RecordID = logbook[l].search.referenceID;
					else fbGetPreviousRecord.RecordID = search[l][r - 1].ID;
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
					order_time_bytes((unsigned long)fbReadRecord.TimeStamp.sec, fbReadRecord.TimeStamp.nsec, search[l][r].time);
					search[l][r].valid = true;
				}
				/* Reset execution */
				fbReadRecord.Execute = false;
				ArEventLogRead(&fbReadRecord);
				
				if(r == 0) logbook[l].search.newestSearchID = search[l][r].ID;
				logbook[l].search.oldestSearchID = search[l][r].ID;
				valid = true;
				
			} /* Record loop */
		} /* Logbook loop */
		
		/* Sort through searched records */
		for(s = 0; s < WEBLOG_SORT_MAX; s++) {
			sortedBytes[s] = unsortedBytes[s] = search[s / WEBLOG_RECORD_MAX][(WEBLOG_RECORD_MAX - 1) - s % WEBLOG_RECORD_MAX].time;
			sortedIndices[s] = unsortedIndices[s] = (s / WEBLOG_RECORD_MAX) * WEBLOG_RECORD_MAX + (WEBLOG_RECORD_MAX - 1) - s % WEBLOG_RECORD_MAX;
		}
		radix_sort(unsortedBytes, unsortedIndices, sortedBytes, sortedIndices, WEBLOG_SORT_MAX, WEBLOG_BYTE_MAX, 1);
		
		/* Get the last WEBLOG_RECORD_MAX records of valid sorted records */
		if(up) {
			for(s = 0; s < WEBLOG_SORT_MAX; s++) {
				dl = sortedIndices[s] / WEBLOG_RECORD_MAX;
				dr = sortedIndices[s] % WEBLOG_RECORD_MAX;
				if(!search[dl][dr].valid) {
					/* Find the first invalid sorted record, then look RECORD_MAX entries up */
					displayOffset = MAX(s - WEBLOG_RECORD_MAX, 0);
					break;
				}
			}
		}
		else {
			displayOffset = 0;
		}
		
	} /* Refresh command */
	
	for(d = d0; d < WEBLOG_RECORD_MAX; d++) {
		switch(state) {
			/* Client request */
			case 0:
				if((refresh && !prevRefresh) || (down && !prevDown) || (up && !prevUp)) {
					for(l = 0; l < WEBLOG_LOGBOOK_MAX; l++) {
						logbook[l].search.newestDisplayID = 0;
						logbook[l].search.oldestDisplayID = 0;
					}
					
					if(!valid) {
						state = 201;
						break;
					}
					
					switch(prevCmd) {
						case 0: 
							tableOffset = 0;
							break;
						case 1: 
							tableOffset += WEBLOG_RECORD_MAX;
							break;
						case 2: 
							tableOffset = prevValid ? tableOffset - WEBLOG_RECORD_MAX : tableOffset + displayCount - WEBLOG_RECORD_MAX; 
							if(tableOffset < 0) tableOffset = 0;
							break;
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
				dl = sortedIndices[displayOffset + d] / WEBLOG_RECORD_MAX;
				dr = sortedIndices[displayOffset + d] % WEBLOG_RECORD_MAX;
				
				/* Check if valid */
				if(!search[dl][dr].valid) {
					display[d].severity = 10; /* Give invalid severity to avoid displaying "Success" */
					if(d >= WEBLOG_RECORD_MAX - 1) {
						state = 201;
						break;
					}
					continue; /* Skip this record because it is invalid */
				}
				displayCount++;
				
				strncpy(display[d].logbook, logbook[dl].description, WEBLOG_STRLEN_LOGBOOK);
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
				display[d].sec = fbReadRecord.TimeStamp.sec;
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
				replace_char(display[d].description, '"', ' ');
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
				replace_char(display[d].asciiData, '"', ' ');
				
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
				if(logbook[dl].search.newestDisplayID == 0) logbook[dl].search.newestDisplayID = display[d].ID;
				logbook[dl].search.oldestDisplayID = display[d].ID;
				
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
	
	prevRefresh = refresh;
	prevDown = down;
	prevUp = up;
	prevValid = valid;
	
}

/* Format sortable time data */
void order_time_bytes(unsigned long sec, unsigned long nsec, unsigned char *bytes) {
	const int val = 1;
	unsigned char i;
	/* 0-3 seconds 4-7 nanoseconds (big endian) */
	for(i = 0; i < 4; i++) { /* Access each byte of unsigned long, if little endian then rotate bytes */
		bytes[i] = *(((unsigned char *)&sec) + (*((unsigned char*)&val) ? 3 - i : i));
		bytes[4 + i] = *(((unsigned char *)&nsec) + (*((unsigned char*)&val) ? 3 - i : i));
	}
}

/* Replace quotes which are invalid in JSON value */
void replace_char(char *str, char find, char replace) {
	unsigned char i = 0;
	while(str[i] && i < UCHAR_MAX) {
		if(str[i] == find) str[i] = replace;
		i++;
	}
}
