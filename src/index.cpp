#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>

FILE* fp;                   // XML file to process
char buf[1024 * 1024];      // Buffer for processing XML file
char token[1024 * 1024];    // The token being processed
char* pos;                  // Current position in buffer

/**
 * Get the next token in the buffer.
 * 
 * @return The character following the new token, else NULL on buffer end
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

int main(int argc, char** argv)
{
    uint docs_indexed = 0;

    if (argc != 2) {
        printf("Usage: %s <infile.xml>", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "%s: can't open %s\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    while ((fgets(buf, sizeof(buf), fp)) != NULL) {
        pos = buf;
        while (get_next_token() != NULL) {
            if (!strcmp(token, "<DOC>"))
                docs_indexed++;
        }
    }

    printf("%d docs indexed\n", docs_indexed);

    fclose(fp);

    exit(EXIT_SUCCESS);
}
