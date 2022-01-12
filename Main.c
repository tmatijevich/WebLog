
#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

#include <string.h>
#include <stdbool.h>

/* Function prototypes */
//void MSB_radixSort(unsigned char *arr[], unsigned int n, const unsigned char k, unsigned char *sort[]);
void setTimestamp(unsigned long sec, unsigned long nsec, char *str);
void setTimeBytes(unsigned long sec, unsigned long nsec, unsigned char *bytes);

void _INIT ProgramInit(void)
{
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
}

void _CYCLIC ProgramCyclic(void)
{	
	/* Main case statement */
	switch(state) {
		case 0:
			fbGetLocalTime.enable = true;
			DTGetTime(&fbGetLocalTime);
			
			fbGetUtcTime.enable = true;
			UtcDTGetTime(&fbGetUtcTime);
			
			if(fbGetLocalTime.status != ERR_OK || fbGetUtcTime.status != ERR_OK) {
				state = 255;
				break;
			}
			
			utcToLocalOffset = ((long)(fbGetLocalTime.DT1 - fbGetUtcTime.DT1) / 3600L) * 3600L;
			
			for(li = 0; li < WEBLOG_LOGBOOK_MAX; li++) {
				strcpy(fbGetIdent.Name, logbook[li].ID);
				fbGetIdent.Execute = true;
				ArEventLogGetIdent(&fbGetIdent);
				logbook[li].ident = fbGetIdent.Ident; /* Zero if error (NOT_FOUND) */
				fbGetIdent.Execute = false;
				ArEventLogGetIdent(&fbGetIdent);
			}
			
			state = 10;	
			break;
			
		case 10:
			break;
		case 20:
		case 30:
		case 255:
			break;
	}
	
	for(li = 0; li < WEBLOG_LOGBOOK_MAX; li++) { /* Every logbook */
		for(ri = r0[li]; ri < WEBLOG_RECORD_MAX; ri++) { /* Attempt to go through every record */
			switch(lstate[li]) {
				case 0:
					if(refresh && !prevRefresh && state == 10 && logbook[li].ident) {
						memset(record[li], 0, sizeof(record[0]));
						r0[li] = 0;
						ri = 0;
						logbook[li].scans = 1;
						lstate[li] = 10; /* Proceed to next state */
					}
					else
						break; /* Break case and record loop */
					
				case 10:
					/* Get record ID - syncrhonous execution (every logbook every record) */
					if(ri == 0) { /* Get latest */
						fbGetLatestRecord.Ident 	= logbook[li].ident;
						fbGetLatestRecord.Execute 	= true;
						ArEventLogGetLatestRecordID(&fbGetLatestRecord);
						if(fbGetLatestRecord.StatusID != ERR_OK || !fbGetLatestRecord.Done || fbGetLatestRecord.Error) {
							lstate[li] = 255;
							fbGetLatestRecord.Execute = false;
							ArEventLogGetLatestRecordID(&fbGetLatestRecord);
							break; /* Break record loop */
						}
						record[li][ri].ID 			= fbGetLatestRecord.RecordID;
						fbGetLatestRecord.Execute 	= false;
						ArEventLogGetLatestRecordID(&fbGetLatestRecord);
					}
					else { /* Get previous */
						fbGetPreviousRecord.Ident 		= logbook[li].ident;
						fbGetPreviousRecord.RecordID 	= record[li][ri - 1].ID;
						fbGetPreviousRecord.Execute 	= true;
						ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
						if(fbGetPreviousRecord.StatusID == arEVENTLOG_ERR_RECORDID_INVALID) {
							r0[li] = ri;
							lstate[li] = 201;
							fbGetPreviousRecord.Execute = false;
							ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
							break; /* Break record loop */
						}
						else if(fbGetPreviousRecord.StatusID != ERR_OK || !fbGetPreviousRecord.Done || fbGetPreviousRecord.Error) {
							lstate[li] = 255;
							fbGetPreviousRecord.Execute = false;
							ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
							break; /* Break record loop */
						}
						record[li][ri].ID 				= fbGetPreviousRecord.PrevRecordID;
						fbGetPreviousRecord.Execute 	= false;
						ArEventLogGetPreviousRecordID(&fbGetPreviousRecord);
					}
					
					/* Read event ID and timestamp - syncrhonous execution (every logbook every record) */
					fbReadRecord.Ident 		= logbook[li].ident;
					fbReadRecord.RecordID	= record[li][ri].ID;
					fbReadRecord.Execute 	= true;
					ArEventLogRead(&fbReadRecord);
					if((fbReadRecord.StatusID != ERR_OK && fbReadRecord.StatusID != arEVENTLOG_WRN_NO_EVENTID) || !fbReadRecord.Done || fbReadRecord.Error) {
						lstate[li] = 255;
						fbReadRecord.Execute = false;
						ArEventLogRead(&fbReadRecord);
						break; /* Break record loop */
					}
					if(fbReadRecord.StatusID == arEVENTLOG_WRN_NO_EVENTID) {
						fbReadErrorNumber.Ident 	= logbook[li].ident;
						fbReadErrorNumber.RecordID 	= record[li][ri].ID;
						fbReadErrorNumber.Execute 	= true;
						ArEventLogReadErrorNumber(&fbReadErrorNumber);
						if(fbReadErrorNumber.StatusID != ERR_OK || !fbReadErrorNumber.Done || fbReadErrorNumber.Error) { 
							lstate[li] = 255;
							fbReadErrorNumber.Execute = false;
							ArEventLogReadErrorNumber(&fbReadErrorNumber);
							break; /* Break record loop */
						}
						record[li][ri].errorNumber 	= fbReadErrorNumber.ErrorNumber;
						record[li][ri].severity 	= fbReadErrorNumber.Severity;
						fbReadErrorNumber.Execute 	= false;
						ArEventLogReadErrorNumber(&fbReadErrorNumber);
					}
					else {
						record[li][ri].eventID 	= fbReadRecord.EventID;
						record[li][ri].severity = (unsigned char)((fbReadRecord.EventID >> 30) & 0x3);
						record[li][ri].facility = (unsigned short)((fbReadRecord.EventID >> 16) & 0xFFFF);
						record[li][ri].code 	= (unsigned short)(fbReadRecord.EventID & 0xFFFF);
					}
					setTimestamp((unsigned long)((long)fbReadRecord.TimeStamp.sec + utcToLocalOffset), fbReadRecord.TimeStamp.nsec, record[li][ri].timestamp);
					setTimeBytes((unsigned long)((long)fbReadRecord.TimeStamp.sec + utcToLocalOffset), fbReadRecord.TimeStamp.nsec, record[li][ri].timeBytes);
					fbReadRecord.Execute = false;
					ArEventLogRead(&fbReadRecord);
					
					lstate[li] = 20;
					
				case 20:
					/* Read description */
					fbReadDescription[li].Ident 			= logbook[li].ident;
					fbReadDescription[li].RecordID 			= record[li][ri].ID;
					fbReadDescription[li].TextBuffer 		= (unsigned long)record[li][ri].description;
					fbReadDescription[li].TextBufferSize 	= sizeof(record[li][ri].description);
					fbReadDescription[li].Execute 			= true;
					ArEventLogReadDescription(&fbReadDescription[li]);
					if(fbReadDescription[li].Busy) {
						r0[li] = ri;
						break; /* Break record loop - use another scan to complete this asynchronous function block */
					}
					else if((fbReadDescription[li].StatusID != ERR_OK && fbReadDescription[li].StatusID != arEVENTLOG_WRN_NO_EVENTID) || !fbReadDescription[li].Done || fbReadDescription[li].Error) {
						lstate[li] = 255;
						fbReadDescription[li].Execute = false;
						ArEventLogReadDescription(&fbReadDescription[li]);
						break; /* Break record loop */
					}
					fbReadDescription[li].Execute = false;
					ArEventLogReadDescription(&fbReadDescription[li]);
					
					lstate[li] = 30;
					
				case 30:
					/* Read additional (ascii) data */
					fbReadAsciiData.Ident 		= logbook[li].ident;
					fbReadAsciiData.RecordID 	= record[li][ri].ID;
					fbReadAsciiData.AddData 	= (unsigned long)record[li][ri].asciiData;
					fbReadAsciiData.BytesToRead = sizeof(record[li][ri].asciiData);
					fbReadAsciiData.Execute 	= true;
					ArEventLogReadAddData(&fbReadAsciiData);
					if(fbReadAsciiData.StatusID != ERR_OK || !fbReadAsciiData.Done || fbReadAsciiData.Error) {
						lstate[li] = 255;
						fbReadAsciiData.Execute = false;
						ArEventLogReadAddData(&fbReadAsciiData);
						break; /* Break record loop */
					}
					fbReadAsciiData.Execute = false;
					ArEventLogReadAddData(&fbReadAsciiData);
					
					lstate[li] = 200;
					
				case 200:
					record[li][ri].valid = true;
					if(ri == WEBLOG_RECORD_MAX - 1) {
						lstate[li] = 201;
						break;
					}
					lstate[li] = 10;
					continue;
					
				case 201:
					if(!refresh) lstate[li] = 0;
					break;
					
				case 255:
					break;
			} /* Logbook state */
			if(lstate[li] != 201 && lstate[li] != 0) logbook[li].scans++;
			break; /* If switch statement is broken, break the record for loop as well */
		} /* Record */
	} /* Logbook */
	
	done = true;
	ready = true;
	for(li = 0; li < WEBLOG_LOGBOOK_MAX; li++) {
		if(lstate[li] != 201 && logbook[li].ident) {
			done = false;
		}
		else if(lstate[li] != 0) { 
			ready = false; 
		}
	}
	
	if(done && !prevDone && refresh) {
		for(ri = 0; ri < WEBLOG_RECORD_MAX; ri++) {
			display[ri] = record[0][ri];
		}
	}
	
	prevRefresh = refresh;
	prevDone = done;
	
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

