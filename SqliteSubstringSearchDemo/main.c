//
//  main.c
//  SqliteSubstringSearchDemo
//
//  Created by Hai Feng Kao on 4/6/13.
//  Copyright (c) 2013 com.haifeng. All rights reserved.
//
//  This program demos how to use sqlite FTS (full text search) and character tokenizer to query substrings efficiently

#include <stdio.h>
#include <sqlite3.h>
#include "character_tokenizer.h"

static char *removesql = "DROP TABLE Book;";

static char *createsql = "CREATE VIRTUAL TABLE Book USING fts3(name TEXT NOT NULL, author TEXT, tokenize=character);";

// The query sql commands. Note that they are phrase queries.
static char *querysql_prefix = "SELECT * FROM Book WHERE Book MATCH '\"baren\"'";
static char *querysql_suffix = "SELECT * FROM Book WHERE Book MATCH '\"akatosh\"'";
static char *querysql_infix = "SELECT * FROM Book WHERE Book MATCH '\"end\"'";

/*
 ** Register a tokenizer implementation with FTS3 or FTS4.
 */
static int registerTokenizer(
                             sqlite3 *db,
                             char *zName,
                             const sqlite3_tokenizer_module *p
                             ){
    int rc;
    sqlite3_stmt *pStmt;
    const char *zSql = "SELECT fts3_tokenizer(?, ?)";
    
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    if( rc!=SQLITE_OK ){
        return rc;
    }
    
    sqlite3_bind_text(pStmt, 1, zName, -1, SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 2, &p, sizeof(p), SQLITE_STATIC);
    sqlite3_step(pStmt);
    
    return sqlite3_finalize(pStmt);
}

int main(void)
{
    int i, j;
    int rows, cols;
    sqlite3 *db;
    char *errMsg = NULL;
    char **result;
    const sqlite3_tokenizer_module *ptr;
    char token_name[] = "character";
    
    // create the database
    if (sqlite3_open_v2("example.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
        return 0;
    }
    
    // get the tokenizer
    get_character_tokenizer_module(&ptr);
    
    // register character tokenizer, note that you need to register it everytime the database is opened
    registerTokenizer(db, token_name, ptr);
    
    // remove existing table
    sqlite3_exec(db, removesql, 0, 0, &errMsg);
    
    // create table with sqlite FTS3 support
    sqlite3_exec(db, createsql, 0, 0, &errMsg);
    
    // insert test data
    sqlite3_exec(db, "INSERT INTO Book VALUES('Biography of Barenziah, v1', 'Stern Gamboge');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('Biography of Barenziah, v2', 'Stern Gamboge');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('Biography of Barenziah, v3', 'Stern Gamboge');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('The Alduin-Akatosh Dichotomy', 'Alexandre Simon');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('The Legendary Sancre Tor', 'Matera Chapel');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO Book VALUES('The Legendary Scourge, v3', 'Marobar Sul');", 0, 0, &errMsg);
    
    // test prefix matches
    sqlite3_get_table(db , querysql_prefix, &result , &rows, &cols, &errMsg);
    
    printf("query results for %s\n", querysql_prefix);
    for (i=0;i<=rows;i++) {
        for (j=0;j< cols;j++) {
            printf("%s\t", result[i*cols+j]);
        }
        printf("\n");
    }
    sqlite3_free_table(result);
    printf("\n");
    
    // test suffix matches
    sqlite3_get_table(db , querysql_suffix, &result , &rows, &cols, &errMsg);
    
    printf("query results for %s\n", querysql_suffix);
    for (i=0;i<=rows;i++) {
        for (j=0;j< cols;j++) {
            printf("%s\t", result[i*cols+j]);
        }
        printf("\n");
    }
    sqlite3_free_table(result);
    printf("\n");
    
    // test infix matches
    sqlite3_get_table(db , querysql_infix, &result , &rows, &cols, &errMsg);
    
    printf("query results for %s\n", querysql_infix);
    for (i=0;i<=rows;i++) {
        for (j=0;j< cols;j++) {
            printf("%s\t", result[i*cols+j]);
        }
        printf("\n");
    }
    sqlite3_free_table(result);

    // close database
    sqlite3_close(db);
    
    return 0;
}

