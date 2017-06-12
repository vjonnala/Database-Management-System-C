/*
 * node_linked_list.h
 *
 *  Created on: Mar 23, 2015
 *      Author: vjonnala and team
 */

#ifndef NODE_LINKED_LIST_H_
#define NODE_LINKED_LIST_H_

#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
//#include "storage_mgr.h"

/*Represents node in linked list*/
struct node {
	int data;
	BM_PageHandle bm_ph;
	int fix_count;
	bool dirty_bit;
	int originalval;
	int ref_bit;
	bool flag;
	struct node *next;
}*new_node, *current, *last, *temp;

/*Used for statistics purpose*/
struct BM_statistics {

	PageNumber *frameContents;
	bool *dirty_bit_stats;
	int *fix_count_stats;
	int numReadIO ;
	int numWriteIO ;

}stats_data;

RC intialize_linked_list(int n);//initialize the link list

void display_linked_list(int n);//display the link list

struct node* return_frame_linked_list(int pageNumber,int n);

int search_linked_list(int pageNumber,int n);//search the link list for a page

void insertFrame_linked_list(BM_BufferPool * const bm, int pageNumber);//fifo

void insertFrame_lru(int totalPages, int pageNumber) ;//lru

int findNextVictim();

int hitFault(int pageNumber);

int emptyFrames(int n) ;

void insertFrame_clock(int totalPages, int pageNumber);//clock

int hitFault_clock(int pageNumber);

void pageReplacement(int pagenumber, int n);

int getPointerLocation(int n);

int checkIfRefBitIsZero(int n);

struct node* mini();

int emptyFrames1();

void insertFrame_lfu(int pageNumber, int n, int temp_arr[],int numberOfPages);

int search(int number);
#endif

/* NODE_LINKED_LIST_H_ */

