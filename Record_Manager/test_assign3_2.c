#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "buffer_mgr.h"
#include "record_mgr.h"
#include "storage_mgr.h"
#include "tables.h"
#include "test_helper.h"

#define ASSERT_EQUALS_RECORDS(_l,_r, schema, message)			\
  do {									\
    Record *_lR = _l;   						 \
    Record *_rR = _r;                                                    \
    ASSERT_TRUE(memcmp(_lR->data,_rR->data,getRecordSize(schema)) == 0, message); \
    int i;							\
    for(i = 0; i < schema->numAttr; i++)				\
      {										\
        Value *lVal, *rVal;          	                                   \
		char *lSer, *rSer; 					\
        getAttr(_lR, schema, i, &lVal); 	                                 \
        getAttr(_rR, schema, i, &rVal);                                  \
		lSer = serializeValue(lVal); \
		rSer = serializeValue(rVal); \
        ASSERT_EQUALS_STRING(lSer, rSer, "attr same");	\
		free(lVal); \
		free(rVal); \
		free(lSer); \
		free(rSer); \
      }									\
  } while(0)

// test methods
static void testCreateTableAndInsert(void);

// struct for test records
typedef struct TestRecord {
	int a;
	char *b;
	int c;
} TestRecord;

// helper methods
Record *testRecord(Schema *schema, int a, char *b, int c);
Schema *testSchema(void);
Record *fromTestRecord(Schema *schema, TestRecord in);

// test name
char *testName;
char *table_name_temp;
// main method
int main(void) {
	testName = "";

	testCreateTableAndInsert();

	return 0;
}

// ************************************************************ 
void testCreateTableAndInsert(void) {

	printf("-----------------------------\n");
	printf("        WELCOME                     \n");
	printf("------------------------------\n");
	testName = "test menu-based user interface";
	int op = 0;
	do {
		printf("Select an Option Below!!\n");
		printf("1.Create Table \n2.Insert Records into the Table \n3.Update Record in the Table\n4.Delete the Table \n5.Exit\n");
		scanf("%d", &op);BM_BufferPool *bm1;

		RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
		Schema *schema; 
		Record *r1;
		RID *rids1;
		switch (op) {
		case 1:{
			printf("Enter Table Name\n");
			char *table_name = malloc(20);
			scanf("%s", &(*table_name));
			schema = testSchema();
			TEST_CHECK(initRecordManager(NULL));
			TEST_CHECK(createTable(table_name, schema));
			TEST_CHECK(openTable(table, table_name));
			bm1 = (BM_BufferPool *) table->mgmtData;
			table_name_temp = table_name;
			}
			break;
		case 2: {
			int numInserts;
			int i2 = 0;
			printf("Enter the number of Records to insert \n");
			scanf("%d", &numInserts);
			TestRecord inserts[numInserts];
			table->mgmtData = (BM_BufferPool *)bm1;
			for (i2 = 0; i2 < numInserts; i2++) {
				printf("Enter Column1 for Row[%d]\n", i2);
				scanf("%d", &inserts[i2].a);
				inserts[i2].b = malloc(10);
				char *rec = malloc(10);
				printf("Enter Column2 for Row[%d]\n", i2);
				scanf("%s", &(*rec));
				strcpy(inserts[i2].b, rec);
				printf("Enter Column3 for Row[%d]\n", i2);
				scanf("%d", &inserts[i2].c);
			}

			int i;
			Record *r;
			RID *rids;
			rids = (RID *) malloc(sizeof(RID) * numInserts);
			// insert rows into table
			for (i = 0; i < numInserts; i++) {
				r = fromTestRecord(schema, inserts[i]);
				TEST_CHECK(insertRecord(table, r));
				rids[i] = r->id;
			}

			// randomly retrieve records from the table and compare to inserted ones
			for (i = 0; i < numInserts*5; i++) {
				int pos = rand() % numInserts;
				RID rid = rids[pos];
				TEST_CHECK(getRecord(table, rid, r));

				ASSERT_EQUALS_RECORDS(fromTestRecord(schema, inserts[pos]), r,schema, "compare records");
			}
			rids1 = rids; r1 = r;

		}
			break;
		case 4:
			TEST_CHECK(deleteTable(table_name_temp));
			TEST_CHECK(shutdownRecordManager());
			printf("Table Deleted Succesfully\n");
			break;
		case 5:
			TEST_DONE();
			exit(0);
			break;
		
		case 3:{
			int numUpdates;
			int i5 = 0;int i;
			printf("Enter the number of Records to update \n");
			scanf("%d", &numUpdates);
			TestRecord updates[numUpdates];
			table->mgmtData = (BM_BufferPool *)bm1;
			for (i5 = 0; i5 < numUpdates; i5++) {
				printf("Enter Column1 for Row[%d]\n", i5);
				scanf("%d", &updates[i5].a);
				updates[i5].b = malloc(10);
				char *rec = malloc(10);
				printf("Enter Column2 for Row[%d]\n", i5);
				scanf("%s", &(*rec));
				strcpy(updates[i5].b, rec);
				printf("Enter Column3 for Row[%d]\n", i5);
				scanf("%d", &updates[i5].c);
			}
			  for(i = 0; i < numUpdates; i++)
			    {
			      r1 = fromTestRecord(schema, updates[i]);
			      r1->id = rids1[i];
			      TEST_CHECK(updateRecord(table,r1)); 
			    }

		}
		  TEST_DONE();
			break;

		default:
			printf("Try Again");
			exit(0);
		}

	} while (op < 6);

}

Schema *
testSchema(void) {
	printf("Enter Schema Details. Default Number of columns are 3.\n");
	Schema *result;
	int i1 = 0;

	char *names[3]; // = { "a", "b", "c" };
	DataType dt[3]; // = { DT_INT, DT_STRING, DT_INT };
	int type[3];
	for (i1 = 0; i1 < 3; i1++) {
		names[i1] = malloc(20);
		printf("Enter Column Name \n");
		scanf("%s", &(*names[i1]));
		printf(
				"Enter Column Data Type [0 = INT ; 1 = STRING ; 2= FLOAT ; 3 = BOOLEAN] \n");

		scanf("%d", &type[i1]);
		dt[i1] = type[i1];
	}

	int sizes[] = { 0, 4, 0 };
	int keys[] = { 0 };
	int i;
	char **cpNames = (char **) malloc(sizeof(char*) * 3);
	DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
	int *cpSizes = (int *) malloc(sizeof(int) * 3);
	int *cpKeys = (int *) malloc(sizeof(int));

	for (i = 0; i < 3; i++) {
		cpNames[i] = (char *) malloc(2);
		strcpy(cpNames[i], names[i]);
	}
	memcpy(cpDt, dt, sizeof(DataType) * 3);
	memcpy(cpSizes, sizes, sizeof(int) * 3);
	memcpy(cpKeys, keys, sizeof(int));

	result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

	return result;
}

Record *
fromTestRecord(Schema *schema, TestRecord in) {
	return testRecord(schema, in.a, in.b, in.c);
}

Record *
testRecord(Schema *schema, int a, char *b, int c) {
	Value *value;
	Record *result1;
	TEST_CHECK(createRecord(&result1, schema));
	MAKE_VALUE(value, DT_INT, a);
	TEST_CHECK(setAttr(result1, schema, 0, value));
	freeVal(value);
	MAKE_STRING_VALUE(value, b);
	TEST_CHECK(setAttr(result1, schema, 1, value));
	freeVal(value);
	MAKE_VALUE(value, DT_INT, c);
	TEST_CHECK(setAttr(result1, schema, 2, value));
	freeVal(value);

	return result1;
}

