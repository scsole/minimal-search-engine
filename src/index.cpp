#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

char buf[1024 * 1024];   // Buffer for processing XML file
char token[1024 * 1024]; // The token being processed
char* pos;               // Current position in buffer

struct posting_t { // Posting type
    int32_t docid; // Indexed doc id
    int32_t tf;    // Document's term frequency
};
std::unordered_map<std::string, std::vector<posting_t>> file_index; // Inverted file index
std::vector<std::string> docnos; // The TREC DOCNOs
std::vector<int32_t> doclens;    // Document lengths

/**
 * Get the next token in the buffer.
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
 * Build an inverted file index for fp.
 *
 * Return the number of documents indexed.
 */
int index_file(FILE* fp)
{
    int docid = -1;             // Index of <DOCNO> tags
    int doclen = 0;             // Length of the current document
    bool save_docno = false;    // The next token is the DOCNO

    while ((pos = fgets(buf, sizeof(buf), fp)) != NULL)
    {
        while (get_next_token(save_docno) != NULL)
        {
            // Do not index XML tags
            if (*token == '<')
            {
                if (!strcmp(token, "<DOC>"))
                {
                    // Save document at the end of each document
                    if (docid != -1)
                        doclens.push_back(doclen);
                    doclen = 0;

                    // Increment document ID
                    if (++docid % 10000 == 0)
                        std::cout << docid << " documents indexed\n";
                }
                else if (!strcmp(token, "<DOCNO>"))
                    save_docno = true;
                continue;
            }

            // Save the TREC DOCNO
            if (save_docno)
            {
                docnos.push_back(token);
                save_docno = false;
                continue;
            }

            // Ensure the token is in lowercase
            std::string lowercase(token);
            std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
            [](unsigned char c) {
                return std::tolower(c);
            });

            // Update the term frequency inside the in-memory index
            std::vector<posting_t>& postings = file_index[lowercase];
            if (postings.empty() || postings.back().docid != docid)
                postings.push_back({docid, 1});
            else
                postings.back().tf++;

            // Update current document length
            doclen++;
        }
    }

    // Length of last document
    doclens.push_back(doclen);

    return docid + 1;
}

/**
 * Write the in-memory index to disk.
 *
 * index stored as:
 * | word_length (4) | word (variable) | position (4) | size (4) |
 *
 * postings stored as:
 * | doc_id (4) | tf (4) |
 */
void write_index_to_disk()
{
    std::ofstream docnos_file ("docnos.bin");   // TREC DOCNOs
    std::ofstream doclens_file ("lengths.bin"); // Document lengths

    if (!docnos_file.is_open() || !doclens_file.is_open())
    {
        std::cerr << "Unable to open files for writing\n";
        exit(EXIT_FAILURE);
    }

    // Write DOCNOs to disk
    for (auto &docno : docnos)
    {
        docnos_file << docno << '\n';
    }

    // Write document lengths to disk
    for (auto &doclen : doclens)
    {
        doclens_file << doclen << '\n';
    }

    docnos_file.close();
    doclens_file.close();

    FILE* dictionary_fp = fopen("dictionary.bin", "wb"); // Word list
    FILE* postings_fp = fopen("postings.bin", "wb");     // Postings list

    if (dictionary_fp == NULL || postings_fp == NULL)
    {
        std::cerr << "Unable to open files for writing\n";
        exit(EXIT_FAILURE);
    }

    // Write index to disk: split into word dictionary and posting lists
    for (auto &item : file_index)
    {
        // Length of the current token
        int32_t word_length = item.first.size();

        // Current write position inside postings.bin
        int32_t position = ftell(postings_fp);

        // Size of the current postings list
        int32_t size = sizeof(item.second[0]) * item.second.size();

        // Write postings
        fwrite(&item.second[0], 1, size, postings_fp);

        // Write dictionary
        fwrite(&word_length, sizeof(word_length), 1, dictionary_fp);
        fwrite(item.first.c_str(), 1, word_length+1, dictionary_fp);
        fwrite(&position, sizeof(position), 1, dictionary_fp);
        fwrite(&size, sizeof(size), 1, dictionary_fp);
    }

    fclose(dictionary_fp);
    fclose(postings_fp);
}

/**
 * Indexer for the TREC WSJ collection.
 */
int main(int argc, char** argv)
{
    FILE* fp; // XML file to process

    // Verify arguments
    if (argc != 2)
    {
        printf("Usage: %s <infile.xml>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open file for reading
    if ((fp = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "%s: can't open %s\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    // Index the file
    if (index_file(fp) == 0)
    {
        std::cout << "No documents indexed!\n";
        fclose(fp);
        exit(EXIT_SUCCESS);
    }
    std::cout << docnos.size() << " documents indexed\n";
    fclose(fp);

    // Write index to disk
    std::cout << "Writing index to disk\n";
    write_index_to_disk();
    std::cout << "DONE\n";

    exit(EXIT_SUCCESS);
}