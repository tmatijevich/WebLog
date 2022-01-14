
TYPE
	WebLogRecordType : 	STRUCT 
		ID : ArEventLogRecordIDType;
		eventID : DINT;
		severity : USINT;
		code : UINT;
		facility : UINT;
		errorNumber : UDINT;
		timestamp : STRING[WEBLOG_STRLEN_TIMESTAMP]; (*YYYY-MM-DD HH:MM:SS,UUUUUU*)
		timeBytes : ARRAY[0..WEBLOG_BYTE_INDEX]OF BYTE; (*DO NOT MODIFY. Store seconds and nanosecnds in byte array for radix byte sort*)
		objectName : STRING[WEBLOG_STRLEN_LOGBOOK];
		description : STRING[WEBLOG_STRLEN_DESCIPTION];
		asciiData : STRING[WEBLOG_STRLEN_DESCIPTION];
		valid : BOOL;
		active : BOOL; (*This record is currently active on the logger web display*)
	END_STRUCT;
	WebLogBookType : 	STRUCT 
		ID : STRING[WEBLOG_STRLEN_LOGBOOK];
		name : STRING[WEBLOG_STRLEN_LOGBOOK];
		ident : ArEventLogIdentType;
		scans : USINT;
		end : BOOL;
	END_STRUCT;
	WebLogDisplayType : 	STRUCT 
		show : BOOL;
	END_STRUCT;
END_TYPE
