
TYPE
	WebLogRecordSearchType : 	STRUCT  (*Store information about each record's timestamp for each logbook*)
		ID : ArEventLogRecordIDType; (*Record ID*)
		time : ARRAY[0..WEBLOG_BYTE_INDEX]OF USINT; (*Seconds and nanoseconds in byte array from record timestamp (big endian)*)
		status : USINT; (*Bit 0: Valid record/timestamp (zero time if invalid)*)
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
