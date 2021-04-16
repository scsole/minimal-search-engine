#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

char* _prog; // Program name

struct binary_data_t {  // Binary data type
    size_t size;        // Size of the binary block
    char* block;        // Binary data
};

std::vector<std::string> docnos; // TREC DOCNOs

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
binary_data_t* load_binary(const char* infile, binary_data_t& data) {
    FILE* fp = fopen(infile, "rb");

    if (fp == NULL) {
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
                        binary_data_t &bdict) {
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
void load_docnos() {
    std::ifstream infile;   // Input file
    char buf[1024];         // Input buffer

    infile.open("docnos.bin");
    if (infile.is_open()) {
        while (infile.getline(buf, sizeof(buf))) {
            docnos.push_back(std::string(buf));
        }
        infile.close();
    } else {
        std::cerr << _prog << ": unable to open docnos.bin\n";
        exit(1);
    }
}

/**
 * Get the next search term in the buffer.
 * 
 * A search term is defined as an alphnumeric token.
 * 
 * Return the character following the end of the current token, else NULL on
 * buffer end.
 */
char* get_next_term(char* term, char* pos, char* buffer) {
    // Skip non alphanumeric characters
    while (!isalnum(*pos) && *pos != '\n')
        pos++;

    char* end = pos;

    if (isalnum(*pos)) {
        while (isalnum(*end))
            end++;
    } else {
        return NULL;
    }

    // Return the search term in lowercase
    int i = 0;
    while (i < end - pos) {
        term[i] = pos[i];
        i++;
    }
    term[i] = '\0';

    return end;
}

/**
 * Minimal search engine.
 */
int main(int argc, char** argv) {
    _prog = argv[0];

    // Binary dictionary data
    binary_data_t bdict;

    // Load index from disk
    load_docnos();
    load_binary("dictionary.bin", bdict);
    FILE* postings_fp = fopen("postings.bin", "rb");

    // Build the in-memory dictionary
    std::unordered_map<std::string, posting_location> dictionary;
    build_dictionary(dictionary, bdict);

    char buffer[1024];      // Buffer for search terms from stdin
    char term[1024];        // Current filtered search term from buffer
    char* pos;              // Positions in buffer

    int32_t* postings_buffer = new int32_t[docnos.size() * 2]; // Postings buffer for search queries
    int32_t* rsv             = new int32_t[docnos.size()];     // RSV values for each document

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
                    rsv[postings->docid] += postings->tf;
            }
        }
    }

    free(bdict.block);
    return 0;
}
