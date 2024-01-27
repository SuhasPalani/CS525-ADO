# Buffer Pool Readme

A buffer pool is an essential component of a database management system (DBMS) responsible for efficiently managing pages in memory to improve performance and minimize disk I/O. This readme provides an overview of the key concepts and functions related to buffer pools in a DBMS.

## Buffer Pool Overview

A buffer pool consists of a fixed amount of page frames (pages in memory) used to store disk pages in memory. Clients of the buffer manager can request pages by their position in the page file, which is known as "pinning" a page. The buffer manager's primary responsibilities include:

1. **Pinning Pages:** When a client requests a page, the buffer manager checks whether the page is already cached in a page frame. If so, it returns a pointer to the cached page. Otherwise, it reads the page from disk and determines where to store it (using a replacement strategy). Once found, it returns a pointer to the page frame for the client to read or modify.

2. **Unpinning Pages:** When a client is done with a page, it informs the buffer manager, which is known as "unpinning." The buffer manager also needs to know if the page was modified by the client, as this affects replacement decisions.

3. **Fix Count:** Pages can be pinned by multiple clients, and the number of clients pinning a page is called the "fix count." A fix count greater than zero indicates that at least one client is still using the page. Only pages with a fix count of 0 can be evicted from the buffer pool.

## Buffer Pool Functions

### `initBufferPool`
- Creates a new buffer pool with a specified number of page frames.
- Initializes the pool to cache pages from a given page file.
- Accepts a page replacement strategy and any additional strategy-specific data.
- All page frames are initially empty.

### `shutdownBufferPool`
- Destroys a buffer pool, freeing all associated resources.
- Writes back any dirty pages to disk before destroying the pool.
- It is an error to shut down a buffer pool with pinned pages.

### `forceFlushPool`
- Writes all dirty pages (fix count 0) from the buffer pool to disk.

## Page Management Functions

These functions are used to interact with pages in the buffer pool:

### `pinPage`
- Pins a page by its page number.
- Sets the `pageNum` field of the page handle.
- The `data` field points to the page frame where the page is stored.

### `unpinPage`
- Unpins a page specified by the page handle's `pageNum` field.

### `markDirty`
- Marks a page as dirty.

### `forcePage`
- Writes the current content of a page back to the page file on disk.

## Statistics Functions

These functions provide statistics about the buffer pool and its contents:

- `getFrameContents`: Returns an array of page numbers, where each element represents the page stored in a page frame. Empty frames are represented by `NO PAGE`.

- `getDirtyFlags`: Returns an array of booleans, indicating whether each page in a frame is dirty. Empty frames are considered clean.

- `getFixCounts`: Returns an array of integers representing the fix count of pages in the buffer pool. Empty frames have a fix count of 0.

- `getNumReadIO`: Returns the number of pages read from disk since the buffer pool was initialized.

- `getNumWriteIO`: Returns the number of pages written to the page file since the buffer pool was initialized.



## Compiling and Running the Project

To compile and run the program, use the following command:

```make run```

To clean the compiled program, use the following command:

```make clean```

## License

This project is licensed under the Illinois Institute of technology.
Copyright (c) [2023] [Akash Didigi Kashinath], [Harshitha Satish Reddy], [Nikhil Goud]