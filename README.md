# Minimal Search Engine

A minimal search engine written in C++ for the TREC WSJ collection.

## Usage

### Build

The program can be built using `make`. Just `cd` into the `src` directory and run

```
make
```

### Index

To index a TREC collection located at `<infile.xml>`, use

```
./index <infile.xml>
```

This will create four files required by the search engine: `dictionary.bin`, `docnos.bin`, `lengths.bin` & `postings.bin`

### Search

The search program reads queries line by line from `stdin`. All relevant documents are printed to `stdout` sorted by document normalized term frequencies. This executable must be run in the same directory as the previously generated index.

```
./search
```

Search terms are expected to be separated by whitespace (although any non-alphanumeric characters also act as token delimiters)

The output will be in the form `<docno> <rsv>` with one document on each line.

## Notes

Tested on my laptop (AMD® Ryzen 7 4800u) running Fedora 33.

- It takes approximately 15s to index the TREC WSJ collection containing 173252 documents.
- It takes less than a second to search the 50 queries in `test.queries`

Some ideas for improving/extending this project:

- Refactor code using a more consistent C++ style
- Speed up document parsing by ignoring line breaks
- Use a better ranking function
- Merge the index files into one binary
- Verify the index integrity
- Investigate using a compiled index
