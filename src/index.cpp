#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>

char buf[1024 * 1024];      // Buffer for processing XML file
char token[1024 * 1024];    // The token being processed
char* pos;                  // Current position in buffer

typedef struct {
    std::string docno;  // TREC DOCNO
    uint32_t tf;        // Document's term frequency
} postings;
std::unordered_map<std::string, std::vector<postings>> file_index; // Inverted file index

/**
 * Get the next token in the buffer.
 * Returns the character following the new token, else NULL on buffer end.
 */
char* get_next_token()
{
    while (!isalnum(*pos) && *pos != '<' && *pos != '\n')
        pos++;
    
    char* end = pos;    // End of the token being processed

    if (*pos == '<') {          // XML tag
        while (*++end != '>')
            ;
        ++end;
    } else if (isalnum(*pos)) { // Body
        while (isalnum(*end) || *end == '-') // Dont split DOCNO
            ++end;
    } else {                    // End of buffer
        return NULL;
    }

    memcpy(token, pos, end-pos);
    token[end-pos] = '\0';

    return pos = end;
}

/**
 * Indexer for the TREC WSJ collection.
 */
int main(int argc, char** argv)
{
    FILE* fp;               // XML file to process
    uint32_t docs_indexed = 0;  // Number of <DOC> tags indexed
    bool save_docno = false;

    if (argc != 2) {
        printf("Usage: %s <infile.xml>", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open file for reading
    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "%s: can't open %s\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    // Index document
    while ((pos = fgets(buf, sizeof(buf), fp)) != NULL)
    {
        while (get_next_token() != NULL)
        {
            if (token == "<") {
                if (!strcmp(token, "<DOC>"))
                    docs_indexed++;
                else if (strcmp(token, "<DOCNO>"))
                    save_docno = true;

            } else if (save_docno) {
                // TODO:Save docno
            }
        }
    }

    printf("%d docs indexed\n", docs_indexed);

    fclose(fp);

    exit(EXIT_SUCCESS);
}
