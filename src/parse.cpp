#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

char buf[1024 * 1024];   // Buffer for processing XML file
char token[1024 * 1024]; // The token being processed
char* pos;               // Current position in buffer

/**
 * Get the next token in the buffer.
 * 
 * Ignores all punctuation but keeps DOCNOs and XML tags in tact.
 *
 * Return the character following the new token, else NULL on buffer end.
 */
char* get_next_token(bool is_docnum)
{
    while (!isalnum(*pos) && *pos != '<' && *pos != '\n')
        pos++;

    char* end = pos; // End of the token being processed

    if (*pos == '<')        // XML tag
    {
        while (*++end != '>')
            ;
        ++end;
    }
    else if (isalnum(*pos)) // Body
    {
        if (is_docnum)
            while (isalnum(*end) || *end == '-') // Dont split DOCNO
                ++end;
        else
            while (isalnum(*end))
                ++end;
    }
    else                    // End of buffer
        return NULL;

    memcpy(token, pos, end-pos);
    token[end-pos] = '\0';

    return pos = end;
}

/**
 * Parser for the TREC WSJ collection.
 * 
 * Outputs a stream of tokens ready for the indexer.
 */
int main(int argc, char** argv)
{
    FILE* fp; // XML file to process

    // Verify arguments
    if (argc != 2)
    {
        printf("Usage: %s <infile.xml>", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open file for reading
    if ((fp = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "%s: can't open %s\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    bool save_docno = false; // The next token is the DOCNO

    // Print out each token 
    while ((pos = fgets(buf, sizeof(buf), fp)) != NULL)
    {
        while (get_next_token(save_docno) != NULL)
        {
            // XML tag
            if (*token == '<')
            {
                if (!strcmp(token, "<DOC>"))
                    std::cout << '\n'; // Print a blank line between each document

                else if (!strcmp(token, "<DOCNO>"))
                    save_docno = true; // Ensure the DOCNO is not split
                
            }

            // Ensure the token is in lowercase
            std::string lowercase(token);
            std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
            [](unsigned char c) {
                return std::tolower(c);
            });

            // Print the token
            std::cout << token << '\n';
        }
    }

    fclose(fp);

    exit(EXIT_SUCCESS);
}