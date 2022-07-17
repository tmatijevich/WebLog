# WebLog

WebLog is an Automation Studio program for searching and sorting logbook records.  It's lightweight design allows records "in view" to be displayed quickly to users.  This is achieved by only searching and sorting records necessary to fill the viewable table.  [Radix sort](https://en.wikipedia.org/wiki/Radix_sort) was implemented to efficient sort by timestamp.  **NOTE**: WebLog is not an official program and is provided as-is under the [GNU GPL v3.0](https://choosealicense.com/licenses/gpl-3.0/) license.

The WebLog program is intended for use with [WebLogClient](https://github.com/tmatijevich/WebLogClient), a front-end package to request and display results from WebLog's search and sort.

## Features

Instead of loading all logbook records in one giant scrollable list, WebLog is designed to load only the *viewable* records in the interest of speed.  Forward seeking and backward seeking inputs are available to the user to prompt a reload of records.

- Refresh to read the latest N records from M logbooks
- Seek backward (down) or forward (up) in time
- Table view of all record data
    - Severity
    - Time
    - ID
    - Logbook
    - Entered by
    - Description
    - Additional data
- Filter by 
    - Severity (planned)
    - Logbook (planned)

## Installation

1. Download and unarchive the WebLog program
2. In Automation Studio, navigate to Logical View and select the top of the project tree
3. From the toolbox, select Existing Program and choose the WebLog directory
4. Deploy the program to a low priority cyclic class

## Customization

Add custom logbooks

- Increase `WEBLOG_LOGBOOK_MAX` (default 10) in WebLog.h to account for additional logbooks 
- Initialize the custom logbook identifier to `book[i].name` in WebLog.c
    - Logbooks are uniquely identified by a 10 character name
- Provide a readable description of the logbook in `book[i].description` in WebLog.c
    - Due to the shortened character name, an alternative description up to 125 characters is available
    - If the logbook name is "MachDiag", the description could be "Machine Diagnostics"
    - The description string should be populated and will be displayed in the table
    - It is not necessary to have a description different from the name, but both strings must be populated
- Do not exceed the lengths of the strings to avoid memory overflow

WebLog.h

```c
#define WEBLOG_LOGBOOK_MAX 11U /* Modify to include all desired logbooks (system and custom) */
```

WebLog.c

```c
strcpy(book[10].name, "MachDiag"); strcpy(book[10].description, "Machine Diagnostics");
```

## Dependencies

- ArEventLog
