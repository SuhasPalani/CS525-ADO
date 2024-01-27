# Storage Manager Documentation


## Introduction

This document provides comprehensive documentation for the Storage Manager module, a component responsible for managing page files and handling read/write operations.

## Contents

1. [Installation](#installation)
2. [Usage](#usage)
   - [Initializing Storage Manager](#initializing-storage-manager)
   - [Creating a Page File](#creating-a-page-file)
   - [Opening a Page File](#opening-a-page-file)
   - [Closing a Page File](#closing-a-page-file)
   - [Destroying a Page File](#destroying-a-page-file)
   - [Reading and Writing Blocks](#reading-and-writing-blocks)
   - [Managing Block Positions](#managing-block-positions)
   - [Appending Empty Blocks](#appending-empty-blocks)
   - [Ensuring Capacity](#ensuring-capacity)
3. [Compile and Run](#compile-and-run)
4. [License](#license)

## Installation

To incorporate the Storage Manager module into your project, follow these steps:

## Usage

1. Clone the repository.
2. Include the necessary header files in your project:

1. Initializing store Manager.
This function initializes the Storage Manager module. It doesn't take any arguments. Its purpose is to set up any necessary internal data structures or resources that the module may need.

2. Creating a Page File
This function creates a new page file with the given fileName. It first attempts to open the file in write mode. If successful, it allocates memory for a blank page, writes it to the file, and then closes the file. If the file creation is successful, it returns RC_OK. If the file could not be created, it returns RC_FILE_NOT_FOUND.

3. Opening a Page File
This function opens an existing page file with the given fileName. It attempts to open the file in read/write mode. If successful, it retrieves information about the file (such as total number of pages) and initializes the SM_FileHandle structure. If the file is successfully opened, it returns RC_OK. If the file does not exist, it returns RC_FILE_NOT_FOUND.

4. Closing a Page File
This function closes a page file and performs cleanup of the associated file handle. It first checks if the file handle is initialized. If so, it attempts to open the file in read mode to check if it's a valid file. If successful, it closes the file and resets the file handle properties. If the file handle is not initialized, it returns RC_FILE_HANDLE_NOT_INIT. If there was an issue with opening or closing the file, it returns RC_FILE_NOT_FOUND.

5. Destroying a Page File
This function destroys (deletes) a page file with the given fileName. It attempts to remove the file. If successful, it returns RC_OK. If the file could not be found or there was an error while deleting, it returns RC_FILE_NOT_FOUND.

6. Reading blocks
This function reads a block specified by pageNum from the page file associated with fHandle. It first performs error checks to ensure that the file handle is initialized and pageNum is within the valid range. It then calculates the offset and seeks to the appropriate position in the file. If the seek operation is successful, it reads the page into memPage and returns RC_OK. If there's an issue with the file or page number, it returns an appropriate error code.

7. Get Block Position
This function returns the current position of the page in the file. It first checks if the file handle is initialized. If so, it returns the curPagePos property. If the file handle is not initialized, it returns -1.

8. Read first block
This function reads the first block from the page file associated with fHandle. It simply calls readBlock with pageNum set to 0 (since the first block is at position 0) and returns the result.

9. Read previous block
This function reads the block immediately before the current block in the page file. It calculates the previous block number, and then calls readBlock with that number.

10. Read current block
This function reads the current block in the page file. It calls readBlock with pageNum set to the current page position.

11. read next block
This function reads the block immediately after the current block in the page file. It calculates the next block number, and then calls readBlock with that number.

12. read last block
This function reads the last block from the page file associated with fHandle. It calculates the page number of the last block and then calls readBlock with that number.

13. Write block
This function writes the content of currentMemPage to the block specified by pageNum in the page file associated with sMFHandle. It first checks for various error conditions like uninitialized file handle or out-of-range page number. It then opens the file in read/write mode, seeks to the appropriate position, writes the page, and closes the file.

14. Write current block
This function writes the content of memPage to the current block in the page file associated with sMFHandle. It calls writeBlock with the current page position.

15. Append empty block
This function appends an empty block to the page file associated with sMFHandle. It first checks for various error conditions like uninitialized file handle. It then opens the file in read/write mode, seeks to the end, writes an empty page, and updates the total number of pages.

16. Ensure capacity
This function ensures that the page file associated with sMFHandle has at least numberOfPages available. It first checks if the current total number of pages is sufficient. If not, it attempts to allocate and free memory to simulate adding empty pages (though no actual data is written). It always returns RC_OK.

## Compiling and Running the Project

To compile and run the program, use the following command:

```make run```

To clean the compiled program, use the following command:

```make clean```

## License

This project is licensed under the Illinois Institute of technology.
Copyright (c) [2023] [Akash Didigi Kashinath], [Harshitha Satish Reddy], [Nikhil Goud]