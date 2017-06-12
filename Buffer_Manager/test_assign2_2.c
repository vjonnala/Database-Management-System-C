#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// var to store the current test's name
char *testName;

// check whether two the content of a buffer pool is the same as an expected content 
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)			        \
  do {									\
    char *real;								\
    char *_exp = (char *) (expected);                                   \
    real = sprintPoolContent(bm);					\
    if (strcmp((_exp),real) != 0)					\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
	free(real);							\
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
    free(real);								\
  } while(0)

// test and helper methods
static void testCreatingAndReadingDummyPages (void);
static void createDummyPages(BM_BufferPool *bm, int num);
static void checkDummyPages(BM_BufferPool *bm, int num);

static void testCLOCK (void);
static void testLFU (void);

// main method
int 
main (void) 
{
  initStorageManager();
  testName = "";

  testCLOCK();
  testLFU();
}

// create n pages with content "Page X" and read them back to check whether the content is right
void
testCreatingAndReadingDummyPages (void)
{
  BM_BufferPool *bm = MAKE_POOL();
  testName = "Creating and Reading Back Dummy Pages";

  CHECK(createPageFile("testbuffer.bin"));

  createDummyPages(bm, 20);

  checkDummyPages(bm, 20);

  createDummyPages(bm, 10000);
  checkDummyPages(bm, 10000);

  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  TEST_DONE();
}


void 
createDummyPages(BM_BufferPool *bm, int num)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));
      sprintf(h->data, "%s-%i", "Page", h->pageNum);
      CHECK(markDirty(bm, h));
      CHECK(unpinPage(bm,h));
    }

  CHECK(shutdownBufferPool(bm));

  free(h);
}


void 
checkDummyPages(BM_BufferPool *bm, int num)
{

  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  char *expected = malloc(sizeof(char) * 512);

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));

      sprintf(expected, "%s-%i", "Page", h->pageNum);
      ASSERT_EQUALS_STRING(expected, h->data, "reading back dummy page content");

      CHECK(unpinPage(bm,h));
    }

  CHECK(shutdownBufferPool(bm));

  free(expected);
  free(h);
}

void
testCLOCK ()
{
  // expected results
  const char *poolContents[] = { 
    "[1 0],[-1 0],[-1 0]" , 
    "[1 0],[3 0],[-1 0]", 
    "[1 0],[3 0],[6 0]", 
    "[2 0],[3 0],[6 0]", 
    "[2 0],[4 0],[6 0]",
    "[2 0],[4 0],[5 0]",
    "[2 0],[4 0],[5 0]",
    "[2 0],[4 0],[5 0]",
    "[0 0],[4 0],[5 0]",
    "[0 0],[3 0],[5 0]",
    "[0 0],[3 0],[1 0]",
    "[2 0],[3 0],[1 0]",
    "[2 0],[5 0],[1 0]",
    "[2 0],[5 0],[4 0]",
    "[1 0],[5 0],[4 0]",
    "[1 0],[0 0],[4 0]",
  };
  const int requests[] = {1, 3, 6, 2, 4, 5, 2, 5, 0, 3, 1, 2, 5, 4, 1,0 };
  const int numLinRequests = 16;

  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing CLOCK page replacement";

  CHECK(createPageFile("testbuffer.bin"));

  createDummyPages(bm, 100);

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_CLOCK, NULL));

  printPoolContent(bm);

  // reading some pages linearly with direct unpin and no modifications
  for(i = 0; i < numLinRequests; i++)
    {
      pinPage(bm, h, requests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
    }

  // check number of write IOs
  ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
  ASSERT_EQUALS_INT(14, getNumReadIO(bm), "check number of read I/Os");

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}

// test the LRU page replacement strategy

void
testLFU()
{
  // expected results
  const char *poolContents[] = { 
    "[7 0],[-1 0],[-1 0]" , 
    "[7 0],[0 0],[-1 0]", 
    "[7 0],[0 0],[1 0]", 
    "[2 0],[0 0],[1 0]", 
    "[2 0],[0 0],[1 0]",
    "[3 0],[0 0],[1 0]",
    "[3 0],[0 0],[1 0]",
    "[4 0],[0 0],[1 0]",
    "[2 0],[0 0],[1 0]",
    "[2 0],[0 0],[3 0]",
    "[2 0],[0 0],[3 0]",
    "[2 0],[0 0],[3 0]",
    "[2 0],[0 0],[3 0]",
    "[1 0],[0 0],[3 0]",
    "[2 0],[0 0],[3 0]",
    "[2 0],[0 0],[3 0]",
    "[2 0],[0 0],[1 0]",
    "[2 0],[0 0],[7 0]",
    "[2 0],[0 0],[7 0]",
    "[2 0],[0 0],[1 0]",
  };
  const int requests[] = {7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1 };
  const int numLinRequests = 20;

  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing LFU page replacement";

  CHECK(createPageFile("testbuffer.bin"));

  createDummyPages(bm, 100);

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_LFU, NULL));

  printPoolContent(bm);

  // reading some pages linearly with direct unpin and no modifications
  for(i = 0; i < numLinRequests; i++)
    {
      pinPage(bm, h, requests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
    }

  // check number of write IOs
  ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
  ASSERT_EQUALS_INT(13, getNumReadIO(bm), "check number of read I/Os");

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}

