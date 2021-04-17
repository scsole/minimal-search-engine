#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

char* _prog; // Program name

struct binary_data_t {  // Binary data type
    size_t size;        // Size of the binary block
    char* block;        // Binary data
};

struct posting_location { // Postings location type
    int32_t position;     // Position of entry in postings.bin
    int32_t size;         // Size of posting list
};

struct posting_t {  // Posting type
    int32_t docid;  // Indexed doc id
    int32_t tf;     // Document's term frequency
};

/**
 * Load a binary file into memory.
 *
 * Return the binary file if opened correctly.
 */
binary_data_t* load_binary(const char* infile, binary_data_t& data)
{
    FILE* fp = fopen(infile, "rb");

    if (fp == NULL)
    {
        std::cerr << _prog << ": unable to open " << infile << '\n';
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    data.size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data.block = (char*) malloc(data.size);
    fread(data.block, 1, data.size, fp);

    fclose(fp);
    return &data;
}

/**
 * Build a dictionary from an in-memory binary.
 */
void build_dictionary(std::unordered_map<std::string, posting_location>& dict,
                      binary_data_t &bdict)
{
    char* current = bdict.block;
    while (current < bdict.block + bdict.size) {
        std::string token(current + sizeof(uint32_t));

        int32_t token_length = *current;
        int32_t pos = *((int32_t*) (current + token_length + 1 + sizeof(uint32_t)));
        int32_t size = *((int32_t*) (current + token_length + 1 + 2*sizeof(uint32_t)));

        dict[token] = {pos, size};

        current += token_length + 1 + 3*sizeof(uint32_t);
    }
}

/**
 * Load DOCNOs from disk into memory.
 */
void load_docnos(std::vector<std::string>& docnos)
{
    std::ifstream infile;   // Input file
    char buf[1024];         // Input buffer

    infile.open("docnos.bin");
    if (infile.is_open())
    {
        while (infile.getline(buf, sizeof(buf)))
            docnos.push_back(std::string(buf));
        infile.close();
    }
    else
    {
        std::cerr << _prog << ": unable to open docnos.bin\n";
        exit(1);
    }
}

/**
 * Load document lengths from disk into memory.
 */
void load_doclens(std::vector<int32_t>& doclens)
{
    std::ifstream infile;   // Input file
    int32_t buf;         // Input buffer

    infile.open("lengths.bin");
    if (infile.is_open())
    {
        while (infile >> buf)
            doclens.push_back(buf);
        infile.close();
    }
    else
    {
        std::cerr << _prog << ": unable to open lengths.bin\n";
        exit(1);
    }
}

/**
 * Get the next search term in the buffer.
 *
 * A search term is defined as an alphanumeric token.
 *
 * Return the character following the end of the current token, else NULL on
 * buffer end.
 */
char* get_next_term(char* term, char* pos, char* buffer)
{
    // Skip non alphanumeric characters
    while (!isalnum(*pos) && *pos != '\n' && *pos != '\0')
        pos++;

    char* end = pos;

    if (isalnum(*pos))
    {
        while (isalnum(*end))
            end++;
    }
    else
        return NULL;

    // Return the search term in lowercase
    int i = 0;
    while (i < end - pos && i < (int) sizeof(buffer) - 1)
    {
        term[i] = pos[i];
        i++;
    }
    term[i] = '\0';

    return end;
}

/**
 * Compare two RSV values.
 */
int rsvcomp(const void * a, const void * b)
{
    return *(double *)a > *(double *)b;
}

/**
 * Minimal search engine.
 * 
 * Implements naive document ranking by using term densities.
 */
int main(int argc, char** argv)
{
    _prog = argv[0];

    // Binary dictionary data
    binary_data_t bdict;
    std::vector<std::string> docnos; // TREC DOCNOs
    std::vector<int32_t> doclens;    // Document lengths

    // Load index from disk
    load_docnos(docnos);
    load_doclens(doclens);
    load_binary("dictionary.bin", bdict);
    FILE* postings_fp = fopen("postings.bin", "rb");

    // Build the in-memory dictionary
    std::unordered_map<std::string, posting_location> dictionary;
    build_dictionary(dictionary, bdict);

    char buffer[1024];      // Buffer for search terms from stdin
    char term[1024];        // Current filtered search term from buffer
    char* pos;              // Positions in buffer

    int32_t* postings_buffer = new int32_t[(docnos.size() + 1) * 2]; // Postings buffer for search queries
    double* rsv              = new double[docnos.size()];           // RSV values for each document
    std::vector<double *> rsv_pointers(docnos.size());              // RSV pointer vector for sorting

    for (size_t i = 0; i < docnos.size(); i++)
        rsv_pointers[i] = &rsv[i];

    // Process search queries line by line
    while ((pos = fgets(buffer, sizeof(buffer), stdin)) != NULL)
    {
        // Prepare for a new search
        memset(rsv, 0, sizeof(*rsv) * docnos.size());

        // Process each term in the query
        while ((pos = get_next_term(term, pos, buffer)) != NULL)
        {
            posting_location term_postings;
            if ((term_postings = dictionary[std::string(term)]).size != 0)
            {
                // Results found: read the revelant postings list from disk
                fseek(postings_fp, term_postings.position, SEEK_SET);
                fread(postings_buffer, 1, term_postings.size, postings_fp);

                int32_t hits = term_postings.size / sizeof(int32_t) / 2;
                posting_t* postings = (posting_t *)(&postings_buffer[0]);

                // Accumulate RSV values for each posting
                for (int32_t hit = 0; hit < hits; hit++, postings++)
                    rsv[postings->docid] += postings->tf / (double) doclens[postings->docid];
            }
        }

        // Sort results
        std::sort(std::begin(rsv_pointers), std::end(rsv_pointers), rsvcomp);

        // Print all relevant postings
        for (size_t i = 0; i < docnos.size(); i++)
        {
            if (*rsv_pointers[i] == 0)
                break;
            std::cout << docnos[rsv_pointers[i] - rsv] << ' ' << *rsv_pointers[i] << '\n';
        }
    }

    free(bdict.block);
    return 0;
}