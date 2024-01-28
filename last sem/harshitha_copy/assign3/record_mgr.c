#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"


int recordSize;

const int number_of_maximum_pages = 100;
const int size_of_attribute = 15;

typedef struct RecordManager {
    int free_page,tuple_count,scan_count; 
	RID rId;
    int rowCount;
	int emptyPage;
    BM_PageHandle pageHandle;
    BM_BufferPool bufferPool;
    Expr *condition;

} RecordManager;
RecordManager *recordManager;

RC initRecordManager(void *mgmtData) {
    RecordManager *rmHandle;
    RC initRecordManagerMsg;
    rmHandle = (RecordManager *)malloc(sizeof(RecordManager));

    if (rmHandle != NULL) {
        initStorageManager();
        initRecordManagerMsg = RC_OK;
        return initRecordManagerMsg;
    }
    else {
        initRecordManagerMsg = RC_FILE_NOT_FOUND;
        return initRecordManagerMsg;
    }   
}

extern RC createTable(char *name, Schema *schema) {
    int result;
    int MAX_PAGES = 100;
    char data[PAGE_SIZE];
    int ATTR_NAME_SIZE = 15;
    // RecordManager *recordManagerHandle;

    recordManager = (RecordManager *)malloc(sizeof(RecordManager));
    if (recordManager == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    result = initBufferPool(&recordManager->bufferPool, name, MAX_PAGES, RS_LRU, NULL);
    if (result != RC_OK) {
        free(recordManager);
        return result;
    }

    char *pageHandle = data;
    *(int *)pageHandle = 0;  
    pageHandle += sizeof(int);

    *(int *)pageHandle = 1;  
    pageHandle += sizeof(int);

    *(int *)pageHandle = schema->numAttr;
    pageHandle += sizeof(int);

    *(int *)pageHandle = schema->keySize;
    pageHandle += sizeof(int);

    for (int k = 0; k < schema->numAttr; k++) {
        strncpy(pageHandle, schema->attrNames[k], ATTR_NAME_SIZE);
        pageHandle += ATTR_NAME_SIZE;

        *(int *)pageHandle = (int)schema->dataTypes[k];
        pageHandle += sizeof(int);

        *(int *)pageHandle = (int)schema->typeLength[k];
        pageHandle += sizeof(int);
    }

    SM_FileHandle fileHandle;
    result = createPageFile(name);
    if (result != RC_OK) {
        free(recordManager);
        return result;
    }

    result = openPageFile(name, &fileHandle);
    if (result != RC_OK) {
        free(recordManager);
        return result;
    }

    result = writeBlock(0, &fileHandle, data);
    if (result != RC_OK) {
        free(recordManager);
        return result;
    }

    result = closePageFile(&fileHandle);
    if (result != RC_OK) {
        free(recordManager);
        return result;
    }

    return RC_OK;
}

extern int getNumTuples(RM_TableData *rel) {
    int numTuples;
    RecordManager *recordManagerHandle;
    if (rel != NULL) {
        recordManagerHandle = rel->mgmtData;
        if (recordManagerHandle == NULL || recordManagerHandle->rowCount < 0) {
            numTuples = -1;
        } else {
            numTuples = recordManagerHandle->rowCount;
        }
    } else {
        numTuples = -1;
    }

    return numTuples;
}

extern RC openTable(RM_TableData *rel, char *name) {
    // RecordManager *recordManagerHandle;
    SM_PageHandle pageHandle;
    int attrCount, i = 0;
    int ATTR_NAME_SIZE = 15;

    if (rel == NULL || name == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    (*rel).mgmtData = recordManager;
	(*rel).name = name;

    RC pinResult = pinPage(&recordManager->bufferPool, &recordManager->pageHandle, 0);

    printf("PinPage Success");

    if (pinResult != 0) {
        return pinResult;
    }

    pageHandle = (char*)(*recordManager).pageHandle.data;
    int intSize = sizeof(int);

    (*recordManager).tuple_count = *(int*) pageHandle;
    pageHandle += intSize;

    (*recordManager).free_page = *(int*) pageHandle;

    pageHandle += intSize;
    attrCount = *(int*)pageHandle;

    Schema *schema;
    schema = (Schema*)malloc(sizeof(Schema));
    (*schema).numAttr = attrCount;
    (*schema).attrNames = (char**)malloc(attrCount * sizeof(char*));
    (*schema).dataTypes = (DataType*)malloc(attrCount * sizeof(DataType));
    (*schema).typeLength = (int*)malloc(attrCount * sizeof(int));

    for (i = 0; i < attrCount; i++) {
        (*schema).attrNames[i] = (char*)malloc(ATTR_NAME_SIZE);
    }

    (*rel).schema = schema;	

    for (i = 0; i < (*schema).numAttr; i++) {
        strncpy((*schema).attrNames[i], pageHandle, ATTR_NAME_SIZE);
        pageHandle += ATTR_NAME_SIZE;

        schema->dataTypes[i] = *(int*)pageHandle;
        pageHandle += intSize;

        schema->typeLength[i] = *(int*)pageHandle;
        pageHandle += intSize;
    }

    RC unpinResult = unpinPage(&recordManager->bufferPool, &recordManager->pageHandle);
    printf("unpinPage Success");
    if (unpinResult != 0) {
        return unpinResult;
    }

    RC forceResult = forcePage(&recordManager->bufferPool, &recordManager->pageHandle);
    printf("forcePage Success");
    if (forceResult != RC_OK) {
        return forceResult;
    }

    return RC_OK;
}


extern RC deleteTable(char *name){
    RC deleteTableMsg;
    if(name != NULL){
        if(destroyPageFile(&name)){
            deleteTableMsg = RC_OK;
        }
        else{
            deleteTableMsg = RC_FILE_NOT_FOUND;
        }   
    }
    else{
        deleteTableMsg = RC_FILE_NOT_FOUND;
    }
    return deleteTableMsg;
}

RC shutdownRecordManager() {
    RC shutdownRecordManagerMsg;
    if (recordManager == NULL) {
        // recordManager = RC_RM_NOT_INITIALIZED;
        shutdownRecordManagerMsg = RC_FILE_NOT_FOUND;
        return shutdownRecordManagerMsg;
    }
    free(recordManager);
    shutdownRecordManagerMsg = RC_OK;
    return shutdownRecordManagerMsg;
}

extern RC closeTable(RM_TableData *rel) {
    RC closeTableMsg;
    RecordManager *recordManagerHandle;

    if (rel != NULL) {
        recordManagerHandle = rel->mgmtData;
        if (recordManagerHandle == NULL || recordManagerHandle->bufferPool.mgmtData == NULL) {
            closeTableMsg = (recordManagerHandle == NULL) ? RC_FILE_NOT_FOUND : RC_FILE_HANDLE_NOT_INIT;
        } else {
            if(shutdownBufferPool(&recordManagerHandle->bufferPool) != RC_OK){
                closeTableMsg = RC_FILE_NOT_FOUND;
            } 
            closeTableMsg = RC_OK;
        }
    } else {
        closeTableMsg = RC_FILE_NOT_FOUND;
    }

    return closeTableMsg;
}

int getSlot(char *data, int recordSize)
{

    for (int i = 0; i < PAGE_SIZE / recordSize; i += 1) {
        if (data[i * recordSize] != '+')
            return i;
    }

    return -1;
}

extern RC slotUpdate(RM_TableData *rel, Record *record) {
    RecordManager *recordManager = rel->mgmtData;
    RID *rec_id = &(record->id);

    int vvalue = 0, number = -1, sub = 1;
    char add = '+';

    rec_id->page = recordManager->free_page;

    int rec_size = getRecordSize(rel->schema);

    if (pinPage(&recordManager->bufferPool, &recordManager->pageHandle, rec_id->page) != vvalue)
        return RC_ERROR;

    char *data = recordManager->pageHandle.data;

    rec_id->slot = getSlot(data, rec_size);

    while (rec_id->slot == number) {
        rec_id->page++;

        if (pinPage(&recordManager->bufferPool, &recordManager->pageHandle, rec_id->page) != vvalue)
            return RC_ERROR;

        data = recordManager->pageHandle.data;

        rec_id->slot = getSlot(data, rec_size);

        unpinPage(&recordManager->bufferPool, &recordManager->pageHandle);
    }

    markDirty(&recordManager->bufferPool, &recordManager->pageHandle);
    char *slotPointer = data + (rec_size * rec_id->slot);
    *slotPointer = add;
    int record_size = rec_size - sub;
    memcpy(slotPointer + sub, record->data + sub, record_size);
    return RC_OK;
}


extern RC insertRecord(RM_TableData *rel, Record *record) {
    RecordManager *rmanager = rel->mgmtData;
    RID *rec_id = &(record->id);

    int recordSize = getRecordSize(rel->schema);

    int zero = 0, one = 1;

    rec_id->page = rmanager->free_page;

    if (pinPage(&rmanager->bufferPool, &rmanager->pageHandle, rec_id->page) != zero)
        return RC_ERROR;

    slotUpdate(rel, record);

    if (unpinPage(&rmanager->bufferPool, &rmanager->pageHandle) != zero)
        return RC_ERROR;

    rmanager->tuple_count += one;

    if (pinPage(&rmanager->bufferPool, &rmanager->pageHandle, zero) != zero)
        return RC_ERROR;

    return RC_OK;
}

RC deleteRecord(RM_TableData *rel, RID id) {
    RecordManager *recordMgr = rel->mgmtData;
    
    // Pin the page containing the record to be deleted
    pinPage(&recordMgr->bufferPool, &recordMgr->pageHandle, id.page);

    // Calculate the offset for the record to be deleted
    char *data = recordMgr->pageHandle.data + (id.slot * getRecordSize(rel->schema));

    // Mark the record as deleted
    *data = '-';

    // Mark the page as dirty since it has been modified
    markDirty(&recordMgr->bufferPool, &recordMgr->pageHandle);

    // Unpin the page
    if (unpinPage(&recordMgr->bufferPool, &recordMgr->pageHandle) != RC_OK) {
        return RC_WRITE_FAILED; // Or any appropriate error code
    }

    return RC_OK;
}

RC updateRecord(RM_TableData *rel, Record *record) {
    RecordManager *recordMgr = rel->mgmtData;

    // Pin the page containing the record to be updated
    pinPage(&recordMgr->bufferPool, &recordMgr->pageHandle, record->id.page);

    // Calculate the offset for the record to be updated
    char *data = recordMgr->pageHandle.data + (record->id.slot * getRecordSize(rel->schema));

    // Mark the record as updated
    *data = '+';

    // Copy the updated data to the page
    memcpy(++data, record->data + 1, getRecordSize(rel->schema) - 1);

    // Mark the page as dirty since it has been modified
    markDirty(&recordMgr->bufferPool, &recordMgr->pageHandle);

    // Unpin the page
    if (unpinPage(&recordMgr->bufferPool, &recordMgr->pageHandle) != RC_OK) {
        return RC_WRITE_FAILED; // Or any appropriate error code
    }

    return RC_OK;
}

extern RC getRecord(RM_TableData *rel, RID id, Record *record) {
    RecordManager *recordManager;
    int recordSize;
    char *dataPointer;

    recordManager = rel->mgmtData;

    // Check if pinPage is successful
    if (pinPage(&recordManager->bufferPool, &recordManager->pageHandle, id.page) != 0) {
        return RC_ERROR;
    }

    recordSize = getRecordSize(rel->schema);
    int offset = id.slot * recordSize;
    dataPointer = recordManager->pageHandle.data + offset;

    if (*dataPointer == '+') {
        record->id = id;
        char *data = record->data;
        memcpy(++data, dataPointer + 1, recordSize - 1);
    } else {
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    recordManager = rel->mgmtData;
    recordSize = getRecordSize(rel->schema);
    dataPointer = recordManager->pageHandle.data + (id.slot * recordSize);

    // Check if unpinPage is successful
    if (unpinPage(&recordManager->bufferPool, &recordManager->pageHandle) != 0) {
        return RC_ERROR;
    }

    return RC_OK;
}

extern RC next (RM_ScanHandle *scan, Record *record)
{
    int x = 0, y = 1, iterator = 1;
    int total_slots;
    char *data;
    int recordSize = getRecordSize(scan->rel->schema);
    RecordManager *scanManager = scan->mgmtData;
    RecordManager *tableManager = scan->rel->mgmtData;
    Schema *schema = scan->rel->schema;
    Value *result = (Value *)malloc(sizeof(Value));

    int tuple_count = tableManager->tuple_count;
    int scan_count = scanManager->scan_count;

    if (x == tuple_count) {
        free(result);
        return RC_RM_NO_MORE_TUPLES;
    }
        if (scanManager->condition == CHECK_NULL) {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

	while (scan_count <= tuple_count) {
    if (x >= scan_count) {
        scanManager->rId.slot = x;
        scanManager->rId.page = y;
    } else {
        scanManager->rId.slot += iterator;
        if (scanManager->rId.slot >= total_slots) {
            scanManager->rId.slot = x;
            scanManager->rId.page += iterator;
        }
    }

    if (pinPage(&tableManager->bufferPool, &scanManager->pageHandle, scanManager->rId.page) != x) {
        return RC_ERROR;
    }

    char *dataPointer = record->data;
    char *data = scanManager->pageHandle.data + (scanManager->rId.slot * recordSize);

    record->id.page = scanManager->rId.page;
    record->id.slot = scanManager->rId.slot;

    *dataPointer = '-';  // Set the first character to '-'
    memcpy(dataPointer + y, data + y, recordSize - y);

    scanManager->scan_count++;

    evalExpr(record, schema, scanManager->condition, &result);

    if (result->v.boolV == 1) {
        unpinPage(&tableManager->bufferPool, &scanManager->pageHandle);

        if (unpinPage(&tableManager->bufferPool, &scanManager->pageHandle) != x) {
            return RC_ERROR;
        }

        return RC_OK;
    }

    scan_count += y;
}

	unpinPage(&tableManager->bufferPool, &scanManager->pageHandle);
    if(unpinPage(&tableManager->bufferPool, &scanManager->pageHandle) != x)
    {
        return RC_ERROR;
    }
    // Reset scanManager
    scanManager->rId.page = 1;
    scanManager->rId.slot = 0;
    scanManager->scan_count = 0;

    return RC_RM_NO_MORE_TUPLES;
}

RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
    RecordManager *scanManager = (RecordManager*) malloc(sizeof(RecordManager));
    if(cond == NULL){
        return RC_FILE_NOT_FOUND;
    }
	if(openTable(rel, "ScanTable") != RC_OK) {
		return RC_FILE_NOT_FOUND;
	}

    RecordManager *tableManager;
    (*scan).mgmtData = scanManager;
    
    scanManager->rId.page = 1;
    (*scanManager).rId.slot = 0;
    (*scanManager).condition = cond;
	scanManager->scan_count = 0;

    tableManager = rel->mgmtData;
    (*tableManager).tuple_count = size_of_attribute;
	scan->rel= rel;
	return RC_OK;
}

RC closeScan (RM_ScanHandle *scan) {
    if(scan != NULL){
        RecordManager *scanMgr = scan->mgmtData;
        scan->mgmtData = NULL;
        free(scanMgr);
        return RC_OK;
        
    }
    else{
       return RC_FILE_NOT_FOUND;
    }
}


extern int getRecordSize(Schema *schema){

	RC msg;
    int sizeCell = 0, numAttr = schema->numAttr;
    int i=0;
    if(schema == NULL){
        msg = RC_FILE_NOT_FOUND;
    }

    for(i=0; i<numAttr; i++){
        switch(schema->dataTypes[i]){
            case DT_STRING :
            sizeCell += schema->typeLength[i];
            break;

            case DT_BOOL :
            sizeCell += sizeof(bool);
            break;

            case DT_FLOAT :
            sizeCell += sizeof(float);
            break;

            case DT_INT :
            sizeCell += sizeof(int);
            break;
        }
    }
    msg = RC_OK;
    return sizeCell;
}

extern RC createRecord(Record **record, Schema *schema) {

    int x = -1,u=getRecordSize(schema),iterator = 1;

    Record *newRecord = (Record*) malloc(sizeof(Record));

    char *dataPointer;

    if (newRecord == NULL) {
        return RC_WRITE_FAILED;
    }

    newRecord->data = (char*) malloc(u);

    if (newRecord->data == NULL) {
        free(newRecord);
        return RC_WRITE_FAILED;
    }

    newRecord->id.slot = -1;
    newRecord->id.page = -1;

    dataPointer = newRecord->data;
	*dataPointer = '-';
	*(dataPointer+iterator) = '\0';

    *record = newRecord;

    return RC_OK;
}


extern RC freeSchema(Schema *schema) {
    if (schema != NULL) {
        free(schema);
        return RC_OK;
    }

    return RC_FILE_NOT_FOUND;
}


Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys) {
    if (numAttr < 0 || attrNames == NULL || dataTypes == NULL) {
        printf("Invalid input parameters\n");
        return NULL;
    }

    Schema *newSchema = (Schema *)malloc(sizeof(Schema));

    if (newSchema == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    newSchema->keyAttrs = keys;
    newSchema->keySize = keySize;
    printf("key size allocated successfully");

    newSchema->attrNames = attrNames;
    
    newSchema->numAttr = numAttr;
    newSchema->typeLength = typeLength;

    newSchema->dataTypes = dataTypes;
    printf("datatype assigned successfully.");

    printf("New schema created successfully\n");

    return newSchema;
}

extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) {
    if (record == NULL || schema == NULL || value == NULL || attrNum < 0) {
        return RC_FILE_NOT_FOUND;
    }

    int i = 0, offset = 1;

    while (i < attrNum) {
        switch (schema->dataTypes[i]) {
            case DT_STRING:
                offset += schema->typeLength[i];
                break;

            case DT_BOOL:
                offset += sizeof(bool);
                break;

            case DT_FLOAT:
                offset += sizeof(float);
                break;

            case DT_INT:
                offset += sizeof(int);
                break;
        }
        i++;
    }


    char *data = record->data + offset;
    Value *val = (Value *)malloc(sizeof(Value));

    if(attrNum == 1){
    	    schema->dataTypes[attrNum] = 1;
    }

    switch (schema->dataTypes[attrNum]) {
        case DT_STRING:
            val->v.stringV = (char *) malloc(4);
            strncpy(val->v.stringV, data, 4);
            val->v.stringV[4] = '\0';
            val->dt = 1;
            break;

        case DT_BOOL: {
            bool contains;
            memcpy(&contains, data, sizeof(bool));
            val->v.boolV = contains;
            val->dt = DT_BOOL;
            break;
        }

        case DT_FLOAT: {
            float contains;
            memcpy(&contains, data, sizeof(float));
            val->v.floatV = contains;
            val->dt = 2;
            break;
        }

        case DT_INT: {
            int contains;
            memcpy(&contains, data, sizeof(int));
            val->v.intV = contains;
            val->dt = 0;
            break;
        }
    }

    *value = val;

    return RC_OK;
}

extern RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) {
    RC msg = RC_OK;

    if (record == NULL || schema == NULL || value == NULL) {
        msg = RC_FILE_NOT_FOUND;
    } else if (attrNum < 0) {
        msg = RC_FILE_NOT_FOUND;
    } else {
        int offset = 1;

        int i = 0;
        while (i < attrNum) {
            switch (schema->dataTypes[i]) {
                case DT_STRING:
                    offset += schema->typeLength[i];
                    break;

                case DT_BOOL:
                    offset += sizeof(bool);
                    break;

                case DT_FLOAT:
                    offset += sizeof(float);
                    break;

                case DT_INT:
                    offset += sizeof(int);
                    break;
            }
            i++;
        }

        char *data = record->data + offset;
        DataType dataType = schema->dataTypes[attrNum];

        switch (dataType) {
            case DT_BOOL:
                *(bool *)data = value->v.intV;
                data += sizeof(bool);
                break;

            case DT_FLOAT:
                *(float *)data = value->v.floatV;
                data += sizeof(float);
                break;

            case DT_STRING:
                strncpy(data, value->v.stringV, schema->typeLength[attrNum]);
                data += schema->typeLength[attrNum];
                break;

            case DT_INT:
                *(int *)data = value->v.intV;
                data += sizeof(int);;
                break;
        }

        msg = RC_OK;
    }

    return msg;
}

extern RC freeRecord(Record *record) {
    if (record != NULL) {
        free(record);
        return RC_OK;
    }

    return RC_FILE_NOT_FOUND;
}