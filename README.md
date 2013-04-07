SqliteSubstringSearch
=====================
An open source tokenizer which supports fast substring search with sqlite FTS(full text search).

## How to use it
* register the "character_tokenizer"
* create a FTS table with character_tokenizer. For example:
    CREATE VIRTUAL TABLE Book USING fts3(name TEXT NOT NULL, author TEXT, tokenize=character);
* to search for a substring, use [phrase queries](http://www.sqlite.org/fts3.html#section_3). For example:
    -- Query for all documents that contain a substring "lin". This will match
    -- the strings like "Adrenalines", "Linux", "Penicillin".
    SELECT * FROM docs WHERE docs MATCH '"lin"';

See SqliteSubstringSearchDemo for an example.

## Motivation
The full text search in CJK(Chinese, Japanese, Korean) is basically a substring search, which is not supported by the default sqlite tokenizer.

The third-party tokenizers [mmseg](https://code.google.com/p/pymmseg-cpp/)(Chinese), [MeCab](http://mecab.googlecode.com/svn/trunk/mecab/doc/index.html)(Japanese), [ChaSen](http://chasen-legacy.sourceforge.jp/)(Japanese) try to partition the content info different words.
Since the meaning of "word" in these languages are not well-defined, they use sophisticated and memory intensive approaches to partition the content.

For simple applications like name or address search, a simple substring search is a more reasonable choice than a sophisticated tokenizer.

## How it works
The character tokenizer partitions each character as an individual token. 
Searching for a substring is equivalent to finding consecutive tokens in the document, which are provided by FTS as [phrase queries](http://www.sqlite.org/fts3.html#section_3). 

