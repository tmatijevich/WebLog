
TYPE
	WebLoggerRecordType : 	STRUCT 
		ID : ArEventLogRecordIDType;
		EventID : DINT;
		Timestamp : STRING[25]; (*YYYY-MM-DD HH:MM:SS,UUUUUU*)
		TimeBytes : ARRAY[0..7]OF BYTE; (*Store seconds and nanosecnds in byte array for radix byte sort*)
		Active : BOOL; (*This record is currently active on the logger web display*)
		ObjectID : STRING[36];
		Description : STRING[125];
		AdditionalData : STRING[125];
	END_STRUCT;
END_TYPE
