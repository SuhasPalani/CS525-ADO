--------------------------------       
        Group 16 Members:
--------------------------------

Name - Diksha Sharma 
CWID - A20523396 
Email- dsharma19@hawk.iit.edu 


Name - Bhoomika Panduranga 
CWID - A20503493 
Email- bpanduranga@hawk.iit.edu


Name - Yashas Shashidhar    
CWID - A20516827 
Email- yshashidhar@hawk.iit.edu

=======================================
RUNNING THE SCRIPT
=======================================

1) Launch a Terminal window and navigate to Project root (assign3).

2) To verify that we are in the right directory, use ls to list the files.

3) To remove outdated compiled.o files, type "make clean".

4) Press "make" to compile the "test assign3 1.c" file as well as the rest of the project files.

5) To run the "test assign3 1.c" program, use "make run".

=======================================
SOLUTION DESCRIPTION
=======================================

By releasing any reserved space when appropriate and reducing the use of variables, 
we have assured adequate memory management when developing this record manager. 

=======================================
1. TABLE AND RECORD MANAGER FUNCTIONS
=======================================
-------------------------
Author: Yashas Shashidhar
-------------------------
Record manager has functions such as init and shutdown to perform initialization and shutdown on record manager. Storage manager and buffer manager files from previous assignments are used as well.

InitRecordManager 
--> Used to perform initialization for the record manager. 
--> Storage manager function is called to perform initialization of the storage manager.

shutdownRecordManager 
--> It deallocates resources allocated and shutsdown the record manager.

CreateTable 
--> It opens the table, Initializes the values to the table and sets attributes of the table.  
--> We initialize buffer pool in this function. 
--> This function performs page operations like creating and opening page file with writing block having table in that page file.

openTable 
--> This function is used to open the table of given name.

closeTable 
--> This function is used to close the table with given name. 
--> We use shutdown buffer pool function here. 
--> The buffer manager will make sure the content is written to the page file before shutting down.

deleteTable
--> It is used to delete the table of given name. 
--> destroyPagefile function is used to perform the deletion and deallocate the memory.

getNumTuples- It returns the number of records/tuple in the table. 


=======================================
2. RECORD FUNCTIONS - 
=======================================
----------------------
Author: Diksha Sharma
----------------------
These operations are used to insert a new record, delete a record with a certain RID, 
retrieve a record with a specific RID, and update a record with new data.

insertRecord
--> The insertRecord() function adds a record to the table and updates the'record' argument with the Record ID supplied there.
--> For the record being inserted, we set the Record ID.
--> The page with the open slot is pinnable. Once we have an empty slot, we find the data pointer and add a "+" to indicate that a new record has been added.
--> In order for the Buffer Manager to write the page's content back to the disk, we also label the page as dirty.
--> Using the memcpy() C method, we copy the record's data (provided through parameter "record") into the new record before unpinning the page.

deleteRecord
--> This function removes a record with the Record ID 'id' from the table that is referred by the parameter'rel'.
--> In order for this space to be utilised by a new record later, we set our table's meta-data freePage to the Page ID of this page whose record is to be erased.
--> To indicate that a record has been destroyed and is no longer needed, we pin the page, go to its data pointer, and change its initial character to a '-'.
--> After saving the page's contents back to disk using the BUffer Manager, we label the page as dirty and unpin it.

updateRecord
--> A record in the table referred to by the parameter "r" that is referenced by the function's "rec: record" parameter is updated.
--> Using the meta-data of the table, it locates the page where the record is located and pins that page in the buffer pool.
--> It sets the Record ID and moves on to the record's data storage place.
--> Using the memcpy() C function, we copy the record's data (provided through parameter "record") into the new record, dirty-mark the page, and then unpin the page.

getRecord
--> This function pulls a record from the table that is referred by the parameter "rel" and has the Record ID "id" supplied in the parameter. The location referred to by the argument "record" is where the result record is kept.
--> Using the table's meta-data, it locates the page where the record is stored, and using the record's "id," it pins that page in the buffer pool.
--> It copies the data and sets the Record ID of the "record" argument to the ID of the record that is present on the page.
--> The page is then unpinned.

=======================================
3. SCAN FUNCTIONS
=======================================
----------------------
Author: Diksha Sharma
----------------------

Using the Scan-related functions, you can retrieve all tuples from a table that meet a specific requirement (represented as an Expr). The RM ScanHandle data structure given as an input to startScan is initialized when a scan is started. The next function is then called, returning the subsequent tuple that satisfies the scan condition. It returns RC SCAN CONDITION NOT FOUND, if NULL is used as a scan condition. after the scan is finished, returns RC RM NO MORE TUPLES, otherwise RC OK (unless an error occurs).

startScan
--> The RM ScanHandle data structure, which is supplied as an argument to the startScan() function, is accessed by this function to begin a scan.
--> We set the scan-related variables in our unique data structure.
--> If the condition is NULL, an error code is returned. RC SCAN CONDITION NOT FOUND

next
--> This function returns the following tuple that meets the requirement (test expression).
--> If the condition is NULL, the error RC SCAN CONDITION NOT FOUND is returned.
--> We return error code RC RM NO MORE TUPLES if the table contains no tuples.
—> We cycle through the table's tuples. Then, bookmark the page containing that tuple, go to the site where the data is stored, transfer the data into a temporary buffer, and then run eval to evaluate the test expression (....)
--> The tuple satisfies the criteria if the result (v.boolV) of the test expression is TRUE. Then we remove the pin and come back. RC OK
—> We return error code RC RM NO MORE TUPLES if none of the tuples satisfy the requirement.

closeScan
The scan operation is ended by this function.
--> By examining the scanCount value in the table's metadata, we determine whether the scan was complete or not. If it is higher than 0, the scan was not fully completed.
--> If the scan was insufficient, we unpin the page and reset all variables linked to the scan method in the meta-data of our table (custom data structure).
—> The space that the metadata had taken up is then freed (de-allocated).

=========================================
4. SCHEMA FUNCTIONS
=========================================

---------------------------
Author: Bhoomika Panduranga
---------------------------

The Schema functions are used to create the schema, get the size of records in the schema and to free the schema.

getRecordSize 
--> This function is used to get the size of records of given schema in bytes. 
--> Here we check for the datatype of attributes in schema and add all the size to get the record size.

freeSchema 
--> This function is used to deallocate the memory assigned to the schema.

createSchema 
--> In this function we create a new schema according to the parameters in memory.
--> We initialize number of attributes, names of attributes, datatype of attributes and length of attributes to the new schema.

free_mem
--> This function is used to free that is deallocate the memory space allocated.

=========================================
5. ATTRIBUTE FUNCTIONS
=========================================

---------------------------
Author: Bhoomika Panduranga
---------------------------

Attribute functions are used to create the record, free the space allocated to the record and to get the attributes of the record as well as in setting the attributes of the records.

CreateRecord 
--> This function is used to create the record through allocating the memory space for the record and the parameters passed for the record is allocated to this new record.

freeRecord 
--> This function is used to deallocate the memory assigned to the record.

Posattr
--> This function is used to obtain the position of the attribute.

getAttr
--> This function is used to get the attributes of the record required. 
--> Here we will verify the datatype of the attribute and then copy’s the attribute’s value.

setAttr 
--> This function is used to set the value of the attribute of the record. 
--> Here also we will verify the datatype of the attribute value before setting the value.