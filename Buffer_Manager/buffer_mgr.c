/****************************************
 * Buffer Manager - Implementation      *
 *       *	*	*	*	*
 * Author: vjonnala and team		*
 *	*	*	*	*	*
 * Date : 03/23/2015			*
 ****************************************/

/*Link Section - Header Files*/
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "node_linked_list.h"
#include "dt.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

/* ------------------- Buffer Manager Interface Pool Handling ----------------------------*/

/*
 initBufferPool creates a new buffer pool with numPages page frames using the page replacement strategy strategy.
 The pool is used to cache pages from the page file with name pageFileName.
 Initially, all page frames should be empty.
 The page file should already exist, i.e., this method should not generate a new page file.
 stratData can be used to pass parameters for the page replacement strategy.
 */

struct node * start;
FILE *fpAEB = NULL;
struct BM_statistics* bm_statistics = NULL;
int temp_arr[20];//20


RC initBufferPool(BM_BufferPool * const bm, const char * const pageFileName,
		const int numPages, ReplacementStrategy strategy, void *stratData) {

	bm->pageFile = (char*)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	bm->mgmtData = stratData;

	fpAEB = fopen(pageFileName, "rb+");
	bm_statistics = (struct BM_statistics*)malloc(sizeof(struct BM_statistics));
	bm_statistics->frameContents = (PageNumber*)malloc(sizeof(PageNumber));
	bm_statistics->dirty_bit_stats = (bool*)malloc(sizeof(bool));
	bm_statistics->fix_count_stats = (int*)malloc(sizeof(int));

	bm_statistics->numReadIO=0;
	bm_statistics->numWriteIO=0;

	intialize_linked_list(bm->numPages);//Call method to initialize linked list with -1 values.
	
	int i;
	for (i = 0; i < 20; i++) {
		temp_arr[i] = -2;
	}

	return RC_OK;

}

/*
 shutdownBufferPool destroys a buffer pool.
 This method should free up all resources associated with buffer pool.
 For example, it should free the memory allocated for page frames. If the buffer pool contains any dirty pages, then these pages should be written back to disk before destroying the pool. It is an error to shutdown a buffer pool that has pinned pages.
 */

RC shutdownBufferPool(BM_BufferPool * const bm) {

	 bool isDirtyBit;

	 struct node *temp = start;
	 do
	 {
	 if(temp->fix_count!=0){
	 return RC_BM_CANNOT_WRITE_A_UNPIN_PAGE;
	 }
	 else
	 {
	 if (temp->dirty_bit == TRUE)
	 {
	 forceFlushPool(bm);// Call Force Flush Pool to shut down all the pools that have a dirty bit.
	 isDirtyBit = TRUE;
	 }
	 }
	 temp=temp->next;
	 }while(temp!=start);

	 if(!isDirtyBit){
		 free(temp);
	 }

	return RC_OK;
}
/*
 forceFlushPool causes all dirty pages (with fix count 0) from the buffer pool to be written to disk.
 */

RC forceFlushPool(BM_BufferPool * const bm) {

	 SM_FileHandle fHandle;
	 fHandle.fileName = bm->pageFile;

	 struct node *temp = start;
	 do
	 {
	 if(temp->dirty_bit == TRUE && temp->fix_count == 0){
	 writeBlock1(temp->data,&fHandle, (SM_PageHandle)temp->bm_ph.data);// Write back the data from Page Handler to File Handler.
	 temp->dirty_bit = FALSE;
	 }
	 temp=temp->next;
	 }while(temp!=start);

	return RC_OK;

}

/* ------------------- Buffer Manager Interface Access Pages ----------------------------*/

/*markDirty marks a page as dirty.*/

RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) {

	struct node* temp = start;

	int i = 0;

	do {
		if (i < bm->numPages) {
			if (temp->bm_ph.pageNum == page->pageNum) {
				temp->dirty_bit = TRUE; // Mark the page as dirty when it is requested from the user.
				break;
			} else {
				i = i + 1;
				temp = temp->next;
			}
		} else
			break;
	} while (temp != NULL);

	return RC_OK;
}

/*
 unpinPage unpins the page page. The pageNum field of page should be used to figure out which page to unpin.
 */

RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page) {

	int pageNum = page->pageNum;
	 SM_FileHandle fHandle;
	 fHandle.fileName = bm->pageFile;

	struct node *frameToUnpinPage;
	frameToUnpinPage = start;

	if (bm == NULL) {
		return RC_BM_POOL_INIT_ERROR;
	} else {
		do {
			if (frameToUnpinPage->data == pageNum){
				frameToUnpinPage->fix_count = 0;
				if (frameToUnpinPage->dirty_bit) {
					frameToUnpinPage->bm_ph.data = page->data ;
					writeBlock1(page->pageNum,&fHandle,(SM_PageHandle)page->data);//Write Block writes the data from Page Handler to File Handler.

	 				bm_statistics->numWriteIO++; // Increement the number of IO writes.

					frameToUnpinPage->dirty_bit = TRUE;
				}
				if (frameToUnpinPage->fix_count < 0) {
					return RC_BM_CANNOT_UNPIN_A_PAGE;
					break;
				}
				break;
			}
			else{
			frameToUnpinPage = frameToUnpinPage->next;
			}
		} while (frameToUnpinPage != start);

		return RC_OK;
	}
}

/*
 forcePage should write the current content of the page back to the page file on disk.
 */

RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page) {

	 SM_FileHandle fHandle;
	 fHandle.fileName = bm->pageFile;

	 if(bm == NULL){
	 return RC_BM_POOL_INIT_ERROR;
	 }
	 else{
	 int statusofWriteBlock = writeBlock1(page->pageNum,&fHandle,(SM_PageHandle)page->data);// Force the page to write back the block of data.
	 bm_statistics->numWriteIO++;

	 return RC_OK;
	 }

}

/*
 pinPage pins the page with page number pageNum.
 The buffer manager is responsible to set the pageNum field of the page handle passed to the method.
 Similarly, the data field should point to the page frame the page is stored in (the area in memory storing the content of the page).
 */
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum) {

	SM_FileHandle fHandle;
	page->pageNum = pageNum;

	int hitOrFault = search_linked_list(pageNum,bm->numPages);
	
	// FIFO Page Replacement Strategy
	if(bm->strategy == RS_FIFO){
		if (hitOrFault) {
		} else {
			bm_statistics->numReadIO++;
			insertFrame_linked_list(bm, pageNum);
		}
	}
	// LRU Page Replacement Strategy
	else if(bm->strategy == RS_LRU){
		if (hitOrFault) {
		} else {
			bm_statistics->numReadIO++;
		}
		insertFrame_lru(bm->numPages, pageNum);
	}
	// CLOCK Page Replacement Strategy
	else if(bm->strategy == RS_CLOCK){
		if (hitOrFault) {
		} else {
			bm_statistics->numReadIO++;
		}
		insertFrame_clock(bm->numPages, pageNum);
	}
	// LFU Page Replacement Strategy
	else if(bm->strategy == RS_LFU){
		int numberOfPages = 20;
		if (hitOrFault) {
		} else {
			bm_statistics->numReadIO++;
		}
		int i, j;
		int n = bm->numPages;//3
		for (i = 0; i < numberOfPages; i++) {
			if (temp_arr[i] == -2) {
				temp_arr[i] = pageNum;
				break;
			}
		}
		insertFrame_lfu(pageNum, bm->numPages, temp_arr,numberOfPages);
	}
	else{
		return RC_BM_INVALID_STRATEGY;
	}	

	struct node* temp = return_frame_linked_list(pageNum,bm->numPages);

	fHandle.fileName = bm->pageFile;
	//Add Blocks to the file based on the page number.
	int statusofEnsureCapacity = ensureCapacity1(pageNum + 1, &fHandle);
	//Read the block from File Hanlder to Page Handler.
	int statusofReadBlock = readBlock1(pageNum, &fHandle,
			(SM_PageHandle) temp->bm_ph.data);

	page->pageNum = pageNum;
	page->data = temp->bm_ph.data;

	if (hitOrFault == 1) {
		temp->fix_count = 1; // If HIT increement the fix count.
	}

	return RC_OK;
}

// This method returns the frame contents used by the test case to verify the pool contents.
PageNumber *getFrameContents(BM_BufferPool * const bm) {
	struct node *new_node = start;
	PageNumber* frame_contents_array = bm_statistics->frameContents;

	int i = 0;

	do {
		if (i < bm->numPages) {
			frame_contents_array[i] = new_node->data;
			i = i + 1;
			new_node = new_node->next;
		} else
			break;
	} while (new_node != NULL);


	return frame_contents_array;
}

// This method returns the number of Dirty Flags.
bool *getDirtyFlags(BM_BufferPool * const bm) {
	struct node *new_node = start;
	bool *dirty_bit_stats_array = bm_statistics->dirty_bit_stats;

	int i = 0;

	do {
		if (i < bm->numPages) {
			dirty_bit_stats_array[i] = new_node->dirty_bit;
			i = i + 1;
			new_node = new_node->next;
		} else
			break;
	} while (new_node != NULL);

	return dirty_bit_stats_array;
}

// This method returns the total fix counts .
int *getFixCounts(BM_BufferPool * const bm) {
	struct node *new_node = start;
	int *fix_count_stats_array = bm_statistics->fix_count_stats;

	int i = 0;

	do {
		if (i < bm->numPages) {
			fix_count_stats_array[i] = new_node->fix_count;
			i = i + 1;
			new_node = new_node->next;
		} else
			break;
	} while (new_node != NULL);

	return fix_count_stats_array;
}

// Return the number of IO Read.
int getNumReadIO (BM_BufferPool *const bm)
{
	return bm_statistics->numReadIO;
}

// Return the number of IO Write.
int getNumWriteIO (BM_BufferPool *const bm)
{
	return bm_statistics->numWriteIO;
}

// Ensure Capacity increements the block size base on the number of pages.
RC ensureCapacity1 (int numberOfPages, SM_FileHandle *fHandle){

		int i, j, bytesWritten;

		 // Open the file in Append mode to append an empty block at the file end.
		if (fpAEB == NULL) {
			return RC_FILE_NOT_FOUND;
		}

		int seekStatus = fseek(fpAEB, 0, SEEK_END); // Move the cursor to the end postion to append an empty block.

		int ctr = 0;
		if (seekStatus == -1) {
			return RC_SEEK_FAILED;
		}

		int TOTAL_BLOCKS = getTotalNumberOfBlocks1(fpAEB) + 1; // Since the block counting starts from zero , increase the total by 1.
		int ADD_BLOCKS = numberOfPages - TOTAL_BLOCKS; // Get the number of blocks needed to be appended at the end.

		if (ADD_BLOCKS <= 0) {
			return RC_OK; // If the number is less than zero , it has sufficient capacity and hence no need to append blocks.
		} else {
			//Need to ensure capacity by adding ADD_BLOCKS Blocks.
			for (i = 1; i <= ADD_BLOCKS; i++) {

				for (ctr = 0 ; ctr < PAGE_SIZE ; ctr ++)
					fprintf (fpAEB, "%c", '\0');
					}

				/*char *buffer[PAGE_SIZE]; //  Create a Buffer anf instantiate a memory of 4096 Bytes to it.
				memset(buffer, '\0', PAGE_SIZE);

				bytesWritten = fwrite(buffer, 1, PAGE_SIZE, fpAEB);*/
			}

			int seekStatus1 = fseek(fpAEB, 0, SEEK_END); // Move the cursor to the end postion to append an empty block.
			if (seekStatus1 == -1) {
				return RC_SEEK_FAILED;
			}

			fHandle->mgmtInfo = fpAEB;
			/*
			if (bytesWritten < PAGE_SIZE) {
				return RC_WRITE_FAILED; // Return error if the number of bytes written are less than PAGE_SIZE.
			} else
				return RC_OK;
*/
			return RC_OK;
		}

// Read block reads the data from File Handler to Page Handler instance.
RC readBlock1 (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	FILE *fpRB = fpAEB; // Open the file in Read mode to read the pageNumth block.
		if (fpRB == NULL) {
			return RC_FILE_NOT_FOUND; // Return error if file does not exists.
		}

		int seekStatus = fseek(fpRB, pageNum * PAGE_SIZE, SEEK_SET); // Seek to the initial position to read the pageNumth block.
		if (seekStatus == -1) {
			return RC_SEEK_FAILED; // Return error if failed to move to the pageNumth position.
		}

		//memPage = malloc(PAGE_SIZE); // Allocate PAGE_SIZE memory to Page Handler.
		int bytesRead = fread(memPage, PAGE_SIZE, 1 , fpRB); // Read the blocks from File Handler to Page Handler.

		fHandle->mgmtInfo = fpRB;

		if (bytesRead != 1) {
					return RC_READ_FAILED;  // Read failed to read PAGE_SIZE bytes.
				} else
					return RC_OK;

}

// Write block reads the data from Page Handler to File Handler instance.
RC writeBlock1 (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	int bytesWritten;

	FILE *fpWB = fpAEB; // Open the file in Write mode to write the pageNumth block.
	if (fpWB == NULL) {
		return RC_FILE_NOT_FOUND;  // Return error if file does not exists.
	}

	int TOTAL_BLOCKS = getTotalNumberOfBlocks1(fpWB); // Find the total Number of Blocks in the given file.(Block Number starts with 0)

	int seekStatus = fseek(fpWB, PAGE_SIZE * pageNum, SEEK_SET); // Seek to the initial position to write the pageNumth block.
	if (seekStatus == -1) {
		return RC_SEEK_FAILED;
	}

	//printf("MEMPAGE:-\n", memPage);
	//memPage = malloc(PAGE_SIZE);
	bytesWritten = fwrite(memPage, PAGE_SIZE,1, fpWB); //Write the bytes from Page Handler to File Handler.
	fHandle->mgmtInfo = fpWB;

	if (bytesWritten !=1) {
		return RC_WRITE_FAILED; // Return error if the number of bytes written are less than PAGE_SIZE.
	} else
		return RC_OK;

}

int getTotalNumberOfBlocks1(FILE *fpNOB) {
	struct stat status;
	fstat(fileno(fpNOB), &status);
	/*Get the file size using built-in function fstat()*/
	int FILE_SIZE = status.st_size;
	int numberOfBlocks;
	if (FILE_SIZE <= PAGE_SIZE) {
		numberOfBlocks = 0; // If the file size is less than zero , it will be the 0th Block.
	} else {
		numberOfBlocks = (FILE_SIZE / PAGE_SIZE) - 1;
	}

	return numberOfBlocks;
}
/**********************************************
 *       *	*	*	*	*     *
 * END - Buffer Manager - Implementation      *
 *       *	*	*	*	*     *
 ***********************************************/
