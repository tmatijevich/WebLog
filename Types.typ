
TYPE
	WebLogRecordSearchType : 	STRUCT  (*Store information about each record's timestamp for each logbook*)
		ID : ArEventLogRecordIDType; (*Record ID*)
		time : ARRAY[0..WEBLOG_BYTE_INDEX]OF USINT; (*Seconds and nanoseconds in byte array from record timestamp (big endian)*)
		status : USINT; (*Bit 0: Valid record/timestamp (zero time if invalid), Bit 1: Visible displayed to web page*)
	END_STRUCT;
	WebLogBookType : 	STRUCT 
		ID : STRING[WEBLOG_STRLEN_LOGBOOK];
		name : STRING[WEBLOG_STRLEN_LOGBOOK];
		ident : ArEventLogIdentType;
		scans : USINT;
		end : BOOL;
	END_STRUCT;
	WebLogRecordDisplayType : 	STRUCT 
		ID : ArEventLogRecordIDType;
		logbook : STRING[WEBLOG_STRLEN_LOGBOOK];
		event : DINT;
		errorNumber : UDINT;
		severity : USINT;
		sec : UDINT;
		nsec : UDINT;
		description : STRING[WEBLOG_STRLEN_DESCIPTION];
		asciiData : STRING[WEBLOG_STRLEN_DESCIPTION];
		object : STRING[WEBLOG_STRLEN_LOGBOOK];
	END_STRUCT;
END_TYPE
