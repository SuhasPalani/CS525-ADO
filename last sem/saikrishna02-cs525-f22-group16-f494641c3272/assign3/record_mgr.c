#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

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

const int MAX_NUMBER_OF_PAGES = 100;
const int SIZE_OF_ATTRIBUTE = 15; // Size of the name of the attribute

Rec_Manager *recordManager;
Rec_Manager *scan_Manager;
Rec_Manager *table_Manager;

// ******** CUSTOM FUNCTIONS ******** //

// This function returns a free slot within a page
int findFreeSlot(char *data, int recordSize)
{
	int i=0;
	int no_of_Slots = PAGE_SIZE / recordSize; 
	while (i < no_of_Slots)
	{
		if (data[i * recordSize] != '+')
		{
			return i;
		}

		i++;
	}

	return -1;
	
}

void checker()
{

}

extern void free_mem(void *pt) 
{
    free(pt);
}

// ******** TABLE AND RECORD MANAGER FUNCTIONS ******** //

// This function initializes the Record Manager

extern RC initRecordManager (void *mgmtData)
{
	// Initiliazing Storage Manager
	int ret_value;
	initStorageManager();
	ret_value = RC_OK;
	return ret_value;
}

// This functions shuts down the Record Manager
extern RC shutdownRecordManager ()
{	
	int ret_value;
	
	ret_value = shutdownBufferPool(&recordManager->buffer);
	
	if(ret_value == RC_ERROR){
		checker();
		return RC_ERROR;
	}
	recordManager = NULL;
	free(recordManager);
	ret_value = RC_OK;
	return ret_value;
}

// This function creates a TABLE with table name "name" having schema specified by "schema"
extern RC createTable (char *name, Schema *schema)
{
	char data[PAGE_SIZE];
	char *p_handle;
	
	int res=0;
	int i;
	int j;
	SM_FileHandle f_handle;
	int k=0;

	recordManager = (Rec_Manager*)malloc(sizeof(Rec_Manager));

	initBufferPool(&recordManager->buffer, name, MAX_NUMBER_OF_PAGES, RS_LRU, NULL);

	 p_handle = data;
	 i =  0;

	 for( k = 0; k<4; k++) {

		switch(k){

			case 0: *(int*)p_handle = 0;
					printf("*(int*)p_handle = 0; \n");
					break;
			case 1: *(int*)p_handle = 1;
					printf("*(int*)p_handle = 1; \n");
					break;
			case 2 :  *(int*)p_handle = schema->numAttr;
					printf("*(int*)p_handle = schema->numAttr; \n");
					break;
			case 3 : *(int *)p_handle = schema->keySize;
					printf(" *(int *)p_handle = schema->keySize; \n");
					break;

		}

		p_handle = p_handle + sizeof(int);
		printf("%d\n",i);
	 }

	

	 for (i =0; i<schema->numAttr; i++,j++){
		

		strncpy(p_handle, schema->attrNames[i], SIZE_OF_ATTRIBUTE);
		checker();
		p_handle = p_handle + SIZE_OF_ATTRIBUTE;
		
		*(int*)p_handle = (int)schema->dataTypes[i];
		p_handle = p_handle + sizeof(int);
		checker();
		*(int*) p_handle = (int) schema->typeLength[i];
		p_handle = p_handle +sizeof(int);
		checker();

	 }

	if( createPageFile(name) == RC_OK){

		if(openPageFile(name, &f_handle) == RC_OK) {

			if(writeBlock(0,&f_handle,data) == RC_OK){
				
				if(closePageFile(&f_handle) == RC_OK) {
					return RC_OK;
				}
			}
		}
	}

	return RC_ERROR;
	
}

// This function opens the table with table name "name"
extern RC openTable (RM_TableData *rel, char *name)
{


	int atr_count;
	SM_PageHandle p_handle;
	int j;
	int i;
	int k =0;
	int ret_value;

	
	rel->name = name;

	rel->mgmtData = recordManager;
	
	ret_value = pinPage(&recordManager->buffer, &recordManager->pagefiles, 0);

	if(ret_value == RC_ERROR){
		return ret_value;
	}

	p_handle = (char *) recordManager->pagefiles.data;
	
	
	
	for( k = 0; k<4; k++) {

		switch(k){

			case 0: recordManager->count_of_tuples = *(int*)p_handle;
					printf("recordManager->count_of_tuples = *(int*)p_handle; \n");
					break;
			case 1: recordManager->pages_free = *(int*)p_handle;
					printf("recordManager->pages_free = *(int*)p_handle\n");
					break;
			case 2 :  atr_count = *(int*)p_handle;
					printf("atr_count = *(int*)p_handle; \n");
					break;
			

		}

		p_handle = p_handle + sizeof(int);
		printf("%d\n",i);
	 }


	
	Schema *schema;

	schema = (Schema*) malloc(sizeof(Schema));

	(*schema).numAttr = atr_count;
	checker();
	(*schema).attrNames = (char **)malloc(sizeof(char*) * atr_count);
	(*schema).dataTypes = (DataType*)malloc(sizeof(DataType) * atr_count);
	checker();
	(*schema).typeLength = (int*)malloc(sizeof(int) * atr_count);

	for(i = 0; i < atr_count; i++){
	
		schema->attrNames[i] = (char*) malloc(SIZE_OF_ATTRIBUTE);
	
	}
	
	for( j = 0; j<schema->numAttr; j++) {
		strncpy(schema->attrNames[j], p_handle, SIZE_OF_ATTRIBUTE);
		p_handle = p_handle + SIZE_OF_ATTRIBUTE;
		checker();
		schema->dataTypes[j] = *(int*) p_handle;
		p_handle = p_handle +sizeof(int);
		checker();
		schema->typeLength[j] = *(int*)p_handle;
		p_handle =p_handle + sizeof(int);
		checker();
	}

	rel->schema = schema;

	ret_value = unpinPage(&recordManager->buffer, &recordManager->pagefiles);

	if(ret_value == RC_ERROR){
		return ret_value;
	}


	ret_value = forcePage(&recordManager->buffer, &recordManager->pagefiles);

	if(ret_value == RC_ERROR){
		return ret_value;
	}

	ret_value = RC_OK;
	
	return ret_value;
	
}   
  
// CloseTable closes the table which was referenced by "rel"

extern RC closeTable (RM_TableData *rel)
{	
	int ret_value;

	Rec_Manager *rMgr = (*rel).mgmtData;	
	ret_value = shutdownBufferPool(&rMgr->buffer);
	if(ret_value == RC_ERROR){
		checker();
		return ret_value;
	}

	ret_value = RC_OK;
	return ret_value;
}

// deleteTable function deletes the table

extern RC deleteTable (char *name)
{
	int ret_value;
	ret_value = destroyPageFile(name);
	if(ret_value == RC_ERROR){
		checker();
		return ret_value;
	}

	ret_value = RC_OK;
	return ret_value;

}

// The number of tuples or records  in the table 
// referenced by "rel" is returned by getNumTuples function
extern int getNumTuples (RM_TableData *rel)
{
	Rec_Manager *rMgr = (*rel).mgmtData;
	return (*rMgr).count_of_tuples;

}


// ******** RECORD FUNCTIONS ******** //
/**************************
 * Author: Diksha Sharma
**************************/
// insertRecord function inserts a new record into the table
// Also updates the 'record' with the Record ID of he newly inserted record

extern RC insertRecord (RM_TableData *r, Record *rec) {

	RID *rec_ID = &rec->id;
	int ret_value;
	char *data;


	Rec_Manager *rec_Manager = r->mgmtData;	
	rec_ID->page = rec_Manager->pages_free ;

	ret_value = pinPage(&rec_Manager->buffer, &rec_Manager->pagefiles, rec_ID->page);
	
	if(ret_value == RC_ERROR) {
		checker();
		return RC_ERROR;
	}

	data = rec_Manager->pagefiles.data;
	rec_ID->slot = findFreeSlot(data, getRecordSize(r->schema));
	checker();

	while(rec_ID->slot == -1) {
		
		ret_value = unpinPage(&rec_Manager->buffer, &rec_Manager->pagefiles);	
		if(ret_value == RC_ERROR){
			checker();
			return RC_ERROR;
		}
		
		rec_ID->page++;
		checker();

		ret_value = pinPage(&rec_Manager->buffer, &rec_Manager->pagefiles, rec_ID->page);
		
		if(ret_value == RC_ERROR) {
			checker();
			return RC_ERROR;
		}

		data = rec_Manager->pagefiles.data;
		checker();
		rec_ID->slot = findFreeSlot(data, getRecordSize(r->schema));

	} 
	
	char *slot_of_Pointer = data;
	
	markDirty(&rec_Manager->buffer, &rec_Manager->pagefiles);
	slot_of_Pointer = slot_of_Pointer + (rec_ID->slot * getRecordSize(r->schema));
	*slot_of_Pointer = '+';
	memcpy(++slot_of_Pointer, rec->data + 1, getRecordSize(r->schema) - 1);
	
	ret_value = unpinPage(&rec_Manager->buffer, &rec_Manager->pagefiles);
	
	if(ret_value == RC_ERROR){
		checker();
		return RC_ERROR;
	}
	rec_Manager->count_of_tuples++;
	
	ret_value = pinPage(&rec_Manager->buffer, &rec_Manager->pagefiles, 0);
	if(ret_value == RC_ERROR){
		checker();
		return RC_ERROR;
	}
	ret_value = RC_OK;
	return ret_value;	
}


/**************************
 * Author: Diksha Sharma
**************************/
// The deleteRecord function deletes a record in the table

extern RC deleteRecord (RM_TableData *r, RID id) {
	
	char *data;
	int ret_value;

	Rec_Manager *recManager = (Rec_Manager *)r->mgmtData;
	ret_value = pinPage(&recManager->buffer, &recManager->pagefiles, id.page);
	
	if(ret_value == RC_ERROR) {
		checker();
		return RC_ERROR;
	}
	
	recManager->pages_free = id.page;
	data = recManager->pagefiles.data;
	data = data + (id.slot * getRecordSize(r->schema));
	*data = '-';

	markDirty(&recManager->buffer, &recManager->pagefiles);
	checker();

	ret_value = unpinPage(&recManager->buffer, &recManager->pagefiles);
	if(ret_value == RC_ERROR){
		checker();
		return RC_ERROR;
	}
	return RC_OK;

}

// The updateRecord function updates a record in the table referenced by "rel"
/**************************
 * Author: Diksha Sharma
**************************/
extern RC updateRecord (RM_TableData *r, Record *rec)
{	

	char *data;
	RC ret_value;
	RID id;

	Rec_Manager *recManager = (Rec_Manager *)r->mgmtData;
	ret_value = pinPage(&recManager->buffer, &recManager->pagefiles, rec->id.page);
	if(ret_value == RC_ERROR){
		checker();
		return RC_ERROR;
	}

	id = rec->id;
	data = recManager->pagefiles.data;
	data = data + (id.slot * getRecordSize(r->schema));
	*data = '+';
	memcpy(++data, rec->data + 1, getRecordSize(r->schema) - 1 );
	markDirty(&recManager->buffer, &recManager->pagefiles);
	checker();
	ret_value = unpinPage(&recManager->buffer, &recManager->pagefiles);
	if(ret_value == RC_ERROR){
		checker();
		return RC_ERROR;
	}

	return RC_OK;
	
}

/**************************
 * Author: Diksha Sharma
**************************/
// The getRecord function retrieves a record with the given Record ID from the table.
// and record is fetched and stored in rec

extern RC getRecord (RM_TableData *r, RID id, Record *rec) {
	Rec_Manager *recManager = r->mgmtData;
	int ret_value;
	char *Pointer;

	ret_value = pinPage(&recManager->buffer, &recManager->pagefiles, id.page);
	if(ret_value == RC_ERROR){
		checker();
		return RC_ERROR;
	}

	Pointer = recManager->pagefiles.data;
	Pointer = Pointer + (id.slot * getRecordSize(r->schema));
	if(*Pointer != '+')
	{
		// Return error if no matching record for Record ID 'id' is found in the table
		return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
	}
	else
	{
		char *data = rec->data;
		rec->id = id;
		memcpy(++data, Pointer + 1, getRecordSize(r->schema) - 1);
	}
	checker();
	ret_value = unpinPage(&recManager->buffer, &recManager->pagefiles);
	if(ret_value == RC_ERROR){
		checker();
		return RC_ERROR;
	}
	
	return RC_OK;
	
}


// ******** SCAN FUNCTIONS ******** //
/**************************
 * Author: Diksha Sharma
**************************/
extern RC startScan (RM_TableData *r, RM_ScanHandle *s_handle, Expr *condition)
{

	while (condition == NULL)
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	}
	openTable(r, "ScanTable");
	scan_Manager = (Rec_Manager*) malloc(sizeof(Rec_Manager));
    s_handle->mgmtData = scan_Manager;
    checker();
	scan_Manager->r_id.page = 1;
    (*scan_Manager).r_id.slot = 0;
	scan_Manager->count_for_scan = 0;
	printf("");
	scan_Manager->condition = condition;
    
	table_Manager = r->mgmtData;
	checker();
	table_Manager->count_of_tuples = SIZE_OF_ATTRIBUTE;
	(*s_handle).rel= r;
	checker();
	return RC_OK;
}

/**************************
 * Author: Diksha Sharma
**************************/
extern RC next (RM_ScanHandle *scan, Record *rec)
{	
	Rec_Manager *scan_Manager = scan->mgmtData;
	int totalSlots;
	
	Rec_Manager *table_Manager = scan->rel->mgmtData;
	
	Value *output;
	int flag=true;
	
   	 Schema *schema = scan->rel->schema;

	while (scan_Manager->condition == NULL)
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	}

	output = (Value *) malloc(sizeof(Value));
	totalSlots = PAGE_SIZE / getRecordSize(schema);


	while (table_Manager->count_of_tuples == 0)
	{
		return RC_RM_NO_MORE_TUPLES;
	}

	while(scan_Manager->count_for_scan <= table_Manager->count_of_tuples)
	{  
		// If all the tuples have been scanned, execute this block
		if (scan_Manager->count_for_scan <= 0)
		{
			if(flag)
				scan_Manager->r_id.page = 1;
			scan_Manager->r_id.slot = 0;
		}
		else
		{
			scan_Manager->r_id.slot++;
			if(flag){
				if(scan_Manager->r_id.slot >= totalSlots)
				{
					scan_Manager->r_id.slot = 0;
					scan_Manager->r_id.page++;
				}
			}
		}

		pinPage(&table_Manager->buffer, &scan_Manager->pagefiles, scan_Manager->r_id.page);
		char *data = scan_Manager->pagefiles.data;
		data = data + (scan_Manager->r_id.slot * getRecordSize(schema));
		rec->id.page = scan_Manager->r_id.page;
		rec->id.slot = scan_Manager->r_id.slot;
		checker();
		char *dataPointer = rec->data;
		*dataPointer = '-';
		
		memcpy(++dataPointer, data + 1, getRecordSize(schema) - 1);
		scan_Manager->count_for_scan++;
		
		evalExpr(rec, schema, scan_Manager->condition, &output); 
		while (output->v.boolV == TRUE)
		{
			unpinPage(&table_Manager->buffer, &scan_Manager->pagefiles);		
			checker();
			return RC_OK;
		}
	}
	checker();
	unpinPage(&table_Manager->buffer, &scan_Manager->pagefiles);
	scan_Manager->r_id.page = 1;
	checker();
	scan_Manager->r_id.slot = 0;
	checker();
	scan_Manager->count_for_scan = 0;
	
	return RC_RM_NO_MORE_TUPLES;
}


/**************************
 * Author: Diksha Sharma
**************************/
// This function closes the scan operation.

extern RC closeScan (RM_ScanHandle *scan)
{	
	scan_Manager = scan->mgmtData;
	Rec_Manager *rec_Manager = scan->rel->mgmtData;

	while (scan_Manager->count_for_scan > 0){
		unpinPage(&rec_Manager->buffer, &scan_Manager->pagefiles);
		
		scan_Manager->r_id.page = 1;

		scan_Manager->count_for_scan = 0;
		checker();
		scan_Manager->r_id.slot = 0;
	}
		scan->mgmtData = NULL;
    	free(scan->mgmtData);  
		checker();
	return RC_OK;
}


// ******** SCHEMA FUNCTIONS ******** //

// This function returns the record size of the schema referenced by "schema"
extern int getRecordSize (Schema *schema)
{	
	int val=0,size_dt=0; // changed count to val, count was 0 and loop was while i have changed to do while and made count at 1 as do while executes 1 extra time
	
	while( val < schema->numAttr){
		if(schema->dataTypes[val-1]==DT_INT) // as count/val starts with 1
			size_dt=size_dt+sizeof(int);
		else if(schema->dataTypes[val-1]==DT_FLOAT) 
			size_dt=size_dt+sizeof(float);
		else if(schema->dataTypes[val-1]==DT_STRING)
			size_dt=size_dt+schema->typeLength[val-1];
		else if(schema->dataTypes[val-1]==DT_BOOL)
			size_dt=size_dt+sizeof(bool);
		else
			printf("data type unidentified\n");
		
		val = val + 1;
	}
	size_dt = size_dt + 1;
	return size_dt;

}

// This function creates a new schema
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{	
	
	if(keySize>0 && keySize!=0)  {       // changed if to else condition
     
		Schema *sch=(Schema*)malloc(sizeof(Schema));// changed sc to sch and malloc to calloc
		if(keySize!=0)
    		sch->dataTypes=dataTypes;	
		if(keySize!=0)
			sch->keySize=keySize;
		if(keySize!=0)
			sch->numAttr=numAttr;
		if(keySize!=0)
    		sch->typeLength=typeLength;
		if(keySize!=0)
			sch->keyAttrs=keys;
		if(keySize!=0)
			sch->attrNames=attrNames;
		return sch; 
    }

    return NULL;


}

// This function removes a schema from memory and de-allocates all the memory space allocated to the schema.
extern RC freeSchema (Schema *schema)
{
	// De-allocating memory space occupied by 'schema'
	int ret_value;
	schema = NULL;
	free_mem(schema); 
    ret_value = RC_OK; // added ret_value
	return ret_value;
}


// ******** DEALING WITH RECORDS AND ATTRIBUTE VALUES ******** //

// This function creates a new record in the schema referenced by "schema"
extern RC createRecord (Record **record, Schema *schema)
{
	int ret_value;
	Record *n_rec = (Record*) malloc(sizeof(Record));// changed malloc to calloc changed new record to n_rec
	printf("allocating memory to new record");
	int size_rec = getRecordSize(schema);
	printf("fetching record size");
	
	n_rec->data= (char*) malloc(size_rec);// changed malloc to calloc
	printf("allocating memory for the data in new record ");
	n_rec->id.page = n_rec->id.slot = -1;
	
	char *pointer_data = n_rec->data;
	
	// '-' is used for Tombstone mechanism. We set it to '-' because the record is empty.
	*pointer_data = '-';
	pointer_data = pointer_data + 1;
	// Append '\0' which means NULL in C to the record after tombstone. ++ because we need to move the position by one before adding NULL
	*(pointer_data) = '\0';

	// Set the newly created record to 'record' which passed as argument
	*record = n_rec;
	ret_value=RC_OK;
	return ret_value;	


}

// This function sets the offset (in bytes) from initial position to the specified attribute of the record into the 'result' parameter passed through the function
RC attrOffset (Schema *schema, int attrNum, int *result)
{	
		*result = 1;
	int k=0;
	int ret_value = RC_OK;
	if(attrNum>=0){
		execute:
		if(schema->dataTypes[k]== DT_STRING){
				if(k<attrNum)
					*result = *result + (*schema).typeLength[k];
		}
		else if(schema->dataTypes[k]== DT_INT){
				if(k<attrNum)
					*result=sizeof(int)+*result;
		}else if(schema->dataTypes[k]== DT_BOOL){
				if(k<attrNum)
					*result=sizeof(bool)+*result;	
		}else if(schema->dataTypes[k]== DT_FLOAT){
				if(k<attrNum)
					*result=sizeof(float)+*result;
		}else
				printf("Incorrect Datatype\n");
		k++;
		if(k<attrNum)
			goto execute;

	}
	return ret_value;


}

// This function removes the record from the memory.
extern RC freeRecord (Record *record)
{
	// De-allocating memory space allocated to record and freeing up that space
	int ret_value;
	free_mem(record); // used free mem func
    ret_value=RC_OK;
	return ret_value;
}

// This function retrieves an attribute from the given record in the specified schema
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
	int val=0;//position to val
	int ret_value;
	if(attrNum<0){
		//ret_value= RC_SCHEMA_ERROR;
		ret_value = RC_ERROR;
	}
	else{
		char *pointer_d = (*record).data; 
		attrOffset(schema,attrNum,&val);
		Value *attr = (Value*) malloc(sizeof(Value));//malloc to calloc attribute to attr
		
		pointer_d = pointer_d + val;
		
		if(val!=0){
			if(attrNum!= 1)
				schema->dataTypes[attrNum] = (*schema).dataTypes[attrNum];
			else
				schema->dataTypes[attrNum] = 1;
			
		}
		if(val!=0){
			if((*schema).dataTypes[attrNum]==DT_INT){
					if(val!=0){
						int value = 0;
						memcpy(&value, pointer_d, sizeof(int));
						if(val!=0)
							attr->dt = DT_INT;
						attr->v.intV = value;
					}
			}
			else if((*schema).dataTypes[attrNum]==DT_STRING){
					if(val!=0){
						int attrnum_le = (*schema).typeLength[attrNum];
						(*attr).v.stringV = (char *) malloc(attrnum_le + 1);
						strncpy((*attr).v.stringV, pointer_d,attrnum_le);
						if(val!=0)
							(*attr).v.stringV[attrnum_le] = '\0';
						(*attr).dt = DT_STRING;
					}
			}
			else if((*schema).dataTypes[attrNum]== DT_BOOL){
					if(((*schema).dataTypes[attrNum])==DT_BOOL){
						bool value;
						memcpy(&value,pointer_d, sizeof(bool));
						if(val!=0)
							(*attr).v.boolV = value;
						(*attr).dt = DT_BOOL;
					}
			}
			else if((*schema).dataTypes[attrNum]== DT_FLOAT){
					if(val!=0){
						float value;
						memcpy(&value, pointer_d, sizeof(float));
						if(val!=0)
							attr->dt=DT_FLOAT;
						attr->v.floatV=value;
					}
			}
			else
				printf("No Serializer available for datatype \n");
		
			*value = attr;
			ret_value=RC_OK;
		}	
	}
	return ret_value;
}

// This function sets the attribute value in the record in the specified schema
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
		int val = 0;
	int ret_value = RC_OK;
	if(attrNum >= 0){	
	attrOffset(schema,attrNum,&val);
	char *pointer_d = (*record).data;
	pointer_d=pointer_d+val;
	if(schema->dataTypes[attrNum]== DT_INT){
		if(attrNum>=0){
			*(int *)pointer_d=(*value).v.intV;	  
			pointer_d =sizeof(int)+pointer_d;
		}
	}
	else if(schema->dataTypes[attrNum]==DT_FLOAT){
		if(attrNum>=0){
			*(float *)pointer_d=(*value).v.floatV;
			pointer_d=sizeof(float)+pointer_d;
		}
	}
	else if(schema->dataTypes[attrNum]==DT_STRING){
		if(attrNum>=0){
			strncpy(pointer_d,(*value).v.stringV,(*schema).typeLength[attrNum]);
			pointer_d=(*schema).typeLength[attrNum]+pointer_d;
		}
	}
	else if(schema->dataTypes[attrNum]== DT_BOOL){
		if(attrNum>=0){
			*(bool*)pointer_d=(*value).v.boolV;
			pointer_d=sizeof(bool)+pointer_d;
		}
	}
	else 
		printf("Datatype not available\n");

	}
			
	return ret_value;

}
