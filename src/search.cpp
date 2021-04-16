#include <iostream>
#include <fstream>
#include <string>
#include <vector>

char* _prog; // Program name

std::vector<std::string> docnos; // TREC DOCNOs

/**
 * Load index from disk into memory.
 */
void load_index() {
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

    load_index();

    std::cout << docnos.size() << '\n';

    return 0;
}
