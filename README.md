# WebLog

WebLog is an Automation Studio program for searching and sorting logbook records.  It's lightweight design allows records "in view" to be displayed quickly to users.  This is achieved by only searching and sorting records necessary to fill the viewable table.  [Radix sort](https://en.wikipedia.org/wiki/Radix_sort) was implemented to efficient sort by timestamp.  **NOTE**: WebLog is not an official program and is provided as-is under the [GNU GPL v3.0](https://choosealicense.com/licenses/gpl-3.0/) license.

# Features

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

# Installation

1. Download and unarchive the WebLog program
2. In Automation Studio, navigate to Logical View and select the top of the project tree
3. From the toolbox, select Existing Program and choose the WebLog directory
4. Deploy the program to a low priority cyclic class
