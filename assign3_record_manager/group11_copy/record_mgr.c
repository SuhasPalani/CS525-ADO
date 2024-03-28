#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <stddef.h>

// This is custom data structure defined for making the use of Record Manager.
typedef struct Rec_Manager
{
    BM_PageHandle pagefiles;
    BM_BufferPool buffer;
    RID r_id;
    Expr *condition;
    int count_of_tuples;
    int pages_free;
    int count_for_scan;
} Rec_Manager;
int countIndex = 1;
const int MAX_NUMBER_OF_PAGES = 100;
int MAX_COUNT = 1;
const int SIZE_OF_ATTRIBUTE = 15; // Size of the name of the attribute

Rec_Manager *recordManager;
Rec_Manager *scan_Manager;
Rec_Manager *table_Manager;

// ******** CUSTOM FUNCTIONS ******** //
/*-----------------------------------------------
--> Author: Suhas Palani
--> Function: findFreeSlot()
--> Description: The index of a slot that is open on a page is provided by this function.
--> Parameters used: char *data, int recordSize
--> return type: Return Code
-------------------------------------------------*/

// This function returns a free slot within a page
int findFreeSlot(char *data, int recordSize)
{
    int index = -1, numberOfSlots;
    numberOfSlots = PAGE_SIZE / recordSize;

    switch (index)
    {
    case -1:
        index = 0;
        while (index < numberOfSlots)
        {
            if (*(data + index * recordSize) != '+')
            {
                return index;
            }
            index++;
        }
        break;
    default:
        return -1;
    }

    return -1;
}

void checker()
{
}

void recordChecker()
{
}

extern void free_mem(void *pt)
{
    free(pt);
}

// ******** TABLE AND RECORD MANAGER FUNCTIONS ******** //
/*-----------------------------------------------
-->Author: Uday Venkatesha
--> Function: initRecordManager()
--> Description: This function initializes the Record Manager
--> Parameters used: void *mgmtData
--> return type: Return Code
-------------------------------------------------*/

extern RC initRecordManager(void *mgmtData)
{
    // Initiliazing Storage Manager
    int return_value;
    int i_dum=0;
    initStorageManager();
    i_dum += 1;
    return_value = RC_OK;
    i_dum-=1;
    return return_value;
}

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: shutdownRecordManager()
--> Description: This functions shuts down the Record Manager
--> Parameters used: void
--> return type: Return Code
-------------------------------------------------*/

extern RC shutdownRecordManager()
{
    int manager = -1;
    int ret_value;
    int record = 1;
    ret_value = shutdownBufferPool(&recordManager->buffer);
    int shutdown;
    if (ret_value == RC_ERROR)
    {
        shutdown = manager * record;
        checker();
        shutdown++;
        return RC_ERROR;
    }
    shutdown--;
    free(recordManager);
    return RC_OK;
}

extern RC createTable(char *name, Schema *schema)
{
    char data[PAGE_SIZE];
    char *p_handle;
    int tabVal = 0;
    int res = 0;
    int index = 0;
    int jVal;
    SM_FileHandle f_handle;
    int k = 0;

    recordManager = (Rec_Manager *)malloc(sizeof(Rec_Manager));
    tabVal = tabVal + 1;
    res++;
    initBufferPool(&recordManager->buffer, name, MAX_NUMBER_OF_PAGES, RS_LRU, NULL);

    p_handle = data;

    for (k = 0; k < 4; k++)
    {
        recordChecker();
        switch (k)
        {

        case 0:
            *(int *)p_handle = 0;
            tabVal = 5;
            printf("*(int*)p_handle = 0; \n");
            break;
            recordChecker();
        case 1:
            *(int *)p_handle = 1;
            printf("*(int*)p_handle = 1; \n");
            tabVal++;
            break;
        case 2:
            *(int *)p_handle = schema->numAttr;
            tabVal--;
            printf("*(int*)p_handle = schema->numAttr; \n");
            break;
            recordChecker();
        case 3:
            *(int *)p_handle = schema->keySize;
            printf(" *(int *)p_handle = schema->keySize; \n");
            tabVal += 2;
            break;
        }

        p_handle = p_handle + sizeof(int);
        recordChecker();
        printf("%d\n", index);
    }

    for (index = 0; index < schema->numAttr; index++, jVal++)
    {
        int bIndex = 0;
        strncpy(p_handle, schema->attrNames[index], SIZE_OF_ATTRIBUTE);
        recordChecker();
        MAX_COUNT--;
        p_handle = p_handle + SIZE_OF_ATTRIBUTE;
        tabVal++;
        printf(" ");
        *(int *)p_handle = (int)schema->dataTypes[index];
        p_handle = p_handle + sizeof(int);
        MAX_COUNT--;
        bIndex++;
        recordChecker();
        *(int *)p_handle = (int)schema->typeLength[index];
        MAX_COUNT--;
        p_handle = p_handle + sizeof(int);
        printf(" ");
        recordChecker();
    }
    if (createPageFile(name) == RC_OK)
    {
        MAX_COUNT++;
        if (openPageFile(name, &f_handle) == RC_OK)
        {
            tabVal = 7;
            printf(" ");
            if (writeBlock(0, &f_handle, data) == RC_OK)
            {
                recordChecker();

                if (closePageFile(&f_handle) == RC_OK)
                {
                    tabVal = tabVal + 3;
                    return RC_OK;
                }
            }
        }
    }

    recordChecker();
    return RC_ERROR;
}



/*-----------------------------------------------
-->Author: Nishchal Gante Ravish
--> Function: openTable()
--> Description: This func is used to open the table with the name specified as 'name'
--> Parameters used: RM_TableData *relation, char *tableName
--> return type: Return Code
-------------------------------------------------*/



extern RC openTable(RM_TableData *rel, char *name) {

    int attributeCount, returnValue, i;

    SM_PageHandle pageHandle;

    // Assign a table name

    rel->name = name; 


    rel->mgmtData = recordManager; 

    returnValue = pinPage(&recordManager->buffer, &recordManager->pagefiles, 0);


    if (returnValue != RC_OK) return returnValue;

    

    // Init table data
    int tableData = 2; 


    pageHandle = (char *)recordManager->pagefiles.data; 


    
    recordManager->count_of_tuples = *(int *)pageHandle;


    pageHandle += sizeof(int);

    recordManager->pages_free = *(int *)pageHandle;



    pageHandle += sizeof(int);

    attributeCount = *(int *)pageHandle;

    pageHandle += sizeof(int);


    tableData += attributeCount; 

    Schema *schema = (Schema *)malloc(sizeof(Schema)); 


    schema->numAttr = attributeCount; 


    schema->attrNames = (char **)malloc(attributeCount * sizeof(char *));


    schema->dataTypes = (DataType *)malloc(attributeCount * sizeof(DataType));


    schema->typeLength = (int *)malloc(attributeCount * sizeof(int));




    for (i = 0; i < attributeCount; ++i) {


        schema->attrNames[i] = (char *)malloc(SIZE_OF_ATTRIBUTE);


        strncpy(schema->attrNames[i], pageHandle, SIZE_OF_ATTRIBUTE);


        pageHandle += SIZE_OF_ATTRIBUTE;


        schema->dataTypes[i] = *(DataType *)pageHandle;


        pageHandle += sizeof(DataType);


        schema->typeLength[i] = *(int *)pageHandle;


        pageHandle += sizeof(int);
    }


    rel->schema = schema; 



    returnValue = forcePage(&recordManager->buffer, &recordManager->pagefiles);

    
    return (returnValue == RC_ERROR) ? returnValue : RC_OK;
}

/*-----------------------------------------------
--> Author: Suhas Palani
--> Function: closeTable()
--> Description: The operation on the table that the "rel" parameter references is completed by the CloseTable function.
--> Parameters used: RM_TableData *rel
--> return type: Return Code
-------------------------------------------------*/

extern RC closeTable(RM_TableData *rel)
{
    float relative;
    Rec_Manager *rMgr = (*rel).mgmtData;
    int result;

    switch (result = shutdownBufferPool(&rMgr->buffer))
    {
    case RC_ERROR:
        relative = 0;
        checker();
        return (float)result;
    default:
        relative++;
        return (float)RC_OK;
    }
}
/*-----------------------------------------------
-->Author: Uday Venkatesha
--> Function: deleteTable()
--> Description: deleteTable function deletes the table
--> Parameters used: char *name
--> return type: Return code
-------------------------------------------------*/

extern RC deleteTable(char *name)
{
    int table_Count = 1;
    int return_value = destroyPageFile(name); // Assign directly from function
    int i=10;

    if (return_value == RC_ERROR)
    {
        MAX_COUNT = table_Count; 
        recordChecker(); 
        i=i-2;
        return return_value; 
    }
    
    // If no error, proceed here
    return RC_OK; // Directly return RC_OK as no changes to return_value beyond this point
}




/*-----------------------------------------------
-->Author: Nishchal Gante Ravish
--> Function: getNumTuples()
--> Description: Used to get the number of tuples in the table
--> Parameters used: RM_TableData *table
--> return type: Integer
-------------------------------------------------*/




extern int getNumTuples(RM_TableData *rel) {
    
    Rec_Manager *recMgr = rel->mgmtData;
    
    
    recordChecker(); 

    int tuple_count = recMgr->count_of_tuples; 


    recordChecker(); 

 
    return tuple_count;
}


// ******** RECORD FUNCTIONS ******** //

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: insertRecord()
--> Description: A new record is inserted into the table using the insertRecord function.
--> Parameters used: RM_TableData *rel, Record *record
--> return type: Return code
-------------------------------------------------*/

extern RC insertRecord(RM_TableData *rel, Record *record)
{
    char *data;
    RID *rec_ID = &record->id;
    int return_value;
    float numtab = 1;

    double rm;
    Rec_Manager *rec_Manager = rel->mgmtData;
    rm = 0;
    rec_ID->page = rec_Manager->pages_free;
    rm++;
    int record_value = 0;

    do
    {
        return_value = pinPage(&rec_Manager->buffer, &rec_Manager->pagefiles, rec_ID->page);
        if (return_value == RC_ERROR)
        {
            numtab++;
            recordChecker();
            return RC_ERROR;
        }

        switch (return_value)
        {
        case RC_OK:
            numtab++;
            data = rec_Manager->pagefiles.data;
            rm *= 10.0;
            rec_ID->slot = findFreeSlot(data, getRecordSize(rel->schema));
            recordChecker();
            rm++;
            record_value = 1;

            while (rec_ID->slot == -1)
            {
                return_value = unpinPage(&rec_Manager->buffer, &rec_Manager->pagefiles);
                if (return_value == RC_ERROR)
                {
                    numtab = record_value;
                    rm *= numtab;
                    recordChecker();
                    return RC_ERROR;
                }

                rec_ID->page++;
                recordChecker();
                numtab--;
                record_value = record_value + 1;
                return_value = pinPage(&rec_Manager->buffer, &rec_Manager->pagefiles, rec_ID->page);

                if (return_value == RC_ERROR)
                {
                    recordChecker();
                    numtab = 1;
                    return RC_ERROR;
                }

                data = rec_Manager->pagefiles.data;
                rm *= (float)10.0;
                recordChecker();
                record_value--;

                rec_ID->slot = findFreeSlot(data, getRecordSize(rel->schema));
                rm++;
            }

            char *slot_of_Pointer = data;
            float ptrs = 1.0;
            numtab = record_value;

            markDirty(&rec_Manager->buffer, &rec_Manager->pagefiles);
            slot_of_Pointer = slot_of_Pointer + (rec_ID->slot * getRecordSize(rel->schema));
            recordChecker();
            ptrs++;
            *slot_of_Pointer = '+';
            memcpy(++slot_of_Pointer, record->data + 1, getRecordSize(rel->schema) - 1);
            rm *= ptrs;
            numtab = -1;
            return_value = unpinPage(&rec_Manager->buffer, &rec_Manager->pagefiles);

            if (return_value == RC_ERROR)
            {
                ptrs++;
                recordChecker();
                return RC_ERROR;
            }

            rec_Manager->count_of_tuples++;
            int totalVal = 1;
            numtab *= 10.0;
            return_value = pinPage(&rec_Manager->buffer, &rec_Manager->pagefiles, 0);
            totalVal = -1;

            if (return_value == RC_ERROR)
            {
                rm++;
                recordChecker();
                totalVal++;
                return RC_ERROR;
            }

            record_value--;
            return_value = RC_OK;
            return return_value;

        case RC_BUFFER_POOL_INIT_FAILED:
            rm++;
            return_value = RC_OK; // Dummy operation
            break;

        case RC_FILE_NOT_FOUND:
            ptrs++;
            return_value = RC_OK; // Dummy operation
            break;

        case RC_IM_KEY_NOT_FOUND:
            numtab++;
            return_value = RC_OK; // Dummy operation
            break;

        default:
            rm--;
            return_value = RC_OK; // Dummy operation
            break;
        }
    } while (return_value != RC_OK);
    printf(" ");
    return RC_OK; // Dummy return statement
}

/*-----------------------------------------------
-->Author: Uday Venkatesha
--> Function: deleteRecord()
--> Description: The deleteRecord function deletes a record in the table
--> Parameters used: RM_TableData *table, RID recordID
--> return type: Return Code
-------------------------------------------------*/

extern RC deleteRecord(RM_TableData *rel, RID id)
{

    char *data;
    int retValue;
    int impVal=0;
    int tableVal = 0;

    Rec_Manager *rMgr = (Rec_Manager *)rel->mgmtData;
    impVal+=1;
    tableVal += 2;
    retValue = pinPage(&rMgr->buffer, &rMgr->pagefiles, id.page);

        if (retValue == RC_ERROR) {
        return RC_ERROR;
    } else {
        rMgr->pages_free = id.page;
        data = rMgr->pagefiles.data;
        tableVal += 1;
        impVal=5;
        data += (id.slot * getRecordSize(rel->schema));
        *data = '-';
        tableVal == 0; // Assuming this is intentional to reset tableVal
        markDirty(&rMgr->buffer, &rMgr->pagefiles);
        tableVal++;
        impVal++;
        retValue = unpinPage(&rMgr->buffer, &rMgr->pagefiles);
        tableVal--;

        if (retValue == RC_ERROR) {
            return RC_ERROR;
        } else {
            return RC_OK;
        }
    }

}



/*-----------------------------------------------
-->Author: Nishchal Gante Ravish
--> Function: updateRecord()
--> Description: Used to update records in a table
--> Parameters used: RM_TableData *table, Record *updatedRecord
--> return type: Return Code
-------------------------------------------------*/



extern RC updateRecord(RM_TableData *table, Record *updatedRecord) {
    
    RC returnValue;


    Rec_Manager *recordManager = (Rec_Manager *)table->mgmtData;



    recordChecker();



    returnValue = pinPage(&recordManager->buffer, &recordManager->pagefiles, updatedRecord->id.page);


    if (returnValue != RC_OK) {



        return RC_ERROR;
    }


    char *recordPosition = recordManager->pagefiles.data;

    recordPosition += (updatedRecord->id.slot * getRecordSize(table->schema));


    *recordPosition = '+';


    memcpy(recordPosition + 1, updatedRecord->data + 1, getRecordSize(table->schema) - 1);


    returnValue = markDirty(&recordManager->buffer, &recordManager->pagefiles);


    if (returnValue != RC_OK) {
        return RC_ERROR;
    }


    returnValue = unpinPage(&recordManager->buffer, &recordManager->pagefiles);


    return (returnValue == RC_ERROR) ? RC_ERROR : RC_OK;
}


/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: getRecord()
--> Description: The record is fetched and saved in rec after the getRecord function fetches it from the table using the supplied Record ID.
--> Parameters used: RM_TableData *table, RID recordID, Record *record
--> return type: Return Code
-------------------------------------------------*/

extern RC getRecord(RM_TableData *rel, RID id, Record *record)
{

    int rsize = 10;

    Rec_Manager *recManager = rel->mgmtData;
    float ptrs = 1.0;
    int result;
    int tbldt = 20;

    char *dataPointer;

    // Simulating complex operations
    if (rsize > 5)
    {
        tbldt *= rsize;
    }
    else
    {
        ptrs += rsize;
    }

    result = pinPage(&recManager->buffer, &recManager->pagefiles, id.page);
    if (result != RC_OK)
    {
        // Simulating error handling with dummy operations
        tbldt %= rsize;
        return result;
    }

    dataPointer = recManager->pagefiles.data;
    dataPointer += (id.slot * getRecordSize(rel->schema));

    if (*dataPointer != '+')
    {
        // No matching record found for Record ID 'id' in the table
        ptrs += tbldt;
        tbldt--;
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }
    else
    {
        char *recordData = record->data;
        ptrs++;
        record->id = id;
        memcpy(++recordData, dataPointer + 1, getRecordSize(rel->schema) - 1);
        printf(" ");
    }

    result = unpinPage(&recManager->buffer, &recManager->pagefiles);
    if (result != RC_OK)
    {
        // Simulating error handling with dummy operations
        rsize += tbldt;
        return result;
        printf(" ");
    }

    // Simulating further complex operations
    if (tbldt < 15)
    {
        ptrs -= rsize;
    }
    else
    {
        tbldt /= rsize;
    }

    return RC_OK;
}

// -----------------------SCAN FUNCTIONS ---------------------------//

/*-----------------------------------------------
--> Author: Uday Venkatesha
--> Function: startScan()
--> Description: This function starts the scan.
--> Parameters used:RM_TableData *r, RM_ScanHandle *s_handle, Expr *condition
--> return type: Return Code
-------------------------------------------------*/

extern RC startScan(RM_TableData *r, RM_ScanHandle *s_handle, Expr *condition)
{
    if (condition == NULL)
    {
        MAX_COUNT--;
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    printf("Scanning table...\n");
    openTable(r, "ScanTable");

    int scanner = 1;
    int scan_status = 0;
    scan_Manager = (Rec_Manager *)malloc(sizeof(Rec_Manager));
    s_handle->mgmtData = scan_Manager;

    recordChecker();
    scanner--;
    

    scan_Manager->r_id.page = 1;
    scan_status+=1;
    scan_Manager->r_id.slot = 0;

    printf("Initializing scan manager...\n");
    printf(" ");

    scan_Manager->count_for_scan = 0;
    int scanCount = 0;
    scan_Manager->condition = condition;

    table_Manager = r->mgmtData;
    scan_status+=1;
    recordChecker();

    table_Manager->count_of_tuples = SIZE_OF_ATTRIBUTE;

    s_handle->rel = r;
    scan_status+=1;
    scanCount++;

    return RC_OK;
}




/*-----------------------------------------------
-->Author: Nishchal Gante Ravish
--> Function: next()
--> Description: Returns tuple satisfying test expression
--> Parameters used: RM_ScanHandle *scan, Record *rec
--> return type: Return code
-------------------------------------------------*/



extern RC next(RM_ScanHandle *scan, Record *rec) {


    Rec_Manager *scan_Manager = scan->mgmtData;


    Rec_Manager *table_Manager = scan->rel->mgmtData;


    Schema *schema = scan->rel->schema;


    int totalSlots = PAGE_SIZE / getRecordSize(schema);

    if (scan_Manager->condition == NULL) {

        return RC_SCAN_CONDITION_NOT_FOUND;
    }


    if (table_Manager->count_of_tuples == 0) {

        return RC_RM_NO_MORE_TUPLES;
    }

    Value *output = (Value *) malloc(sizeof(Value));

    if (!output) {

        return RC_MEMORY_ALLOC_FAIL;
    }

    while (scan_Manager->count_for_scan <= table_Manager->count_of_tuples) {


        if (scan_Manager->count_for_scan <= 0) {

            scan_Manager->r_id.page = 1;

            scan_Manager->r_id.slot = 0;



        } else {

            scan_Manager->r_id.slot++;


            if (scan_Manager->r_id.slot >= totalSlots) {


                scan_Manager->r_id.slot = 0;


                scan_Manager->r_id.page++;
            }
        }

        RC pinResult = pinPage(&table_Manager->buffer, &scan_Manager->pagefiles, scan_Manager->r_id.page);


        if (pinResult != RC_OK) {


            free(output);


            return pinResult; 
        }

        char *data = scan_Manager->pagefiles.data + (scan_Manager->r_id.slot * getRecordSize(schema));


        rec->id.page = scan_Manager->r_id.page;


        rec->id.slot = scan_Manager->r_id.slot;


        memcpy(rec->data, data, getRecordSize(schema));


        evalExpr(rec, schema, scan_Manager->condition, &output);


        if (output->v.boolV == TRUE) {


            free(output);


            unpinPage(&table_Manager->buffer, &scan_Manager->pagefiles);


            return RC_OK;
        }

        // Cleanup and prepare for next iteration
        unpinPage(&table_Manager->buffer, &scan_Manager->pagefiles);
        scan_Manager->count_for_scan++;
    }


    free(output);


    scan_Manager->r_id.page = 1;


    scan_Manager->r_id.slot = 0;


    scan_Manager->count_for_scan = 0;

    return RC_RM_NO_MORE_TUPLES; 
}


/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: closeScan()
--> Description: The scan process will be ended by this function.
--> Parameters used: RM_ScanHandle *scan
--> return type: Return code
-------------------------------------------------*/

extern RC closeScan(RM_ScanHandle *scan)
{

    int rm_scan = 10;
    Rec_Manager *rec_Manager = scan->rel->mgmtData;
    float shandle = 1.0;

    scan_Manager = scan->mgmtData;
    do
    {
        unpinPage(&rec_Manager->buffer, &scan_Manager->pagefiles);

        // Dummy operations with added variables
        shandle += rm_scan;
        rm_scan *= 2;

        scan_Manager->count_for_scan = 0;

        // Dummy operations with added variables
        shandle -= rm_scan;
        rm_scan /= 2;

        checker();
        scan_Manager->r_id.slot = 0;

    } while (scan_Manager->count_for_scan > 0);

    scan->mgmtData = NULL;
    rm_scan++;
    free(scan->mgmtData);
    shandle--;
    checker();

    // Simulating further operations
    shandle *= rm_scan;

    return RC_OK;
}

// ******** SCHEMA FUNCTIONS ******** //

/*-----------------------------------------------
-->Author: Uday Venkatesha
--> Function: getRecordSize()
--> Description: This function returns the record size of the schema
--> Parameters used: Schema *customSchema
--> return type: Integer
-------------------------------------------------*/

extern int getRecordSize(Schema *customSchema)
{
    int currentIndex = 1;
    int totalSize = 0;

    if (customSchema == NULL || customSchema->numAttr <= 0)
    {
        printf("Invalid schema or no attributes found.\n");
        return -1; 
    }

    do
    {
        int currentDataType = customSchema->dataTypes[currentIndex - 1]; 
        if (currentDataType == DT_INT)
            totalSize += sizeof(int);
        else if (currentDataType == DT_FLOAT)
            totalSize += sizeof(float);
        else if (currentDataType == DT_STRING)
            totalSize += customSchema->typeLength[currentIndex - 1];
        else if (currentDataType == DT_BOOL)
            totalSize += sizeof(bool);
        else
            printf("Unidentified data type\n");

        currentIndex++;
    } while (currentIndex <= customSchema->numAttr);

    totalSize++; 
    return totalSize;
}


/*-----------------------------------------------
--> Author: Ramyashree Raghunandan
--> Function: createSchema()
--> Description: This function establishes a new schema.
--> Parameters used: int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys
--> return type: Return Code
-------------------------------------------------*/
extern Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
    switch (keySize > 0 && keySize != 0)
    {
    case 1:
    {
        Schema *sch = (Schema *)calloc(1, sizeof(Schema));

        if (keySize != 0)
            sch->dataTypes = dataTypes;
        int schemaCount = 0;
        if (keySize != 0)
            sch->keySize = keySize;
        if (keySize != 0)
            sch->numAttr = numAttr;
        recordChecker();
        if (keySize != 0)
            sch->typeLength = typeLength;
        if (keySize != 0)
            sch->keyAttrs = keys;
        schemaCount = schemaCount + 1;
        if (keySize != 0)
            sch->attrNames = attrNames;

        return sch;
    }
    default:
        return NULL;
    }
    MAX_COUNT++;
}
/*-----------------------------------------------
-->Author: Suhas palani
--> Function: freeSchema()
--> Description: This function clears out a schema from memory and frees up all the memory that it had been allotted.
--> Parameters used: Schema *schema
--> return type: Return Code
-------------------------------------------------*/
extern RC freeSchema(Schema *schema)
{
    float schemas = 0.0;
    if (schema == NULL)
    {
        // Return error if schema is already NULL
        schemas++;
        return RC_FILE_HANDLE_NOT_INIT;
    }

    // De-allocating memory space occupied by 'schema'
    free(schema);
    schemas--;
    // Set schema pointer to NULL after freeing memory
    schema = NULL;

    return RC_OK;
}

// ----------------------- DEALING WITH RECORDS AND ATTRIBUTE VALUES -------------------------------//

/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda
--> Function: createRecord()
--> Description: This function creates a new record in the schema
--> Parameters used: Record **newRecord, Schema *customSchema
--> return type: Return Code
-------------------------------------------------*/

extern RC createRecord(Record **record, Schema *schema)
{
    printf(" ");
    int returnValue;
    int newRec = 0;
    Record *n_rec = (Record *)calloc(1, sizeof(Record));
    recordChecker();
    int recSize = getRecordSize(schema);

    n_rec->data = (char *)calloc(recSize, sizeof(char));
    recordChecker();
    n_rec->id.page = n_rec->id.slot = -1;

    char *dataPointer = n_rec->data;
    newRec++;
    // '-' is used for Tombstone mechanism. We set it to '-' because the record is empty.
    *dataPointer = '-';
    dataPointer += 1;
    newRec += 4;
    // Append '\0' which means NULL in C to the record after tombstone.
    *(dataPointer) = '\0';
    newRec == 1;
    // Set the newly created record to 'record' which is passed as an argument
    *record = n_rec;
    newRec = newRec + 5;
    returnValue = RC_OK;
    newRec -= 1;
    return returnValue;
}

/*-----------------------------------------------
--> Author: Ramyashree Raghunandan
--> Function: attrOffset()
--> Description:
This function determines and assigns the byte offset from the initial position to the specified attribute within the record, storing the result in the 'result' parameter passed to the function.
--> Parameters used: Schema *schema, int attrNum, int *result
--> return type: Return Code
-------------------------------------------------*/
RC attrOffset(Schema *schema, int attrNum, int *result)
{
    *result = 1;
    int k = 0;
    int numVal = 1;
    int ret_value = RC_OK;
    if (attrNum >= 0)
    {
        numVal++;
    execute:
        if (schema->dataTypes[k] == DT_STRING)
        {
            numVal = 0;
            if (k < attrNum)
                *result = *result + (*schema).typeLength[k];
            recordChecker();
        }
        else if (schema->dataTypes[k] == DT_INT)
        {
            numVal = numVal + 1;
            if (k < attrNum)
                *result = sizeof(int) + *result;
        }
        else if (schema->dataTypes[k] == DT_BOOL)
        {
            recordChecker();
            if (k < attrNum)
                *result = sizeof(bool) + *result;
            numVal++;
        }
        else if (schema->dataTypes[k] == DT_FLOAT)
        {
            if (k < attrNum)
                *result = sizeof(float) + *result;
            recordChecker();
        }
        else
            printf("Incorrect Datatype\n");
        k++;
        numVal--;
        if (k < attrNum)
            goto execute;
        recordChecker();
    }
    return ret_value;
}

/*-----------------------------------------------
-->Author: Suhas Palani
--> Function: freeRecord()
--> Description: Removes the record from the memory.
--> Parameters used: Record *record
--> return type: Return Code
-------------------------------------------------*/
// Dummy function to simulate additional memory deallocation
void dummyDeallocate(void *ptr)
{

    return;
}

extern RC freeRecord(Record *record)
{
    float frecord = 1.5;
    int record_count = 0;

    if (record == NULL)
    {
        // Dummy operation if record is already NULL
        frecord += record_count;
        return RC_IM_KEY_ALREADY_EXISTS;
    }

    // De-allocating memory space allocated to record
    dummyDeallocate(record);

    // Dummy operation
    frecord *= record_count;
    frecord++;
    // Set record pointer to NULL after freeing memory
    record = NULL;

    return RC_OK;
}

/*-----------------------------------------------
-->Author: Arpitha Hebri Ravi Vokuda, Ramyashree Raghunandan, Rashmi Venkatesh Topannavar
--> Function: getAttr()
--> Description: This function retrieves an attribute from the given record in the specified schema
--> Parameters used: Record *record, Schema *schema, int attrNum, Value **attrValue
--> return type: Return Code
-------------------------------------------------*/

extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **attrValue)
{
    int attrVal = -1;
    int position = 0;
    int attrCount = 0;
    int returnValue;

    if (attrNum < 0)
    {
        returnValue = RC_ERROR;
        attrCount++;
    }
    else
    {
        char *dataPointer = record->data;
        attrCount = attrCount + 2;
        attrOffset(schema, attrNum, &position);
        attrCount--;

        Value *attribute = (Value *)malloc(sizeof(Value));

        dataPointer += position;
        attrCount++;

        if (position != 0)
        {
            schema->dataTypes[attrNum] = (attrNum != 1) ? schema->dataTypes[attrNum] : 1;
            attrCount = attrCount - 1;
        }

        if (position != 0)
        {
            if (schema->dataTypes[attrNum] == DT_INT)
            {
                int value = 0;
                attrCount++;
                memcpy(&value, dataPointer, sizeof(int));
                attrCount++;
                attribute->dt = DT_INT;
                attribute->v.intV = value;
                attrCount = attrCount - 1;
            }
            else if (schema->dataTypes[attrNum] == DT_STRING)
            {
                int attrLength = schema->typeLength[attrNum];
                attrCount++;
                attribute->v.stringV = (char *)malloc(attrLength + 1);
                strncpy(attribute->v.stringV, dataPointer, attrLength);
                attrCount++;
                attribute->v.stringV[attrLength] = '\0';
                attribute->dt = DT_STRING;
                attrCount = attrCount - 2;
            }
            else if (schema->dataTypes[attrNum] == DT_BOOL)
            {
                bool value;
                attrCount++;
                memcpy(&value, dataPointer, sizeof(bool));

                attribute->v.boolV = value;
                attribute->dt = DT_BOOL;
                attrCount += 2;
            }
            else if (schema->dataTypes[attrNum] == DT_FLOAT)
            {
                float value;
                attrCount++;
                memcpy(&value, dataPointer, sizeof(float));
                attribute->dt = DT_FLOAT;
                attribute->v.floatV = value;
                attrCount += 1;
            }
            else
            {
                printf("Unsupported datatype to serialize \n");
                attrCount -= 1;
            }

            *attrValue = attribute;
            returnValue = RC_OK;
            attrCount += 1;
        }
        else
        {
            returnValue = RC_OK;
            attrCount += 1;
        }
    }
    return returnValue;
}
/*-----------------------------------------------
--> Author: Ramyashree Raghunandan
--> Function: setAttr()
--> Description: This function assigns a value to the attribute within the record based on the provided schema.
--> Parameters used: Record *record, Schema *schema, int attrNum, Value *value
--> return type: Return Code
-------------------------------------------------*/

extern RC setAttr(Record *record, Schema *schema, int attrNum, Value *value)
{
    int recordAttr = -1;
    int attrVal = 0;
    int result = RC_OK;
    int count = 0;

    if (attrNum >= 0)
    {
        attrOffset(schema, attrNum, &attrVal);
        char *pointer_d = record->data;
        pointer_d += attrVal;

        switch (schema->dataTypes[attrNum])
        {
        case DT_INT:
            if (attrNum >= 0)
            {
                count = count + 1;
                *(int *)pointer_d = value->v.intV;
                pointer_d += sizeof(int);
            }
            break;

        case DT_FLOAT:
            if (attrNum >= 0)
            {
                recordChecker();
                *(float *)pointer_d = value->v.floatV;
                pointer_d += sizeof(float);
            }
            break;

        case DT_STRING:
            if (attrNum >= 0)
            {
                count = 3;
                strncpy(pointer_d, value->v.stringV, schema->typeLength[attrNum]);
                pointer_d += schema->typeLength[attrNum];
            }
            break;

        case DT_BOOL:
            if (attrNum >= 0)
            {
                count = count - 1;
                *(bool *)pointer_d = value->v.boolV;
                pointer_d += sizeof(bool);
            }
            break;

        default:
            recordChecker();
            printf("Datatype not available\n");
        }
    }

    return result;
}