SqliteSubstringSearch
=====================
An open source tokenizer which supports fast substring search with sqlite FTS(full text search).

If you feel `LIKE '%text%'` is too slow, this is the right solution for you.

## How to use it
* register the "character_tokenizer" module
* create a FTS table with character_tokenizer. For example:

    CREATE VIRTUAL TABLE Book USING fts3(name TEXT NOT NULL, author TEXT, tokenize=character);
* to search for a substring, use [phrase queries](http://www.sqlite.org/fts3.html#section_3). For example, to match strings such as "Adrenalines", "Linux", or "Penicillin", use:

    SELECT * FROM docs WHERE docs MATCH '"lin"';

See SqliteSubstringSearchDemo for a complete example.

## Motivation
English uses a space to separate words, but Chinese and Japanese do not.
Since built-in FTS tokenizers relies on spaces to separate words, it will treat a whole sentence in Chinese or Japanese as a single word, which makes FTS not useful at all in these languages.

The third-party Chinese and Japanese tokenizers([mmseg](https://code.google.com/p/pymmseg-cpp/)(Chinese), [MeCab](http://mecab.googlecode.com/svn/trunk/mecab/doc/index.html)(Japanese), [ChaSen](http://chasen-legacy.sourceforge.jp/)(Japanese)) use sophisticated and memory intensive approaches to find the ambiguous boundary between words.

For simple applications such as querying for people names, a simple substring search is a more reasonable choice than a sophisticated tokenizer.

## How it works
The character tokenizer partitions each character as an individual token. 
Searching for a substring is equivalent to finding consecutive tokens in the document, which are provided by FTS as [phrase queries](http://www.sqlite.org/fts3.html#section_3). 

