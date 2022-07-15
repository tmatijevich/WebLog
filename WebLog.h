/*******************************************************************************
 * File: WebLog.h
 * Author: Tyler Matijevich
 * Created: 2022-01-26
********************************************************************************
 * Description: Common includes, constants, type definitions, and prototypes
*******************************************************************************/

#ifndef WEBLOG_H
#define WEBLOG_H

/* PLC */
#include <bur/plc.h>
#include <bur/plctypes.h>

/* B&R */
#include <ArEventLog.h>

/* Standard C */
#include <string.h>
#include <stdbool.h>
#include <limits.h>

/* Constants */
#define WEBLOG_RECORD_MAX 20U /* Modify to desired number of table row */
#define WEBLOG_LOGBOOK_MAX 10U /* Modify to include all desired logbooks (system and custom) */
#define WEBLOG_SORT_MAX (WEBLOG_RECORD_MAX * WEBLOG_LOGBOOK_MAX)
#define WEBLOG_STRLEN_LOGBOOK 36U
#define WEBLOG_STRLEN_DESCRIPTION 125U
#define WEBLOG_BYTE_MAX 8U

/* Macros */
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

/* Structures */
struct webLogBookSearchType {
	unsigned char count; /* Count how many records have been searched */
	unsigned char skip; /* Skip search on this logbook */
    ArEventLogRecordIDType newestID; /* Newest searched ID (zero if empty search) */
	ArEventLogRecordIDType oldestID; /* Oldest searched ID (zero if empty search) */
	ArEventLogRecordIDType startID; /* Start search with this ID (search for latest if zero) */
	ArEventLogRecordIDType nextID; /* Start search with previous record after this ID (startID must be zero) */
	ArEventLogRecordIDType stopID; /* Stop search at this ID if searched */
};

struct webLogBookDisplayType {
	unsigned char count; /* Count how many records are actively displayed */
	ArEventLogRecordIDType newestID; /* Newest ID displayed (zero if nothing displayed) */
	ArEventLogRecordIDType oldestID; /* Oldest ID displayed (zero if nothing displayed) */
};

struct webLogBookType {
	plcstring name[WEBLOG_STRLEN_LOGBOOK + 1]; /* Must use plcstring for client WebPrint call */
	plcstring description[WEBLOG_STRLEN_DESCRIPTION + 1]; /* User readable description alternative to name */
	ArEventLogIdentType ident; /* Logbook reference for ArEventLog functions */
	ArEventLogRecordIDType latestID; /* Latest record ID discovered after refresh search */
	struct webLogBookSearchType search; /* Search data specific to logbook */
	struct webLogBookDisplayType display; /* Display data specific to logbook */
};

struct webLogRecordSearchType { /* Minimal data required to sort all logbook records by time */
	ArEventLogRecordIDType ID; /* Record ID of any logbook */
	unsigned char time[WEBLOG_BYTE_MAX]; /* Timestamp sec (32 bits) and nsec (32 bits) for a total of 8 bytes (64 bits) */
};

struct webLogDisplayType { /* All record data available for display */
	ArEventLogRecordIDType ID; /* Record ID */
	plcstring logbook[WEBLOG_STRLEN_LOGBOOK + 1]; /* Record logbook */
	long event; /* 32-bit Event ID */
	unsigned long errorNumber; /* 32-bit error number (legacy) */
	unsigned char severity; /* Error, Warning, Information, or Success */
	unsigned long sec; /* Seconds since 1970-01-01-00:00:00 */
	unsigned long nsec; /* Nanoseconds */
	plcstring description[WEBLOG_STRLEN_DESCRIPTION + 1]; /* Record description (message) */
	plcstring asciiData[WEBLOG_STRLEN_DESCRIPTION + 1]; /* Record ascii data (supplemental message) */
	plcstring object[WEBLOG_STRLEN_LOGBOOK + 1]; /* Record object (entered by) */
};

/* Enumerations */
enum webLogCommandEnum {
	WEBLOG_COMMAND_NONE = 0,
	WEBLOG_COMMAND_REFRESH,
	WEBLOG_COMMAND_DOWN,
	WEBLOG_COMMAND_UP
};

enum webLogDisplayStateEnum {
	WEBLOG_DISPLAY_IDLE = 0,
	WEBLOG_DISPLAY_READ,
	WEBLOG_DISPLAY_DESCRIPTION,
	WEBLOG_DISPLAY_ASCII,
	WEBLOG_DISPLAY_NEXT,
	WEBLOG_DISPLAY_COMPLETE,
	WEBLOG_DISPLAY_ERROR
};

/* Prototypes */
void radixSort(unsigned char *in[], unsigned short idx[], unsigned char *sort[], unsigned short sortIdx[], unsigned short n, unsigned char k, unsigned char descending);

#endif
