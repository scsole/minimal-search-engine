#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>

/* Get the next token in the buffer following position */
char* get_next_token(char* buf, char* pos, char* token)
{
    char* end;  // Pointer to the end of the next token

    while (!isalnum(*pos) && *pos != '<' && *pos != '\n')
        pos++;
    
    end = pos;

    if (*pos == '<') {
        while (*++end != '>');
        ++end;
    } else if (isalnum(*pos)) {
        while (isalnum(*++end));
    } else {
        return NULL;    // end of buffer
    }

    memcpy(token, pos, end - pos);
    token[end - pos] = '\0';

    return end;
}

int main(int argc, char** argv)
{
    FILE* fp;                   // XML file to process
    char buf[1024 * 1024];      // Buffer for processing XML file
    char token[1024 * 1024];    // The token being processed
    char* pos;                  // Current position in buffer
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
        while ((pos = get_next_token(buf, pos, token)) != NULL) {
            if (!strcmp(token, "<DOC>"))
                docs_indexed++;
        }
    }

    printf("%d docs indexed\n", docs_indexed);

    fclose(fp);

    exit(EXIT_SUCCESS);
}
