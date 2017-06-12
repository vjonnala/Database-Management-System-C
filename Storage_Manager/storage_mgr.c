/****************************************
* Storage Manager - Implementation      *
*       *	*	*	*	*
* Author: vjonnala and team		*
*	*	*	*	*	*
* Date : 02/21/2012			*
****************************************/


/*Link Section - Header Files*/
#include "storage_mgr.h"

#include <stdio.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <fcntl.h> 

/*Definition - MACRO - FILE_NAME*/
#define FILE_NAME "test_pagefile.bin"


/* manipulating page files */
void initStorageManager (void){ 

}


/**METHOD:-createPageFile()
  Use-Case:-Create a File with one Page of 4096 null charecters(bytes) in it .
  Arguments:-fileName = The name of the file to be created.
  Return-Value:- To catch the possible exceptions.
**/
 RC createPageFile (char *fileName){ 

	char *buffer[PAGE_SIZE];int i = 0;int bytesWritten;
	
	FILE *fp;
	fp = fopen(fileName,"wb+");    //File Pointer used to create a file and write contents to it.

	if(fp == NULL){
		return RC_FILE_NOT_FOUND;	// Could not find the file.
	    }
	
	/*Instantiate a Buffer and write null charecters to it .*/
	memset(buffer,'\0',PAGE_SIZE);
	bytesWritten = fwrite(buffer ,1 , PAGE_SIZE,fp);  
	
	fclose(fp); //Close the file after use.
	fp = NULL;  // Instantiate to NULL to prevent memory leaks , if caused.

	if(bytesWritten < PAGE_SIZE){
		return RC_WRITE_FAILED; // Returns the error if number of bytes are written less than 4096
	      }
	else return RC_OK;
}

/**METHOD:-openPageFile()
  Use-Case:-Open the existing page and initialise all the variables in structure SM_FileHandle with details of the page.
  Arguments:-fileName = The name of the file created , fHandle = File Handler which handles information of the file.
  Return-Value:- To catch the possible exceptions.
**/
 RC openPageFile (char *fileName, SM_FileHandle *fHandle){ 

	/*Open an existing page file.*/
	FILE *fpr;
	fpr = fopen(fileName , "rb+");   //File Pointer used to open an existing file .  
	if(fpr == NULL){
		return RC_FILE_NOT_FOUND; // Could not find the file.
	}

	//Initialze the fields in SM_FileHandle
	fHandle->fileName = fileName;
	fHandle->curPagePos = 0;

	int status = fseek(fpr,0,SEEK_END); 
	   if(status == -1){
		return RC_SEEK_FAILED;  // Return the seek status if failed to seek end.
	    }

	fHandle->totalNumPages = (ftell(fpr) / PAGE_SIZE );
	
	fHandle->mgmtInfo = (void *)(ftell(fpr) - 1); // Manage the cursor to its last position in mgmtInfo.
				      // Eg.,If the ftell returns the page position(in multiples of PAGE_SIZE), move to its previous position 

	return RC_OK;
}

/**METHOD:-closePageFile()
  Use-Case:-Close the open Page file.
  Arguments:-fileName = The name of the file to be closed.
  Return-Value:- To catch the possible exceptions.
**/
 RC closePageFile (SM_FileHandle *fHandle){

	FILE *fpd = fopen(fHandle->fileName,"r");
	int status = fseek(fpd,0,SEEK_SET);
	   if(status == -1){
		return RC_SEEK_FAILED;  // Return the seek status if failed to seek end.
	    }

	fclose(fpd); // Close the file after use.
	fpd = NULL; // Reset File Pointer to ensure file is closed properly to prevent memory leaks.

	return RC_OK;

}

/**METHOD:-destroyPageFile()
  Use-Case:-Delete the opened Page File.
  Arguments:-fileName = The name of the file to be deleted.
  Return-Value:- To catch the possible exceptions.
**/
 RC destroyPageFile (char *fileName){ 

	int ret = remove(fileName); // Remove the given file Name
	if(ret == 0){
	return RC_OK;
   	}
   	else {
	return RC_DELETE_FAILED; // Return Error if the file is not deleted properly.
   	}

}

/*METHOD:-readFirstBlock()
  Use-Case:-Read first block from 0 to 4095 Bytes to Page Handle 
  Arguments:-SM_FileHandle = Contains all the details relating to the File , memPage = Page Handler which reads the data 
  Return-Value:- To catch the possible exceptions. 
1. Read first block from 0 to 4096 Byte to Page Handle */
 RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){ 
	
	FILE *fpRFB = fopen(fHandle->fileName,"rb+"); // Open the file in Read mode to read the first block.
	  if(fpRFB == NULL){
		return RC_FILE_NOT_FOUND; // Return error if file does not exists.
	   }

	int seekStatus = fseek(fpRFB, 0, SEEK_SET); // Seek to the initial position to read the first block.
	   if(seekStatus == -1){
		return RC_SEEK_FAILED; // Return error if failed to move to the first position.
	    }

	fHandle->mgmtInfo = (void *)ftell(fpRFB); // Set the present cursor position to allow for further usage.

	/*As the test case is throwing a memory leak exception , Instantiating the memory allocated to Page Handler to NULL.*/
	free(memPage);

	int status = readBlocks(fpRFB,fHandle,memPage); // Read Commom Method : readBlocks() for reading the contents.

	return status;
 }
/* 2. Read block of 4096 Bytes pecified by Page number */
/**METHOD:-readBlock()
  Use-Case:-Read a block based on the page number of 4096 Bytes to Page Handle 
  Arguments:-pageNum = The page number of the file to be read , SM_FileHandle = Contains all the details relating to the File , 
             memPage = Page Handler which reads the data.
  Return-Value:- To catch the possible exceptions.
**/
 RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){ 
	
	FILE *fpRB = fopen(fHandle->fileName,"rb+"); // Open the file in Read mode to read the pageNumth block.
	   if(fpRB == NULL){
		return RC_FILE_NOT_FOUND; // Return error if file does not exists.
	    }

	int seekStatus = fseek(fpRB, pageNum*PAGE_SIZE, SEEK_SET); // Seek to the initial position to read the pageNumth block.
	   if(seekStatus == -1){
		return RC_SEEK_FAILED; // Return error if failed to move to the pageNumth position.
	    }

	fHandle->mgmtInfo = (void *)ftell(fpRB); // Set the present cursor position to allow for further usage.
	
	int status = readBlocks(fpRB,fHandle,memPage); // Read Commom Method : readBlocks() for reading the contents.
	
	return status;
 }

/*3. Read Previous Block based on the current cursor position*/
/**METHOD:-readPreviousBlock()
  Use-Case:-Read Previous Block based on the current cursor position.
  Arguments:-SM_FileHandle = Contains all the details relating to the File , memPage = Page Handler which reads the data 
  Return-Value:- To catch the possible exceptions.
**/
 RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){ 
	
	/*Find the previous block locations based on the Management Info position*/
	int currentCursorPosition = fHandle->mgmtInfo;
	int currentBlock = (currentCursorPosition / PAGE_SIZE );  
	int previousBlock; 
	if(currentBlock == 0)
	previousBlock = 0;
	else					//Eg.,CursorPosition(Block Number) = 300(0),4096(0),5000(0)8012(1)
	previousBlock = currentBlock - 1 ;

	FILE *fpRPB = fopen(fHandle->fileName,"rb+"); // Open the file in Read mode to read the previous block.
	   if(fpRPB == NULL){
		return RC_FILE_NOT_FOUND; // Return error if file does not exists.
	    }

	int seekStatus = fseek(fpRPB, PAGE_SIZE*previousBlock , SEEK_SET); // Seek to the initial position to read the previous block.
	    if(seekStatus == -1){
		return RC_SEEK_FAILED;	// Return error if failed to move to the previous position.
	    }

	fHandle->mgmtInfo = (void *)ftell(fpRPB); // Set the present cursor position to allow for further usage.

	int status = readBlocks(fpRPB,fHandle,memPage); // Read Commom Method : readBlocks() for reading the contents.
	
	return status;
 
 }

/*4. Read Current Block based on the current cursor position*/
/**METHOD:-readCurrentBlock()
  Use-Case:-Read Current Block based on the current cursor position
  Arguments:-SM_FileHandle = Contains all the details relating to the File , memPage = Page Handler which reads the data 
  Return-Value:- To catch the possible exceptions.
**/
 RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){ 

	/*Find the Current block locations based on the Management Info position*/
	int currentBlock ;
	int currentCursorPosition = fHandle->mgmtInfo;    	 //Eg.,CursorPosition(Block Number) = 300(0),4096(1),5000(1)8012(2)
	currentBlock = (currentCursorPosition / PAGE_SIZE ) ; 
	
	FILE *fpRCB = fopen(fHandle->fileName,"rb+"); // Open the file in Read mode to read the current block.
	    if(fpRCB == NULL){
		return RC_FILE_NOT_FOUND;  // Return error if file does not exists.
	    }

	int seekStatus = fseek(fpRCB, PAGE_SIZE*currentBlock , SEEK_SET); // Seek to the initial position to read the current block.
	    if(seekStatus == -1){
		return RC_SEEK_FAILED;  // Return error if failed to move to the current position.
	    }

	fHandle->mgmtInfo = (void *)ftell(fpRCB);  // Set the present cursor position to allow for further usage.

	int status = readBlocks(fpRCB,fHandle,memPage);  // Read Commom Method : readBlocks() for reading the contents.

	return status;

 }

/*5. Read Next Block based on the current cursor position*/
/**METHOD:-readNextBlock()
  Use-Case:-Read Next Block based on the current cursor position.
  Arguments:-SM_FileHandle = Contains all the details relating to the File , memPage = Page Handler which reads the data 
  Return-Value:- To catch the possible exceptions.
**/
 RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){ 

	/*Find the Next block locations based on the Management Info position*/
	int nextBlock ;
	int currentCursorPosition = fHandle->mgmtInfo;    	 //Eg.,CursorPosition(Block Number) = 300(1),4096(2),5000(2)8012(3)
	nextBlock = (currentCursorPosition / PAGE_SIZE ) + 1 ;

	FILE *fpRNB = fopen(fHandle->fileName,"rb+");  // Open the file in Read mode to read the next block.
	    if(fpRNB == NULL){
		return RC_FILE_NOT_FOUND;  // Return error if file does not exists.
	    }

	int seekStatus = fseek(fpRNB, PAGE_SIZE*nextBlock , SEEK_SET);  // Seek to the initial position to read the next block.
	    if(seekStatus == -1){
		return RC_SEEK_FAILED;  // Return error if failed to move to the next position.
	    }

	fHandle->mgmtInfo = (void *)ftell(fpRNB);   // Set the present cursor position to allow for further usage.

	int status = readBlocks(fpRNB,fHandle,memPage);  // Read Commom Method : readBlocks() for reading the contents.

	return status;
	
 }
/*6. Read Last Block based on the current cursor position*/
/**METHOD:-readLastBlock()
  Use-Case:-Read Last Block based on the current cursor position.
  Arguments:-SM_FileHandle = Contains all the details relating to the File , memPage = Page Handler which reads the data 
  Return-Value:- To catch the possible exceptions.
**/
 RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){ 
	
	FILE *fpRFB = fopen(fHandle->fileName,"rb+");  // Open the file in Read mode to read the last block.
	   if(fpRFB == NULL){
		return RC_FILE_NOT_FOUND;  // Return error if file does not exists.
	    }

	int seekStatus = fseek(fpRFB, -(PAGE_SIZE-1) , SEEK_END);  // Seek to the initial position to read the last block.
           if(seekStatus == -1){
		return RC_SEEK_FAILED;  // Return error if failed to move to the last position.
	    }

	fHandle->mgmtInfo = (void *)ftell(fpRFB);  // Set the present cursor position to allow for further usage.

	int status = readBlocks(fpRFB,fHandle,memPage);  // Read Commom Method : readBlocks() for reading the contents.

	return status;

 }
 

/*7.Write the block back based on pagenumber*/
/**METHOD:-writeBlock()
  Use-Case:-Write the block back based on pagenumber in the page handler to the file Handler
  Arguments:-pageNum = The page number of the file to be written , SM_FileHandle = Contains all the details relating to the File , 
             memPage = Page Handler which writes the data.
  Return-Value:- To catch the possible exceptions.
**/
 RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){ 

	int bytesWritten;

	FILE *fpWB = fopen(fHandle->fileName,"wb+");  // Open the file in Write mode to write the pageNumth block.
	    if(fpWB == NULL){
		return RC_FILE_NOT_FOUND;// Return error if file does not exists.
	      }

	int TOTAL_BLOCKS =  getTotalNumberOfBlocks(fpWB); // Find the total Number of Blocks in the given file.(Block Number starts with 0)

	/* Check the Page Number is more than total blocks . If , true , seek the cursor to initial position.*/
	if(pageNum>TOTAL_BLOCKS)
	{ 
		fseek(fpWB,0,SEEK_SET);

		fHandle->mgmtInfo = (void *)ftell(fpWB); // Set the present cursor position to allow for further usage.

		return RC_READ_NON_EXISTING_PAGE;
	}

	int seekStatus = fseek(fpWB, PAGE_SIZE*pageNum , SEEK_SET);  // Seek to the initial position to write the pageNumth block.
	    if(seekStatus == -1){
		return RC_SEEK_FAILED;
	      }
 
	bytesWritten = fwrite(memPage,1,PAGE_SIZE,fpWB);  //Write the bytes from Page Handler to File Handler.
	fHandle->mgmtInfo = (void *)(ftell(fpWB) - 1);

	if(bytesWritten < PAGE_SIZE) 
	      {
		return RC_WRITE_FAILED; // Return error if the number of bytes written are less than PAGE_SIZE.
	      }
	else  return RC_OK;

}

/*8.Write Current Block to a page file */
/**METHOD:-writeCurrentBlock()
  Use-Case:-Write Current Block from a page handler to the file Handler .
  Arguments:-SM_FileHandle = Contains all the details relating to the File,memPage = Page Handler which writes the data.
  Return-Value:- To catch the possible exceptions.
**/
 RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){ 
	
	int bytesWritten;

	int currentBlock = getBlockPos(fHandle);  // Get current Block position from the getBlockPos() method.

	FILE *fpWCB = fopen(fHandle->fileName,"wb+");	// Open the file in Write mode to write the pageNumth block.
	   if(fpWCB == NULL){
		return RC_FILE_NOT_FOUND;
	     }

	int seekStatus = fseek(fpWCB, PAGE_SIZE*currentBlock , SEEK_SET);  // Seek to the initial position to write the pageNumth block.
	   if(seekStatus == -1){
		return RC_SEEK_FAILED;
	     }

	bytesWritten = fwrite(memPage,1,PAGE_SIZE,fpWCB);
	fHandle->mgmtInfo = (void *)(ftell(fpWCB) - 1);  //Write the bytes from Page Handler to File Handler.

	if(bytesWritten < PAGE_SIZE) 
	      {
		return RC_WRITE_FAILED;  // Return error if the number of bytes written are less than PAGE_SIZE.
	      }
	else  return RC_OK;
	
 }
/*9.Append empty block to the end of the file obtained from the last cursor position.*/
/**METHOD:-appendEmptyBlock()
  Use-Case:-Append a block at the end of the file . Move the cursor to the last position.
  Arguments:-SM_FileHandle = Contains all the details relating to the File
  Return-Value:- To catch the possible exceptions.
**/
 RC appendEmptyBlock (SM_FileHandle *fHandle){ 

	char *buffer[PAGE_SIZE];	
	memset(buffer,'\0',PAGE_SIZE);

	FILE *fpAEB = fopen(fHandle->fileName,"a+");  // Open the file in Append mode to append an empty block at the file end.
	    if(fpAEB == NULL){
		return RC_FILE_NOT_FOUND;
	     }

	int seekStatus = fseek(fpAEB,0,SEEK_END);  // Move the cursor to the end postion to append an empty block.
            if(seekStatus == -1){
		return RC_SEEK_FAILED;
	     }

	int bytesWritten =fwrite(buffer ,1, PAGE_SIZE ,fpAEB);
	if(bytesWritten < PAGE_SIZE) 
	      {
		return RC_WRITE_FAILED;  // Return error if the number of bytes written are less than PAGE_SIZE.
	      }
	else{
	fHandle->mgmtInfo = (void *)(ftell(fpAEB) - 1);

	return RC_OK;
	    }
 }

/*10. If the file has less than numberOfPages pages then increase the size to numberOfPages .*/
/**METHOD:-ensureCapacity()
  Use-Case:-If the file has less than numberOfPages pages then increase the size to numberOfPages .
  Arguments:-numberOfPages = Capacity of the page . If it is greater than the number of pages present , append the remaining blocks.
  Return-Value:- To catch the possible exceptions.
**/
 RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){ 

	int i , j,bytesWritten;	

	FILE *fpAEB = fopen(fHandle->fileName,"a+");  // Open the file in Append mode to append an empty block at the file end.
	    if(fpAEB == NULL){
		return RC_FILE_NOT_FOUND;
	    }

	int seekStatus = fseek(fpAEB,0,SEEK_END);  // Move the cursor to the end postion to append an empty block.
            if(seekStatus == -1){
		return RC_SEEK_FAILED;
	    }

	int TOTAL_BLOCKS =  getTotalNumberOfBlocks(fpAEB) + 1;  // Since the block counting starts from zero , increase the total by 1.
	int ADD_BLOCKS = numberOfPages - TOTAL_BLOCKS;  // Get the number of blocks needed to be appended at the end.

	if(ADD_BLOCKS<=0)
	{
		return RC_OK;  // If the number is less than zero , it has sufficient capacity and hence no need to append blocks.
	}
	else
	{
		//Need to ensure capacity by adding ADD_BLOCKS Blocks.
		for(i=1;i<=ADD_BLOCKS;i++)
		   {
			int seekStatus = fseek(fpAEB, 1 ,SEEK_END); //  Seek the cursor a byte more than the SEEK_END of the file.
			    if(seekStatus == -1){
				return RC_SEEK_FAILED;
			    }

			char *buffer[PAGE_SIZE];  //  Create a Buffer anf instantiate a memory of 4096 Bytes to it.
			memset(buffer,'\0',PAGE_SIZE);
	
			bytesWritten = fwrite(buffer , 1,PAGE_SIZE ,fpAEB);
		   }

		int seekStatus1 = fseek(fpAEB,0,SEEK_END);  // Move the cursor to the end postion to append an empty block.
		   if(seekStatus1 == -1){
			return RC_SEEK_FAILED;
	    	    }

		fHandle->mgmtInfo = (void *)(ftell(fpAEB) - 1);

		if(bytesWritten < PAGE_SIZE) 
	      	{
			return RC_WRITE_FAILED;   // Return error if the number of bytes written are less than PAGE_SIZE.
	      	}
		else return RC_OK;

	}

 }
 
/**METHOD:-readBlocks()
  Use-Case:-A common method for all the read operations.Moves the cursor to the specific position and read the block from 
	    the File Handler to Page Handler
  Arguments:-FILE = A File Pointer to catch the instance of the file used in the operation.
	     SM_FileHandle = Contains all the details relating to the File,memPage = Page Handler which writes the data.
  Return-Value:- To catch the possible exceptions.
**/
 RC readBlocks(FILE *fpRB, SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	int bytesRead;

	int currentBlock = getBlockPos(fHandle);	// Get the current Block from the getBlockPos() method.

	int TOTAL_BLOCKS =  getTotalNumberOfBlocks(fpRB);  // Get the total Number of Blocks in the file.

	/*If the current Block does not exists , return a non-existing page error.*/
	if(currentBlock>TOTAL_BLOCKS || currentBlock <0)
	{ 
		fseek(fpRB,0,SEEK_SET);
		fHandle->mgmtInfo = (void *)ftell(fpRB);

		return RC_READ_NON_EXISTING_PAGE;
	}
	else{
		
		int seekStatus = fseek(fpRB, PAGE_SIZE*currentBlock , SEEK_SET); // Move the cursor position to respective block position.
		   if(seekStatus == -1){
			return RC_SEEK_FAILED;
	    	    }

		memPage = malloc(PAGE_SIZE);  // Allocate PAGE_SIZE memory to Page Handler.
		bytesRead = fread(memPage,1, PAGE_SIZE, fpRB);  // Read the blocks from File Handler to Page Handler.

		fHandle->mgmtInfo = (void *)(ftell(fpRB) - 1);

		if(bytesRead < PAGE_SIZE) 
	      	    {
			return RC_READ_FAILED;  // Read failed to read PAGE_SIZE bytes.
	            }
		else  return RC_OK;
	}
 }

/**METHOD:-getTotalNumberOfBlocks()
  Use-Case:-Used to get Total number of Blocks(/Pages) in a file .
  Arguments:-FILE = The file pointer for which the number of blocks need to be calculated
  Return-Value:- The Number of Blocks . Count starts with 0(zero).
**/
 int getTotalNumberOfBlocks(FILE *fpNOB){
	struct stat status;
    	fstat(fileno(fpNOB),&status);
	/*Get the file size using built-in function fstat()*/
	int FILE_SIZE = status.st_size;
	int numberOfBlocks;
	if(FILE_SIZE <= PAGE_SIZE){
	numberOfBlocks = 0;  // If the file size is less than zero , it will be the 0th Block.
	}
	else{
	numberOfBlocks = (FILE_SIZE / PAGE_SIZE) ;
	}

	return numberOfBlocks;
 }


/**METHOD:-getBlockPos()
  Use-Case:-Used to get the Block position of the file where the cursor is pointed in based on the cursor position.
  Arguments:-fileName = The name of the file to be created.
  Return-Value:- The Block Number starts with 0(zero).
**/
int getBlockPos (SM_FileHandle *fHandle){ 

	int currentCursorPosition = fHandle->mgmtInfo;
	/*Find the Block Position from the Current cursor position .*/
	int blockPosition = (currentCursorPosition / PAGE_SIZE );	

	return blockPosition;	
 }

/**********************************************
*       *	*	*	*	*     *
* END - Storage Manager - Implementation      *
*       *	*	*	*	*     *
***********************************************/
