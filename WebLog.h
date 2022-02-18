/*******************************************************************************
 * File:      WebLog\WebLog.h
 * Author:    Tyler Matijevich
 * Created:   2022-01-26
********************************************************************************
 * Description: Declare constants, structures, and function prototypes
*******************************************************************************/

#ifndef WEBLOG_H
#define WEBLOG_H

/* PLC declarations */
#include <bur/plc.h>
#include <bur/plctypes.h>

/* B&R libraries */
#include <ArEventLog.h>

/* Standard C libraries */
#include <string.h>
#include <stdbool.h>
#include <limits.h>

/* User constants */
#define WEBLOG_RECORD_MAX 20U
#define WEBLOG_LOGBOOK_MAX 10U
#define WEBLOG_SORT_MAX (WEBLOG_RECORD_MAX * WEBLOG_LOGBOOK_MAX)
#define WEBLOG_STRLEN_LOGBOOK 36U
#define WEBLOG_STRLEN_DESCRIPTION 125U
#define WEBLOG_BYTE_MAX 8U

/* Other macros */
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

/* User structures */
struct weblog_recordsearch_typ {
	ArEventLogRecordIDType ID;
	unsigned char time[WEBLOG_BYTE_MAX];
	unsigned char valid;
};

struct weblog_logbooksearch_typ {
	ArEventLogRecordIDType latestID;
	ArEventLogRecordIDType newestSearchID;
	ArEventLogRecordIDType newestDisplayID;
	ArEventLogRecordIDType oldestDisplayID;
	ArEventLogRecordIDType oldestSearchID;
	unsigned char skip;
	ArEventLogRecordIDType skipID;
	ArEventLogRecordIDType readID;
	ArEventLogRecordIDType referenceID;
};

struct weblog_logbook_typ {
	plcstring name[WEBLOG_STRLEN_LOGBOOK + 1];
	plcstring description[WEBLOG_STRLEN_DESCRIPTION + 1];
	ArEventLogIdentType ident;
	struct weblog_logbooksearch_typ search;
};

struct weblog_display_typ {
	ArEventLogRecordIDType ID;
	plcstring logbook[WEBLOG_STRLEN_LOGBOOK + 1];
	long event;
	unsigned long errorNumber;
	unsigned char severity;
	unsigned long sec;
	unsigned long nsec;
	plcstring description[WEBLOG_STRLEN_DESCRIPTION + 1];
	plcstring asciiData[WEBLOG_STRLEN_DESCRIPTION + 1];
	plcstring object[WEBLOG_STRLEN_LOGBOOK + 1];
};

/* Function prototypes */
void order_time_bytes(unsigned long sec, unsigned long nsec, unsigned char *bytes);
void replace_char(char *str, char find, char replace);
void radix_sort(unsigned char *in[], unsigned short idx[], unsigned char *sort[], unsigned short sortIdx[], unsigned short n, unsigned char k, unsigned char descending);

#endif
