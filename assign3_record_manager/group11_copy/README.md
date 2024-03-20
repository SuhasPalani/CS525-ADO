---

CS 525 - Advanced Database Organisation
Assignment 3 - Record Manager

---

        Group 11 Members:

---

Name - Suhas Palani
CWID - A20548277
Email- spalani3@hawk.iit.edu

Name - Bhoomika Panduranga
CWID - A20503493
Email- bpanduranga@hawk.iit.edu

Name - Yashas Shashidhar  
CWID - A20516827
Email- yshashidhar@hawk.iit.edu

=======================================
RUNNING THE SCRIPT
=======================================

1. Launch a Terminal window and navigate to Project root (assign3).

2. To verify that we are in the right directory, use ls to list the files.

3. To remove outdated compiled.o files, type "make clean".

4. Press "make" to compile the "test assign3 1.c" file as well as the rest of the project files.

5. To run the "test assign3 1.c" program, use "make run".

=======================================
SOLUTION DESCRIPTION
=======================================

When creating this record manager, we made sure that there would be sufficient memory management by releasing any reserved space when needed and using fewer variables.

=======================================

1. # TABLE AND RECORD MANAGER FUNCTIONS

---

## Author: Suhas Palani

## findFreeSlot

Description:
This function is responsible for finding a free slot within a page where a record can be inserted. It takes the data pointer to the page and the size of the record as input parameters. The function iterates through the page's slots, checking if each slot is empty (marked with '+'). If an empty slot is found, the index of that slot is returned. If no free slot is available, the function returns -1, indicating that the page is full and a new record cannot be inserted until the page is expanded.

InitRecordManager
--> Used to perform initialization for the record manager.
--> Storage manager function is called to perform initialization of the storage manager.

---

## Author: Suhas Palani

## shutdownRecordManager

Description:
This function is responsible for shutting down the record manager and deallocating any resources that were allocated during initialization. It typically involves shutting down the buffer pool and releasing any memory associated with the record manager. Upon successful shutdown, it returns RC_OK; otherwise, it returns an error code indicating the failure.

---

## Author: Suhas Palani

## closeTable

Description:
The closeTable function is used to close a table with the given name. It involves performing necessary cleanup operations, such as shutting down the buffer pool associated with the table and releasing any resources allocated during table operations. If the operation is successful, it returns RC_OK; otherwise, it returns an appropriate error code.

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

======================================= 2. RECORD FUNCTIONS -
=======================================
These operations are used to insert a new record, delete a record with a certain RID,
retrieve a record with a specific RID, and update a record with new data.

---

## Author: Suhas Palani

## insertRecord

Description:
The insertRecord function is responsible for inserting a new record into the table. It takes the table data and the record to be inserted as input parameters. The function locates a suitable slot within a page where the record can be inserted, marks the slot as occupied, and copies the record data into the page. If the insertion is successful, it returns RC_OK; otherwise, it returns an error code.

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

---

## Author: Suhas Palani

## getRecord

Description:
The getRecord function retrieves a record with a specific Record ID (RID) from the table. It locates the page containing the record, retrieves the record data from the page, and populates the provided record structure with the retrieved data. If the record is found, it returns RC_OK; otherwise, it returns an appropriate error code.

======================================= 3. SCAN FUNCTIONS
=======================================

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

---

## Author: Suhas Palani

## closeScan

Description:
The closeScan function is used to close a scan operation initiated using the RM_ScanHandle. It involves performing necessary cleanup operations, such as releasing any resources associated with the scan and resetting scan-related variables. If the operation is successful, it returns RC_OK; otherwise, it returns an appropriate error code.

========================================= 4. SCHEMA FUNCTIONS
=========================================

---

## Author: Bhoomika Panduranga

The Schema functions are used to create the schema, get the size of records in the schema and to free the schema.

getRecordSize
--> This function is used to get the size of records of given schema in bytes.
--> Here we check for the datatype of attributes in schema and add all the size to get the record size.

---

## Author: Suhas Palani

## freeSchema

Description:
The freeSchema function deallocates the memory occupied by the schema structure. It is responsible for releasing any memory resources allocated during schema creation or manipulation. If the deallocation is successful, it returns RC_OK; otherwise, it returns an appropriate error code.

createSchema
--> In this function we create a new schema according to the parameters in memory.
--> We initialize number of attributes, names of attributes, datatype of attributes and length of attributes to the new schema.

free_mem
--> This function is used to free that is deallocate the memory space allocated.

========================================= 5. ATTRIBUTE FUNCTIONS
=========================================

Attribute functions are used to create the record, free the space allocated to the record and to get the attributes of the record as well as in setting the attributes of the records.

CreateRecord
--> This function is used to create the record through allocating the memory space for the record and the parameters passed for the record is allocated to this new record.

---

## Author: Suhas Palani

## freeRecord

Description:
The freeRecord function is used to free the memory occupied by a record structure. It deallocates any memory resources allocated during record creation or manipulation. If the deallocation is successful, it returns RC_OK; otherwise, it returns an appropriate error code.

Posattr
--> This function is used to obtain the position of the attribute.

getAttr
--> This function is used to get the attributes of the record required.
--> Here we will verify the datatype of the attribute and then copy’s the attribute’s value.

setAttr
--> This function is used to set the value of the attribute of the record.
--> Here also we will verify the datatype of the attribute value before setting the value.
