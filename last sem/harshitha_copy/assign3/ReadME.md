# Record Manager

The Record Manager is a simple implementation for handling tables with a fixed schema. Clients can perform various operations on records within a table, including inserting, deleting, updating, and scanning through records based on a specified search condition. This implementation stores each table in a separate page file and utilizes a buffer manager for accessing pages.

## Assignment Overview

### Record Representation

All data types considered for this assignment are of fixed length. The size of a record is fixed for a given schema.

### Page Layout

Records on pages need a defined layout, and space must be reserved on each page for managing entries. Considerations should include representing slots for records on pages and managing free space efficiently.

### Table Information Pages

One or more pages may need to be reserved in the page file to store the schema of the table.

### Record IDs

Record IDs are a combination of page and slot number.

### Free Space Management

To support record deletion, available free space on pages needs to be tracked. Options include linking pages with free space using pointers or using multiple pages to store a directory of free space for each page.

## `tables.h`

### Overview

The `tables.h` header defines basic data structures for schemas, tables, records, record IDs (RIDs), and values. It also provides functions for serializing these data structures as strings.

### Schema

A schema consists of attributes, each with a name and data type. For strings, the size is recorded. The schema can also have a defined key represented as an array of integers.

### Data Types and Binary Representation

Values of a data type are represented using the `Value` struct. The binary representation follows standard C data types. Special considerations are made for strings.

## Record Manager Interface (`record_mgr.h`)

### Function Types

1. **Table and Record Manager Functions**: Initialize and shutdown the record manager, create, open, and close a table. Operations on a table, such as scanning or inserting records, require the table to be opened first. Closing a table ensures outstanding changes are written to the page file. The `getNumTuples` function returns the number of tuples in the table.

2. **Record Functions**: Retrieve, delete, insert, and update records. When inserting a new record, the record manager assigns an RID and updates the record parameter.

3. **Scan Functions**: Initiate a scan to retrieve tuples from a table that fulfill a given condition. Starting a scan initializes the `RM ScanHandle` data structure. The `next` method returns the next tuple based on the scan condition.

4. **Schema Functions**: Functions related to dealing with schemas.

5. **Attribute Values and Record Creation Functions**: Functions for dealing with attribute values and creating records.

## Compiling and Running the Project

To compile and run the program, use the following command:

```make run```

To clean the compiled program, use the following command:

```make clean```

## License

This project is licensed under the Illinois Institute of technology.
Copyright (c) [2023] [Akash Didigi Kashinath], [Harshitha Satish Reddy], [Nikhil Goud]