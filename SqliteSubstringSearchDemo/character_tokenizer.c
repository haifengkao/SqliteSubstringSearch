//
//  character_tokenizer.c
//
//  Created by Hai Feng Kao on 4/6/13.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//
// Implementation of the "simple" full-text-search tokenizer.

#include <sqlite3.h>
#include <ctype.h> //for tolower
#include <string.h> //for memset
#import "character_tokenizer.h"

typedef struct character_tokenizer {
    sqlite3_tokenizer base;
} character_tokenizer;

typedef struct character_tokenizer_cursor {
    sqlite3_tokenizer_cursor base;
    const char *pInput; // input we are tokenizing
    int nBytes;         // size of the input
    int iPosition;      // current position in pInput
    int iToken;         // index of next token to be returned
    char *pToken;       // storage for current token
} character_tokenizer_cursor;

static int characterCreate(
                     int argc, const char * const *argv,
                     sqlite3_tokenizer **ppTokenizer
                     ){
    character_tokenizer *t;
    t = (character_tokenizer *) sqlite3_malloc(sizeof(*t));
    if( t == NULL
       ) return SQLITE_NOMEM;
    memset(t, 0, sizeof(*t));
    
    *ppTokenizer = &t->base;
    return SQLITE_OK;
}

static int characterDestroy(sqlite3_tokenizer *pTokenizer){
    sqlite3_free(pTokenizer);
    return SQLITE_OK;
}

static int characterOpen(
                   sqlite3_tokenizer *pTokenizer,         /* The tokenizer */
                   const char *pInput, int nBytes,        /* String to be tokenized */
                   sqlite3_tokenizer_cursor **ppCursor    /* OUT: Tokenization cursor */
                   ){
    character_tokenizer_cursor *c;
    if(pInput == 0){
        nBytes = 0;
    }else if(nBytes < 0){
        nBytes = (int)strlen(pInput);
    }
    c = (character_tokenizer_cursor *) sqlite3_malloc(sizeof(*c));
    if(c == NULL){
        return SQLITE_NOMEM;
    }
    c->iToken = c->iPosition = 0;
    c->pToken = NULL;
    c->nBytes = nBytes;
    c->pInput = pInput;
    *ppCursor = &c->base;
    return SQLITE_OK;
}

static int characterClose(sqlite3_tokenizer_cursor *pCursor){
    character_tokenizer_cursor *c = (character_tokenizer_cursor *) pCursor;
    
    if(c->pToken != NULL){
        sqlite3_free(c->pToken);
        c->pToken = NULL;
    }

    sqlite3_free(c);
    return SQLITE_OK;
}

static int characterNext(
                   sqlite3_tokenizer_cursor *pCursor, /* Cursor returned by cusOpen */
                   const char **ppToken,               /* OUT: *ppToken is the token text */
                   int *pnBytes,                       /* OUT: Number of bytes in token */
                   int *piStartOffset,                 /* OUT: Starting offset of token */
                   int *piEndOffset,                   /* OUT: Ending offset of token */
                   int *piPosition                     /* OUT: Position integer of token */
                   ){
    character_tokenizer_cursor *c = (character_tokenizer_cursor *) pCursor;
    if(c->pToken != NULL){
        sqlite3_free(c->pToken);
        c->pToken = NULL;
    }
    
    if (c->iPosition >= c->nBytes) {
        return SQLITE_DONE;
    }
    
    int length = 1; // the size of current character, which can be at most 4 bytes
    
    const char* token = &(c->pInput[c->iPosition]);
    *piStartOffset = c->iPosition;
    
    // find the beginning of next utf8 character
    c->iPosition++;
    while (c->iPosition < c->nBytes) {
        char byte = c->pInput[c->iPosition];
        if (((byte & 0x80) == 0) || ((byte & 0xc0) == 0xc0)) {
            // we have reached the first byte of next utf8 character
            break;
        }
        length++;
        c->iPosition++;
    }
    
    c->pToken = (char *)sqlite3_malloc(length+1);
    if(c->pToken == NULL){
        return SQLITE_NOMEM;
    }
    
    c->pToken[length] = 0;
    memcpy(c->pToken, token, length);
    
    for (int i = 0; i < length; ++i) {
        unsigned char byte = c->pToken[i];

        if (byte < 0x80) {
            // ascii character, make it case-insensitive
            c->pToken[i] = tolower(byte);
        }
    }
    
    *ppToken = c->pToken;
    *pnBytes = length;
    
    *piEndOffset = *piStartOffset+length;
    *piPosition = c->iToken++;
    return SQLITE_OK;
}

static const sqlite3_tokenizer_module characterTokenizerModule = {
    0,
    characterCreate,
    characterDestroy,
    characterOpen,
    characterClose,
    characterNext,
};

void get_character_tokenizer_module(const sqlite3_tokenizer_module **ppModule){
    *ppModule = &characterTokenizerModule;
}

