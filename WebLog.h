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

/* B&R libraries */
#include <ArEventLog.h>

/* Standard C libraries */
#include <string.h>
#include <stdbool.h>
#include <limits.h>

/* User constants */
#define WEBLOG_RECORD_MAX_ 20
#define WEBLOG_LOGBOOK_MAX_ 10
#define WEBLOG_SORT_MAX_ (WEBLOG_RECORD_MAX_ * WEBLOG_LOGBOOK_MAX_)
#define WEBLOG_STRLEN_LOGBOOK_ 36
#define WEBLOG_STRLEN_DESCRIPTION_ 125
#define WEBLOG_BYTE_MAX_ 8

/* Other macros */
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

/* User structures */
struct weblog_recordsearch_typ {
	ArEventLogRecordIDType ID;
	unsigned char time[WEBLOG_BYTE_MAX_];
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
	char name[WEBLOG_STRLEN_LOGBOOK_ + 1];
	char description[WEBLOG_STRLEN_DESCRIPTION_ + 1];
	ArEventLogIdentType ident;
	struct weblog_logbooksearch_typ;
};

struct weblog_display_typ {
	ArEventLogRecordIDType ID;
	char logbook[WEBLOG_STRLEN_LOGBOOK_ + 1];
	long event;
	unsigned long errorNumber;
	unsigned char severity;
	unsigned long sec;
	unsigned long nsec;
	char description[WEBLOG_STRLEN_DESCRIPTION_ + 1];
	char asciiData[WEBLOG_STRLEN_DESCRIPTION_ + 1];
	char object[WEBLOG_STRLEN_LOGBOOK_ + 1];
};

/* Function prototypes */
void order_time_bytes(unsigned long sec, unsigned long nsec, unsigned char *bytes);
void radix_sort(unsigned char *in[], unsigned short idx[], unsigned char *sort[], unsigned short sortIdx[], unsigned short n, unsigned char k, unsigned char descending);

#endif
