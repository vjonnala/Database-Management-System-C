/*
 * node_linked_list.c
 *
 *  Created on: Mar 23, 2015
 *      Author: VJonnala and Team
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include "node_linked_list.h"
#include "buffer_mgr.h"
#include "dberror.h"

struct node * start;

int position = 0;

/*Initialize each node data to -1 and set the first node as start an last node as current,Then link the last to start(Circular link list)*/
RC intialize_linked_list(int n) {
	struct node *new_node;
	int i;
	//start = NULL;
	for (i = 0; i < n; i++) {
		new_node = (struct node *) malloc(sizeof(struct node));
		if (new_node == NULL) {
			return RC_BM_NODE_CREATION_FAILED;

		}
		new_node->data = -1;     // -99 = NULL or Empty Characcter
		new_node->next = NULL;
		new_node->bm_ph.pageNum = -1;
		new_node->bm_ph.data = NULL;
		new_node->dirty_bit = FALSE;
		new_node->fix_count = 0;
		if (start == NULL || start->data != -1) {
			start = new_node;
			current = new_node;
		} else {
			current->next = new_node;
			current = new_node;
		}
	}
	current->next = start;

	return RC_OK;
}

/*Displays the nodes in the frame*/

void display_linked_list(int n) {

	struct node *new_node = start;

	int i = 0;

	do {
		if (i < n) {
printf(
					"VALUE = [%d] and FRAME [%d] and Fix Count [%d] and Ref bit [%d]\n",
					new_node->data, i, new_node->fix_count, new_node->ref_bit);
			i = i + 1;
			new_node = new_node->next;
		} else
			break;
	} while (new_node != NULL);

}

/*Search based on the page number and return the frame*/

struct node* return_frame_linked_list(int pageNumber, int n) {

	struct node* temp = start;

	int i = 0;

	do {
		if (i < n) {
			if (temp->data == pageNumber) {
				temp->bm_ph.data = malloc(4096);
				return temp;
				break;
			} else {
				i = i + 1;
				temp = temp->next;
			}
		} else
			break;
	} while (temp != NULL);

	return temp;
}

/*Search for the page number in linked list,Returns 0 if not found*/
int search_linked_list(int pageNumber, int n) {

	struct node *new_node = start;
	struct node *temp = NULL;

	bool found = false;
	int i = 0;
	do {
		if (i < n) {
			if (new_node->data == pageNumber) {
				found = true;
				break;
			} else {
				temp = new_node;
				new_node = new_node->next;
			}
			i = i + 1;
		} else {
			break;
		}
	} while (new_node != NULL);

	if (true == found) {
		return 1;
	} else {
		return 0;
	}

}

/*Replaces the page which is in memory for the longest(FIFO)*/
void insertFrame_linked_list(BM_BufferPool * const bm, int pageNumber) {

	struct node *temp = (struct node*) bm->mgmtData;
	if (temp == NULL) {
		temp = start;
		temp->bm_ph.pageNum = pageNumber;
		temp->data = pageNumber;
		temp->bm_ph.data = NULL;
		temp = temp->next;
		bm->mgmtData = temp;
	} else {
		while (temp->fix_count == 1) {
			temp = temp->next;
			bm->mgmtData = temp;
		}

		temp->bm_ph.data = NULL;
		temp->bm_ph.pageNum = pageNumber;
		temp->data = pageNumber;

		temp = temp->next;
		bm->mgmtData = temp;
	}

}

/*Replaces the page which was refferenced longest time ago*/
void insertFrame_lru(int totalPages, int pageNumber) {
	int numberofEmptyFrames = emptyFrames(totalPages);
	int hit = hitFault(pageNumber);
	struct node *new_node;
	new_node = start;
	if (!hit) {
		do {
			if (new_node->data == -1) {
				new_node->data = pageNumber;
				new_node->flag = 0;
				new_node->ref_bit = position;
				position = position + 1;
				break;
			} else if (numberofEmptyFrames > 0) {
				new_node = new_node->next;
			} else if (numberofEmptyFrames == 0) {
				int pagePositionToReplace = findNextVictim(pageNumber);
				if (new_node->ref_bit == pagePositionToReplace) {
					new_node->data = pageNumber;
					new_node->ref_bit = position;
					position = position + 1;
					break;
				}
				new_node = new_node->next;
			}
		} while (new_node != start);
	} else {
	}
}

/**/

int findNextVictim() {
	struct node *new_node = start;
	int min = new_node->ref_bit;
	int victim;
	do {
		if (new_node->ref_bit < min) {
			min = new_node->ref_bit;
		}
		new_node = new_node->next;
	} while (new_node != start);
	return min;
}
/*Function returns 0 for fault and 1 for hit*/
int hitFault(int pageNumber) {
	struct node *new_node = start;
	int hit = 0;
	do {
		if (new_node->data == pageNumber) {
			new_node->flag = new_node->flag + 1;
			new_node->ref_bit = position;
			position = position + 1;
			hit = 1;
			break;
		} else {
			new_node = new_node->next;
		}
	} while (new_node != start);
	return hit;
}
/*Returns total number of NULL nodes*/
int emptyFrames(int n) {
	int count = 0, i = 0;
	struct node *new_node = start;
	do {
		if (new_node->data == -1) {
			count = count + 1;
		}
		new_node = new_node->next;
	} while (new_node != start);
	return count;
}

/*Replaces the page whose ref_bit is 0(CLOCK)*/
void insertFrame_clock(int totalPages, int pageNumber) {
	int numberofEmptyFrames = emptyFrames(totalPages);
	int hit = hitFault_clock(pageNumber);
	struct node *new_node;
	new_node = start;
	if (!hit) {
		do {
			if (new_node->data == -1) {
				new_node->data = pageNumber;
				new_node->ref_bit = 1;
				new_node->flag = true;
				break;
			} else if (numberofEmptyFrames > 0) {
				new_node->flag = false;
				new_node = new_node->next;
			} else if (numberofEmptyFrames == 0) {
				pageReplacement(pageNumber, totalPages);
			}
		} while (new_node != start);
	} else {
	}
}

/*To check if hit or fault*/
int hitFault_clock(int pageNumber) {
	struct node *new_node = start;
	int hit = 0;
	do {
		if (new_node->data == pageNumber) {
			hit = 1;
			break;
		} else {
			new_node = new_node->next;
		}
	} while (new_node != start);
	return hit;
}

/*Replaces the page whose ref_bit is 0 and for pages with ref_bit is 1 it negates the value(CLOCK)*/
void pageReplacement(int pagenumber, int n) {
	struct node*ptr = start;
	int i = 0;
	int framePointer = getPointerLocation(n);
	int frameWithRefBitIsZero = checkIfRefBitIsZero(n);
	if (frameWithRefBitIsZero == -1) { //No frame contains a reference bit zero. Replace all 1->0.
		while (ptr != NULL) {
			if (i < n) {
				if (i == framePointer) {
					int j = 0;
					while (j < n) {
						ptr->flag = false;
						ptr = ptr->next;
						ptr->ref_bit = 0;
						j = j + 1;
					}
					ptr = ptr->next;
					ptr->ref_bit = 1;
					ptr->flag = true;
					ptr->data = pagenumber;
				} else {
					ptr = ptr->next;
//DO NOTHING.
				}
				i = i + 1;
			} else {
				break;
			}
		}
	} else { //There is a frame which has reference bit zero.framePointer=0
		while (ptr != NULL) {
			if (i < n) {
				if (i == framePointer) {
					ptr = ptr->next;
					ptr->ref_bit = 1;
					ptr->flag = true;
					ptr->data = pagenumber;
				} else {
					ptr = ptr->next;
					ptr->flag = false;
//DO NOTHING.
				}
				i = i + 1;
			} else {
				break;
			}
		}
	}

}

/*Returns the frame position of the pointer*/

int getPointerLocation(int n) {
	struct node *ptr = start;
	int i = 0;
	int framePointer = -1;
	while (ptr != NULL) {
		if (i < n) {
			if (ptr->flag == true) {
				framePointer = i;
				break;
			} else {
				i = i + 1;
				ptr = ptr->next;
			}
		} else
			break;
	}
	return framePointer;
}

/*Function to check ref_bit in CLOCK*/
int checkIfRefBitIsZero(int n) {
	struct node *ptr = start;
	int i = 0;
	int frameWithRefBitOne = -1;
	while (ptr != NULL) {
		if (i < n) {
			if (ptr->ref_bit == 0) {
				frameWithRefBitOne = i;
				break;
			} else {
				i = i + 1;
				ptr = ptr->next;
			}
		} else
			break;
	}
	return frameWithRefBitOne;
}

struct node* mini() {
	//struct node *nex;
	struct node *temp;
	temp = start;
	int min = temp->ref_bit; //ref_bit of start is stored in min
	do {
		temp = temp->next;
		if (temp->ref_bit < min) {
			min = temp->data; //add min
			break;
		}
	} while (temp != start);
	return temp; //return min
}

int emptyFrames1 () {
	int count = 0, i = 0;

	struct node *new_node = start;

	do {
		if (new_node->data == -1) {
			count = count + 1;
		}
		new_node = new_node->next;
	} while (new_node != start);

	return count;
}

void insertFrame_lfu(int pageNumber, int n, int temp_arr[],int numberOfPages) {
	int i, found = -1;

	for (i = 0; i < numberOfPages; i++) {
		if (temp_arr[i] == pageNumber) { //check if the page had come earlier
			found++;
		}

	}
	current = start;

	struct node *temp;
	int res;
	res = search(pageNumber);
	if (res == 1) { //hit
		current->ref_bit++;
	} else { //fault
		do {
			if (current->data == -1) { 		//check if data is added in first node
				current->data = pageNumber;     //add page to current
				break;

			} else if (emptyFrames1() > 0) {	//if more frames are free then move current to next node
				current = current->next;
			} else if (emptyFrames1() == 0) {       //all frames are full,Page replacement to be done

				temp = mini();			//returns the node with minimum ref_bit

				if (found >= 1) {
					temp->ref_bit = found;
				}
				temp->data = pageNumber;        //add page to this node(frame)
				break;
			}
		} while (current != start);
	}
}

int search(int number) {			
	int flag;
	struct node *temp;
	temp = start;
	do {
		if (temp->data == number) {
			current = temp;
			return (1);	
		} else
			flag = 0;
		temp = temp->next;
	} while (temp != start);
	if (flag == 0)
		return (0);			//not foung return 0
}


