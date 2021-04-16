#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char** argv) {
    std::string line;
    std::ifstream infile ("docnos.bin");

    if (infile.is_open()) {
        while (std::getline(infile, line)) {
            std::cout << line << '\n';
        }
        infile.close();
    } else
        std::cout << "Unable to open file\n";

    return 0;
}
