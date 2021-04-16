#include <cstdio>
#include <cstdlib>

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

int main(int argc, char** argv) {
    _prog = argv[0];

    // Binary dictionary data
    binary_data_t bdict;

    // Load index from disk
    load_docnos();
    load_binary("dictionary.bin", bdict);

    // Build the in-memory dictionary
    std::unordered_map<std::string, posting_location> dictionary;
    build_dictionary(dictionary, bdict);

    free(bdict.block);
    return 0;
}
