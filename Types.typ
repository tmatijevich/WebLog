
TYPE
	WebLogRecordSearchType : 	STRUCT  (*Store information about each record's timestamp for each logbook*)
		ID : ArEventLogRecordIDType; (*Record ID*)
		time : ARRAY[0..WEBLOG_BYTE_INDEX]OF USINT; (*Seconds and nanoseconds in byte array from record timestamp (big endian)*)
		valid : BOOL;
	END_STRUCT;
	WebLogBookType : 	STRUCT 
		name : STRING[WEBLOG_STRLEN_LOGBOOK];
		description : STRING[WEBLOG_STRLEN_LOGBOOK];
		ident : ArEventLogIdentType;
		search : WebLogBookSearchType;
	END_STRUCT;
	WebLogBookSearchType : 	STRUCT 
		latestID : ArEventLogRecordIDType; (*(search loop) Newet record found in this logbook*)
		newestSearchID : ArEventLogRecordIDType; (*(search loop) *)
		newestDisplayID : ArEventLogRecordIDType; (*(display loop) *)
		oldestDisplayID : ArEventLogRecordIDType; (*(display loop) *)
		oldestSearchID : ArEventLogRecordIDType; (*(search loop) *)
		skip : BOOL; (*(search parameter) Skip logbook search*)
		skipID : ArEventLogRecordIDType; (*(search parameter)*)
		readID : ArEventLogRecordIDType; (*(search parameter) Read this record first in search, skip if 0*)
		referenceID : ArEventLogRecordIDType; (*(search parameter) Reference this for previous ID, get latest if 0*)
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
