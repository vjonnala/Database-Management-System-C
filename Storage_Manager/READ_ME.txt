		**Assignment 1: Storage Manager**
****************************************************

The goal of this assignment is to implement a simple storage manager - a module that is capable of reading blocks from a file on
disk into memory and writing blocks from memory to a file on disk. The storage manager deals with pages (blocks) of fixed size
(PAGE_SIZE). In addition to reading and writing pages from a file, it provides methods for creating, opening, and closing files.
The storage manager has to maintain several types of information for an open file: The number of total pages in the file, the
current page position (for reading and writing), the file name, and a POSIX file descriptor or FILE pointer.


CONTENTS
*********
1)Instructions to run the code
2)Description of functions used
3)Additional Test Cases
4)Implementation

*****************************************************************

1)Instructions to run the code
*******************************

a)For executing mandatory test cases:

1) In the terminal,navigate to the assignment directory.

2) Type: 
	make -f makefile1

3) ./storagembr1

b) For executing additional test cases:

1) In the terminal,navigate to the assignment directory.

2) Type: 
	make -f makefile2

3) ./storagembr2




*****************************************************************


2)Description of functions used
********************************

	## CreatePageFile  ##
	---------------------
		
1)Check for the file existence.If the file exists,throw an error message that the file is already present.

2)If the file does not exist,create the file and fill the first page with 4096 null('\0') bytes.

	## OpenPageFile  ##
	-------------------
	
1) Check if the file with the provided file name exists and also the acess for that particular file.

2) If it does not exist, throw an error.

3) If opening the file is successful,then the fields of this file handle are initialized with the information about the opened file.

4) The cursor position in the file is stored for book keeping i.e. in mgmtInfo 

	##  ClosePageFile  ##
	---------------------
1) Close the file and return a success message upon success.

2) Upon failure, return an appropriate error message.


	## DestroyPageFile  ##
	----------------------
	
1) If the file is present, remove the file.

2) Upon success, return a success message.

3) Upon failure, return a failure message.


	##  readBlock  ##
	------------------
1) Reads the block with the given page number.
2) Check for file existence and seek to the block starting position . If both are sucess, move to readBlocks() method , or throw an error.
3) If the block number is more than the total blocks , throw an error .Block Number starts with zero.
4) Read the block contents from the File Handle to the page handle by allocating PAGE_SIZE bytes to the block .

	##  getBlockPos  ##
	-------------------
1) Get the block position using the cursor Position.

	## readFirstBlock  ##
	---------------------
1) Reads the block with the first page.
2) Check for file existence and seek to the block starting position . If both are sucess, move to readBlocks() method , or throw an error.
3) If the block number is more than the total blocks , throw an error . First Block Number will be zero.
4) Read the block contents from the File Handle to the page handle by allocating PAGE_SIZE bytes to the block .

	##  readPreviousBlock  ##
	--------------------------
1) Reads the previous block based on the block position where the cursor resides.
2) Check for file existence and seek to the block starting position . If both are sucess, move to readBlocks() method , or throw an error.
3) If the block number is more than the total blocks , throw an error .Block Number is the previous block number.
4) Read the block contents from the File Handle to the page handle by allocating PAGE_SIZE bytes to the block .

	##  readCurrentBlock  ##
	------------------------
1) Reads the current block where the cursor is residing.
2) Check for file existence and seek to the block starting position . If both are sucess, move to readBlocks() method , or throw an error.
3) If the block number is more than the total blocks , throw an error .Block Number will be the Current Block.
4) Read the block contents from the File Handle to the page handle by allocating PAGE_SIZE bytes to the block .

	##  readNextBlock  ##
	----------------------
1) Reads the next block based on the block position where the cursor resides.
2) Check for file existence and seek to the block starting position . If both are sucess, move to readBlocks() method , or throw an error.
3) If the block number is more than the total blocks , throw an error .Block Number is the next block.
4) Read the block contents from the File Handle to the page handle by allocating PAGE_SIZE bytes to the block .

	##  readLastBlock  ##
	---------------------
1) Reads the last block in the file.
2) Check for file existence and seek to the block starting position . If both are sucess, move to readBlocks() method , or throw an error.
3) If the block number is more than the total blocks , throw an error .Block Number will be the last block in the file.
4) Read the block contents from the File Handle to the page handle by allocating PAGE_SIZE bytes to the block .

	##  writeBlock  ##
	------------------
1) Writes the contents of block with the given page number.
2) Check for file existence and seek to the block starting position . If both are sucess, go to the block position , or throw an error.
3) If the block number is more than the total blocks , throw an error .Block Number will be specified.
4) Write the block contents from the Page Handle to the File handle  .


	##  writeCurrentBlock  ##
	-------------------------
1) Writes the current contents of block where the pointer resides in.
2) Check for file existence and seek to the block starting position . If both are sucess, go to the block position , or throw an error.
3) If the block number is more than the total blocks , throw an error .Block Number will be based on the cursor position.
4) Write the block contents from the Page Handle to the File handle  .

	##  appendEmptyBlock  ##
	------------------------
1) Move to the last cursor point in the file (SEEK_END).
2) Print'\0' for PAGE_SIZE bytes into the new block.

	##  ensureCapacity  ##
	----------------------
1) Get the required capacity of the block file is needed. Seek to the ending of the file.
2) If the required blocks are more than currently existing total blocks , append the remaining blocks to the file.
3) If the required blocks are less , then return OK .

3) Additional Test Cases
------------------------

We have included additional test cases for executing the following functions.


-GetBlockPosition

-ReadCurrentBlock

-ReadPreviousBlock

-ReadNextBlock

-WriteCurrentBlock

-ReadLastBlock

-EnsureCapacity


4) Implementation
-----------------

All the test cases and additional test cases required for the project are analysed and task got divided into various categories .
The present cursor postion is taken as the book-keeping information (mgmntInfo). Based on the cursor position , the block numbers for the current , next and previous blocks are calculated.
The implementation covers the following concepts:-
i)   Create , Open , Close , Destroy of a file.
ii)  Read blocks based on the page number - previous , current , next and last .
iii) Write blocks based on the page number.
iv)  Ensure (Increase) capacity of the file by appending the required blocks .

In the use case , all the blocks start with zero . i.e., 0th block = 0 to 4095 bytes(PAGE_SIZE bytes) , 1st block = 4096 to 8191(PAGE_SIZE bytes). If the cursor is in between 0 to 4095 bytes , the previous and current page are the 0th Page . And the total number of blocks returned will be based on the file size . So , if file size is 4096 - the number of blocks are 0 (since 0 is the first block).



------------------------------------------------------END------------------------------------------------------------------------------------
