#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

char buf[1024 * 1024];      // Buffer for processing XML file
char token[1024 * 1024];    // The token being processed
char* pos;                  // Current position in buffer

typedef struct {
    int32_t docid; // Indexed doc id
    int32_t tf;    // Document's term frequency
} posting;
std::unordered_map<std::string, std::vector<posting>> file_index; // Inverted file index
std::vector<std::string> docnos; // The TREC DOCNOs

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
    FILE* fp;                   // XML file to process
    int docid = -1;             // Index of <DOC> tags
    bool save_docno = false;    // The next token is the DOCNO

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
            if (*token == '<') {
                if (!strcmp(token, "<DOC>"))
                    docid++;
                else if (!strcmp(token, "<DOCNO>"))
                    save_docno = true;

            } else if (save_docno) {
                docnos.push_back(token);
                save_docno = false;

            } else {
                std::string lowercase(token);
                std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
                    [](unsigned char c){ return std::tolower(c); });
                
                std::vector<posting>& postings = file_index[lowercase];
                if (postings.empty() || postings.back().docid != docid)
                    postings.push_back({docid, 1});
                else
                    postings.back().tf++;
                
            }
        }
    }

    std::cout << docnos.size() << " docs indexed\n";

    for (auto& item : file_index)
    {
        std::cout << item.first << ": ";
        for (auto p : item.second)
        {
            std::cout << p.docid << '=' << p.tf << ',';
        }
        std::cout << '\n';
    }
    

    fclose(fp);

    exit(EXIT_SUCCESS);
}
